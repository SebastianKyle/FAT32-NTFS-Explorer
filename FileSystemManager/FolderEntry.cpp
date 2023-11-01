#include "FolderEntry.h"

map<ATTRIBUTE, string> mode_map = {
	{ATTR_SUB_DIRECTORY, "d----"},
	{ATTR_ARCHIVE, "-a---"},
	{ATTR_READONLY, "--r--"},
	{ATTR_HIDDEN, "---h-"},
	{ATTR_SYSTEM, "----s"}
};

FolderEntry::FolderEntry(FAT_DIR_ENTRY fatDirEntry, LFN_ENTRY* lfnEntries, WORD numOfLFNEntries)
	:Entry(fatDirEntry, lfnEntries, numOfLFNEntries)
{
	setFirstSector(getFirstClusterInDataChain());
	_parentFolder = nullptr;
}

FolderEntry::FolderEntry(const FolderEntry& other)
	:Entry(other._rootDirEntry, other._chainedLFNEntry, other._numOfLFNEntries)
{
	_parentFolder = nullptr;
}


FolderEntry::FolderEntry()
	:Entry()
{
	
}

FolderEntry::~FolderEntry()
{
	
}

void FolderEntry::load()
{
	DWORD bytesSizeOfChainedCluster = 0;

	if (!VolumeAccess::getInstance()->readChainedClusters(getFirstClusterInDataChain(), NULL, &bytesSizeOfChainedCluster))
	{
		cout << "\n Could not load folder \"" << reinterpret_cast<const char*>(_rootDirEntry.DIR_Name) << "\", Error: " << GetLastError();
	}
	else if (bytesSizeOfChainedCluster == 0)
	{
		string name = getName();
		cout << "\n Not data was found in folder \"" << name << "\".";
	}
	else
	{
		BYTE* clustersData = new BYTE[bytesSizeOfChainedCluster];

		if (!VolumeAccess::getInstance()->readChainedClusters(getFirstClusterInDataChain(), clustersData, &bytesSizeOfChainedCluster))
		{
			cout << "\n Can not load content for folder \"" << reinterpret_cast<const char*>(_rootDirEntry.DIR_Name) << "\", Error: " << GetLastError();
		}
		else
		{
			if (!isLoaded())
			{
				DWORD currBytePos = 0;

				// Read till the last entry
				while (bytesSizeOfChainedCluster - currBytePos >= sizeof(FAT_DIR_ENTRY) && clustersData[currBytePos] != 0x00)
				{
					FATDirEntryUn fatCurrEntry;

					memcpy(&fatCurrEntry, clustersData + currBytePos, sizeof(FAT_DIR_ENTRY));
					currBytePos += sizeof(FAT_DIR_ENTRY);

					if (isSubDirEntry(fatCurrEntry)) // Check for sdet
					{
						SubDirEntry* sdet = new SubDirEntry(fatCurrEntry.ShortEntry);
						_subDirEntries.push_back(sdet);
					}
					else
					{
						LFN_ENTRY* lfnEntries = nullptr;
						WORD numOfLFNEntries = 0;

						if (isLFNEntry(fatCurrEntry) && !isDeletedEntry(fatCurrEntry.ShortEntry))
						{
							numOfLFNEntries = fatCurrEntry.LongEntry.LDIR_SeqNum & 0x1F;
							lfnEntries = new LFN_ENTRY[numOfLFNEntries];
							lfnEntries[0] = fatCurrEntry.LongEntry;

							// Read the rest of the LFNs
							for (int i = 1; i < numOfLFNEntries && bytesSizeOfChainedCluster - currBytePos >= sizeof(LFN_ENTRY); i++)
							{
								memcpy(&lfnEntries[i], clustersData + currBytePos, sizeof(LFN_ENTRY));
								currBytePos += sizeof(LFN_ENTRY);
							}

							// Last entry must be the main entry (root dir entry)
							memcpy(&fatCurrEntry, clustersData + currBytePos, sizeof(FAT_DIR_ENTRY));
							currBytePos += sizeof(FAT_DIR_ENTRY);
						}

						if (isFolderEntry(fatCurrEntry) && !isDeletedEntry(fatCurrEntry.ShortEntry)) // if entry is a folder, add to folders list
						{
							addSubFolder(fatCurrEntry.ShortEntry, lfnEntries, numOfLFNEntries);
						}
						else if (!isDeletedEntry(fatCurrEntry.ShortEntry)) // else, add to files list
						{
							addFile(fatCurrEntry.ShortEntry, lfnEntries, numOfLFNEntries);
						}

					}
				}

				_isLoaded = true;
			}
		}

		delete[] clustersData;
	}
}

bool FolderEntry::isLoaded()
{
	return _isLoaded;
}

void FolderEntry::addSubFolder(FAT_DIR_ENTRY fatDirEntry, LFN_ENTRY* lfnEntries, WORD numOfLFNEntries)
{
	FolderEntry* folder = new FolderEntry(fatDirEntry, lfnEntries, numOfLFNEntries);
	folder->addParentFolder(this);

	if (!folder->isDeleted())
		_folders.push_back(folder);
}

void FolderEntry::addFile(FAT_DIR_ENTRY fatDirEntry, LFN_ENTRY* lfnEntries, WORD numOfLFNEntries)
{
	FileEntry* file = new FileEntry(fatDirEntry, lfnEntries, numOfLFNEntries);

	if (!file->isDeleted())
		_files.push_back(file);
}

void FolderEntry::addParentFolder(FolderEntry* parent)
{
	_parentFolder = parent;
}

FolderEntry* FolderEntry::getParentFolder()
{
	return _parentFolder;
}

FolderEntry* FolderEntry::navigateToPath(string path)
{
	vector<string> folders;
	// Check if folderName is in the right format
	size_t prevPos = 0;
	size_t pos = path.find('/');

	while (pos != string::npos)
	{
		if (pos == prevPos)
		{
			//throw exception("Empty folder is not allowed");
			return nullptr;
		}

		folders.push_back(path.substr(prevPos, pos - prevPos));
		prevPos = pos + 1;
		pos = path.find('/', prevPos);
	}

	if (prevPos == path.length())
	{
		if (folders.empty())
			return nullptr;
	}
	else
	{
		if (prevPos > 0)
			folders.push_back(path.substr(prevPos));
		else
			if (path.length() > 0)
				folders.push_back(path);
	}

	FolderEntry* currFolder = this; // TODO: use copy constructor
	for (int i = 0; i < folders.size(); i++)
	{
		currFolder = currFolder->navigateToDirectory(folders[i]);

		if (currFolder == nullptr)
		{
			return nullptr;
		}

		currFolder->load();
	}

	return currFolder;
}

FolderEntry* FolderEntry::navigateToDirectory(string folderName)
{
	if (folderName == ".") // Current folder
	{
		return this;
	}
	else if (folderName == "..") // Parent folder
	{
		return this->getParentFolder();
	}

	std::transform(folderName.begin(), folderName.end(), folderName.begin(), ::tolower);

	if (_folders.size() > 0)
	{
		for (int i = 0; i < _folders.size(); i++)
		{
			string childFolder = _folders[i]->getName();
			std::transform(childFolder.begin(), childFolder.end(), childFolder.begin(), ::tolower);

			if (folderName == childFolder)
			{
				return _folders[i];
			}
		}

		return nullptr;
	}

	return nullptr;
}


void FolderEntry::listFoldersAndFiles()
{
	int sectorPadding = 15;
	int	lastWrtTimePadding = 28;
	int lengthPadding = 15;

	cout << std::endl;
	cout << "Mode " << std::setw(sectorPadding) << "Sector" << std::setw(lastWrtTimePadding) << "LastWriteTime" << std::setw(lengthPadding) << "Length" << " " << "Name";
	cout << std::endl;
	cout << "---- " << std::setw(sectorPadding) << "------" << std::setw(lastWrtTimePadding) << "-------------" << std::setw(lengthPadding) << "------" << " " << "----";

	// List Folders
	for (int i = 0; i < _folders.size(); i++)
	{
		cout << std::endl;

		// Mode
		cout << mode_map[_folders[i]->getAttribute()];

		// Starting sector
		cout << std::setw(sectorPadding);
		cout << _folders[i]->getFirstSector();

		// Last write time
		cout << std::setw(lastWrtTimePadding);
		cout << _folders[i]->getLastWriteTime();

		// Size (empty)
		cout << std::setw(lengthPadding);
		cout << "";
		cout << " ";

		// Folder name
		cout << _folders[i]->getName();
	}

	// List files
	for (int i = 0; i < _files.size(); i++)
	{
		cout << std::endl;

		// Mode
		cout << mode_map[_files[i]->getAttribute()];

		// Starting sector
		cout << std::setw(sectorPadding);
		cout << _files[i]->getFirstSector();

		// Last write time
		cout << std::setw(lastWrtTimePadding);
		cout << _files[i]->getLastWriteTime();

		// Size
		cout << std::setw(lengthPadding);
		cout << _files[i]->getFileSize();
		cout << " ";

		// File name
		cout << _files[i]->getName();
	}
}

void FolderEntry::printTree()
{
	
}

void FolderEntry::printFileData(string fileName)
{
	std::transform(fileName.begin(), fileName.end(), fileName.begin(), ::tolower);

	if (_files.size() > 0)
	{
		for (int i = 0; i < _files.size(); i++)
		{
			string file = _files[i]->getName();
			std::transform(file.begin(), file.end(), file.begin(), ::tolower);

			if (fileName == file)
			{
				_files[i]->printData();

				return;
			}
		}
	}

	cout << "\n No files found.";
}



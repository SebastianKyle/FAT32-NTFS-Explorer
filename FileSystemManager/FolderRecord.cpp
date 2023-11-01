#include "FolderRecord.h"
#include "algorithm"

map<ATTR_FILENAME_FLAG, string> ntfs_modes_map = {
	{ ATTR_FILENAME_FLAG_DIRECTORY, "d----" },
	{ ATTR_FILENAME_FLAG_ARCHIVE, "-a---" },
	{ ATTR_FILENAME_FLAG_READONLY, "--r--" },
	{ ATTR_FILENAME_FLAG_HIDDEN, "---h-" },
	{ ATTR_FILENAME_FLAG_SYSTEM, "----s" }
};

FolderRecord::FolderRecord()
{
	_isRootFolder = true;
	_recordID = 5;
}

FolderRecord::FolderRecord(BYTE* mftRecord)
	:Record(mftRecord)
{

}

FolderRecord::~FolderRecord()
{
	_folders.clear();
	_files.clear();

	if (_parentFolder != nullptr)
	{
		delete _parentFolder;
	}
}

void FolderRecord::addSubFolder(FolderRecord* subFolder)
{
	if (!subFolder->isDeleted() && !subFolder->isHidden() && !subFolder->isSystem())
	{
		_folders.push_back(subFolder);
	}
}

void FolderRecord::addFile(FileRecord* file)
{
	if (!file->isDeleted() && !file->isHidden() && !file->isSystem())
	{
		_files.push_back(file);
	}
}

void FolderRecord::setParentFolder(FolderRecord* parent)
{
	_parentFolder = parent;
}

void FolderRecord::setRecordID()
{
	_recordID = _recordHeader.FRH_RecordID;
}

void FolderRecord::setParentRecordID()
{
	_parentRecordID = _fileName->getParentRecordID();
}

bool FolderRecord::load()
{
	if (Record::load())
	{
		setRecordID();
		setParentRecordID();
		_isLoaded = true;

		return true;
	}

	return false;
}

bool FolderRecord::isLoaded()
{
	return _isLoaded;
}

std::wstring FolderRecord::getUnicodeName()
{
	if (_isRootFolder)
	{
		char driveLetter = VolumeAccess::getWorkingDriveLetter();
		string strName = string(1, driveLetter) + ":";
		std::wstring name(strName.begin(), strName.end());

		return name;
	}
	else
	{
		return _fileName->getUnicodeFilename();
	}
}

string FolderRecord::getAnsiName()
{
	string fileName = _fileName->getAnsiFilename();
	if (fileName == ".")
	{
		string driveLetter(1, VolumeAccess::getWorkingDriveLetter());
		driveLetter += ":";

		return driveLetter;
	}

	return fileName;
}


FolderRecord* FolderRecord::getParentFolder()
{
	return _parentFolder;
}

FolderRecord* FolderRecord::navigateToDirectory(string folderName)
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
			string childFolder = _folders[i]->getAnsiName();
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

FolderRecord* FolderRecord::navigateToPath(string path)
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

	FolderRecord* currFolder = this; // TODO: use copy constructor
	for (int i = 0; i < folders.size(); i++)
	{
		currFolder = currFolder->navigateToDirectory(folders[i]);

		if (currFolder == nullptr)
		{
			return nullptr;
		}
	}

	return currFolder;
	
}

void FolderRecord::listFoldersAndFiles()
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
		if (_folders[i]->isReadOnly())
		{
			cout << ntfs_modes_map[ATTR_FILENAME_FLAG_READONLY];
		}
		else if (_folders[i]->isArchive())
		{
			cout << ntfs_modes_map[ATTR_FILENAME_FLAG_ARCHIVE];
		}
		else if (_folders[i]->isDirectory())
		{
			cout << ntfs_modes_map[ATTR_FILENAME_FLAG_DIRECTORY];
		}
		else if (_folders[i]->isSystem())
		{
			cout << ntfs_modes_map[ATTR_FILENAME_FLAG_SYSTEM];
		}
		else if (_folders[i]->isHidden())
		{
			cout << ntfs_modes_map[ATTR_FILENAME_FLAG_HIDDEN];
		}

		// Starting sector
		cout << std::setw(sectorPadding);
		cout << _folders[i]->getFirstSector();

		// Last write time
		cout << std::setw(lastWrtTimePadding);
		cout << _folders[i]->getLastAccessTime();

		// Size (empty)
		cout << std::setw(lengthPadding);
		cout << "";
		cout << " ";

		// Folder name
		cout << _folders[i]->getAnsiName();
	}

	// List files
	for (int i = 0; i < _files.size(); i++)
	{
		cout << std::endl;

		// Mode
		if (_files[i]->isReadOnly())
		{
			cout << ntfs_modes_map[ATTR_FILENAME_FLAG_READONLY];
		}
		else if (_files[i]->isArchive())
		{
			cout << ntfs_modes_map[ATTR_FILENAME_FLAG_ARCHIVE];
		}
		else if (_files[i]->isDirectory())
		{
			cout << ntfs_modes_map[ATTR_FILENAME_FLAG_DIRECTORY];
		}
		else if (_files[i]->isSystem())
		{
			cout << ntfs_modes_map[ATTR_FILENAME_FLAG_SYSTEM];
		}
		else if (_files[i]->isHidden())
		{
			cout << ntfs_modes_map[ATTR_FILENAME_FLAG_HIDDEN];
		}

		// Starting sector
		cout << std::setw(sectorPadding);
		cout << _files[i]->getFirstSector();

		// Last write time
		cout << std::setw(lastWrtTimePadding);
		cout << _files[i]->getLastAccessTime();

		// Size
		cout << std::setw(lengthPadding);
		cout << _files[i]->getFileSize();
		cout << " ";

		// File name
		cout << _files[i]->getAnsiName();
	}
}

void FolderRecord::printFileData(string fileName)
{
	std::transform(fileName.begin(), fileName.end(), fileName.begin(), ::tolower);

	if (_files.size() > 0)
	{
		for (int i = 0; i < _files.size(); i++)
		{
			string file = _files[i]->getAnsiName();
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





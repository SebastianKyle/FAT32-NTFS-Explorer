#include "FileSystem.h"

FileSystem::FileSystem(char driveLetter)
{
	VolumeAccess::setWorkingDriveLetter(driveLetter);
	_rootDir = nullptr;
}

FileSystem::~FileSystem()
{
	if (_rootDir != nullptr)
	{
		delete _rootDir;
	}

	if (_curFATDir != nullptr)
	{
		delete _curFATDir;
	}

	if (_rootFolder != nullptr)
	{
		delete _rootFolder;
	}

	if (_mftFile != nullptr) 
	{
		delete _mftFile;
	}

	if (_curNTFSDir != nullptr)
	{
		delete _curNTFSDir;
	}
}

RootFolder* FileSystem::getRootFolder()
{
	return _rootDir;
}

FolderEntry* FileSystem::getFATCurDir()
{
	return _curFATDir;
}

FolderRecord* FileSystem::getNTFSCurDir()
{
	return _curNTFSDir;
}

bool FileSystem::initialize()
{
	if (_rootDir != nullptr)
		delete[] _rootDir;

	if (VolumeAccess::getInstance() == nullptr)
	{
		cout << "\n The device is not ready...";
		return false;
	}
	else
	{
		if (VolumeAccess::getInstance()->getFileSystemName() == "FAT32")
		{
			_fileSystem = "FAT32";
			_rootDir = new RootFolder();
			_rootDir->load();

			_curFATDir = _rootDir;
			_curPath = _rootDir->getName() + "/";
		}
		else
		{
			_fileSystem = "NTFS";
			buildDirectoryTree();

			_curPath = _rootFolder->getAnsiName() + "/";
		}

		return true;
	}
}

void FileSystem::changeDriveLetter(char driveLetter)
{
	VolumeAccess::setWorkingDriveLetter(driveLetter);
}

char FileSystem::getCurrentDriveLetter()
{
	return VolumeAccess::getWorkingDriveLetter();
}

FolderRecord* FileSystem::getRootRecord()
{
	return _rootFolder;
}

void FileSystem::buildDirectoryTree()
{
	//_rootFolder = new FolderRecord();

	/* Read all records in $MFT */
	BYTE* mftFileRecord = VolumeAccess::getInstance()->getMFTFileRecord();

	_mftFile = new FileRecord(mftFileRecord);
	_mftFile->load();
	_mftFile->loadData();

	BYTE* mftFileData = _mftFile->getData();
	DWORD bytesPerRecord = VolumeAccess::getInstance()->getBytesPerRecord();
	ULONGLONG dataSize = _mftFile->getFileSize();
	ULONGLONG bytesRead = 0;
	map<ULONGLONG, ULONGLONG> dataRunsOffsets = _mftFile->getDataRunsOffsets();

	map<DWORD, FolderRecord*> foldersMap = {};
	//foldersMap[5] = _rootFolder;
	map<DWORD, FileRecord*> filesMap = {};

	ULONGLONG offset = 0;

	// TODO: handle the case when the a record is empty (all bytes zero)
	while (bytesRead < dataSize)
	{
		if (dataRunsOffsets.count(offset))
		{
			offset = dataRunsOffsets[bytesRead];
		}

		BYTE* mftRecord = new BYTE[bytesPerRecord];
		memcpy(mftRecord, mftFileData + bytesRead, bytesPerRecord);

		if (mftRecord[0] == 0x00) // record is empty
		{
			bytesRead += bytesPerRecord;

			continue;
		}

		offset += bytesRead;
		Record* newRecord = new Record(mftRecord);
		if (newRecord->load()) 
		{
			if (newRecord->isDirectory())
			{
				FolderRecord* newFolder = new FolderRecord(mftRecord);
				if (newFolder->load())
				{
					newFolder->setFirstSector(0, offset / VolumeAccess::getInstance()->getBytesPerSec());
					foldersMap[newFolder->getRecordID()] = newFolder;
				}
			}
			else if (newRecord->isFile())
			{
				FileRecord* newFile = new FileRecord(mftRecord);
				if (newFile->load())
				{
					newFile->setFirstSector(0, offset / VolumeAccess::getInstance()->getBytesPerSec());
					filesMap[newFile->getRecordID()] = newFile;
				}
			}
		}

		bytesRead += bytesPerRecord;
	}

	/* Build dir tree */

	// Find root dir
	for (auto i : foldersMap) 
	{
		DWORD parentRecordID = i.second->getParentRecordID();
		if (parentRecordID == i.second->getRecordID())
		{
			_rootFolder = i.second;
			_curNTFSDir = _rootFolder;
			break;
		}
	}

	// Build dirs
	for (auto i : foldersMap)
	{
		DWORD parentRecordID = i.second->getParentRecordID();
		if (foldersMap.count(parentRecordID) && parentRecordID != i.second->getRecordID())
		{
			foldersMap[parentRecordID]->addSubFolder(i.second);
			i.second->setParentFolder(foldersMap[parentRecordID]);
		}
	}

	// Build files
	for (auto i : filesMap)
	{
		DWORD parentRecordID = i.second->getParentRecordID();
		if (foldersMap.count(parentRecordID))
		{
			foldersMap[parentRecordID]->addFile(i.second);
		}
	}

}

void FileSystem::printFSStat()
{
	if (_fileSystem == "NTFS") 
	{
		cout << "\n Volume name: " << _rootFolder->getAnsiName();
		cout << "\n Volume information: ";
		cout << "\n OEM_ID: " << _fileSystem;
		cout << "\n Bytes per sector: " << VolumeAccess::getInstance()->getBytesPerSec();
		cout << "\n Sectors per clusters: " << VolumeAccess::getInstance()->getSecPerClus();
		cout << "\n Reserved sectors: " << VolumeAccess::getInstance()->getReservedSector();
		cout << "\n Total sectors in volume: " << VolumeAccess::getInstance()->getTotalSecs();
		cout << "\n First cluster of $MFT: " << VolumeAccess::getInstance()->getMFTClus();
		cout << "\n First cluster of $MFTMirr: " << VolumeAccess::getInstance()->getMFTMirrClus();
		cout << "\n MFT record size: " << VolumeAccess::getInstance()->getBytesPerRecord();
	}
	else
	{
		cout << "\n Volume name: " << _rootDir->getName();
		cout << "\n Volume information: ";
		cout << "\n Bytes per sector: " << VolumeAccess::getInstance()->getBytesPerSec();
		cout << "\n Sectors per clusters: " << VolumeAccess::getInstance()->getSecPerClus();
		cout << "\n Reserved sectors: " << VolumeAccess::getInstance()->getReservedSector();
		cout << "\n Sectors per FAT: " << VolumeAccess::getInstance()->getSecPerFAT();
		cout << "\n Number of FAT copies: " << VolumeAccess::getInstance()->getFATsNum();
		cout << "\n Total sectors in volume: " << VolumeAccess::getInstance()->getTotalSecs();
		cout << "\n Starting cluster of RDET: " << VolumeAccess::getInstance()->getRDETFirstClus();
		cout << "\n FAT name: " << _fileSystem;
	}
}


void FileSystem::navigateToPath(string path)
{
	if (_fileSystem == "NTFS")
	{
		FolderRecord* curFolder = _curNTFSDir->navigateToPath(path);
		if (curFolder == nullptr)
		{
			cout << "\n No folder found.";
		}
		else
		{
			_curNTFSDir = curFolder;

			updateCurPath(path);
		}
	}
	else
	{
		FolderEntry* curFolder = _curFATDir->navigateToPath(path);
		if (curFolder == nullptr)
		{
			cout << "\n No folder found.";
		}
		else
		{
			_curFATDir = curFolder;

			updateCurPath(path);
		}
	}
}

void FileSystem::list()
{
	if (_fileSystem == "NTFS")
	{
		_curNTFSDir->listFoldersAndFiles();
	}
	else
	{
		_curFATDir->listFoldersAndFiles();
	}
}

string FileSystem::getCurPath()
{
	return _curPath;
}

void FileSystem::updateCurPath(string path)
{
	std::istringstream iss(path);
    std::vector<std::string> tokens;
    std::string token;

    // Tokenize the userInput based on '/'
    while (std::getline(iss, token, '/')) {
        tokens.push_back(token);
    }

    for (const std::string& token : tokens) {
        if (token == "..") {
            // Find the last occurrence of '/' to remove the last directory
            size_t lastSlash = _curPath.find_last_of('/');
            if (lastSlash != std::string::npos) {
                _curPath = _curPath.substr(0, lastSlash);
            }
        } else {
            // Append the directory to the current path
            _curPath += "/" + token;
        }
    }	
}


void FileSystem::printFileData(string fileName)
{
	if (_fileSystem == "NTFS")
	{
		_curNTFSDir->printFileData(fileName);
	}
	else
	{
		_curFATDir->printFileData(fileName);
	}
}


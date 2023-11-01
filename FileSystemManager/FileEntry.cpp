#include "FileEntry.h"

FileEntry::FileEntry(FAT_DIR_ENTRY rootDirEntry, LFN_ENTRY* lfnEntries, WORD numOfLFNEntries)
	:Entry(rootDirEntry, lfnEntries, numOfLFNEntries)
{
	setFirstSector(getFirstClusterInDataChain());
	setFileSize();
}

FileEntry::~FileEntry()
{
	
}

void FileEntry::setFileSize()
{
	_fileSize = _rootDirEntry.DIR_FileSize;
}

DWORD FileEntry::getFileSize()
{
	return _fileSize;
}

void FileEntry::readData()
{
	_fileData = new BYTE[_fileSize];

	DWORD bufferSz = _fileSize % 512 == 0 ? _fileSize : _fileSize + (512 - _fileSize % 512);
	BYTE* buffer = new BYTE[bufferSz];

	if (VolumeAccess::getInstance()->readBytesFromCluster(buffer, bufferSz, getFirstClusterInDataChain()))
	{
		memcpy(_fileData, buffer, _fileSize);
		delete[] buffer;
	}
	else
	{
		cout << "\n Error occurred, can not read file data";
	}
}

void FileEntry::printData()
{
	if (getFileExtension() == "txt")
	{
		readData();

		string data(reinterpret_cast<char*>(_fileData), _fileSize);
		cout << std::endl;
		cout << data;
	}
	else
	{
		cout << "\n Not a text file, please choose a proper software to open file.";
	}
}

string FileEntry::getFileExtension()
{
	string extension = reinterpret_cast<const char*>(_rootDirEntry.DIR_Ext);
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

	return extension;
}



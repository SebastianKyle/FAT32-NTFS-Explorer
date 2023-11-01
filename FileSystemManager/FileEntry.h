#pragma once

#include "Entry.h"
#include "algorithm"

class FileEntry : public Entry
{
private:
	DWORD _fileSize;
	BYTE* _fileData;

	void setFileSize();
public:
	FileEntry(FAT_DIR_ENTRY rootDirEntry, LFN_ENTRY* LFNEntries, WORD numOfLFNEntries);
	~FileEntry();

	string getFileExtension();
	DWORD getFileSize();
	void readData();
	void printData();
};

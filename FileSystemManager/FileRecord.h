#pragma once

#include "Record.h"

class FileRecord : public Record
{
private:
	DWORD _fileSize;

	void setFileSize();

public:
	FileRecord(BYTE* mftRecord);
	virtual ~FileRecord();

	string getFileExtension();
	DWORD getFileSize();

	virtual bool load() override;

	map<ULONGLONG, ULONGLONG> getDataRunsOffsets();
	void loadData();
	BYTE* getData();
	void printData(); // for text files
};

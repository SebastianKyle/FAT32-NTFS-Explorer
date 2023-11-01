#include "FileRecord.h"

FileRecord::FileRecord(BYTE* mftRecord)
	:Record(mftRecord)
{

}

FileRecord::~FileRecord()
{
	
}

void FileRecord::setFileSize()
{
	if (_nResData)
	{
		_fileSize = _nonResidentData->getDataSize();
	}
	else
	{
		_fileSize = _residentData->getDataSize();
	}
}

string FileRecord::getFileExtension()
{
	string name = getAnsiName();
	int dot = name.find('.');
	string ext = name.substr(dot + 1, name.length() - dot);

	return ext;
}

DWORD FileRecord::getFileSize()
{
	return _fileSize;
}

bool FileRecord::load()
{
	if (Record::load())
	{
		setFileSize();
		return true;
	}

	return false;
}

map<ULONGLONG, ULONGLONG> FileRecord::getDataRunsOffsets()
{
	map<ULONGLONG, ULONGLONG> dataRunsOffsets = {};

	if (_nResData)
	{
		dataRunsOffsets = _nonResidentData->getDataRunsOffsets();
	}

	return dataRunsOffsets;
}

void FileRecord::loadData()
{
	if (_nResData)
	{
		_nonResidentData->readData(_mftRecord, &_curOffset);
	}
}

BYTE* FileRecord::getData()
{
	if (_nResData)
	{
		return _nonResidentData->getData();
	}
	else
	{
		return _residentData->getData();
	}
}

void FileRecord::printData()
{
	if (getFileExtension() == "txt")
	{
		BYTE* rawData;
		if (_nResData)
		{
			rawData = _nonResidentData->getData();
		}
		else
		{
			rawData = _residentData->getData();
		}

		string data(reinterpret_cast<char*>(rawData), getFileSize());
		cout << std::endl;
		cout << data;
	}
	else
	{
		cout << "\n Not a text file, please choose a proper software to open file.";
	}
}



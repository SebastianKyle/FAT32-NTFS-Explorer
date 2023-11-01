#include "Record.h"

Record::Record()
{
	
}

Record::Record(BYTE* mftRecord)
{
	_mftRecord = mftRecord;
	_curOffset = 0;

	_stdInfo = nullptr;
	_fileName = nullptr;
	_residentData = nullptr;
	_nonResidentData = nullptr;
}

Record::~Record()
{
	if (_mftRecord != nullptr)
	{
		delete[] _mftRecord;
	}

	if (_stdInfo != nullptr)
	{
		delete _stdInfo;
	}

	if (_fileName != nullptr)
	{
		delete _fileName;
	}

	if (_residentData != nullptr)
	{
		delete _residentData;
	}

	if (_nonResidentData != nullptr)
	{
		delete _nonResidentData;
	}
}

void Record::readRecordHeader()
{
	memcpy(&_recordHeader, _mftRecord, sizeof(NTFS_FILE_RECORD_HEADER));
	_curOffset += sizeof(NTFS_FILE_RECORD_HEADER);
}

void Record::readAttrHeaderCommon(NTFS_ATTR_HEADER_COMMON* attrHeaderCommon)
{
	memcpy(attrHeaderCommon, _mftRecord + _curOffset, sizeof(NTFS_ATTR_HEADER_COMMON));
	_curOffset += sizeof(NTFS_ATTR_HEADER_COMMON);
}

bool Record::loadStdInfoAttr()
{
	return _stdInfo->load(_mftRecord, &_curOffset);
}

bool Record::loadFileNameAttr()
{
	return _fileName->load(_mftRecord, &_curOffset);
}

bool Record::loadDataAttr()
{
	if (_nResData)
	{
		return _nonResidentData->load(_mftRecord, &_curOffset);
	}
	else
	{
		return _residentData->readData(_mftRecord, &_curOffset);
	}
}

void Record::setFirstSector(DWORD startCluster, DWORD startSector)
{
	if (startCluster > 0)
	{
		_firstSector = startCluster * VolumeAccess::getInstance()->getSecPerClus();
	}
	else
	{
		_firstSector = startSector;
	}
}

bool Record::parseAttr()
{
	NTFS_ATTR_HEADER_COMMON attrHeaderCommon = {};
	readAttrHeaderCommon(&attrHeaderCommon);
	if (ATTR_TYPE_MAP[attrHeaderCommon.AttrType] == ATTR_TYPE_STANDARD_INFORMATION)
	{
		_stdInfo = new AttrStdInfo(attrHeaderCommon);
		loadStdInfoAttr();

		return true;
	}
	else if (ATTR_TYPE_MAP[attrHeaderCommon.AttrType] == ATTR_TYPE_FILE_NAME)
	{
		if (_fileName != nullptr)
		{
			delete _fileName;
		}
		_fileName = new AttrFileName(attrHeaderCommon);
		loadFileNameAttr();

		return true;
	}
	else if (isFile() && ATTR_TYPE_MAP[attrHeaderCommon.AttrType] == ATTR_TYPE_DATA)
	{
		bool loadNeeded = false;
		if (attrHeaderCommon.NonResident == 1)
		{
			_nResData = true;

			if (_nonResidentData != nullptr)
			{
				delete _nonResidentData;
				_nonResidentData = nullptr;
			}

			if (_residentData != nullptr)
			{
				delete _residentData;
				_residentData = nullptr;
			}

			_nonResidentData = new AttrData<AttrNonResident>(attrHeaderCommon);
			loadNeeded = true;
		}
		else
		{
			if (_nonResidentData == nullptr)
			{
				loadNeeded = true;
				_nResData = false;

				if (_residentData != nullptr)
				{
					delete _residentData;
					_residentData = nullptr;
				}
				_residentData = new AttrData<AttrResident>(attrHeaderCommon);
			}
			else
			{
				loadNeeded = false;
			}
		}

		if (loadNeeded && !loadDataAttr())
		{
			if (_nonResidentData != nullptr)
			{
				delete _nonResidentData;
				_nonResidentData = nullptr;
			}
			
			if (_residentData != nullptr)
			{
				delete _residentData;
				_residentData = nullptr;
			}

			return false;
		}
		else if (!loadNeeded)
		{
			_curOffset += attrHeaderCommon.TotalSize - sizeof(NTFS_ATTR_HEADER_COMMON);
		}

		return true;
	}
	else // if not one of the 3 wanted attrs, pass
	{
		_curOffset += attrHeaderCommon.TotalSize - sizeof(NTFS_ATTR_HEADER_COMMON);
	}

	return false;
}

bool Record::load()
{
	// TODO: create parseAttr() method to parse each attribute
	readRecordHeader();

	while (_mftRecord[_curOffset] != 0xFF)
	{
		parseAttr();
	}

	if (_stdInfo == nullptr || _fileName == nullptr)
	{
		return false;
	}

	if (isFile() && _nonResidentData == nullptr && _residentData == nullptr) 
	{
		return false;
	}

	return true;
}

DWORD Record::getFirstSector()
{
	return _firstSector;
}

DWORD Record::getRecordID()
{
	return _recordHeader.FRH_RecordID;
}

DWORD Record::getParentRecordID()
{
	return _fileName->getParentRecordID();
}

bool Record::isDeleted()
{
	if (FILE_RECORD_FLAGS_MAP[_recordHeader.FRH_Flags] == FILE_RECORD_FLAGS_DELETED_FILE
	 || FILE_RECORD_FLAGS_MAP[_recordHeader.FRH_Flags] == FILE_RECORD_FLAGS_DELETED_FOLDER)
	{
		return true;
	}

	return false;
}

bool Record::isDirectory()
{
	if (!isDeleted() && FILE_RECORD_FLAGS_MAP[_recordHeader.FRH_Flags] == FILE_RECORD_FLAGS_FOLDER_IN_USE)
	{
		return true;
	}

	return false;
}

bool Record::isFile()
{
	if (!isDeleted() && FILE_RECORD_FLAGS_MAP[_recordHeader.FRH_Flags] == FILE_RECORD_FLAGS_FILE_IN_USE)
	{
		return true;
	}

	return false;
}

bool Record::isReadOnly()
{
	return _fileName->isReadOnly();
}

bool Record::isHidden()
{
	return _fileName->isHidden();
}

bool Record::isSystem()
{
	return _fileName->isSystem();
}

bool Record::isArchive()
{
	return _fileName->isArchive();
}

string Record::getLastAccessTime()
{
	FILETIME lastAccessTm;
	_stdInfo->getLastAccessTime(&lastAccessTm);

	SYSTEMTIME sysTm;
	if (FileTimeToSystemTime(&lastAccessTm, &sysTm))
	{
		stringstream builder;
		builder << std::setfill('0') << std::setw(2) << sysTm.wMonth << "/"
			<< std::setfill('0') << std::setw(2) << sysTm.wDay << "/"
			<< sysTm.wYear << "   "
			<< std::setfill('0') << std::setw(2) << ((sysTm.wHour > 12) ? (sysTm.wHour - 12) : (sysTm.wHour == 0) ? 12 : sysTm.wHour) << ":"
			<< std::setfill('0') << std::setw(2) << sysTm.wMinute
			<< (sysTm.wHour >= 12 ? " PM" : " AM");

		return builder.str();
	}

	return "";
}

std::wstring Record::getUnicodeName()
{
	return _fileName->getUnicodeFilename();
}

std::string Record::getAnsiName()
{
	return _fileName->getAnsiFilename();
}



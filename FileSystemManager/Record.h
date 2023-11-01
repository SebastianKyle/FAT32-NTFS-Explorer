#pragma once

#include "Utilities.h"
#include "VolumeAccess.h"
#include "AttrStdInfo.h"
#include "AttrFileName.h"
#include "AttrData.h"

class Record
{
protected:
	BYTE* _mftRecord; // MFT record
	DWORD _firstSector;

	NTFS_FILE_RECORD_HEADER		_recordHeader;
	AttrStdInfo*				_stdInfo;
	AttrFileName*				_fileName;
	AttrData<AttrResident>*		_residentData;
	AttrData<AttrNonResident>*	_nonResidentData;
	bool						_nResData;
	int							_curOffset;

	Record();

	void readRecordHeader();
	void readAttrHeaderCommon(NTFS_ATTR_HEADER_COMMON* attrHeaderCommon);
	bool parseAttr();
	bool loadStdInfoAttr();
	bool loadFileNameAttr();
	bool loadDataAttr();

public:
	Record(BYTE* mftRecord);
	~Record();

	virtual void setFirstSector(DWORD startCluster = 0, DWORD startSector = 0);

	virtual bool load();

	DWORD getFirstSector();

	DWORD getRecordID();
	DWORD getParentRecordID();

	bool isDeleted();
	bool isDirectory();
	bool isFile();
	bool isReadOnly();
	bool isHidden();
	bool isSystem();
	bool isArchive();
	string getLastAccessTime();
	virtual std::wstring getUnicodeName();
	virtual std::string getAnsiName();
};

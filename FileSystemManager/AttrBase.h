#pragma once

#include "Utilities.h"
#include "VolumeAccess.h"

class AttrBase
{
public:
	AttrBase(NTFS_ATTR_HEADER_COMMON attrHeaderCommon);
	virtual ~AttrBase();

protected:
	NTFS_ATTR_HEADER_COMMON	_attrHeaderCommon;
	BYTE*						_rawBodyData;

	virtual bool loadAttrHeader(BYTE* mftRawData, int* offset) = 0;

public:

	ATTR_TYPE getAttrType();
	DWORD getAttrTotalSize();
	bool isNonResident();
	ATTR_FLAGS getAttrFlags();

	virtual ULONGLONG getDataSize() = 0;
	virtual bool readData(BYTE* mftRawData, int* offset) = 0;
	BYTE* getData();
};

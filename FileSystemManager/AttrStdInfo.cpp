#include "AttrStdInfo.h"

AttrStdInfo::AttrStdInfo(NTFS_ATTR_HEADER_COMMON attrHeaderCommon)
	:AttrResident(attrHeaderCommon)
{
	
}

AttrStdInfo::~AttrStdInfo()
{
	
}

bool AttrStdInfo::load(BYTE* mftRawData, int* offset)
{
	if (*offset > VolumeAccess::getInstance()->getBytesPerRecord()) {
		return false;
	}

	loadAttrHeader(mftRawData, offset);

	memcpy(&_stdInfo, mftRawData + *offset, getDataSize());
	*offset += getDataSize();

	return true;
}

bool AttrStdInfo::isReadOnly()
{
	return _stdInfo.FilePermission && ATTR_STDINFO_PERMISSION_MAP[ATTR_STDINFO_PERMISSION_READONLY] == ATTR_STDINFO_PERMISSION_MAP[ATTR_STDINFO_PERMISSION_READONLY];
}

bool AttrStdInfo::isHidden()
{
	return _stdInfo.FilePermission && ATTR_STDINFO_PERMISSION_MAP[ATTR_STDINFO_PERMISSION_HIDDEN] == ATTR_STDINFO_PERMISSION_MAP[ATTR_STDINFO_PERMISSION_HIDDEN];
}

bool AttrStdInfo::isSystem()
{
	return _stdInfo.FilePermission && ATTR_STDINFO_PERMISSION_MAP[ATTR_STDINFO_PERMISSION_SYSTEM] == ATTR_STDINFO_PERMISSION_MAP[ATTR_STDINFO_PERMISSION_SYSTEM];
}

bool AttrStdInfo::isArchive()
{
	return _stdInfo.FilePermission && ATTR_STDINFO_PERMISSION_MAP[ATTR_STDINFO_PERMISSION_ARCHIVE] == ATTR_STDINFO_PERMISSION_MAP[ATTR_STDINFO_PERMISSION_ARCHIVE];
}

void AttrStdInfo::getLastAccessTime(FILETIME* lastAccessTm)
{
	UTC2Local(_stdInfo.LastAccessTime, lastAccessTm);
}

void AttrStdInfo::UTC2Local(const ULONGLONG& ultm, FILETIME* lftm)
{
	LARGE_INTEGER fti;
	FILETIME ftt;

	fti.QuadPart = ultm;
	ftt.dwHighDateTime = fti.HighPart;
	ftt.dwLowDateTime = fti.LowPart;

	if (!FileTimeToLocalFileTime(&ftt, lftm))
	{
		*lftm = ftt;
	}
}




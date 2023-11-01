#pragma once

#include "AttrResident.h"

class AttrStdInfo : public AttrResident
{
public:
	AttrStdInfo(NTFS_ATTR_HEADER_COMMON attrHeaderCommon);
	virtual ~AttrStdInfo();

private:
	NTFS_ATTR_STANDARD_INFORMATION _stdInfo;

public:
	virtual bool load(BYTE* mftRawData, int* offset) override;

	void getLastAccessTime(FILETIME* lastAccessTm);
	static void UTC2Local(const ULONGLONG& ultm, FILETIME* lftm);

	bool isReadOnly();
	bool isHidden();
	bool isSystem();
	bool isArchive();
};

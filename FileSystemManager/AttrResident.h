#pragma once

#include "AttrBase.h"

class AttrResident : public AttrBase
{
public:
	AttrResident(NTFS_ATTR_HEADER_COMMON attrHeaderCommon);
	virtual ~AttrResident();

protected:
	NTFS_ATTR_HEADER_RESIDENT	_attrHeaderResident;
	WORD						_offsetToBody;
	DWORD						_attrBodySize;

	void setOffsetToBody();
	void setAttrBodySize();

	virtual bool loadAttrHeader(BYTE* mftRawData, int* offset) override;

public:
	virtual ULONGLONG getDataSize() override;
	virtual bool load(BYTE* mftRawData, int* offset);
	virtual bool readData(BYTE* mftRawData, int* offset) override;
};

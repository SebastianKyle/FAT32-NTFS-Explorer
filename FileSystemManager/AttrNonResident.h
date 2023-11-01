#pragma once

#include "AttrBase.h"

class AttrNonResident : public AttrBase
{
public:
	AttrNonResident(NTFS_ATTR_HEADER_COMMON attrHeaderCommon);
	virtual ~AttrNonResident();

protected:
	NTFS_ATTR_HEADER_NON_RESIDENT	_attrHeaderNR;
	vector<DATA_RUN_ENTRY>			_dataRunList;
	map<ULONGLONG, ULONGLONG>		_dataRunsOffsets;

	virtual bool loadAttrHeader(BYTE* mftRawData, int* offset);

	bool pickDataRunUnit(BYTE* mftRawData, LONGLONG* length, LONGLONG* LCNOffset, int* offset, int* bytesRead);
	bool ParseDataRun(BYTE* mftRawData, int* offset);

public:
	virtual ULONGLONG getDataSize() override;
	virtual bool load(BYTE* mftRawData, int* offset);
	virtual bool readData(BYTE* mftRawData, int* offset) override;
	map<ULONGLONG, ULONGLONG> getDataRunsOffsets();
};

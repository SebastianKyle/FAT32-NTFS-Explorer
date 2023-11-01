#include "AttrNonResident.h"

AttrNonResident::AttrNonResident(NTFS_ATTR_HEADER_COMMON attrHeaderCommon)
	:AttrBase(attrHeaderCommon)
{
	
}

AttrNonResident::~AttrNonResident()
{

}

bool AttrNonResident::loadAttrHeader(BYTE* mftRawData, int* offset)
{
	if (*offset >= VolumeAccess::getInstance()->getBytesPerRecord())
	{
		return false;			// offset parameter error
	}

	// Loads up non-resident attribute header
	memcpy(&_attrHeaderNR, mftRawData + *offset, sizeof(NTFS_ATTR_HEADER_NON_RESIDENT));
	*offset += _attrHeaderNR.DataRunOffset - sizeof(NTFS_ATTR_HEADER_COMMON);

	return true;
}

bool AttrNonResident::pickDataRunUnit(BYTE* mftRawData, LONGLONG* length, LONGLONG* LCNOffset, int* offset, int* bytesRead)
{
	//BYTE size = *(mftRawData + *offset);
	//cout << "\n Offset: " << *offset;
	BYTE size = mftRawData[*offset];
	*offset += 1;

	int lengthBytes = size & 0x0F;
	int offsetBytes = size >> 4;
	*bytesRead += 1 + lengthBytes + offsetBytes;

	if (lengthBytes > 8 || offsetBytes > 8)
	{

		return false;
	}

	*length = 0;
	memcpy(length, mftRawData + *offset, lengthBytes);
	if (*length < 0)
	{

		return false;
	}
	*offset += lengthBytes;

	*LCNOffset = 0;
	if (offsetBytes)
	{
		memcpy(LCNOffset, mftRawData + *offset, offsetBytes);
	}
	else
	{
		*LCNOffset = -1;
	}
	*offset += offsetBytes;

	return true;
}

bool AttrNonResident::ParseDataRun(BYTE* mftRawData, int* offset)
{
	//loadAttrHeader(mftRawData, offset);
	// TODO: padding data runs if they dont make up to multiple of 8 bytes

	LONGLONG length;
	LONGLONG LCNOffset;
	LONGLONG LCN = 0;

	int dataRunSize = _attrHeaderCommon.TotalSize - _attrHeaderNR.DataRunOffset;
	while (dataRunSize)
	{
		int bytesRead = 0;
		if (pickDataRunUnit(mftRawData, &length, &LCNOffset, offset, &bytesRead))
		{
			if (bytesRead > 1)
			{
				LCN += LCNOffset;
				if (LCNOffset < 0)
				{
					// Must be sparse data

					*offset += dataRunSize - bytesRead;

					return false;
				}

				DATA_RUN_ENTRY dataRunEntry = {
					length,
					LCN
				};
				_dataRunList.push_back(dataRunEntry);
			}

			dataRunSize -= bytesRead;
		}
		else
		{
			return false;
		}
	}

	// Allocate raw data before reading
	if (_rawBodyData != nullptr)
	{
		delete[] _rawBodyData;
		_rawBodyData = nullptr;
	}
	ULONGLONG dataSize = getDataSize();
	_rawBodyData = new BYTE[dataSize];

	return true;
}

ULONGLONG AttrNonResident::getDataSize()
{
	ULONGLONG size = 0;
	for (int i = 0; i < _dataRunList.size(); i++)
	{
		size += _dataRunList[i].ClusCount * VolumeAccess::getInstance()->getSecPerClus() * VolumeAccess::getInstance()->getBytesPerSec();
	}

	return size;
}

bool AttrNonResident::load(BYTE* mftRawData, int* offset)
{
	loadAttrHeader(mftRawData, offset);

	return ParseDataRun(mftRawData, offset);
}

bool AttrNonResident::readData(BYTE* mftRawData, int* offset)
{
	LONGLONG dataOffset = 0;
	for (int i = 0; i < _dataRunList.size(); i++)
	{
		LONGLONG dataRunBytesSize = _dataRunList[i].ClusCount
				* VolumeAccess::getInstance()->getSecPerClus()
				* VolumeAccess::getInstance()->getBytesPerSec();

		if (!VolumeAccess::getInstance()->readBytesFromCluster(_rawBodyData + dataOffset, dataRunBytesSize, _dataRunList[i].FirstClus))
		{
			return false;
		}

		//_dataRunsOffsets.insert(dataOffset, _dataRunList[i].FirstClus);
		_dataRunsOffsets[dataOffset] = _dataRunList[i].FirstClus;

		dataOffset += dataRunBytesSize;
	}

	return true;
}

map<ULONGLONG, ULONGLONG> AttrNonResident::getDataRunsOffsets()
{
	return _dataRunsOffsets;
}


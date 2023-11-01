#include "AttrResident.h"

AttrResident::AttrResident(NTFS_ATTR_HEADER_COMMON attrHeaderCommon)
	:AttrBase(attrHeaderCommon)
{
	
}

AttrResident::~AttrResident()
{
	
}

void AttrResident::setOffsetToBody()
{
	_offsetToBody = _attrHeaderResident.AttrOffset;
}

void AttrResident::setAttrBodySize()
{
	_attrBodySize = _attrHeaderResident.AttrSize;
}

ULONGLONG AttrResident::getDataSize()
{
	return _attrBodySize;
}

bool AttrResident::loadAttrHeader(BYTE* mftRawData, int* offset)
{
	if (*offset >= VolumeAccess::getInstance()->getBytesPerRecord())
	{
		return false;			// offset parameter error
	}

	// Load up attribute resident header
	memcpy(&_attrHeaderResident, mftRawData + *offset, sizeof(NTFS_ATTR_HEADER_RESIDENT));
	*offset += _attrHeaderResident.AttrOffset - sizeof(NTFS_ATTR_HEADER_COMMON);

	_attrBodySize = _attrHeaderResident.AttrSize;
	//if (_attrBodySize % 8 != 0)
	//{
	//	_attrBodySize += 8 - _attrBodySize % 8;
	//	_attrHeaderResident.AttrSize = _attrBodySize;
	//}

	// Allocate raw body data
	if (_rawBodyData != nullptr)
	{
		delete[] _rawBodyData;
		_rawBodyData = nullptr;
	}
	ULONGLONG dataSize = getDataSize();
	_rawBodyData = (BYTE*) malloc(dataSize);

	return true;
}

bool AttrResident::load(BYTE* mftRawData, int* offset)
{
	return true;
}

bool AttrResident::readData(BYTE* mftRawData, int* offset)
{
	loadAttrHeader(mftRawData, offset);

	if (*offset >= VolumeAccess::getInstance()->getBytesPerRecord())
	{
		return false;			// offset parameter error
	}
		
	// Load raw data ( for resident data attribute )

	// Check for 0xFF bytes: 
	for (int i = 0; i < getDataSize(); i++)
	{
		if (mftRawData[*offset + i] == 0xFF)
		{
			*offset += getDataSize();
			if (*offset % 8 != 0)
			{
				*offset += 8 - *offset % 8;
			}

			return false;
		}
	}

	memcpy(_rawBodyData, mftRawData + *offset, getDataSize());
	*offset += getDataSize();

	if (*offset % 8 != 0)
	{
		*offset += 8 - *offset % 8;
	}

	return true;
}



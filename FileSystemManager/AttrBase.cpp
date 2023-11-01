#include "AttrBase.h"

AttrBase::AttrBase(NTFS_ATTR_HEADER_COMMON attrHeaderCommon)
{
	_attrHeaderCommon = attrHeaderCommon;
	_rawBodyData = nullptr;
}

AttrBase::~AttrBase()
{
	delete[] _rawBodyData;
}

ATTR_TYPE AttrBase::getAttrType()
{
	return ATTR_TYPE_MAP[_attrHeaderCommon.AttrType];
}

DWORD AttrBase::getAttrTotalSize()
{
	return _attrHeaderCommon.TotalSize;
}

bool AttrBase::isNonResident()
{
	return _attrHeaderCommon.NonResident;
}

ATTR_FLAGS AttrBase::getAttrFlags()
{
	return ATTR_FLAGS_MAP[_attrHeaderCommon.Flags];
}

BYTE* AttrBase::getData()
{
	return _rawBodyData;
}



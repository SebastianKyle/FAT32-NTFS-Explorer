#pragma once

#include "AttrResident.h"
#include "AttrNonResident.h"

template <class TYPE_RESIDENT>
class AttrData : public TYPE_RESIDENT
{
public:
	AttrData(NTFS_ATTR_HEADER_COMMON attrHeaderCommon) : TYPE_RESIDENT(attrHeaderCommon)
	{
		
	}

	virtual ~AttrData()
	{
		
	}
	
};

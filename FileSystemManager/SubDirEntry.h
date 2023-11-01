#pragma once

#include "Entry.h"

class SubDirEntry : public Entry
{
public:
	SubDirEntry(FAT_DIR_ENTRY rootDirEntry);
	~SubDirEntry();

	void load();
};

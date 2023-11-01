#pragma once

#include "FolderEntry.h"

// The root directory of which first cluster is from boot sector
class RootFolder : public FolderEntry
{
public:
	RootFolder();
	virtual DWORD	getFirstClusterInDataChain();
	virtual string	getName();

	~RootFolder();
};

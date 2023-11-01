#pragma once

#include "Utilities.h"
#include "VolumeAccess.h"

class Entry
{
protected:
	// Root directory entry
	FAT_DIR_ENTRY		_rootDirEntry;

	// LFN entries
	LFN_ENTRY*		_chainedLFNEntry;
	WORD			_numOfLFNEntries;

	// First sector
	DWORD			_firstSector;

	Entry();

	virtual string getShortName();
	virtual string getLongName();

	void setRootDir(BYTE* rootDir);

	virtual DWORD	getFirstClusterInDataChain();

	// Set the first sector
	void setFirstSector(DWORD startCluster);

public:
	Entry(FAT_DIR_ENTRY rootDirEntry, LFN_ENTRY* lfnEntries, WORD numOfLFNEntries);
	~Entry();

	// Check if file or directory is deleted
	bool isDeleted();

	// Get the bytes data of entry
	BYTE* getRootDirEntry();

	// Get size of the entries
	virtual DWORD getEntrySize();

	// Get file/directory name
	virtual string getName();

	// Get attribute
	ATTRIBUTE getAttribute();

	// Get starting sector
	DWORD getFirstSector();

	// Get last write time
	virtual string getLastWriteTime();
};

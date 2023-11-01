#include "Entry.h"

Entry::Entry(FAT_DIR_ENTRY fatDirEntry, LFN_ENTRY* lfnEntries, WORD numOfLFNEntries)
{
	_rootDirEntry = fatDirEntry;
	_chainedLFNEntry = lfnEntries;
	_numOfLFNEntries = numOfLFNEntries;
}

Entry::Entry()
{
	_rootDirEntry = {};
	_chainedLFNEntry = NULL;
	_numOfLFNEntries = 0;
}

Entry::~Entry()
{
	if (_chainedLFNEntry != nullptr)
		delete[] _chainedLFNEntry;
}

DWORD Entry::getEntrySize()
{
	return sizeof(FAT_DIR_ENTRY) + _numOfLFNEntries * sizeof(LFN_ENTRY);
}

bool Entry::isDeleted()
{
	return isDeletedEntry(_rootDirEntry);
}

BYTE* Entry::getRootDirEntry()
{
	BYTE* entry = new BYTE[getEntrySize()];
	memcpy(entry, _chainedLFNEntry, getEntrySize() - sizeof(FAT_DIR_ENTRY)); // LFN entries
	memcpy(entry + getEntrySize() - sizeof(FAT_DIR_ENTRY), &_rootDirEntry, sizeof(FAT_DIR_ENTRY)); // Root dir entry

	return entry;
}

void Entry::setRootDir(BYTE* rootDir)
{
	memcpy(&_rootDirEntry, rootDir, sizeof(_rootDirEntry));
}

string Entry::getName()
{
	string longName = getLongName();
	if (longName.length() > 0)
	{
		return longName;
	}

	return getShortName();
}

string Entry::getShortName()
{
	stringstream builder;

	string name = reinterpret_cast<const char*>(_rootDirEntry.DIR_Name);
	string extension = reinterpret_cast<const char*>(_rootDirEntry.DIR_Ext);

	if (ATTRIBUTE_MAP[_rootDirEntry.DIR_Attr] == ATTR_SUB_DIRECTORY)
		builder << name;
	else if (ATTRIBUTE_MAP[_rootDirEntry.DIR_Attr] == ATTR_ARCHIVE)
		builder << name << "." << extension;

	return builder.str();
}

string Entry::getLongName()
{
	if (_numOfLFNEntries == 0)
		return "";

	LFN_ENTRY* lfnEntry = _chainedLFNEntry;

	stringstream builder;
	for (int i = _numOfLFNEntries - 1; i >= 0; i--)
	{
		builder << parseLFNName(lfnEntry[i].LDIR_Name1, 5);
		builder << parseLFNName(lfnEntry[i].LDIR_Name2, 6);
		builder << parseLFNName(lfnEntry[i].LDIR_Name3, 2);
	}

	return builder.str();
}

DWORD Entry::getFirstClusterInDataChain()
{
	DWORD firstCluster = 0x00000000;
	firstCluster = firstCluster | _rootDirEntry.DIR_FstClusHw;
	firstCluster = firstCluster << 16; // left shift 16 bits (= 2 bytes) for low word
	firstCluster = firstCluster | _rootDirEntry.DIR_FstClusLw;

	return firstCluster;
}

void Entry::setFirstSector(DWORD startCluster)
{
	_firstSector = VolumeAccess::getInstance()->getSectorNumFromCluster(startCluster);
}

DWORD Entry::getFirstSector()
{
	return _firstSector;
}


ATTRIBUTE Entry::getAttribute()
{
	return ATTRIBUTE_MAP[_rootDirEntry.DIR_Attr];
}

string Entry::getLastWriteTime()
{
	WORD modTime = _rootDirEntry.DIR_ModTime;
	WORD modDate = _rootDirEntry.DIR_ModDate;

	int second = (modTime & 0x1F) * 2;
	int minute = (modTime >> 5) & 0x3F;
	int hour = (modTime >> 11) & 0x1F;
	int day = modDate & 0x1F;
	int month = (modDate >> 5) & 0x0F;
	int year = ((modDate >> 9) & 0x7F) + 1980;

	stringstream builder;
	builder << std::setfill('0') << std::setw(2) << month << "/";
	builder << std::setfill('0') << std::setw(2) << day << "/";
	builder << std::setfill('0') << std::setw(4) << year << " ";
	builder << std::setfill('0') << std::setw(2) << hour << ":";
	builder << std::setfill('0') << std::setw(2) << minute << " ";
	builder << ((hour < 12) ? "AM" : "PM");

	return builder.str();
}


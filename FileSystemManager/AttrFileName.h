#pragma once

#include "AttrResident.h"

class AttrFileName : public AttrResident
{
public:
	AttrFileName(NTFS_ATTR_HEADER_COMMON attrHeaderCommon);
	virtual ~AttrFileName();

protected:
	NTFS_ATTR_FILENAME	_attrFilename;
	string				_ansiFilename;
	wchar_t*			_unicodeFilename;
	int					_nameLength;
	WORD*				_rawName;

private:
	void setAnsiFilename();
	void setUnicodeFilename();
	void getFileNameWUC();

public:
	virtual bool load(BYTE* mftRawData, int* offset);

	ULONGLONG getParentRecordID();
	string getAnsiFilename();
	wchar_t* getUnicodeFilename();

	bool isReadOnly();
	bool isHidden();
	bool isSystem();
	bool isArchive();
	bool isDirectory();

};

#include "AttrFileName.h"

AttrFileName::AttrFileName(NTFS_ATTR_HEADER_COMMON attrHeaderCommon)
	:AttrResident(attrHeaderCommon)
{
	
	_ansiFilename = "";
}

AttrFileName::~AttrFileName()
{
	delete[] _unicodeFilename;
	delete[] _rawName;
}

bool AttrFileName::load(BYTE* mftRawData, int* offset)
{
	if (*offset > VolumeAccess::getInstance()->getBytesPerRecord()) {
		return false;
	}

	loadAttrHeader(mftRawData, offset);

	memcpy(&_attrFilename, mftRawData + *offset, sizeof(NTFS_ATTR_FILENAME));
	*offset += sizeof(NTFS_ATTR_FILENAME);

	if (_rawName != nullptr)
	{
		delete[] _rawName;
		_rawName = nullptr;
	}
	_rawName = new WORD[_attrFilename.NameLength];

	memcpy(_rawName, mftRawData + *offset, _attrFilename.NameLength * sizeof(WORD));
	*offset += _attrFilename.NameLength * sizeof(WORD);

	_ansiFilename = "";
	setAnsiFilename();

	// Padding bytes
	if (*offset % 8 != 0)
	{
		*offset += 8 - (*offset % 8);
	}

	return true;
}

ULONGLONG AttrFileName::getParentRecordID()
{
	unsigned char* bytes = reinterpret_cast<unsigned char*>(&_attrFilename.ParentRef);
	ULONGLONG result = 0;
	for (int i = 0; i < 6; i++)
	{
		result |= static_cast<unsigned long long>(bytes[i]) << (8 * i);
	}

	return result;
}

string AttrFileName::getAnsiFilename()
{
	return _ansiFilename;
}

wchar_t* AttrFileName::getUnicodeFilename()
{
	return _unicodeFilename;
}

void AttrFileName::setAnsiFilename()
{
	char* buffer = new char[_attrFilename.NameLength + 1];

	int len = 0;
	len = WideCharToMultiByte(CP_ACP, 0, (wchar_t*)_rawName, _attrFilename.NameLength, buffer, _attrFilename.NameLength, NULL, NULL);

	if (len)
	{
		buffer[len] = '\0';
		_ansiFilename = buffer;
	}
	else
	{
		_ansiFilename = "";
	}
}

void AttrFileName::getFileNameWUC()
{
	//wcsncpy(_unicodeFilename, (wchar_t*)_attrFilename.Name, _attrFilename.NameLength);
	//wcsncpy_s(_unicodeFilename, _attrFilename.NameLength, (wchar_t*)_attrFilename.Name, _attrFilename.NameLength);
}

bool AttrFileName::isReadOnly()
{
	return _attrFilename.Flags & ATTR_FILENAME_FLAG_MAP[ATTR_FILENAME_FLAG_READONLY];
}

bool AttrFileName::isHidden()
{
	return _attrFilename.Flags & ATTR_FILENAME_FLAG_MAP[ATTR_FILENAME_FLAG_HIDDEN];
}

bool AttrFileName::isSystem()
{
	return _attrFilename.Flags & ATTR_FILENAME_FLAG_MAP[ATTR_FILENAME_FLAG_SYSTEM];
}

bool AttrFileName::isArchive()
{
	return _attrFilename.Flags & ATTR_FILENAME_FLAG_MAP[ATTR_FILENAME_FLAG_ARCHIVE];
}

bool AttrFileName::isDirectory()
{
	return _attrFilename.Flags & ATTR_FILENAME_FLAG_MAP[ATTR_FILENAME_FLAG_DIRECTORY];
}




#pragma once

#include "stdlibs.h"

/////////////////////////////
/// FAT32 Utilities
/////////////////////////////

enum ATTRIBUTE {
	ATTR_READONLY,
	ATTR_HIDDEN,
	ATTR_SYSTEM,
	ATTR_VOLUME_LABEL,
	ATTR_SUB_DIRECTORY,
	ATTR_ARCHIVE,
	ATTR_DEVICE,
	ATTR_UNUSED,
	ATTR_LONG_NAME
};

extern map<int, ATTRIBUTE> ATTRIBUTE_MAP;

#pragma pack(push, 1)

typedef struct
{
	BYTE DIR_Name[8];			// File/Folder name
	BYTE DIR_Ext[3];			// File extension
	BYTE DIR_Attr;				// Attribute
	BYTE DIR_Unused;			// Unused
	BYTE DIR_CrtTimeRef;		// Create time refinement
	WORD DIR_CrtTime;			// Create time
	WORD DIR_CrtDate;			// Create date
	WORD DIR_LstAccDate;		// Last access date
	WORD DIR_FstClusHw;			// First cluster (high word)
	WORD DIR_ModTime;			// Modification time
	WORD DIR_ModDate;			// Modification date
	WORD DIR_FstClusLw;			// First cluster (low word)
	DWORD DIR_FileSize;			// File size
} FAT_DIR_ENTRY;

typedef struct
{
	BYTE LDIR_SeqNum;			// Sequence num
	WORD LDIR_Name1[5];			// File/Folder name (5 chars)
	BYTE LDIR_Attr;				// Attribute
	BYTE LDIR_Type;				// Type (0 = long-name component)
	BYTE LDIR_Checksum;			// Check sum
	WORD LDIR_Name2[6];			// File/Folder name (6 chars)
	WORD LDIR_FstClus;			// First cluster (must be 0)
	WORD LDIR_Name3[2];			// File/Folder name (2 chars)
} LFN_ENTRY;

union FATDirEntryUn
{
	FAT_DIR_ENTRY ShortEntry;
	LFN_ENTRY	LongEntry;
	BYTE		RawData[32];
};

#pragma pack(pop)

bool isLFNEntry(FATDirEntryUn entryToCheck);
bool isFolderEntry(FATDirEntryUn entryToCheck);
bool isSubDirEntry(FATDirEntryUn entryToCheck);
bool isDeletedEntry(FAT_DIR_ENTRY entryToCheck);
string parseLFNName(WORD* name, WORD count);

////////////////////////////
/// NTFS Utilities
////////////////////////////

#pragma pack(push, 1)

#define FILE_RECORD_MAGIC 'ELIF'

enum FILE_RECORD_FLAGS
{
	FILE_RECORD_FLAGS_FILE_IN_USE,
	FILE_RECORD_FLAGS_DELETED_FILE,
	FILE_RECORD_FLAGS_FOLDER_IN_USE,
	FILE_RECORD_FLAGS_DELETED_FOLDER
};

extern map <int, FILE_RECORD_FLAGS> FILE_RECORD_FLAGS_MAP;

typedef struct
{
	DWORD	  FRH_Magic;			// "FILE"
	WORD	  FRH_OffsetToUS;		// Offset to update sequence
	WORD	  FRH_SizeOfUS;			// Size in words of Update sequence
	ULONGLONG FRH_LSN;				// Log file sequence number
	WORD	  FRH_SeqNumber;		// Sequence number
	WORD	  FRH_HardLinks;		// Hard link count
	WORD	  FRH_OffsetAttr;		// Offset to the first attribute
	WORD	  FRH_Flags;			// Flags
	DWORD	  FRH_RealSize;			// Real size of the file record
	DWORD	  FRH_AllocSize;		// Allocated size of the file record
	ULONGLONG FRH_RefToBase;		// Base FILE record
	WORD	  FRH_NextAttrID;		// Next attribute ID
	BYTE	  FRH_Padding1[2];		// Padding 1
	DWORD	  FRH_RecordID;			// ID of this record
	WORD	  FRH_UpdateSeqNum;		// Update sequence number
	DWORD	  FRH_UpdateSeqArray;	// Update sequence array
	BYTE	  FRH_Padding2[2];		// Padding 2

} NTFS_FILE_RECORD_HEADER;

/* Attribute */

enum ATTR_TYPE
{
	ATTR_TYPE_STANDARD_INFORMATION,
	ATTR_TYPE_ATTRIBUTE_LIST,
	ATTR_TYPE_FILE_NAME,
	ATTR_TYPE_OBJECT_ID,
	ATTR_TYPE_SECURITY_DESCRIPTOR,
	ATTR_TYPE_VOLUME_NAME,
	ATTR_TYPE_VOLUME_INFORMATION,
	ATTR_TYPE_DATA,
	ATTR_TYPE_INDEX_ROOT,
	ATTR_TYPE_INDEX_ALLOCATION,
	ATTR_TYPE_BITMAP,
	ATTR_TYPE_REPARSE_POINT,
	ATTR_TYPE_EA_INFORMATION,
	ATTR_TYPE_EA,
	ATTR_TYPE_LOGGED_UTILITY_STREAM
};

extern map<int, ATTR_TYPE> ATTR_TYPE_MAP;

enum ATTR_FLAGS
{
	ATTR_FLAGS_NORMAL,
	ATTR_FLAGS_COMPRESSED,
	ATTR_FLAGS_ENCRYPTED,
	ATTR_FLAGS_SPARSE,
};

extern map<int, ATTR_FLAGS> ATTR_FLAGS_MAP;

typedef struct
{
	DWORD AttrType;				// Attribute type
	DWORD TotalSize;			// Total length (including header)
	BYTE  NonResident;			// 0 - resident, 1 - non resident
	BYTE  NameLength;			// Name length in words
	WORD  NameOffset;			// Name offset
	WORD  Flags;				// Flags
	WORD  AttrID;				// Attribute ID
} NTFS_ATTR_HEADER_COMMON;

typedef struct
{
	//NTFS_ATTR_HEADER_COMMON Header;				// Common header
	DWORD					AttrSize;			// Length of attribute body
	WORD					AttrOffset;			// Offset to the attribute data
	BYTE					IndexedFlag;		// Indexed flag
	BYTE					Padding;			// Padding
} NTFS_ATTR_HEADER_RESIDENT;

typedef struct
{
	//NTFS_ATTR_HEADER_COMMON	Header;				// Common header
	ULONGLONG				FirstVCN;			// First VCN
	ULONGLONG				LastVCN;			// Last VCN
	WORD					DataRunOffset;		// Data run offset
	WORD					CompUnitSize;		// Compression unit size
	BYTE					Padding[4];			// Padding
	ULONGLONG				AllocSize;			// Allocated size of the attribute
	ULONGLONG				RealSize;			// Real size of the attribute
	ULONGLONG				InitSize;			// Initialized size
} NTFS_ATTR_HEADER_NON_RESIDENT;

// Attribute: STANDARD_INFORMATION

enum ATTR_STDINFO_PERMISSION
{
	ATTR_STDINFO_PERMISSION_READONLY,
	ATTR_STDINFO_PERMISSION_HIDDEN,
	ATTR_STDINFO_PERMISSION_SYSTEM,
	ATTR_STDINFO_PERMISSION_ARCHIVE,
	ATTR_STDINFO_PERMISSION_DEVICE,
	ATTR_STDINFO_PERMISSION_NORMAL,
	ATTR_STDINFO_PERMISSION_TEMP,
	ATTR_STDINFO_PERMISSION_SPARSE,
	ATTR_STDINFO_PERMISSION_RESPARSE,
	ATTR_STDINFO_PERMISSION_COMPRESSED,
	ATTR_STDINFO_PERMISSION_OFFLINE,
	ATTR_STDINFO_PERMISSION_NCI,
	ATTR_STDINFO_PERMISSION_ENCRYPTED
};

extern map<ATTR_STDINFO_PERMISSION, int> ATTR_STDINFO_PERMISSION_MAP;

typedef struct
{
	ULONGLONG	CreateTime;				// File created time
	ULONGLONG	ModTime;				// File modified time
	ULONGLONG	MFTTime;				// MFT record changed time
	ULONGLONG	LastAccessTime;			// Last access time
	DWORD		FilePermission;			// File permission
	DWORD		MaxVersionNo;			// Maximum number of versions
	DWORD		VersionNum;				// Version number
	DWORD		ClassID;				// Class ID
	DWORD		OwnerID;				// Owner ID
	DWORD		SecurityID;				// Security ID
	ULONGLONG	QuotaCharged;			// Quota charged
	ULONGLONG	UpdateSeqNumber;		// Update sequence number

} NTFS_ATTR_STANDARD_INFORMATION;

// Attribute: FILE_NAME

enum ATTR_FILENAME_FLAG
{
	ATTR_FILENAME_FLAG_READONLY,
	ATTR_FILENAME_FLAG_HIDDEN,
	ATTR_FILENAME_FLAG_SYSTEM,
	ATTR_FILENAME_FLAG_ARCHIVE,
	ATTR_FILENAME_FLAG_DEVICE,
	ATTR_FILENAME_FLAG_NORMAL,
	ATTR_FILENAME_FLAG_TEMP,
	ATTR_FILENAME_FLAG_SPARSE,
	ATTR_FILENAME_FLAG_RESPARSE,
	ATTR_FILENAME_FLAG_COMPRESSED,
	ATTR_FILENAME_FLAG_OFFLINE,
	ATTR_FILENAME_FLAG_NCI,
	ATTR_FILENAME_FLAG_ENCRYPTED,
	ATTR_FILENAME_FLAG_DIRECTORY,
	ATTR_FILENAME_FLAG_INDEXVIEW
};

extern map<ATTR_FILENAME_FLAG, int> ATTR_FILENAME_FLAG_MAP;

typedef struct
{
	ULONGLONG	ParentRef;				// Parent reference (6 high bytes: parent dir file record num, 3 low bytes: parent dir seq number)
	ULONGLONG	CreateTime;				// File created time (UTC)
	ULONGLONG	ModTime;				// File modified time (UTC)
	ULONGLONG	MFTTime;				// MFT record changed time (UTC)
	ULONGLONG	LastAccessedTime;		// Last accessed time (UTC)
	ULONGLONG	AllocSize;				// Allocated file size
	ULONGLONG	RealSize;				// Real file size
	DWORD		Flags;					// Flags
	DWORD		ER;						// Used by EA and reparse
	BYTE		NameLength;				// File name length
	BYTE		NameSpace;				// Filename space
	//WORD		Name[1];				// Filename
} NTFS_ATTR_FILENAME;

typedef struct
{
	LONGLONG ClusCount;
	LONGLONG FirstClus;
} DATA_RUN_ENTRY;

#pragma pack(pop)


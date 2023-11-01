#pragma once

#include "Utilities.h"

#pragma pack(push, 1)

typedef struct
{
	char BS_BootJmp[3];			// Code to jump to bootstrap
	char BS_OEMName[8];			// OEM name/version
	WORD BS_BytesPerSec;		// Bytes per sector
	BYTE BS_SecPerClus;			// Sectors per cluster
	WORD BS_RsvdSecCount;		// Number of reserved sectors
	BYTE BS_NumFATS;			// Number of FAT copies
	WORD BS_RDEntry;			// Number of root directory entries
	WORD BS_TotalSec16;			// Total number of sectors in file system (FAT 16)
	BYTE BS_Media;				// Media descriptor type
	WORD BS_SecPerFAT16;		// Sectors per FAT (FAT16)
	WORD BS_SecPerTrack;		// Sectors per track
	WORD BS_Head;				// Number of heads
	DWORD BS_HiddenSecs;		// Number of hidden sectors
	DWORD BS_TotalSecs32;		// Total number of sectors in file system (FAT32)
	DWORD BS_SecPerFAT32;		// Sectors per FAT (FAT32)
	WORD BS_ExtFlags;			//
	WORD BS_FSVer;				// File system verison
	DWORD BS_RDETFirstClus;		// First cluster of Root directory entry
	WORD BS_FSInfo;				//
	WORD BS_BKBootSec;			// Index of backup boot sector
	BYTE BS_Reserved[12];		// reserved sectors
	BYTE BS_DrvNum;				// Drive type
	BYTE BS_Reserved1;			//
	BYTE BS_BootSig;			// OS identifier
	DWORD BS_VolId;				// Volume serial number
	BYTE BS_VolLab[11];			// Volume label
	BYTE BS_FSVer2[8];				// File system version
	BYTE BS_BootCode[420];		// Bootstrap code
	WORD BS_Signature;			// Signature
} FATBootSector;

typedef struct
{
	char NBS_BootJmp[3];		// Code to jump bootstrap
	char NBS_OEM[8];			// OEM ID

	WORD NBS_BytesPerSector;	// Bytes per sector
	BYTE NBS_SecPerClus;		// Sectors per cluster
	WORD NBS_ReservedSecs;		// Reserved sectors
	BYTE NBS_Zeros1[3];			// Zeros
	WORD NBS_Unused1;			// Unused
	BYTE NBS_Media;				// Media descriptor
	WORD NBS_Unused2;			// Unused
	WORD NBS_SecsPerTrack;		// Sectors per track
	WORD NBS_NumOfHeads;		// Number of heads
	DWORD NBS_HiddenSecs;		// Hidden sectors
	DWORD NBS_Unused3;			// Unused
	DWORD NBS_Signature;		// Signature
	ULONGLONG NBS_TotalSecs;	// Total sectors
	ULONGLONG NBS_MFTClus;		// MFT cluster number
	ULONGLONG NBS_MFTMirrClus;	// MFT mirror cluster number
	DWORD NBS_ClusPerFileRec;	// Clusters per file record
	DWORD NBS_ClusPerIdxBLock;	// Clusters per index block
	BYTE NBS_VolumeSN[8];		// Volume serial number
	DWORD NBS_Checksum;			// Checksum

	BYTE NBS_Code[426];			// Bootstrap code

	WORD NBS_ClosingSig;		// Closing signature
} NTFSBootSector;

#pragma pack(pop)

class VolumeAccess
{
private:
	HANDLE			_device;
	DWORD			_sectorSize;
	FATBootSector	_bootSector;
	DWORD			_bytesPerCluster;
	DWORD			_bytesPerRecord;
	string			_fileSystem;
	DWORD			_sizePerMFTRecord;

	/* FAT32 */
	DWORD*			_FAT1; // Each FAT element contains 2 bytes
	DWORD*			_FAT2;

	/* NTFS */
	NTFSBootSector	_ntfsBootSector;
	BYTE*			_mftFileRecord;

	static const DWORD FAT_END_CHAIN = 0x0FFFFFFF;
	static char				_driveLetter;
	static VolumeAccess*	_instance;

	VolumeAccess(char driveLetter);

	bool lockAndDismount();
	void readBootSector();
	void initData();
	void readFATsData();
	void readMFTFileRecord();

	// Set pointer to specific sector
	bool goToSector(DWORD sectorNum);

	// Clean resources
	void clean();

public:
	string getFileSystemName();

	DWORD getBytesPerSec();
	BYTE getSecPerClus();
	DWORD getRootDirCluster();
	DWORD getSectorNumFromCluster(DWORD clusterNum);
	WORD getReservedSector();
	ULONGLONG getTotalSecs();
	ULONGLONG getMFTClus();
	ULONGLONG getMFTMirrClus();
	DWORD getMFTRecordSize();
	DWORD getSecPerFAT();
	BYTE getFATsNum();
	DWORD getRDETFirstClus();

	DWORD getBytesPerRecord();
	BYTE* getMFTFileRecord();

	// Read data from a specific cluster
	bool readBytesFromCluster(BYTE* buffer, DWORD dataSize, DWORD startCluster);
	// Read data from a specific sector
	bool readBytesFromSector(BYTE* buffer, DWORD dataSize, DWORD startSector);

	bool readChainedClusters(DWORD startCluster, BYTE* chainedClustersData, DWORD* dataSize);

	static void setWorkingDriveLetter(char driveLetter);
	static char getWorkingDriveLetter();
	static VolumeAccess* getInstance();
	static void cleanResources();

	~VolumeAccess();
};

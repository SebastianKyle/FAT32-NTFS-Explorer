#include "VolumeAccess.h"

VolumeAccess* VolumeAccess::_instance = nullptr;
char VolumeAccess::_driveLetter = 'E';

VolumeAccess* VolumeAccess::getInstance()
{
	try
	{
		if (_instance == nullptr)
		{
			_instance = new VolumeAccess(VolumeAccess::_driveLetter);
		}

		return _instance;
	}
	catch (...)
	{
		return nullptr;
	}
}

VolumeAccess::VolumeAccess(char driveLetter)
{
	wchar_t drivePath[7];
	swprintf(drivePath, 7, L"\\\\.\\%c:", driveLetter);

	HANDLE device = CreateFile(drivePath,		// Drive to open
		GENERIC_READ | GENERIC_WRITE,			// Access mode
		FILE_SHARE_READ | FILE_SHARE_WRITE,		// Share Mode
		NULL,									// Security Descriptor
		OPEN_EXISTING,							// How to create
		FILE_ATTRIBUTE_NORMAL,				// File attributes
		NULL);						// Handle to template

	if (device == INVALID_HANDLE_VALUE)
	{
		cout << "\n Error opening connection to drive " << driveLetter;
		throw "Error creating accessing volume";
	}
	else
	{
		_device = device;

		if (!lockAndDismount())
		{
			clean();
			throw "Error locking/dismounting device";
		}

		initData();
	}
}

VolumeAccess::~VolumeAccess()
{
	clean();
}

void VolumeAccess::cleanResources()
{
	if (_instance != nullptr)
		_instance->clean();
}

void VolumeAccess::clean()
{
	if (_device != nullptr)
	{
		CloseHandle(_device);
	}

	if (_FAT1 != nullptr)
	{
		delete[] _FAT1;
	}

	if (_FAT2 != nullptr)
	{
		delete[] _FAT2;
	}

	if (_mftFileRecord != nullptr)
	{
		delete[] _mftFileRecord;
	}
}

string VolumeAccess::getFileSystemName()
{
	return _fileSystem;
}

void VolumeAccess::setWorkingDriveLetter(char driveLetter)
{
	VolumeAccess::_driveLetter = driveLetter;
}

char VolumeAccess::getWorkingDriveLetter()
{
	return VolumeAccess::_driveLetter;
}

bool VolumeAccess::lockAndDismount()
{
	DWORD returned;
	bool res = DeviceIoControl(_device, FSCTL_DISMOUNT_VOLUME, 0, 0, 0, 0, &returned, 0);

	if (!res)
	{
		cout << "\n Error dismounting the volume, Code: " << GetLastError();
		return false;
	}
	else
	{
		
		//res = DeviceIoControl(_device, FSCTL_LOCK_VOLUME, 0, 0, 0, 0, &returned, 0);
		//if (!res)
		//{
		//	cout << "\n Error locking the volume, Code: " << GetLastError();
		//	return false;
		//}
		//else
		//{
		//	return true;
		//}

		return true;
	}
}

void VolumeAccess::readBootSector()
{
	DWORD pointer = SetFilePointer(_device, 0, NULL, FILE_BEGIN);

	if (pointer == INVALID_SET_FILE_POINTER)
	{
		cout << "\n Set pointer failed, Error: " << GetLastError();
	}
	else
	{
		DWORD bytesRead;
		//BYTE* sector = new BYTE[_sectorSize];

		if (!ReadFile(_device, &_bootSector, sizeof(FATBootSector), &bytesRead, NULL))
		{
			cout << "\n Error reading from the file, Error: " << GetLastError();
		}

		string oemID(_bootSector.BS_OEMName);
		if (oemID.find("NTFS") != string::npos)
		{
			bytesRead = 0;
			DWORD pointer = SetFilePointer(_device, 0, NULL, FILE_BEGIN);
			cout << "\n NTFS boot sector size: " << sizeof(NTFSBootSector);
			ReadFile(_device, &_ntfsBootSector, sizeof(NTFSBootSector), &bytesRead, NULL);

			_fileSystem = "NTFS";
			_bytesPerCluster = _ntfsBootSector.NBS_SecPerClus * _sectorSize;

			int sz = (char)_ntfsBootSector.NBS_ClusPerFileRec;
			if (sz > 0)
			{
				_bytesPerRecord = sz * _bytesPerCluster;
			}
			else
			{
				_bytesPerRecord = 1 << (-sz);
			}
		}
		else
		{
			_fileSystem = "FAT32";
		}

		//cout << "\n Size of boot sector: " << sizeof(_bootSector);
		//memcpy_s(&_bootSector, sizeof(_bootSector), sector, sizeof(sector));
	}
}

void VolumeAccess::initData()
{
	DISK_GEOMETRY_EX diskGeo;
	DWORD bytes;

	if (DeviceIoControl(_device, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &diskGeo, sizeof(DISK_GEOMETRY_EX), &bytes, NULL))
	{
		// Set sector size
		_sectorSize = diskGeo.Geometry.BytesPerSector;
	}

	readBootSector();

	if (_fileSystem == "FAT32")
	{
		readFATsData();
		_bytesPerCluster = _bootSector.BS_SecPerClus * _sectorSize;
	}
	else
	{
		readMFTFileRecord();
	}
}

void VolumeAccess::readFATsData()
{
	cout << "\n Reading FATs data...";

	long fatTableSize = _bootSector.BS_SecPerFAT32 * _sectorSize;

	_FAT1 = (DWORD*)new BYTE[fatTableSize];
	readBytesFromSector((BYTE*) _FAT1, fatTableSize, 0);

	_FAT2 = (DWORD*)new BYTE[fatTableSize];
	readBytesFromSector((BYTE*)_FAT2, fatTableSize, _bootSector.BS_SecPerFAT32);
}

void VolumeAccess::readMFTFileRecord()
{
	cout << "\n Reading $MFT file...";

	ULONGLONG firstClus = _ntfsBootSector.NBS_MFTClus;
	_mftFileRecord = new BYTE[_bytesPerRecord];

	readBytesFromCluster(_mftFileRecord, _bytesPerRecord, firstClus);
}

bool VolumeAccess::readBytesFromCluster(BYTE* buffer, DWORD dataSize, DWORD startCluster)
{
	DWORD startSector = getSectorNumFromCluster(startCluster);

	return readBytesFromSector(buffer, dataSize, startSector);
}

bool VolumeAccess::readBytesFromSector(BYTE* buffer, DWORD dataSize, DWORD startSector)
{
	if (!goToSector(startSector))
	{
		return false;
	}
	else
	{
		DWORD bytesRead;

		if (!ReadFile(_device, buffer, dataSize, &bytesRead, NULL))
		{
			cout << "\n Error reading from device, Code: " << GetLastError();
			return false;
		}

		return true;
	}
}

bool VolumeAccess::goToSector(DWORD sectorNum)
{
	LARGE_INTEGER position;
	position.QuadPart = (static_cast<long long>(_bootSector.BS_RsvdSecCount) + sectorNum) * _sectorSize;

	DWORD pointer = SetFilePointer(_device, position.LowPart, &position.HighPart, FILE_BEGIN);

	if (pointer == INVALID_SET_FILE_POINTER)
	{
		cout << "\n Failed accessing sector number " << sectorNum << " - Set pointer Failed! Code: " << GetLastError();
		return false;
	}

	return true;
}

DWORD VolumeAccess::getBytesPerRecord()
{
	return _bytesPerRecord;
}

BYTE* VolumeAccess::getMFTFileRecord()
{
	return _mftFileRecord;
}

bool VolumeAccess::readChainedClusters(DWORD startCluster, BYTE* chainedClustersData, DWORD* dataSize)
{
	if (chainedClustersData == NULL)
	{
		DWORD totalClustersNum = 0;
		DWORD nextClusterNum = startCluster;

		while (nextClusterNum != FAT_END_CHAIN && nextClusterNum != 0)
		{
			++totalClustersNum;
			nextClusterNum = _FAT1[nextClusterNum];
		}

		if (nextClusterNum == 0)
		{
			*dataSize = 0;
		}
		else
		{
			*dataSize = totalClustersNum * _bytesPerCluster;
		}
	}
	else
	{
		DWORD clustersReadNum = 0;
		DWORD nextClusterNum = startCluster;

		while (nextClusterNum != FAT_END_CHAIN)
		{
			if (!readBytesFromCluster(chainedClustersData + (clustersReadNum * _bytesPerCluster), 
				_bytesPerCluster, 
				nextClusterNum))
			{
				return false;
			}

			nextClusterNum = _FAT1[nextClusterNum];
			++clustersReadNum;
		}
	}

	return true;
}

DWORD VolumeAccess::getBytesPerSec()
{
	return _sectorSize;
}

BYTE VolumeAccess::getSecPerClus()
{
	if (_fileSystem == "FAT32")
		return _bootSector.BS_SecPerClus;

	return _ntfsBootSector.NBS_SecPerClus;
}

DWORD VolumeAccess::getRootDirCluster()
{
	return _bootSector.BS_RDETFirstClus;
}

DWORD VolumeAccess::getSectorNumFromCluster(DWORD clusterNum)
{
	DWORD sectorNum = 0;
	if (_fileSystem == "FAT32")
		sectorNum = _bootSector.BS_NumFATS * _bootSector.BS_SecPerFAT32 + (clusterNum - 2) * _bootSector.BS_SecPerClus;
	else
		sectorNum = clusterNum * _ntfsBootSector.NBS_SecPerClus;

	return sectorNum;
}

WORD VolumeAccess::getReservedSector()
{
	if (_fileSystem == "FAT32")
	{
		return _bootSector.BS_RsvdSecCount;
	}

	return _ntfsBootSector.NBS_ReservedSecs;
}

ULONGLONG VolumeAccess::getTotalSecs()
{
	if (_fileSystem == "FAT32")
	{
		return _bootSector.BS_TotalSecs32;
	}

	return _ntfsBootSector.NBS_TotalSecs;
}

ULONGLONG VolumeAccess::getMFTClus()
{
	return _ntfsBootSector.NBS_MFTClus;
}

ULONGLONG VolumeAccess::getMFTMirrClus()
{
	return _ntfsBootSector.NBS_MFTMirrClus;
}

DWORD VolumeAccess::getMFTRecordSize()
{
	return _bytesPerRecord;
}

DWORD VolumeAccess::getSecPerFAT()
{
	return _bootSector.BS_SecPerFAT32;
}

BYTE VolumeAccess::getFATsNum()
{
	return _bootSector.BS_NumFATS;
}

DWORD VolumeAccess::getRDETFirstClus()
{
	return _bootSector.BS_RDETFirstClus;
}




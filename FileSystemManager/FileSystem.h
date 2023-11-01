#pragma once

#include "VolumeAccess.h"
#include "RootFolder.h"
#include "FileRecord.h"
#include "FolderRecord.h"

class FileSystem
{
private:
	string			_fileSystem;

	// FAT32
	RootFolder*		_rootDir;
	FolderEntry*	_curFATDir;

	// NTFS
	FolderRecord*	_rootFolder;
	FileRecord*		_mftFile;
	FolderRecord*	_curNTFSDir;

	string			_curPath;

	void buildDirectoryTree();

public:
	FileSystem(char driveLetter);
	~FileSystem();

	RootFolder* getRootFolder();

	FolderEntry* getFATCurDir();
	FolderRecord* getNTFSCurDir();
	string getCurPath();
	void updateCurPath(string path);

	void printFSStat();

	void navigateToPath(string path);
	void list();
	void printFileData(string fileName);

	bool initialize();
	void changeDriveLetter(char driveLetter);
	char getCurrentDriveLetter();

	FolderRecord* getRootRecord();
};

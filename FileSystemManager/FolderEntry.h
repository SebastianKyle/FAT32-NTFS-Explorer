#pragma once

#include "Entry.h"
#include "FileEntry.h"
#include "SubDirEntry.h"
#include <algorithm>
#include <iomanip>

extern map<ATTRIBUTE, string> mode_map;

class FolderEntry : public Entry
{
private:
	void			addSubFolder(FAT_DIR_ENTRY fatDirEntry, LFN_ENTRY* lfnEntries, WORD numOfLFNEntries);
	void			addFile(FAT_DIR_ENTRY fatDirEntry, LFN_ENTRY* lfnEntries, WORD numOfLFNEntries);
	void			addParentFolder(FolderEntry* parent);
protected:
	vector<FolderEntry*>	_folders;
	vector<FileEntry*>		_files;
	vector<SubDirEntry*>	_subDirEntries;
	FolderEntry*			_parentFolder;
	bool					_isLoaded;

	FolderEntry();

public:
	FolderEntry(FAT_DIR_ENTRY fatDirEntry, LFN_ENTRY* lfnEntries, WORD numOfLFNEntries);
	FolderEntry(const FolderEntry& other);

	virtual void load();
	bool isLoaded();

	FolderEntry* getParentFolder();
	FolderEntry* navigateToPath(string path);
	FolderEntry* navigateToDirectory(string folderName);
	void listFoldersAndFiles();
	void printFileData(string fileName);
	void printTree();

	~FolderEntry();
};

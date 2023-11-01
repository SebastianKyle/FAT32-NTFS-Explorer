#pragma once

#include "Record.h"
#include "FileRecord.h"

extern map<ATTR_FILENAME_FLAG, string> ntfs_modes_map;

class FolderRecord : public Record
{
protected:
	vector<FolderRecord*>	_folders;
	vector<FileRecord*>		_files;
	FolderRecord*			_parentFolder;
	bool					_isLoaded;
	bool					_isRootFolder;
	DWORD					_recordID;
	ULONGLONG				_parentRecordID;

	void setRecordID();
	void setParentRecordID();

public:
	FolderRecord(BYTE* mftRecord);
	FolderRecord();
	virtual ~FolderRecord();

	void		addSubFolder(FolderRecord* subFolder);
	void		addFile(FileRecord* file);
	void		setParentFolder(FolderRecord* parent);

	virtual bool load() override;

	bool isLoaded();
	virtual std::wstring getUnicodeName() override;
	virtual string getAnsiName() override;
	FolderRecord* getParentFolder();
	FolderRecord* navigateToPath(string path);
	FolderRecord* navigateToDirectory(string folderName);
	void listFoldersAndFiles();
	void printFileData(string fileName);

};

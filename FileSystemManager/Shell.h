#pragma once

#include "FileSystem.h"
#include "lmcons.h"

using std::cin;

class Shell
{
public:
	Shell(FileSystem* file_system);
	~Shell();

	void printPrompt();

	void parseCommand(string cmd);
	void processCommand(const string& cmd, const vector<string>& args);

	void printFSStat();
	void doPWD();
	void doNavigate(string path);
	void doList();
	void doCat(string fileName);
	void doClear();
	void doExit();

	void doCmdLoop();

private:
	FileSystem* _fileSystem;
	wchar_t* _deviceUsername;
	wchar_t* _deviceName;
	bool _exit;
};

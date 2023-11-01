#include "Shell.h"

Shell::Shell(FileSystem* file_system)
	: _fileSystem(file_system)
{
	wchar_t* username = new wchar_t[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    GetUserNameW(username, &username_len);
	_deviceUsername = username;

    wchar_t* computername = new wchar_t[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD computername_len = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerNameW(computername, &computername_len);
	_deviceName = computername;

	_exit = false;
}

Shell::~Shell()
{
	if (_fileSystem != nullptr)
	{
		delete _fileSystem;
	}

	if (_deviceUsername != nullptr)
	{
		delete[] _deviceUsername;
	}

	if (_deviceName != nullptr)
	{
		delete[] _deviceName;
	}
}

void Shell::printPrompt()
{
	string cwd = _fileSystem->getCurPath();

	std::cout << std::endl;

	std::cout << "\033[1;32m";
	std::wcout << _deviceUsername;
	std::cout << "@";
	std::wcout << _deviceName;
	std::cout << "\033[0m:" << "\033[1;34m" << cwd << "\033[0m";
	std::cout << std::endl << "$ ";
}

void Shell::parseCommand(string cmd)
{
	std::istringstream iss(cmd);
    std::vector<std::string> tokens;
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
		break;
    }

	if (tokens[0].length() + 1 < cmd.length())
		tokens.push_back(cmd.substr(tokens[0].length() + 1));

    if (!tokens.empty()) {
        processCommand(tokens[0], std::vector<std::string>(tokens.begin() + 1, tokens.end()));
    }	
}

void Shell::processCommand(const string& cmd, const vector<string>& args)
{
	if (cmd == "cd")
	{
		doNavigate(args[0]);
	}
	else if (cmd == "ls")
	{
		doList();
	}
	else if (cmd == "cat")
	{
		doCat(args[0]);
	}
	else if (cmd == "pwd")
	{
		doPWD();
	}
	else if (cmd == "fsstat")
	{
		printFSStat();
	}
	else if (cmd == "clear" || cmd == "cls")
	{
		doClear();
	}
	else if (cmd == "exit")
	{
		doExit();
	}
	else
	{
		cout << "\n Invalid command: " << cmd << std::endl;
	}
}

void Shell::doPWD()
{
	cout << std::endl << _fileSystem->getCurPath();
}

void Shell::printFSStat()
{
	_fileSystem->printFSStat();
}

void Shell::doNavigate(string path)
{
	_fileSystem->navigateToPath(path);
}

void Shell::doList()
{
	_fileSystem->list();
}

void Shell::doCat(string fileName)
{
	_fileSystem->printFileData(fileName);
}

void Shell::doClear()
{
	system("cls");
}

void Shell::doExit()
{
	cout << "\n Exiting ...";
	_exit = true;
}

void Shell::doCmdLoop()
{
	while (!_exit)
	{
		printPrompt();

		string cmd = "";
		getline(std::cin, cmd);

		parseCommand(cmd);
	}
}


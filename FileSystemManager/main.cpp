#include "Shell.h"

using std::cin;

int main()
{
	cout << " FIT HCMUS - Operating System - FAT32 & NTFS Project";
	cout << "\n ----------------------------";
	cout << "\n *) 21120360 - Tran Y Van";
	cout << "\n *) 21120311 - Ho Phuc";
	cout << "\n *) 21120326 - Doan Manh Tan";
	cout << "\n *) 21120331 - Nguyen Anh Thai";
	cout << "\n ----------------------------";

	// List volumes
	char volumeChar = 'E';
	bool validVolChar = false;

	DWORD drives = GetLogicalDrives();
	vector<char> volumeChars;

	cout << "\n Available volumes: " << std::endl;
    for (char i = 'A'; i <= 'Z'; ++i) {
        if (drives & 1) {
			volumeChars.push_back(i);
            std::wcout << L"1/ " << i << ":" << std::endl;
        }
        drives >>= 1;
    }

	while (!validVolChar)
	{
		cout << "\n Please enter volume character to use: ";
		std::cin >> volumeChar;
		getchar();

		for (int i = 0; i < volumeChars.size(); i++)
		{
			if (volumeChars[i] == volumeChar)
			{
				validVolChar = true;
				break;
			}
		}

		if (!validVolChar)
		{
			cout << "\n Invalid volume character.";
		}
	}

	FileSystem* fileSystem = new FileSystem(volumeChar);
	fileSystem->initialize();

	fileSystem->printFSStat();

	Shell* shell = new Shell(fileSystem);

	shell->doCmdLoop();
	
}

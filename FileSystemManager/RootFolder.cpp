#include "RootFolder.h"

RootFolder::RootFolder()
{
	
}

RootFolder::~RootFolder()
{
	
}

DWORD RootFolder::getFirstClusterInDataChain()
{
	return VolumeAccess::getInstance()->getRootDirCluster();
}

string RootFolder::getName()
{
	string rootFolName(1, VolumeAccess::getInstance()->getWorkingDriveLetter());
	rootFolName += ":";

	return rootFolName;
}


#include "error.h"

using namespace std;

Error::Error()
{
	initialize();
}

void Error::initialize()
{
	dictError_.insert(make_pair(ERROR_FILE_NOT_FOUND, 
			TEXT("The file WinDivert32 or the file WinDivert64 were not found.")));
	dictError_.insert(make_pair(ERROR_ACCESS_DENIED, 
			TEXT("The calling application doesn't have Administrator privileges.")));
	dictError_.insert(make_pair(ERROR_INVALID_PARAMETER,
			TEXT("This indicates an individual packet filter string, layer, priority, or flags.")));
	dictError_.insert(make_pair(ERROR_INVALID_IMAGE_HASH,
			TEXT("The file WinDivert32.sys or WinDivert64.sys driver doesn't have a valid digital signature.")));
	dictError_.insert(make_pair(ERROR_DRIVER_BLOCKED,
			TEXT("WinDivert driver is blocked.")));
}

//string Error::getErrorDescription(DWORD index)
//{
//	if (find(dictError_.begin(), dictError_.end(), index) == dictError_.end())
//		return string();
//	return dictError_[index];
//}

#include "Settings.h"
#include <vector>
#include <windows.h>

using namespace std;

tstring getApplicationPath()
{
	vector<TCHAR> buffer(MAX_PATH);
	GetModuleFileName(NULL, &buffer[0], MAX_PATH);
	tstring path(&buffer[0]);
	return path.substr(0, path.rfind('\\') + 1);
}

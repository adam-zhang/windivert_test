#include "stdafx.h"
#include "Code.h"
#include <vector>
#include <Windows.h>

using namespace std;
//Code::Code(void)
//{
//}


//Code::~Code(void)
//{
//}

std::string UTF16ToUTF8(const std::wstring& source)
{
	int size = WideCharToMultiByte(CP_UTF8, 0, source.c_str(), -1, NULL, 0, NULL, NULL);
	vector<char> result(size + 1);
	size = WideCharToMultiByte(CP_UTF8, 0, source.c_str(), -1, &result[0], size, 0, 0);
	return &result[0];
}

std::wstring UTF8ToUTF16(const std::string& source)
{
	int size = MultiByteToWideChar(CP_UTF8, 0, source.c_str(), -1, NULL, 0);
	vector<WCHAR> buffer(size + 1);
	size = MultiByteToWideChar(CP_UTF8, 0, source.c_str(), -1, &buffer[0], size);
	return &buffer[0];
}

std::wstring AnsiToUTF16(const std::string& source)
{
	int size = MultiByteToWideChar(CP_ACP, 0, source.c_str(), -1, NULL, 0);
	vector<WCHAR> buffer(size + 1);
	size = MultiByteToWideChar(CP_ACP, 0, source.c_str(), -1, &buffer[0], size);
	return &buffer[0];
}
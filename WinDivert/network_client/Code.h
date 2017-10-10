#pragma once

#include <string>
//class Code
//{
//public:
//	Code(void);
//	~Code(void);
//};

std::string UTF16ToUTF8(const std::wstring& source);
std::wstring UTF8ToUTF16(const std::string& source);
std::wstring AnsiToUTF16(const std::string& source);

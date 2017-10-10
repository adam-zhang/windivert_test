#include "stdafx.h"
#include "error.h"
#include "Code.h"
#include <map>
#include <iomanip>

using namespace std;

static map<DWORD, tstring> getErrorMap()
{
	static map<DWORD, tstring> errorMap;
	if (errorMap.size() == 0)
	{
		errorMap.insert(make_pair(INVALID_IP_HEADER, TEXT("Invalid IP header")));
		errorMap.insert(make_pair(INVALID_IPV6_HEADER, TEXT("Invalid IPv6 header")));
		errorMap.insert(make_pair(INVALID_ICMP_HEADER, TEXT("Invalid ICMP header")));
		errorMap.insert(make_pair(INVALID_ICMPV6_HEADER, TEXT("Invalid ICMPv6 header")));
		errorMap.insert(make_pair(INVALID_TCP_HEADER, TEXT("Invalid TCP header")));
		errorMap.insert(make_pair(INVALID_UDP_HEADER, TEXT("Invalid UDP header")));
		errorMap.insert(make_pair(INVALID_BUFFER, TEXT("Invalid buffer")));
		errorMap.insert(make_pair(INVALID_DATA, TEXT("Invalid data")));
	}
	return errorMap;
}

tstring getErrorInfo(DWORD errorNumber)
{
	map<DWORD, tstring> errorMap = getErrorMap();
	if (errorMap.find(errorNumber) != errorMap.end())
		return errorMap[errorNumber];
	return tstring();
}



void outputDebugString(const tstring& text, DWORD errorNumber)
{
	tstringstream ss;
	ss << text << ":(" << errorNumber << ")" << getErrorInfo(errorNumber) << endl;
	OutputDebugString(ss.str().c_str());
}

tstring format(const vector<unsigned char>& data)
{
	tstringstream ss;
	for(auto c : data)
		ss << hex << setw(2) << setfill(TEXT('0')) << unsigned char(c)  << " ";
	return ss.str();
}

void outputData(const vector<unsigned char>& data)
{
	tstringstream ss;
	ss << format(data) << endl;
	outputData(ss.str());
}

void outputData(const unsigned char* data, size_t length)
{
	vector<unsigned char> buffer(length);
	copy(data, data + length, buffer.begin());
	outputData(buffer);
}

void outputData(const tstring& text)
{
	OutputDebugString((text + TEXT("\n")).c_str());
}

void outputData(const string& text)
{
	wstring result = AnsiToUTF16(text);
	outputData(result);
}

void outputData(const tstring& text, const vector<unsigned char>& data)
{
	tstringstream ss;
	ss << text << TEXT(":") << format(data);
	outputData(ss.str());
}

void outputInfo(const tstring& text, const vector<unsigned char>& data)
{
	tstringstream ss;
	ss << text << ":";
	for(auto c : data)
		ss << (char)c;
	outputData(ss.str());
}

void outputInfo(const tstring& text, size_t number)
{
	tstringstream ss;
	ss << text << TEXT(":") << number;
	outputData(ss.str());
}
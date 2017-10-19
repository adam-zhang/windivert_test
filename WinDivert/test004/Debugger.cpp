#include "Debugger.h"
#include <iomanip>

using namespace std;

Debugger::Debugger(void)
{
}


Debugger::~Debugger(void)
{
}

void Debugger::outputData(const vector<unsigned char>& data)
{
	tstringstream ss;
	for(auto c : data)
		ss << hex << setw(2) << setfill(TEXT('0')) << c << " ";
	ss << endl;
	output(ss.str());
}

#ifndef _DEBUG
void writeToFile(const tstring& text)
{
	
}
#endif

void Debugger::output(const tstring& text)
{
#ifdef _DEBUG
	OutputDebugString(text.c_str());
#else
	writeToFile(text);
#endif
}

void Debugger::output(const tstring& text, UINT error)
{
	tstringstream ss;
	ss << text << ":" << error << endl;
	output(ss.str());
}

void Debugger::outputText(const tstring& text)
{
	output(text);
}

void Debugger::outputPlain(const vector<unsigned char>& data)
{
	tstringstream ss;
	for(auto c : data)
		ss << (char)c;
	ss << endl;
	output(ss.str());
}

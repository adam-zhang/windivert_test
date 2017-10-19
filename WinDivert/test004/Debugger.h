#pragma once
#include "Singleton.h"
#include "stdafx.h"
#include <string>
#include <vector>
#include <Windows.h>

class Debugger : public Singleton<Debugger>
{
public:
	Debugger(void);
	~Debugger(void);
public:
	void output(const std::string& text, UINT error);
	void output(const tstring& text, UINT error);
	void output(const tstring& text);
	void outputData(const std::vector<unsigned char>& data);
	void outputPlain(const std::vector<unsigned char>& data);
	void outputText(const tstring& data);
};


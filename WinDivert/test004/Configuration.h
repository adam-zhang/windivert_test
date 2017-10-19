#pragma once
#include "Singleton.h"

class Configuration : public Singleton<Configuration>
{
public:
	Configuration(void);
	~Configuration(void);
public:
	int ip();
	short port();
};


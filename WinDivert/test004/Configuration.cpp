#include <WinSock2.h>
#include "Configuration.h"
#include "Configuration.h"
#include <string>

using namespace std;

Configuration::Configuration(void)
{
}


Configuration::~Configuration(void)
{
}

int Configuration::ip()
{
	string address = "172.16.16.200";
	return inet_addr(address.c_str());
}

short Configuration::port()
{
	return 23456;
}

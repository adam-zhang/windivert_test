#include "stdafx.h"
#include <WinSock2.h>
#include "Proxy.h"
#include "error.h"
#include <cassert>
#include <vector>
#include <winsock2.h>

using namespace std;

Proxy::Proxy(void)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
}


Proxy::~Proxy(void)
{
	WSACleanup();
}

Proxy* Proxy::instance_ = 0;

vector<unsigned char> Proxy::send(const unsigned char* data, size_t length)
{
	vector<unsigned char> ret;
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
	{
		outputDebugString(TEXT("create socket failed."), 0);
		return ret;
	}
	struct sockaddr_in address = {0};
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("172.16.16.200");
	address.sin_port = htons(23456);
	int result = connect(s, (SOCKADDR*)&address, sizeof(address)); 
	if (result == -1)
	{
		outputDebugString(TEXT("connect to server failed."), 0);
		return ret;
	}
	outputData((char*)data);
	result = ::send(s, (char*)data, length, 0);
	if (result == -1)
	{
		outputDebugString(TEXT("send data failed"), 0);
		return ret;
	}
	result = shutdown(s, SD_SEND);
	vector<unsigned char> buffer(256);
	//string ret;
	do
	{
		result = recv(s, (char*)&buffer[0], 256, 0);
		copy(&buffer[0], &buffer[0] + result, back_inserter(ret));
	}while(result > 0);
	outputData(ret);
	closesocket(s);
	return ret;
}

vector<unsigned char> Proxy::send(const vector<unsigned char>& text)
{
	return send(&text[0], text.size());
}
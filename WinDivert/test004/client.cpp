#include <winsock2.h>
#include <Windows.h>
#include <cassert>
#include "client.h"
#include "Configuration.h"
#include "Debugger.h"

using namespace std;

Client::Client() 
{
	WSADATA data;
	int result = WSAStartup(MAKEWORD(2, 2), &data);
	assert(result == 0 );
}

Client::~Client() 
{
	WSACleanup();
}

const vector<unsigned char> Client::send(const vector<unsigned char>& data)
{
	vector<unsigned char> receivedData;
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
	{
		Debugger::instance().output(TEXT("create socket failed."), WSAGetLastError());
		return receivedData;
	}
	SOCKADDR_IN address = {0};
	address.sin_addr.S_un.S_addr = Configuration::instance().ip();
	address.sin_family = AF_INET;
	address.sin_port = htons(Configuration::instance().port());
	int result = connect(s, (SOCKADDR*)&address, sizeof(SOCKADDR_IN));
	if (result == SOCKET_ERROR)
	{
		Debugger::instance().output(TEXT("connect to server failed."), WSAGetLastError());
		return receivedData;
	}
	result = ::send(s, (char*)&data[0], data.size(), 0);
	if (result == SOCKET_ERROR)
	{
		Debugger::instance().output(TEXT("send data to server failed."), WSAGetLastError());
		return receivedData;
	}
	do
	{
		vector<char> buffer(256);
		result = recv(s, &buffer[0], 256, 0);
		copy(buffer.begin(), buffer.begin() + result, back_inserter(receivedData));
	}while(result > 0);
	closesocket(s);
	return receivedData;
}

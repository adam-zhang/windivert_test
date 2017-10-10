#include "stdafx.h"
#include "WinDivertor.h"
#include "error.h"
#include "Settings.h"
#include <sstream>
#include <cassert>



using namespace std;


tstring getModuleName()
{
	return getApplicationPath() + TEXT("WinDivert.dll");
}

#define LOADFUNCTION(name) \
	do { \
		name = (Function_##name)GetProcAddress(module, #name);\
		if (!name) \
		return false; } \
	while(0);

Function_WinDivertOpen WinDivertor::WinDivertOpen = 0;
Function_WinDivertClose WinDivertor::WinDivertClose = 0;
Function_WinDivertRecv WinDivertor::WinDivertRecv = 0;
Function_WinDivertRecvEx WinDivertor::WinDivertRecvEx = 0;
Function_WinDivertSend WinDivertor::WinDivertSend = 0;
Function_WinDivertSendEx WinDivertor::WinDivertSendEx = 0;
Function_WinDivertSetParam WinDivertor::WinDivertSetParam = 0;
Function_WinDivertGetParam WinDivertor::WinDivertGetParam = 0;
Function_WinDivertHelperParsePacket WinDivertor::WinDivertHelperParsePacket = 0; 
Function_WinDivertHelperParseIPv4Address WinDivertor::WinDivertHelperParseIPv4Address = 0;
Function_WinDivertHelperParseIPv6Address WinDivertor::WinDivertHelperParseIPv6Address = 0;
Function_WinDivertHelperCalcChecksums WinDivertor::WinDivertHelperCalcChecksums = 0;
Function_WinDivertHelperEvalFilter WinDivertor::WinDivertHelperEvalFilter = 0;

bool WinDivertor::loadFunctions(HMODULE module)
{
	LOADFUNCTION(WinDivertOpen)
	LOADFUNCTION(WinDivertClose)
	LOADFUNCTION(WinDivertRecv)
	LOADFUNCTION(WinDivertRecvEx)
	LOADFUNCTION(WinDivertSend)
	LOADFUNCTION(WinDivertSendEx)
	LOADFUNCTION(WinDivertSetParam)
	LOADFUNCTION(WinDivertGetParam)
	LOADFUNCTION(WinDivertHelperParsePacket);
	LOADFUNCTION(WinDivertHelperEvalFilter);
	LOADFUNCTION(WinDivertHelperParseIPv4Address);
	LOADFUNCTION(WinDivertHelperParseIPv6Address);
	LOADFUNCTION(WinDivertHelperCalcChecksums);
	return true;
}


HMODULE WinDivertor::loadFunctions()
{
	tstring dllName = getModuleName();	
	HMODULE module = LoadLibrary(dllName.c_str());
	if (!module)
		return false;
	if(!loadFunctions(module))
		return NULL;
	return module;
}


void WinDivertor::initialize()
{
	module_ = loadFunctions();
	if (!module_)
		throw exception("Load functions failed.");
}

WinDivertor::~WinDivertor()
{
	//close();
	//if (module_)
	//{
	//	freeFunctions(module_);
	//	module_ = NULL;
	//}
}



//string generateDirection(int direction)
//{
//	if (direction == (WinDivertor::INBOUND|WinDivertor::OUTBOUND))
//		return "outbound && inbound";
//	if (direction == WinDivertor::INBOUND)
//		return "inbound";
//	if (direction == WinDivertor::OUTBOUND)
//		return "outbound";	
//	return string();
//}

//string generateIP(const string& ip)
//{
//	stringstream ss;
//	ss << "ip && " << "tcp.DstAddr=" << ip;
//      	return ss.str();	
//}

//string generatePort(int port)
//{
//	stringstream ss;
//	ss << "tcp.DstPort=" << port;
//	return ss.str();
//}

//string generateFilter(int direction, const string& ip, int port)
//{
//	stringstream ss;
//	ss << generateDirection(direction)
//		<< " && "
//		<< generateIP(ip)
//		<< " && "
//		<< generatePort(port);
//	return ss.str();
//}

//bool WinDivertor::open(int direction, const string& ip, int port)
//{
//	assert(WinDivertOpen);
//	//string filter = generateFilter(direction, ip, port);
//	string filter = "outbound && ip && tcp.DstPort=80 && tcp.PayloadLength >0";
//	HANDLE handle = WinDivertOpen(filter.c_str(), WINDIVERT_LAYER_NETWORK, 0, 0);
//	if (handle == INVALID_HANDLE_VALUE)
//		return false;
//	handle_ = handle;
//	return true;
//}

//std::vector<unsigned char> WinDivertor::recv()
//{
//	assert(WinDivertRecv);
//	vector<unsigned char> buffer(65535);
//	UINT received = 0;
//	WINDIVERT_ADDRESS address = {0};
//	BOOL ret = WinDivertRecv(handle_, &buffer[0], 65535, &address, &received);
//	vector<unsigned char> data(received);
//	copy(&buffer[0], &buffer[0] + received, data.begin());
//	return data;
//}

//void WinDivertor::close()
//{
//	if (handle_)
//	{
//		WinDivertClose(handle_);
//		handle_ = INVALID_HANDLE_VALUE;
//	}
//}

////WinDivertor::WinDivertor(const string& value)
////	: handle_(INVALID_HANDLE_VALUE)
////	, module_(NULL)
////{
////	initialize();
////	//handle_ = createDivert(value);
////	//if (handle_ == INVALID_HANDLE_VALUE)
////	//	throw exception(Error::instance().getErrorDescription(GetLastError()));
////}



WinDivertor::WinDivertor()
	:/* WinDivertOpen(0)
	  , WinDivertClose(0)
	  , WinDivertRecv(0)
	  , WinDivertRecvEx(0)
	  , WinDivertSend(0)
	  , WinDivertSendEx(0)*/
	  handle_(INVALID_HANDLE_VALUE)
	  , module_(NULL)
{
	initialize();
	//handle_ = createDivert();
	//if (handle_ == INVALID_HANDLE_VALUE)
	//	throw exception(Error::instance().getErrorDescription(GetLastError()));
//	freeFunctions(module_);
}


//void WinDivertor::freeFunctions(HMODULE module)
//{
//	FreeLibrary(module);
//}

//WinDivertor::~WinDivertor()
//{
//	if (handle_ != INVALID_HANDLE_VALUE)
//		WinDivertClose(handle);
//	handle_ = NULL;
//}

#ifndef __ERROR__H
#define __ERROR__H

#include <string>
#include <vector>
#include <Windows.h>
#include "stdafx.h"

#define BASE_ERROR 10000
#define INVALID_IP_HEADER		BASE_ERROR + 1
#define INVALID_IPV6_HEADER 	BASE_ERROR + 2
#define INVALID_ICMP_HEADER 	BASE_ERROR + 3
#define INVALID_ICMPV6_HEADER	BASE_ERROR + 4
#define INVALID_TCP_HEADER		BASE_ERROR + 5
#define INVALID_UDP_HEADER		BASE_ERROR + 6
#define INVALID_DATA			BASE_ERROR + 7
#define INVALID_BUFFER			BASE_ERROR + 8
//#define INVALID_TCPV6_HEADER	BASE_ERROR + 6;

tstring getErrorInfo(DWORD errorNumber);

void outputDebugString(const tstring& text, DWORD errorNumber);
void outputData(const std::vector<unsigned char>& data);
void outputData(const std::string& text, std::vector<unsigned char>& data);
void outputData(const unsigned char* data, size_t length);
void outputData(const tstring& text);
#endif//__ERROR__H
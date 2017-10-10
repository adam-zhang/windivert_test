#ifndef __WINDIVERTOR__H
#define __WINDIVERTOR__H

#pragma once

#include <vector>
#include <string>
#include <Windows.h>
#include "Singleton.h"

typedef struct
{
    UINT32 IfIdx;                       /* Packet's interface index. */
    UINT32 SubIfIdx;                    /* Packet's sub-interface index. */
    UINT8  Direction;                   /* Packet's direction. */
}WINDIVERT_ADDRESS, *PWINDIVERT_ADDRESS;


typedef enum
{
    WINDIVERT_PARAM_QUEUE_LEN  = 0,     /* Packet queue length. */
    WINDIVERT_PARAM_QUEUE_TIME = 1      /* Packet queue time. */
} WINDIVERT_PARAM, *PWINDIVERT_PARAM;

typedef enum
{
    WINDIVERT_LAYER_NETWORK = 0,        /* Network layer. */
    WINDIVERT_LAYER_NETWORK_FORWARD = 1 /* Network layer (forwarded packets) */
} WINDIVERT_LAYER, *PWINDIVERT_LAYER;
typedef struct
{
    UINT8  HdrLength:4;
    UINT8  Version:4;
    UINT8  TOS;
    UINT16 Length;
    UINT16 Id;
    UINT16 FragOff0;
    UINT8  TTL;
    UINT8  Protocol;
    UINT16 Checksum;
    UINT32 SrcAddr;
    UINT32 DstAddr;
}WINDIVERT_IPHDR, *PWINDIVERT_IPHDR;

typedef struct
{
    UINT8  TrafficClass0:4;
    UINT8  Version:4;
    UINT8  FlowLabel0:4;
    UINT8  TrafficClass1:4;
    UINT16 FlowLabel1;
    UINT16 Length;
    UINT8  NextHdr;
    UINT8  HopLimit;
    UINT32 SrcAddr[4];
    UINT32 DstAddr[4];
} WINDIVERT_IPV6HDR, *PWINDIVERT_IPV6HDR;

typedef struct
{
    UINT8  Type;
    UINT8  Code;
    UINT16 Checksum;
    UINT32 Body;
} WINDIVERT_ICMPHDR, *PWINDIVERT_ICMPHDR;

typedef struct
{
    UINT8  Type;
    UINT8  Code;
    UINT16 Checksum;
    UINT32 Body;
} WINDIVERT_ICMPV6HDR, *PWINDIVERT_ICMPV6HDR;

typedef struct
{
    UINT16 SrcPort;
    UINT16 DstPort;
    UINT32 SeqNum;
    UINT32 AckNum;
    UINT16 Reserved1:4;
    UINT16 HdrLength:4;
    UINT16 Fin:1;
    UINT16 Syn:1;
    UINT16 Rst:1;
    UINT16 Psh:1;
    UINT16 Ack:1;
    UINT16 Urg:1;
    UINT16 Reserved2:2;
    UINT16 Window;
    UINT16 Checksum;
    UINT16 UrgPtr;
} WINDIVERT_TCPHDR, *PWINDIVERT_TCPHDR;

typedef struct
{
    UINT16 SrcPort;
    UINT16 DstPort;
    UINT16 Length;
    UINT16 Checksum;
} WINDIVERT_UDPHDR, *PWINDIVERT_UDPHDR;

typedef struct
{
    WINDIVERT_IPHDR  ip;
    WINDIVERT_TCPHDR tcp;
} PACKET, *PPACKET;
typedef struct 
{
    PACKET header;
    UINT8 data[];
} DATAPACKET, *PDATAPACKET;




typedef HANDLE (WINAPI *Function_WinDivertOpen)( __in const char* filter, __in WINDIVERT_LAYER, __in INT16, __in UINT64);
typedef BOOL (WINAPI* Function_WinDivertClose)(HANDLE);
typedef BOOL (WINAPI* Function_WinDivertRecv)(HANDLE, PVOID, UINT, PWINDIVERT_ADDRESS, UINT*);
typedef BOOL (WINAPI* Function_WinDivertRecvEx)(HANDLE, PVOID, UINT, UINT64, PWINDIVERT_ADDRESS, UINT, LPOVERLAPPED);
typedef BOOL (WINAPI* Function_WinDivertSend)(HANDLE, PVOID, UINT, PWINDIVERT_ADDRESS, UINT);
typedef BOOL (WINAPI* Function_WinDivertSendEx)(HANDLE, PVOID, UINT, UINT64, PWINDIVERT_ADDRESS, UINT*, LPOVERLAPPED);
typedef BOOL (WINAPI* Function_WinDivertSetParam)(HANDLE, WINDIVERT_PARAM, UINT64);
typedef BOOL (WINAPI* Function_WinDivertGetParam)(HANDLE, WINDIVERT_PARAM, UINT64*);
typedef BOOL (WINAPI* Function_WinDivertHelperParsePacket)( PVOID packet, UINT, PWINDIVERT_IPHDR*, PWINDIVERT_IPV6HDR*, PWINDIVERT_ICMPHDR*, PWINDIVERT_ICMPV6HDR*, PWINDIVERT_TCPHDR*, PWINDIVERT_UDPHDR*, PVOID*, UINT*);
typedef BOOL (WINAPI* Function_WinDivertHelperParseIPv4Address)(const char*, UINT32);
typedef BOOL (WINAPI* Function_WinDivertHelperParseIPv6Address)(const char*, UINT32);
typedef BOOL (WINAPI* Function_WinDivertHelperCalcChecksums)(PVOID packet, UINT, UINT64);
typedef BOOL (WINAPI* Function_WinDivertHelperCheckFilter)(const char*, WINDIVERT_LAYER, const char**, UINT*);
typedef BOOL (WINAPI* Function_WinDivertHelperEvalFilter)(const char*, WINDIVERT_LAYER, PVOID, UINT, PWINDIVERT_ADDRESS);

#define MAX_BUFFER 0xFFFF



class WinDivertor// : public Singleton
{
	
	public:
		static Function_WinDivertOpen WinDivertOpen;
		static Function_WinDivertClose WinDivertClose;
		static Function_WinDivertRecv WinDivertRecv;
		static Function_WinDivertRecvEx WinDivertRecvEx;
		static Function_WinDivertSend WinDivertSend;
		static Function_WinDivertSendEx WinDivertSendEx;
		static Function_WinDivertSetParam WinDivertSetParam;
		static Function_WinDivertGetParam WinDivertGetParam;
		static Function_WinDivertHelperParsePacket WinDivertHelperParsePacket;
		static Function_WinDivertHelperParseIPv4Address WinDivertHelperParseIPv4Address;
		static Function_WinDivertHelperParseIPv6Address WinDivertHelperParseIPv6Address;
		static Function_WinDivertHelperCalcChecksums WinDivertHelperCalcChecksums;
		static Function_WinDivertHelperEvalFilter WinDivertHelperEvalFilter;

public:
	enum
	{
		INBOUND = 1,
		OUTBOUND
	}Direction;
public:
	//WinDivertor(const std::string&);
	WinDivertor();
	~WinDivertor();
private:
	HANDLE handle_;
	HMODULE module_;
public:
	const HANDLE& handle()const
	{ return handle_; }
	void setHandle(const HANDLE& handle)
	{ handle_ = handle; }
public:
	bool open(int direction, const std::string& ip, int port);
	//bool open(const string& website);
	void close();
	std::vector<unsigned char> recv();
	std::vector<unsigned char> recvex();
	bool send(const std::vector<unsigned char>&);
	bool sendex(const std::vector<unsigned char>&);
	//bool setParam(const WINDIVERT_PARAM&);
	//WINDIVERT_PARAM param();
private:
	HMODULE loadFunctions();
	bool loadFunctions(HMODULE);
	void freeFunctions(HMODULE);
	void initialize();
public:
};

#endif//__WINDIVERTOR__H

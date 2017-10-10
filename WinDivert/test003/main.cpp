#include <iostream>
#include <sstream>
#include <vector>
#include <Windows.h>
#include <tchar.h>
#include "IPHeader.h"
#include "Helper.h"

using namespace std;

typedef basic_stringstream<TCHAR> tstringstream;

const int MAX_BUFFER = 0xffff;

typedef struct
{
	WINDIVERT_IPHDR ip;
	WINDIVERT_TCPHDR tcp;
}PACKET, *PPACKET;

typedef struct 
{
    PACKET header;
    UINT8 data[];
} DATAPACKET, *PDATAPACKET;

//typedef struct 
//{
//	PACKET header;
//	unsigned char[];
//}DATAPACKET, *PDATAPACKET;

void initializePacket(PACKET& packet)
{
	ZeroMemory(&packet, sizeof(PACKET));
	packet.ip.Version = 4;
	packet.ip.HdrLength = sizeof(WINDIVERT_IPHDR) / sizeof(UINT32);
	packet.ip.Length = htons(sizeof(PACKET));
	packet.ip.TTL = 64;
	packet.ip.Protocol = IPPROTO_TCP;
	packet.tcp.HdrLength = sizeof(WINDIVERT_TCPHDR) / sizeof(UINT32);
}

bool sendData(HANDLE handle, PVOID packet, UINT packetLength, WINDIVERT_ADDRESS& address)
{
	WinDivertHelperCalcChecksums(packet, packetLength, 0);
	if (WinDivertSend(handle, packet, packetLength, &address, 0))
		return true;
	return false;
}

void closeRemoteServer(HANDLE handle, 
					   WINDIVERT_IPHDR* ipHeader,
					   WINDIVERT_TCPHDR* tcpHeader,
					   WINDIVERT_ADDRESS& address)
{
	PACKET packet; // = {0};	
	initializePacket(packet);
	packet.tcp.Rst = 1;
	packet.tcp.Ack = 1;
	packet.ip.SrcAddr       = ipHeader->SrcAddr;
	packet.ip.DstAddr       = ipHeader->DstAddr;
	packet.tcp.SrcPort      = tcpHeader->SrcPort;
	packet.tcp.DstPort      = htons(80);
	packet.tcp.SeqNum       = tcpHeader->SeqNum;
	packet.tcp.AckNum       = tcpHeader->AckNum;
	sendData(handle, &packet, sizeof(PACKET), address);
}

unsigned long getServerIP()
{
	string serverIP("172.19.19.200");
	return inet_addr(serverIP.c_str());
}

void sendToProxy(HANDLE handle, 
				 WINDIVERT_IPHDR* ipHeader,
				 WINDIVERT_TCPHDR* tcpHeader,
				 WINDIVERT_ADDRESS& address,
				 unsigned char* data,
				 UINT dataLength)
{
	outputData(TEXT("sending proxy"));
	UINT packetLength = sizeof(DATAPACKET) + dataLength;
	vector<unsigned char> buffer(packetLength);
	PDATAPACKET packet = reinterpret_cast<PDATAPACKET>(&buffer[0]);
	initializePacket(packet->header);
	copy(data, data + dataLength, packet->data);
	packet->header.ip = *ipHeader;
	packet->header.tcp = *tcpHeader;
	packet->header.ip.DstAddr = htons(inet_addr("172.19.19.200"));
	packet->header.tcp.DstPort = htons(23456);
	address.Direction = !address.Direction;
	sendData(handle, packet, packetLength, address);
}

void finish(HANDLE handle, WINDIVERT_IPHDR* ip_header, WINDIVERT_TCPHDR* tcp_header, WINDIVERT_ADDRESS& address, UINT dataLength)
{
	PACKET finish = {0};
	initializePacket(finish);
	finish.tcp.Fin = 1;
	finish.tcp.Ack = 1;
	finish.ip.SrcAddr       = ip_header->DstAddr;
	finish.ip.DstAddr       = ip_header->SrcAddr;
	finish.tcp.SrcPort      = htons(80);
	finish.tcp.DstPort      = tcp_header->SrcPort;
	finish.tcp.SeqNum       = htonl(ntohl(tcp_header->AckNum)); // + sizeof(block_data) - 1); 
	finish.tcp.AckNum       = htonl(ntohl(tcp_header->SeqNum)); // + payload_len);
	sendData(handle, &finish, sizeof(finish), address);
}

void sendDataToProxy(HANDLE handle, WINDIVERT_IPHDR* ipHeader, WINDIVERT_TCPHDR* tcpHeader, WINDIVERT_ADDRESS& address, unsigned char* data, UINT dataLength)
{
	closeRemoteServer(handle, ipHeader, tcpHeader, address);
	sendToProxy(handle, ipHeader, tcpHeader, address, data, dataLength);
	finish(handle, ipHeader, tcpHeader, address, dataLength);
}

void output(WINDIVERT_IPHDR& ip)
{
	tstringstream ss;
	ss << "IP Header\n"
		<< "HdrLength:" << ip.HdrLength
		<< "\nVersion:" << ip.Version
		<< "\nTOS:" << ip.TOS
		<< "\nLength:" << ip.Length
		<< "\nId:" << ip.Id
		<< "\nFrag0ff0:" << ip.FragOff0
		<< "\nTTL:" << ip.TTL
		<< "\nProtocol:" << ip.Protocol
		<< "\nChecksum:" << ip.Checksum
		<< "\nSrcAddr:" << ip.SrcAddr
		<< "\nDstAddr:" << ip.DstAddr << endl;
	OutputDebugString(ss.str().c_str());
}

void output(WINDIVERT_TCPHDR& tcp)
{
	tstringstream ss;
	ss << "TCP Header"
		<< "\nSrcPort:" << tcp.SrcPort
		<< "\nDstPort:" << tcp.DstPort
		<< "\nSeqNum:" << tcp.SeqNum
		<< "\nAckNum:" << tcp.AckNum
		<< "\nReserved1:" << tcp.Reserved1
		<< "\nHdrLength:" << tcp.HdrLength
		<< "\nFin:" << tcp.Fin
		<< "\nSyn:" << tcp.Syn
		<< "\nRst:" << tcp.Rst
		<< "\nPsh:" << tcp.Psh
		<< "\nAck:" << tcp.Ack
		<< "\nUrg:" << tcp.Urg
		<< "\nReserved2" << tcp.Reserved2
		<< "\nWindow:" << tcp.Window
		<< "\nChecksum:" << tcp.Checksum
		<< "\nUrgPtr:" << tcp.UrgPtr << endl;
	OutputDebugString(ss.str().c_str());
}

void catchData(HANDLE handle)
{
	for(;;)
	{
		//vector<unsigned char> buffer(MAX_BUFFER);
		//PVOID packet = &buffer[0];
		UINT8 packet[MAX_BUFFER];
		ZeroMemory(packet, MAX_BUFFER);
		WINDIVERT_ADDRESS address;
		UINT packetLength;
		if (!WinDivertRecv(handle, &packet, MAX_BUFFER, &address, &packetLength))
		{
			outputDebugString(TEXT("receive data"), GetLastError());
			return ;
		}

		WINDIVERT_IPHDR* ipHeader;
		WINDIVERT_TCPHDR* tcpHeader;
		unsigned char* data;
		UINT dataLength;

		if (!ParsePacket(&packet, packetLength, &ipHeader, NULL, NULL, NULL, &tcpHeader, NULL, (PVOID*)&data, &dataLength))
		{
			outputDebugString(TEXT("error"), GetLastError());
			continue;
		}
		output(*ipHeader);
		output(*tcpHeader);
		sendDataToProxy(handle, ipHeader,  tcpHeader, address, data, dataLength);
	}
}

int main(int, char** )
{
	string filter = "outbound && ip && tcp.DstPort=80 && tcp.PayloadLength > 0";
	HANDLE handle = WinDivertOpen(filter.c_str(),
		WINDIVERT_LAYER_NETWORK, 404, 0);
	if (handle == INVALID_HANDLE_VALUE)
	{
		cout << "open windiver failed (" << GetLastError() << ")." << endl;
		return -1;
	}
	catchData(handle);

	WinDivertClose(handle);

	return 0;
}
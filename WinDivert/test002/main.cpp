#include <Windows.h>
#include <sstream>
#include <string>
#include <vector>
#include "WinDivert.h"

typedef std::basic_string<TCHAR> tstring;
typedef std::basic_stringstream<TCHAR> tstringstream;

using namespace std;

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



void outputDebugString(const tstring& text,	size_t error)
{
	tstringstream ss;
	ss << text << error << std::endl;
	OutputDebugString(ss.str().c_str());
}

//static void initializePacket(PPACKET packet)
//{
//	memset(packet, 0, sizeof(PACKET));
//	packet->ip.Version = 4;
//	packet->ip.HdrLength = sizeof(WINDIVERT_IPHDR) / sizeof(UINT32);
//	packet->ip.Length = htons(sizeof(PACKET));
//	packet->ip.TTL = 64;
//	packet->ip.Protocol = IPPROTO_TCP;
//	packet->tcp.HdrLength = sizeof(WINDIVERT_TCPHDR) / sizeof(UINT32);
//}

void initializePacket(PPACKET packet)
{
	packet->ip.Version = 4;
	packet->ip.HdrLength = sizeof(WINDIVERT_IPHDR) / sizeof(UINT32);
	packet->ip.Length = htons(sizeof(PACKET));
	packet->ip.TTL = 64;
	packet->ip.Protocol = IPPROTO_TCP;
	packet->tcp.HdrLength = sizeof(WINDIVERT_TCPHDR) / sizeof(UINT32);
}

const char block_data[] =
    "HTTP/1.1 200 OK\r\n"
    "Connection: close\r\n"
    "Content-Type: text/html\r\n"
    "\r\n"
    "<!doctype html>\n"
    "<html>\n"
    "\t<head>\n"
    "\t\t<title>BLOCKED!</title>\n"
    "\t</head>\n"
    "\t<body>\n"
    "\t\t<h1>BLOCKED!</h1>\n"
    "\t\t<hr>\n"
    "\t\t<p>This URL has been blocked!</p>\n"
    "\t</body>\n"
    "</html>\n";

void closeRemoteServer(HANDLE handle, WINDIVERT_IPHDR* ip_header, WINDIVERT_TCPHDR* tcp_header, WINDIVERT_ADDRESS* addr)
{
	PACKET reset = {0};
	//PacketInit(&reset);
	initializePacket(&reset);
	reset.tcp.Rst = 1;
	reset.tcp.Ack = 1;
	reset.ip.SrcAddr       = ip_header->SrcAddr;
	reset.ip.DstAddr       = ip_header->DstAddr;
	reset.tcp.SrcPort      = tcp_header->SrcPort;
	reset.tcp.DstPort      = htons(80);
	reset.tcp.SeqNum       = tcp_header->SeqNum;
	reset.tcp.AckNum       = tcp_header->AckNum;
	WinDivertHelperCalcChecksums((PVOID)&reset, sizeof(PACKET), 0);
	if (!WinDivertSend(handle, (PVOID)&reset, sizeof(PACKET), addr, NULL))
	{
		fprintf(stderr, "warning: failed to send reset packet (%d)\n",
			GetLastError());
	}
}


//void closeRemoteServer(HANDLE handle, 
//					   WINDIVERT_IPHDR* ipHeader,
//					   WINDIVERT_TCPHDR* tcpHeader,
//					   WINDIVERT_ADDRESS& address)
//{
//	PACKET reset = {0};
//	initializePacket(&reset);
//	reset.tcp.Rst = 1;
//	reset.tcp.Ack = 1;
//	reset.ip.SrcAddr = ipHeader->SrcAddr;
//	reset.ip.DstAddr = ipHeader->DstAddr;
//	reset.tcp.SrcPort = tcpHeader->SrcPort;
//	reset.tcp.DstPort = htons(80);
//	reset.tcp.SeqNum = tcpHeader->SeqNum;
//	reset.tcp.AckNum = tcpHeader->AckNum;
//	WinDivertHelperCalcChecksums(&reset, sizeof(PACKET), 0);
//	if (!WinDivertSend(handle, &reset, sizeof(PACKET), &address, 0))
//	{
//		outputDebugString(TEXT("WinDivertSend failed in closeRemoteServer:"), GetLastError());
//		return;
//	}
//}

//void blockPage(HANDLE handle, WINDIVERT_IPHDR* ipHeader, WINDIVERT_TCPHDR* tcpHeader, WINDIVERT_ADDRESS& address, UINT dataLength)
//{
//	UINT pageLength = sizeof(block_data) + sizeof(DATAPACKET) - 1;
//	outputDebugString(TEXT("blockpage length:"), pageLength);
//	DATAPACKET* block = (DATAPACKET*)malloc(pageLength);
//	initializePacket(&block->header);
//	block->header.ip.Length = htons(pageLength);
//	block->header.tcp.SrcPort = htons(80);
//	block->header.tcp.Psh = 1;
//	block->header.tcp.Ack = 1;
//	memcpy(block->data, block_data, sizeof(block_data) - 1);
//	block->header.ip.SrcAddr = ipHeader->DstAddr;
//	block->header.ip.DstAddr = ipHeader->SrcAddr;
//	block->header.tcp.DstPort = tcpHeader->SrcPort;
//	block->header.tcp.SeqNum = tcpHeader->AckNum;
//	block->header.tcp.AckNum = htonl(ntohl(tcpHeader->SeqNum) + dataLength);

//	WinDivertHelperCalcChecksums(block, pageLength, 0);
//	address.Direction = !address.Direction;
//	if (!WinDivertSend(handle, block, pageLength, &address, 0))
//	{
//		outputDebugString(TEXT("WinDivertSend failed in blockPage:"), GetLastError());
//		return;
//	}
//	free(block);
//}
void finishBlock(HANDLE handle,
				 WINDIVERT_IPHDR* ip_header,
				 WINDIVERT_TCPHDR* tcp_header,
				 WINDIVERT_ADDRESS* addr,
				 UINT payload_len)
{
	PACKET finish = {0};
	//PacketInit(&finish);
	initializePacket(&finish);
	finish.tcp.Fin = 1;
	finish.tcp.Ack = 1;
	finish.ip.SrcAddr       = ip_header->DstAddr;
	finish.ip.DstAddr       = ip_header->SrcAddr;
	finish.tcp.SrcPort      = htons(80);
	finish.tcp.DstPort      = tcp_header->SrcPort;
	finish.tcp.SeqNum       = htonl(ntohl(tcp_header->AckNum) + sizeof(block_data) - 1); 
	finish.tcp.AckNum       = htonl(ntohl(tcp_header->SeqNum) + payload_len);
	//outputPacket(finish);
	WinDivertHelperCalcChecksums((PVOID)&finish, sizeof(PACKET), 0);
	if (!WinDivertSend(handle, (PVOID)&finish, sizeof(PACKET), addr, NULL))
	{
		fprintf(stderr, "warning: failed to send finish packet (%d)\n",
			GetLastError());
	}

}

void blockPage(HANDLE handle,
			   WINDIVERT_IPHDR * ip_header,
			   WINDIVERT_TCPHDR* tcp_header,
			   WINDIVERT_ADDRESS* addr,
			   UINT payload_len)
{

	UINT blockpage_len = sizeof(DATAPACKET) + sizeof(block_data) - 1;
	outputDebugString(TEXT("blockpage length:"), blockpage_len);
	PDATAPACKET blockpage = (PDATAPACKET)malloc(blockpage_len);
	ZeroMemory(blockpage, blockpage_len);
	if (blockpage == NULL)
	{
		fprintf(stderr, "error: memory allocation failed\n");
		exit(EXIT_FAILURE);
	}
	//PacketInit(&blockpage->header);
	initializePacket(&blockpage->header);
	blockpage->header.ip.Length   = htons(blockpage_len);
	blockpage->header.tcp.SrcPort = htons(80);
	blockpage->header.tcp.Psh     = 1;
	blockpage->header.tcp.Ack     = 1;
	memcpy(blockpage->data, block_data, sizeof(block_data)-1);

	blockpage->header.ip.SrcAddr       = ip_header->DstAddr;
	blockpage->header.ip.DstAddr       = ip_header->SrcAddr;
	blockpage->header.tcp.DstPort      = tcp_header->SrcPort;
	blockpage->header.tcp.SeqNum       = tcp_header->AckNum;
	blockpage->header.tcp.AckNum       = htonl(ntohl(tcp_header->SeqNum) + payload_len);
	WinDivertHelperCalcChecksums((PVOID)blockpage, blockpage_len, 0);
	addr->Direction = !addr->Direction;     // Reverse direction.
	if (!WinDivertSend(handle, (PVOID)blockpage, blockpage_len, addr,
		NULL))
	{
		fprintf(stderr, "warning: failed to send block page packet (%d)\n",
			GetLastError());
	}
	free(blockpage);
}


//void finishBlock(HANDLE handle, WINDIVERT_IPHDR* ipHeader, WINDIVERT_TCPHDR* tcpHeader, WINDIVERT_ADDRESS& address, UINT dataLength)
//{
//	PACKET finish = {0};
//	initializePacket(&finish);
//	finish.tcp.Fin = 1;
//	finish.tcp.Ack = 1;
//	finish.ip.SrcAddr = ipHeader->DstAddr;
//	finish.ip.DstAddr = ipHeader->SrcAddr;
//	finish.tcp.SrcPort = htons(80);
//	finish.tcp.DstPort = tcpHeader->SrcPort;
//	finish.tcp.SeqNum = htonl(ntohl(tcpHeader->AckNum) + sizeof(block_data) - 1);
//	finish.tcp.AckNum = htonl(ntohl(tcpHeader->SeqNum) + dataLength);
//	WinDivertHelperCalcChecksums(&finish, sizeof(PACKET), 0);
//	if (!WinDivertSend(handle, &finish, sizeof(PACKET), &address, 0))
//	{
//		outputDebugString(TEXT("WinDivertSend failed in finishBlock:"), GetLastError());
//		return;
//	}
//	
//}


//void blockHttp(HANDLE handle,
//			   WINDIVERT_IPHDR* ipHeader,
//			   WINDIVERT_TCPHDR* tcpHeader,
//			   PVOID payload,
//			   UINT payloadLength,
//			   WINDIVERT_ADDRESS& address)
//{
//	closeRemoteServer(handle, ipHeader, tcpHeader, address);
//	blockPage(handle, ipHeader, tcpHeader, address, payloadLength);
//	finishBlock(handle, ipHeader, tcpHeader, address, payloadLength);
//}


//void catchData(HANDLE handle)
//{

//	PACKET packet = {0};
//	WINDIVERT_ADDRESS address = {0};
//	UINT packetLength = 0;
//	if (!WinDivertRecv(handle, &packet, sizeof(PACKET), &address, &packetLength))
//	{
//		outputDebugString(TEXT("WinDivertRecv failed:"), GetLastError());
//		return;
//	}

//	WINDIVERT_IPHDR* ipHeader = 0;
//	WINDIVERT_TCPHDR* tcpHeader = 0;

//	PVOID payload;
//	UINT payloadLength;
//	if(!WinDivertHelperParsePacket(&packet, packetLength, &ipHeader, NULL, NULL, NULL, &tcpHeader, NULL, &payload, &payloadLength))
//	{
//		outputDebugString(TEXT("WinDivertHelperParsePacket failed:"), GetLastError());
//		return ;
//	}
//	blockHttp(handle, ipHeader, tcpHeader, payload, payloadLength, address);
//}


int main(int, char**)
{
	//SetLastError(0);
	HANDLE handle = WinDivertOpen(
		"outbound && "              // Outbound traffic only
		"ip && "                    // Only IPv4 supported
		"tcp.DstPort == 80  && "     // HTTP (port 80) only
		"tcp.PayloadLength > 0",    // TCP data packets only
		WINDIVERT_LAYER_NETWORK, 404, 0);
	if (handle == INVALID_HANDLE_VALUE)
	{
		outputDebugString(TEXT("WinDivertOpen failed:"), GetLastError());
		return -1;
	}

	//UINT error = GetLastError();

	for(;;)
	{
		//vector<unsigned char> buffer(0xffff);
		//unsigned char* packet = &buffer[0];
		UINT8 packet[0xffff]; // = {0};
		WINDIVERT_ADDRESS address; // = {0};
		UINT packet_len;
		if (!WinDivertRecv(handle, packet, 0xffff, &address, &packet_len))
		{
			outputDebugString(TEXT("WindivertRecv failed:"), GetLastError());
			continue;
		}
		outputDebugString(TEXT("packet length:"), packet_len);
		WINDIVERT_IPHDR* ip_header;
		WINDIVERT_TCPHDR* tcp_header;
		PVOID payload;
		UINT payload_len;
		if (!WinDivertHelperParsePacket(
			&packet,
			packet_len,
			&ip_header,
			NULL,
			NULL,
			NULL,
			&tcp_header,
			NULL,
			&payload,
			&payload_len)) 
		{
			outputDebugString(TEXT("WinDivertHelperParsePacket failed:"), GetLastError());
			continue;
		}

		closeRemoteServer(handle, ip_header, tcp_header, &address);
		blockPage(handle, ip_header, tcp_header, &address, payload_len);
		finishBlock(handle, ip_header, tcp_header, &address, payload_len);
	}

	//while(true)
	//	catchData(handle);

	WinDivertClose(handle);

	return 0;
}
#include "OldOne.h"
#include <Windows.h>
#include <windivert.h>
#include "structures.h"
#include "Debugger.h"
#include "structures.h"


//OldOne::OldOne(void)
//{
//}
//
//
//OldOne::~OldOne(void)
//{
//}
//


//static void catchData(Interceptor& interceptor)
//{
//}
/*
* Prototypes
*/
//static void PacketInit(PPACKET packet);
//static int __cdecl UrlCompare(const void *a, const void *b);
//static int UrlMatch(PURL urla, PURL urlb);
//static PBLACKLIST BlackListInit(void);
//static void BlackListInsert(PBLACKLIST blacklist, PURL url);
//static void BlackListSort(PBLACKLIST blacklist);
//static BOOL BlackListMatch(PBLACKLIST blacklist, PURL url);
//static void BlackListRead(PBLACKLIST blacklist, const char *filename);
//static BOOL BlackListPayloadMatch(PBLACKLIST blacklist, char *data,
//								  UINT16 len);

//void outputToFile(const char* data)
//{
//	FILE* file;
//	fopen_s(&file, "packet.txt", "a+");	
//	fwrite(data, sizeof(char), strlen(data), file);
//	fclose(file);
//}

//void outputPacket(const PACKET* data)
//{
//	char buffer[500] = {0};
//	sprintf(buffer, "ip.Checksum:%d\n"
//		"ip.DstAddr:%d\n"
//		"ip.Frag0ff0:%d\n"
//		"ip.HdrLength:%d\n"
//		"ip.Id:%d\n"
//		"ip.Length:%d\n"
//		"ip.SrcAddr:%d\n"
//		"ip.TOS:%d\n"
//		"ip.TTL:%d\n"
//		"ip.Version:%d\n"
//		"tcp.Ack:%d\n"
//		"tcp.AckNum:%d\n"
//		"tcp.Checksum:%d\n"
//		"tcp.DstPort:%d\n"
//		"tcp.Psh:%d\n"
//		"tcp.Rst\n"
//		"tcp.SeqNum:%d\n"
//		"tcp.SrcPort:%d\n"
//		"tcp.Syn:%d\n"
//		"tcp.Urg:%d\n"
//		"tcp.UrtPtr:%d\n"
//		"tcp.Windows:%d\n",
//		data->ip.Checksum,
//		data->ip.DstAddr,
//		data->ip.FragOff0,
//		data->ip.HdrLength,
//		data->ip.Id,
//		data->ip.Length,
//		data->ip.SrcAddr,
//		data->ip.TOS,
//		data->ip.TTL,
//		data->ip.Version,
//		data->tcp.Ack,
//		data->tcp.AckNum,
//		data->tcp.Checksum,
//		data->tcp.DstPort,
//		data->tcp.Psh,
//		data->tcp.Rst,
//		data->tcp.SeqNum,
//		data->tcp.SrcPort,
//		data->tcp.Syn,
//		data->tcp.Urg,
//		data->tcp.UrgPtr,
//		data->tcp.Window);
//	outputToFile(buffer);
//}

void closeRemoteServer(HANDLE handle, WINDIVERT_IPHDR* ip_header, WINDIVERT_TCPHDR* tcp_header, WINDIVERT_ADDRESS* addr)
{
	PACKET reset;
	PacketInit(&reset);
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
		//fprintf(stderr, "warning: failed to send reset packet (%d)\n",
		//	GetLastError());
		Debugger::instance().output(TEXT("warning: failed to send reset packet (%d)\n"), GetLastError());
	}
}

void blockPage(HANDLE handle,
			   WINDIVERT_IPHDR * ip_header,
			   WINDIVERT_TCPHDR* tcp_header,
			   WINDIVERT_ADDRESS* addr,
			   UINT payload_len)
{

	UINT blockpage_len = sizeof(DATAPACKET) + sizeof(block_data) - 1;
	Debugger::instance().output(TEXT("blockpage length:"), blockpage_len);
	PDATAPACKET blockpage = (PDATAPACKET)malloc(blockpage_len);
	if (blockpage == NULL)
	{
		fprintf(stderr, "error: memory allocation failed\n");
		exit(EXIT_FAILURE);
	}
	PacketInit(&blockpage->header);
	blockpage->header.ip.Length   = htons((short)blockpage_len);
	blockpage->header.tcp.SrcPort = htons(80);
	blockpage->header.tcp.Psh     = 1;
	blockpage->header.tcp.Ack     = 1;
	memcpy(blockpage->data, block_data, sizeof(block_data)-1);

	//UINT32 destination = ip_header->DstAddr;
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

void finishBlock(HANDLE handle,
				 WINDIVERT_IPHDR* ip_header,
				 WINDIVERT_TCPHDR* tcp_header,
				 WINDIVERT_ADDRESS* addr,
				 UINT payload_len)
{
	PACKET finish;
	PacketInit(&finish);
	finish.tcp.Fin = 1;
	finish.tcp.Ack = 1;
	finish.ip.SrcAddr       = ip_header->DstAddr;
	finish.ip.DstAddr       = ip_header->SrcAddr;
	finish.tcp.SrcPort      = htons(80);
	finish.tcp.DstPort      = tcp_header->SrcPort;
	finish.tcp.SeqNum       = htonl(ntohl(tcp_header->AckNum) + sizeof(block_data) - 1); 
	finish.tcp.AckNum       = htonl(ntohl(tcp_header->SeqNum) + payload_len);
	Debugger::instance().output(TEXT("SentDataLength:"), sizeof(block_data) - 1);
	Debugger::instance().output(TEXT("payloadLength:"), payload_len);
	//outputPacket(finish);
	WinDivertHelperCalcChecksums((PVOID)&finish, sizeof(PACKET), 0);
	if (!WinDivertSend(handle, (PVOID)&finish, sizeof(PACKET), addr, NULL))
	{
		fprintf(stderr, "warning: failed to send finish packet (%d)\n",
			GetLastError());
	}
}

void outputInfo(char* payload, UINT payload_len)
{
	payload[payload_len] = 0;
	OutputDebugStringA(payload);
	OutputDebugString(TEXT("\n"));
}

/*
* Entry.
*/
int  oldmain(int /*argc*/, char ** /*argv*/)
{
	HANDLE handle;
	WINDIVERT_ADDRESS addr;
	UINT8 packet[MAXBUF];
	UINT packet_len;
	PWINDIVERT_IPHDR ip_header;
	PWINDIVERT_TCPHDR tcp_header;
	PVOID payload;
	UINT payload_len;
	//PACKET reset0;
	//PPACKET reset = &reset0;
	//PACKET finish0;
	//PPACKET finish = &finish0;
	//PDATAPACKET blockpage;
	//UINT16 blockpage_len;
	//PBLACKLIST blacklist;
	//UINT index;
	//unsigned i;
	INT16 priority = 404;       // Arbitrary.

	// Read the blacklists.
	//if (argc <= 1)
	//{
	//	fprintf(stderr, "usage: %s blacklist.txt [blacklist2.txt ...]\n",
	//		argv[0]);
	//	exit(EXIT_FAILURE);
	//}
	//blacklist = BlackListInit();
	//for (i = 1; i < (UINT)argc; i++)
	//{
	//	BlackListRead(blacklist, argv[i]);
	//}
	//BlackListSort(blacklist);

	// Initialize the pre-frabricated packets:
	//blockpage_len = sizeof(DATAPACKET)+sizeof(block_data)-1;
	//blockpage = (PDATAPACKET)malloc(blockpage_len);
	//if (blockpage == NULL)
	//{
	//    fprintf(stderr, "error: memory allocation failed\n");
	//    exit(EXIT_FAILURE);
	//}
	//PacketInit(&blockpage->header);
	//blockpage->header.ip.Length   = htons(blockpage_len);
	//blockpage->header.tcp.SrcPort = htons(80);
	//blockpage->header.tcp.Psh     = 1;
	//blockpage->header.tcp.Ack     = 1;
	//memcpy(blockpage->data, block_data, sizeof(block_data)-1);
	//PacketInit(reset);
	//reset->tcp.Rst = 1;
	//reset->tcp.Ack = 1;
	//PacketInit(finish);
	//finish->tcp.Fin = 1;
	//finish->tcp.Ack = 1;

	// Open the Divert device:
	//SetLastError(0);
	handle = WinDivertOpen(
		"outbound && "              // Outbound traffic only
		"ip && "                    // Only IPv4 supported
		"tcp.DstPort == 80 && "     // HTTP (port 80) only
		"tcp.PayloadLength > 0",    // TCP data packets only
		WINDIVERT_LAYER_NETWORK, priority, 0
		);
	if (handle == INVALID_HANDLE_VALUE)
	{
		//TCHAR buffer[300] = {0};
		//_stprintf(buffer,  TEXT("error: failed to open the WinDivert device (%d)\n"), GetLastError());
		//tstringstream ss;
		//ss << "error: failed to open WinDivert device (" << GetLastError() << ")" << std::endl;
		//OutputDebugString(ss.str().c_str());
		Debugger::instance().output(TEXT("error: failed to open WinDivert device"), GetLastError()); 
		return EXIT_FAILURE;
		//exit(EXIT_FAILURE);
	}
	printf("OPENED WinDivert\n");

	// Main loop:
	//while (true)
	for(;;)
	{
		if (!WinDivertRecv(handle, packet, sizeof(packet), &addr, &packet_len))
		{
			fprintf(stderr, "warning: failed to read packet (%d)\n",
				GetLastError());
			continue;
		}

		Debugger::instance().output(TEXT("packet lenght:"), packet_len);

		//for(index = 0; index < packet_len; ++index)
		//{
		//	printf("%02x ", (int)packet[index]);
		//}
		//printf("\n\n");

		if (!WinDivertHelperParsePacket(
			packet,
			packet_len,
			&ip_header,
			NULL,
			NULL,
			NULL,
			&tcp_header,
			NULL,
			&payload,
			&payload_len) 
			//|| !BlackListPayloadMatch(blacklist, (char*)payload, (UINT16)payload_len))
			)
		{
			printf("calling WinDivertHelperParsePacket failed:(%d)\n", GetLastError());
			// Packet does not match the blacklist; simply reinject it.
			//WinDivertHelperCalcChecksums(packet, packet_len, 
			//	WINDIVERT_HELPER_NO_REPLACE);
			//if (!WinDivertSend(handle, (PVOID)packet, packet_len, &addr, NULL))
			//{
			//	fprintf(stderr, "warning: failed to reinject packet (%d)\n",
			//		GetLastError());
			//}
			continue;
		}

		outputInfo((char*)payload, payload_len);

		//for(index = 0; index < packet_len; ++index)
		//{
		//	printf("%02x ", (int)packet[index]);
		//}
		//printf("\n\n");

		// The URL matched the blacklist; we block it by hijacking the TCP
		// connection.

		// (1) Send a TCP RST to the server; immediately closing the
		//     connection at the server's end.

		closeRemoteServer(handle, ip_header, tcp_header, &addr);


		// (2) Send the blockpage to the browser:
		blockPage(handle, ip_header, tcp_header, &addr, payload_len);
		// (3) Send a TCP FIN to the browser; closing the connection at the 
		//     browser's end.
		//printf("block_data length:%d\npayload length:%d\n", sizeof(block_data), payload_len);
		finishBlock(handle, ip_header, tcp_header, &addr, payload_len);
	}
}

/*
* Initialize a PACKET.
*/
//static void PacketInit(PPACKET packet)
//{
//	memset(packet, 0, sizeof(PACKET));
//	packet->ip.Version = 4;
//	packet->ip.HdrLength = sizeof(WINDIVERT_IPHDR) / sizeof(UINT32);
//	packet->ip.Length = htons(sizeof(PACKET));
//	packet->ip.TTL = 64;
//	packet->ip.Protocol = IPPROTO_TCP;
//	packet->tcp.HdrLength = sizeof(WINDIVERT_TCPHDR) / sizeof(UINT32);
//	//packet->tcp.Rst = 1;
//	//packet->tcp.Ack = 1;
//}

/*
* Initialize an empty blacklist.
*/
//static PBLACKLIST BlackListInit(void)
//{
//	PBLACKLIST blacklist = (PBLACKLIST)malloc(sizeof(BLACKLIST));
//	UINT size;
//	if (blacklist == NULL)
//	{
//		goto memory_error;
//	}
//	size = 1024;
//	blacklist->urls = (PURL *)malloc(size*sizeof(PURL));
//	if (blacklist->urls == NULL)
//	{
//		goto memory_error;
//	}
//	blacklist->size = size;
//	blacklist->length = 0;

//	return blacklist;

//memory_error:
//	fprintf(stderr, "error: failed to allocate memory\n");
//	exit(EXIT_FAILURE);
//}

/*
* Insert a URL into a blacklist.
*/
//static void BlackListInsert(PBLACKLIST blacklist, PURL url)
//{
//	if (blacklist->length >= blacklist->size)
//	{
//		blacklist->size = (blacklist->size*3) / 2;
//		printf("GROW blacklist to %u\n", blacklist->size);
//		blacklist->urls = (PURL *)realloc(blacklist->urls,
//			blacklist->size*sizeof(PURL));
//		if (blacklist->urls == NULL)
//		{
//			fprintf(stderr, "error: failed to reallocate memory\n");
//			exit(EXIT_FAILURE);
//		}
//	}

//	blacklist->urls[blacklist->length++] = url;
//}

/*
* Sort the blacklist (for searching).
*/
//static void BlackListSort(PBLACKLIST blacklist)
//{
//	qsort(blacklist->urls, blacklist->length, sizeof(PURL), UrlCompare);
//}

/*
* Match a URL against the blacklist.
*/
//static BOOL BlackListMatch(PBLACKLIST blacklist, PURL url)
//{
//	int lo = 0, hi = ((int)blacklist->length)-1;

//	while (lo <= hi)
//	{
//		INT mid = (lo + hi) / 2;
//		int cmp = UrlMatch(url, blacklist->urls[mid]);
//		if (cmp > 0)
//		{
//			hi = mid-1;
//		}
//		else if (cmp < 0)
//		{
//			lo = mid+1;
//		}
//		else
//		{
//			return TRUE;
//		}
//	}
//	return FALSE;
//}


/*
* Read URLs from a file.
*/
//static void BlackListRead(PBLACKLIST blacklist, const char *filename)
//{
//	char domain[MAXURL+1];
//	char uri[MAXURL+1];
//	int c;
//	UINT16 i, j;
//	PURL url;
//	FILE *file = fopen(filename, "r");

//	if (file == NULL)
//	{
//		fprintf(stderr, "error: could not open blacklist file %s\n",
//			filename);
//		exit(EXIT_FAILURE);
//	}

//	// Read URLs from the file and add them to the blacklist: 
//	while (TRUE)
//	{
//		while (isspace(c = getc(file)))
//			;
//		if (c == EOF)
//		{
//			break;
//		}
//		if (c != '-' && !isalnum(c))
//		{
//			while (!isspace(c = getc(file)) && c != EOF)
//				;
//			if (c == EOF)
//			{
//				break;
//			}
//			continue;
//		}
//		i = 0;
//		domain[i++] = (char)c;
//		while ((isalnum(c = getc(file)) || c == '-' || c == '.') && i < MAXURL)
//		{
//			domain[i++] = (char)c;
//		}
//		domain[i] = '\0';
//		j = 0;
//		if (c == '/')
//		{
//			while (!isspace(c = getc(file)) && c != EOF && j < MAXURL)
//			{
//				uri[j++] = (char)c;
//			}
//			uri[j] = '\0';
//		}
//		else if (isspace(c))
//		{
//			uri[j] = '\0';
//		}
//		else
//		{
//			while (!isspace(c = getc(file)) && c != EOF)
//				;
//			continue;
//		}

//		printf("ADD %s/%s\n", domain, uri);

//		url = (PURL)malloc(sizeof(URL));
//		if (url == NULL)
//		{
//			goto memory_error;
//		}
//		url->domain = (char *)malloc((i+1)*sizeof(char));
//		url->uri    = (char *)malloc((j+1)*sizeof(char));
//		if (url->domain == NULL || url->uri == NULL)
//		{
//			goto memory_error;
//		}
//		strcpy(url->uri, uri);
//		for (j = 0; j < i; j++)
//		{
//			url->domain[j] = domain[i-j-1];
//		}
//		url->domain[j] = '\0';

//		BlackListInsert(blacklist, url);
//	}

//	fclose(file);
//	return;

//memory_error:
//	fprintf(stderr, "error: memory allocation failed\n");
//	exit(EXIT_FAILURE);
//}

/*
* Attempt to parse a URL and match it with the blacklist.
*
* BUG:
* - This function makes several assumptions about HTTP requests, such as:
*      1) The URL will be contained within one packet;
*      2) The HTTP request begins at a packet boundary;
*      3) The Host header immediately follows the GET/POST line.
*   Some browsers, such as Internet Explorer, violate these assumptions
*   and therefore matching will not work.
*/
//static BOOL BlackListPayloadMatch(PBLACKLIST blacklist, char *data, UINT16 len)
//{
//	static const char get_str[] = "GET /";
//	static const char post_str[] = "POST /";
//	static const char http_host_str[] = " HTTP/1.1\r\nHost: ";
//	char domain[MAXURL];
//	char uri[MAXURL];
//	URL url = {domain, uri};
//	UINT16 i = 0, j;
//	BOOL result;
//	HANDLE console;

//	if (len <= sizeof(post_str) + sizeof(http_host_str))
//	{
//		return FALSE;
//	}
//	if (strncmp(data, get_str, sizeof(get_str)-1) == 0)
//	{
//		i += sizeof(get_str)-1;
//	}
//	else if (strncmp(data, post_str, sizeof(post_str)-1) == 0)
//	{
//		i += sizeof(post_str)-1;
//	}
//	else
//	{
//		return FALSE;
//	}

//	for (j = 0; i < len && data[i] != ' '; j++, i++)
//	{
//		uri[j] = data[i];
//	}
//	uri[j] = '\0';
//	if (i + sizeof(http_host_str)-1 >= len)
//	{
//		return FALSE;
//	}

//	if (strncmp(data+i, http_host_str, sizeof(http_host_str)-1) != 0)
//	{
//		return FALSE;
//	}
//	i += sizeof(http_host_str)-1;

//	for (j = 0; i < len && data[i] != '\r'; j++, i++)
//	{
//		domain[j] = data[i];
//	}
//	if (i >= len)
//	{
//		return FALSE;
//	}
//	if (j == 0)
//	{
//		return FALSE;
//	}
//	if (domain[j-1] == '.')
//	{
//		// Nice try...
//		j--;
//		if (j == 0)
//		{
//			return FALSE;
//		}
//	}
//	domain[j] = '\0';

//	printf("URL %s/%s: ", domain, uri);

//	// Reverse the domain:
//	for (i = 0; i < j / 2; i++)
//	{
//		char t = domain[i];
//		domain[i] = domain[j-i-1];
//		domain[j-i-1] = t;
//	}

//	// Search the blacklist:
//	result = BlackListMatch(blacklist, &url);

//	// Print the verdict:
//	console = GetStdHandle(STD_OUTPUT_HANDLE);
//	if (result)
//	{
//		SetConsoleTextAttribute(console, FOREGROUND_RED);
//		puts("BLOCKED!");
//	}
//	else
//	{
//		SetConsoleTextAttribute(console, FOREGROUND_GREEN);
//		puts("allowed");
//	}
//	SetConsoleTextAttribute(console,
//		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
//	return result;
//}

/*
* URL comparison.
*/
//static int __cdecl UrlCompare(const void *a, const void *b)
//{
//	PURL urla = *(PURL *)a;
//	PURL urlb = *(PURL *)b;
//	int cmp = strcmp(urla->domain, urlb->domain);
//	if (cmp != 0)
//	{
//		return cmp;
//	}
//	return strcmp(urla->uri, urlb->uri);
//}

/*
* URL matching
*/
//static int UrlMatch(PURL urla, PURL urlb)
//{
//	UINT16 i;

//	for (i = 0; urla->domain[i] && urlb->domain[i]; i++)
//	{
//		int cmp = (int)urlb->domain[i] - (int)urla->domain[i];
//		if (cmp != 0)
//		{
//			return cmp;
//		}
//	}
//	if (urla->domain[i] == '\0' && urlb->domain[i] != '\0')
//	{
//		return 1;
//	}

//	for (i = 0; urla->uri[i] && urlb->uri[i]; i++)
//	{
//		int cmp = (int)urlb->uri[i] - (int)urla->uri[i];
//		if (cmp != 0)
//		{
//			return cmp;
//		}
//	}
//	if (urla->uri[i] == '\0' && urlb->uri[i] != '\0')
//	{
//		return 1;
//	}
//	return 0;
//}


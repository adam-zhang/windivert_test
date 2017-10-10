#pragma once

#include <Windows.h>
#include <windivert.h>
#include "error.h"

BOOL ParsePacket(PVOID pPacket, 
				 UINT packetLen,
				 PWINDIVERT_IPHDR *ppIpHdr,
				 PWINDIVERT_IPV6HDR *ppIpv6Hdr,
				 PWINDIVERT_ICMPHDR *ppIcmpHdr,
				 PWINDIVERT_ICMPV6HDR *ppIcmpv6Hdr,
				 PWINDIVERT_TCPHDR *ppTcpHdr,
				 PWINDIVERT_UDPHDR *ppUdpHdr,
				 PVOID *ppData,
				 UINT *pDataLen);
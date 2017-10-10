#include <WinSock2.h>
#include "Helper.h"
#include "error.h"

static UINT8 WinDivertSkipExtHeaders(UINT8 proto, UINT8 **header, UINT *len)
{
    UINT hdrlen;

    while (TRUE)
    {
        if (*len <= 2)
        {
            return IPPROTO_NONE;
        }

        hdrlen = (UINT)*(*header + 1);
        switch (proto)
        {
            case IPPROTO_FRAGMENT:
                hdrlen = 8;
                break;
            case IPPROTO_AH:
                hdrlen += 2;
                hdrlen *= 4;
                break;
            case IPPROTO_HOPOPTS:
            case IPPROTO_DSTOPTS:
            case IPPROTO_ROUTING:
                hdrlen++;
                hdrlen *= 8;
                break;
            case IPPROTO_NONE:
                return proto;
            default:
                return proto;
        }

        if (hdrlen >= *len)
        {
            return IPPROTO_NONE;
        }

        proto = **header;
        *header += hdrlen;
        *len -= hdrlen;
    }
}

BOOL ParsePacket(PVOID pPacket,
				 UINT packetLen,
				 PWINDIVERT_IPHDR *ppIpHdr,
				 PWINDIVERT_IPV6HDR *ppIpv6Hdr,
				 PWINDIVERT_ICMPHDR *ppIcmpHdr,
				 PWINDIVERT_ICMPV6HDR *ppIcmpv6Hdr,
				 PWINDIVERT_TCPHDR *ppTcpHdr,
				 PWINDIVERT_UDPHDR *ppUdpHdr,
				 PVOID *ppData,
				 UINT *pDataLen)
{
	PWINDIVERT_IPHDR ip_header = NULL;
	PWINDIVERT_IPV6HDR ipv6_header = NULL;
	PWINDIVERT_ICMPHDR icmp_header = NULL;
	PWINDIVERT_ICMPV6HDR icmpv6_header = NULL;
	PWINDIVERT_TCPHDR tcp_header = NULL;
	PWINDIVERT_UDPHDR udp_header = NULL;
	UINT16 header_len;
	UINT8 trans_proto;
	PVOID data = NULL;
	UINT data_len = 0;
	BOOL success;

	if (pPacket == NULL || packetLen < sizeof(UINT8))
	{
		SetLastError(INVALID_BUFFER);
		goto WinDivertHelperParsePacketExit;
	}
	data = pPacket;
	data_len = packetLen;

	ip_header = (PWINDIVERT_IPHDR)data;
	switch (ip_header->Version)
	{
	case 4:
		if (data_len < sizeof(WINDIVERT_IPHDR) ||
			ip_header->HdrLength < 5 ||
			data_len < ip_header->HdrLength*sizeof(UINT32) ||
			ntohs(ip_header->Length) != data_len)
		{
			ip_header = NULL;
			SetLastError(INVALID_IP_HEADER);
			goto WinDivertHelperParsePacketExit;
		}
		trans_proto = ip_header->Protocol;
		header_len = ip_header->HdrLength*sizeof(UINT32);
		data = (PVOID)((UINT8 *)data + header_len);
		data_len -= header_len;
		break;
	case 6:
		ip_header = NULL;
		ipv6_header = (PWINDIVERT_IPV6HDR)data;
		if (data_len < sizeof(WINDIVERT_IPV6HDR) ||
			ntohs(ipv6_header->Length) !=
			data_len - sizeof(WINDIVERT_IPV6HDR))
		{
			ipv6_header = NULL;
			SetLastError(INVALID_IPV6_HEADER);
			goto WinDivertHelperParsePacketExit;
		}
		trans_proto = ipv6_header->NextHdr;
		data = (PVOID)((UINT8 *)data + sizeof(WINDIVERT_IPV6HDR));
		data_len -= sizeof(WINDIVERT_IPV6HDR);
		trans_proto = WinDivertSkipExtHeaders(trans_proto, (UINT8 **)&data,
			&data_len);
		break;
	default:
		ip_header = NULL;
		goto WinDivertHelperParsePacketExit;
	}
	switch (trans_proto)
	{
	case IPPROTO_TCP:
		tcp_header = (PWINDIVERT_TCPHDR)data;
		if (data_len < sizeof(WINDIVERT_TCPHDR) ||
			tcp_header->HdrLength < 5 ||
			data_len < tcp_header->HdrLength*sizeof(UINT32))
		{
			tcp_header = NULL;
			SetLastError(INVALID_TCP_HEADER);
			goto WinDivertHelperParsePacketExit;
		}
		header_len = tcp_header->HdrLength*sizeof(UINT32);
		data = ((UINT8 *)data + header_len);
		data_len -= header_len;
		break;
	case IPPROTO_UDP:
		udp_header = (PWINDIVERT_UDPHDR)data;
		if (data_len < sizeof(WINDIVERT_UDPHDR) ||
			ntohs(udp_header->Length) != data_len)
		{
			udp_header = NULL;
			SetLastError(INVALID_UDP_HEADER);
			goto WinDivertHelperParsePacketExit;
		}
		data = ((UINT8 *)data + sizeof(WINDIVERT_UDPHDR));
		data_len -= sizeof(WINDIVERT_UDPHDR);
		break;
	case IPPROTO_ICMP:
		icmp_header = (PWINDIVERT_ICMPHDR)data;
		if (ip_header == NULL ||
			data_len < sizeof(WINDIVERT_ICMPHDR))
		{
			icmp_header = NULL;
			SetLastError(INVALID_ICMP_HEADER);
			goto WinDivertHelperParsePacketExit;
		}
		data = ((UINT8 *)data + sizeof(WINDIVERT_ICMPHDR));
		data_len -= sizeof(WINDIVERT_ICMPHDR);
		break;
	case IPPROTO_ICMPV6:
		icmpv6_header = (PWINDIVERT_ICMPV6HDR)data;
		if (ipv6_header == NULL ||
			data_len < sizeof(WINDIVERT_ICMPV6HDR))
		{
			icmpv6_header = NULL;
			SetLastError(INVALID_ICMPV6_HEADER);
			goto WinDivertHelperParsePacketExit;
		}
		data = ((UINT8 *)data + sizeof(WINDIVERT_ICMPV6HDR));
		data_len -= sizeof(WINDIVERT_ICMPV6HDR);
		break;
	default:
		break;
	}

	if (data_len == 0)
	{
		data = NULL;
		SetLastError(INVALID_DATA);
	}

WinDivertHelperParsePacketExit:
	success = TRUE;
	if (ppIpHdr != NULL)
	{
		*ppIpHdr = ip_header;
		success = success && (ip_header != NULL);
	}
	if (ppIpv6Hdr != NULL)
	{
		*ppIpv6Hdr = ipv6_header;
		success = success && (ipv6_header != NULL);
	}
	if (ppIcmpHdr != NULL)
	{
		*ppIcmpHdr = icmp_header;
		success = success && (icmp_header != NULL);
	}
	if (ppIcmpv6Hdr != NULL)
	{
		*ppIcmpv6Hdr = icmpv6_header;
		success = success && (icmpv6_header != NULL);
	}
	if (ppTcpHdr != NULL)
	{
		*ppTcpHdr = tcp_header;
		success = success && (tcp_header != NULL);
	}
	if (ppUdpHdr != NULL)
	{
		*ppUdpHdr = udp_header;
		success = success && (udp_header != NULL);
	}
	if (ppData != NULL)
	{
		*ppData = data;
		success = success && (data != NULL);
	}
	if (pDataLen != NULL)
	{
		*pDataLen = data_len;
	}
	return success;
}
#pragma once

#include <WinDivert.h>

class IPHeader : private WINDIVERT_IPHDR
{
public:
	IPHeader(void)
		: WINDIVERT_IPHDR()
	{
		Version = 4;
		HdrLength = sizeof(WINDIVERT_IPHDR)/sizeof(UINT32);
		TTL = 64;
		Protocol = IPPROTO_TCP;
	}
	~IPHeader(void);
public:
	const WINDIVERT_IPHDR& get()const
	{
		return *this;
	}
};


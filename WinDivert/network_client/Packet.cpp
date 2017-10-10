#include "stdafx.h"
#include "Packet.h"
#include <iomanip>
#include <algorithm>


Packet::Packet(void)
	: ipHeader_(WINDIVERT_IPHDR())
	, tcpHeader_(WINDIVERT_TCPHDR())
{
}


Packet::~Packet(void)
{
}

void Packet::setData(const unsigned char* data, size_t length)
{
	std::vector<unsigned char> buffer;
	std::copy(data, data + length, back_inserter(buffer));
	setData(buffer);
}


#pragma once
#include <windivert.h>
#include <vector>

class Packet
{
public:
	Packet(void);
	~Packet(void);
private:
	WINDIVERT_IPHDR ipHeader_;
	WINDIVERT_TCPHDR tcpHeader_;
	std::vector<unsigned char> data_;
public:
	const WINDIVERT_IPHDR& ipHeader()const
	{ return ipHeader_; }
	void setIpHeader(const WINDIVERT_IPHDR& value)
	{ ipHeader_ = value; }
	const WINDIVERT_TCPHDR tcpHeader()const
	{ return tcpHeader_; }
	void setTcpHeader(const WINDIVERT_TCPHDR& value)
	{ tcpHeader_ = value; }
	const std::vector<unsigned char> data()const
	{ return data_; }
	void setData(const std::vector<unsigned char>& data)
	{ data_ = data;}
	void setData(const unsigned char* data, size_t length);
};


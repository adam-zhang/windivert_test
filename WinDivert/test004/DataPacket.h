#pragma once
#include <vector>
#include <windivert.h>
#include <algorithm>
#include <iterator>

class DataPacket
{
public:
	DataPacket(void);
	~DataPacket(void);
private:
	WINDIVERT_IPHDR ipHeader_;
	WINDIVERT_TCPHDR tcpHeader_;
	std::vector<unsigned char> data_;
public:
	void setIpHeader(const WINDIVERT_IPHDR& header)
	{ ipHeader_ = header; }
	const WINDIVERT_IPHDR& ipHeader()const
	{ return ipHeader_; }
	void setTcpHeader(const WINDIVERT_TCPHDR& header)
	{ tcpHeader_ = header; }
	const WINDIVERT_TCPHDR& tcpHeader()const
	{ return tcpHeader_;}
	void setData(const std::vector<unsigned char>& data)
	{ data_ = data; }
	void setData(const char* data, size_t length)
	{
		std::vector<unsigned char> store;
		std::copy(data, data + length, std::back_inserter(store));
		setData(store);
	}
	void setData(const unsigned char* data, size_t length)
	{ 
		std::vector<unsigned char> store;
		std::copy(data, data + length, std::back_inserter(store));
		setData(store);
	}
	std::vector<unsigned char> data()const
	{ return data_;}
public:
	std::vector<unsigned char> package()const;


};


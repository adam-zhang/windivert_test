#include "DataPacket.h"


DataPacket::DataPacket(void)
{
}


DataPacket::~DataPacket(void)
{
}

std::vector<unsigned char> DataPacket::package()const
{
	std::vector<unsigned char> dataPackage(sizeof(WINDIVERT_IPHDR) + sizeof(WINDIVERT_TCPHDR) + data().size());
	copy((char*)&ipHeader_, (char*)&ipHeader_ + sizeof(WINDIVERT_IPHDR), back_inserter(dataPackage));
	copy((char*)&tcpHeader_, (char*)&tcpHeader_ + sizeof(WINDIVERT_TCPHDR), back_inserter(dataPackage));
	copy(data_.begin(), data_.end(), back_inserter(dataPackage));
	return dataPackage;
}

#ifndef __CLIENT__H
#define __CLIENT__H

#include "Singleton.h"
#include <vector>

class Client : public Singleton<Client>
{
	public:
		Client();
		~Client();
	public:
		const std::vector<unsigned char> send(const std::vector<unsigned char>& data);

};
#endif//__CLIENT__H

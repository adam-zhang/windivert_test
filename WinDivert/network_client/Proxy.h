#pragma once

#include <string>
#include <vector>

class Proxy
{
public:
	Proxy(void);
	~Proxy(void);
private:
	static Proxy* instance_;
public:
	static Proxy& instance()
	{
		if (!instance_)
			instance_ = new Proxy;
		return *instance_;
	}

	std::vector<unsigned char> send(const unsigned char* data, size_t length);
	//std::string send(const std::string&);
	std::vector<unsigned char> send(const std::vector<unsigned char>& data);
};


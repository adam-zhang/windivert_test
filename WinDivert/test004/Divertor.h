#pragma once
#include "Singleton.h"
#include <Windows.h>
#include <string>

class Divertor : public Singleton<Divertor>
{
public:
	Divertor(void);
	~Divertor(void);
private:
	HANDLE handle_;
	HANDLE thread_;
	std::string filter_;
private:
	const HANDLE& thread()const;
	void setThread(const HANDLE&);
	void setFilter(const std::string&);
public:
	const std::string& filter()const;
	const HANDLE& handle()const;
	void setHandle(const HANDLE& handle);

	bool start(const std::string& filter);
	void terminate();
};


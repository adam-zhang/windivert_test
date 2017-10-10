#pragma once

#include <string>
#include <Windows.h>
#include <vector>
#include <memory>

class website;

class Interceptor
{
public:
	Interceptor();
	~Interceptor(void);
public:

public:
	void addFilter(const std::string& url, int port);
	//void addFilter(const std::string& filter);
	bool monitor();
	bool monitor(const std::string& filter);
	std::vector<std::shared_ptr<website>> websites()
	{ return websites_; }
public:
	void setHandle(const HANDLE& handle);
	const HANDLE& handle()const
	{ return handle_; }
	void setThreadHandle(const HANDLE& handle)
	{ threadHandle_ = handle;}
	const HANDLE& threadHandle()const
	{ return threadHandle_; }
	const std::string& filter()const
	{ return filter_; }
	void setFilter(const std::string& filter)
	{ filter_ = filter; }
private:
	HANDLE handle_;
	HANDLE threadHandle_;
	std::string filter_;
	std::vector<std::shared_ptr<website>> websites_;		
};


#include "stdafx.h"
#include <map>
#include "Singleton.h"

class Error : public Singleton<Error>
{
	Error();
	~Error();
	private:
		std::map<DWORD, tstring> dictError_;
	private:
		void initialize();
	public:
		std::string getErrorDescription(DWORD);

};

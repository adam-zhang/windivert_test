#include <iostream>
#include <Windows.h>

BOOL IsWow64() 
{ 
    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL); 
    LPFN_ISWOW64PROCESS fnIsWow64Process; 
    BOOL bIsWow64 = FALSE; 
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")),"IsWow64Process"); 
    if (NULL != fnIsWow64Process) 
    { 
        fnIsWow64Process(GetCurrentProcess(),&bIsWow64);
    } 
    return bIsWow64; 
} 

int main(int, char**)
{
	std::cout << IsWow64() << std::endl;
	std::cin.get();
	return 0;
}
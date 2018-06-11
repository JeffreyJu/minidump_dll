#pragma once
#include <Windows.h>
class CDllLoader
{
public:
	CDllLoader();
	virtual ~CDllLoader();
	bool		LoadDll(const TCHAR* path);
	void		Unload();
	FARPROC		GetProcAddress(const char* name);
private:
	HMODULE m_hDll;
};
#include "stdafx.h"
#include "DllLoader.h"

CDllLoader::CDllLoader()
{
	m_hDll = NULL;
}

CDllLoader::~CDllLoader()
{
	Unload();
}

bool		CDllLoader::LoadDll(const TCHAR* path)
{
	m_hDll = ::LoadLibrary(path);
	return m_hDll != NULL;
}

void	CDllLoader::Unload()
{
	if(m_hDll)
	{
		::FreeLibrary(m_hDll);
		m_hDll = NULL;
	}
}

FARPROC		CDllLoader::GetProcAddress(const char* name)
{
	if(m_hDll)
	{
		return ::GetProcAddress(m_hDll,name);
	}
	return NULL;
}

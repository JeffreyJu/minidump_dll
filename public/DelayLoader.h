#pragma once

#include <vector>

class CDelayLoader
{
public:
    CDelayLoader(void)	{}
    virtual ~CDelayLoader(void)				{ FreeAllLibrary(); }
    bool AddDll(LPCTSTR pszDllFileName)
    {
        HMODULE hModule=::LoadLibrary(pszDllFileName);
        if(hModule==NULL)
		{
            return false;
		}
        m_Modeules.push_back(hModule);
        return true;
    }
protected:
    void FreeAllLibrary()
    {
		for (std::vector<HMODULE>::iterator it = m_Modeules.begin(); it != m_Modeules.end(); it++)
			::FreeLibrary(*it);
        m_Modeules.clear();
    }
private:
    std::vector<HMODULE> m_Modeules;
};

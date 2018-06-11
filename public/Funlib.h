#pragma once
#include <Windows.h>
#include <vector>
#include <string>

#ifdef _UNICODE
	typedef std::wstring STDSTRING;
#else
	typedef std::string STDSTRING;
#endif

bool GetModulePath(HMODULE hModule,TCHAR* path,int size);
bool GetModulePathA(HMODULE hModule,char* path,int size);

void ConverToWindowPath(const TCHAR* path,TCHAR* dest);
void GetFileName(const TCHAR* path,TCHAR*name,int namesize,TCHAR* extname,int extsize);
bool GetFileVersion(const TCHAR* file, TCHAR* version, int size);
bool TraversalFiles(const TCHAR* path,std::vector<STDSTRING>& Files,TCHAR* Folder = NULL,const TCHAR* ext = TEXT("*"),int rettype = 0 /*0 文件名 1相对路径 2 绝对路径*/) ;

char* GetFirstLetter(const char* s, char* buf, int len);

TCHAR* FormatInteger(TCHAR* buf, int value, const TCHAR* defaultIfZero = NULL);
TCHAR* FormatFloat(TCHAR* buf, double value, int nDec, bool bTrimZero = true, TCHAR cSplit = 0, const TCHAR* defaultIfZero = NULL);

bool IsFileExist(TCHAR* path);

bool GetHardDiskNum(char* szDiskNo);
bool GetMacAddress(char* szMac);
bool GetCpuName(char* szCpu, int len);

#define lbmax(a,b)    (((a) > (b)) ? (a) : (b))
#define lbmin(a,b)    (((a) < (b)) ? (a) : (b))
#define lbmin_mid_max(a,c,b) lbmin(lbmax(a,c),b)

#define EPS_DOUBLE	1e-10

#ifndef SIZEOF_ARRAY
#define SIZEOF_ARRAY(x) ((sizeof(x))/(sizeof(x[0])))
#endif // SIZEOF_ARRAY

#include "stdafx.h"
#include "Funlib.h"
#include <WinIoCtl.h>
#include <IPHlpApi.h>
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"Version.lib")

bool GetModulePath(HMODULE hModule,TCHAR* path,int size)
{
    TCHAR currentPath[MAX_PATH] = {0};
    int ret = ::GetModuleFileName(hModule,currentPath,sizeof(currentPath));
    TCHAR* pos = _tcsrchr(currentPath,'\\');
    *(pos + 1) = 0;
    _tcsncpy(path,currentPath,size);
    return ret != 0;
}

bool GetModulePathA(HMODULE hModule,char* path,int size)
{
    char currentPath[MAX_PATH] = {0};
    int ret = ::GetModuleFileNameA(hModule,currentPath,sizeof(currentPath));
    char* pos = strrchr(currentPath,'\\');
    *(pos + 1) = 0;
    strncpy(path,currentPath,size);
    return ret != 0;
}

void ConverToWindowPath(const TCHAR* path,TCHAR* dest)
{
	while(*path)
	{
		if(*path == TEXT('/'))
		{
			* dest = TEXT('\\');
		}
		else
		{
			* dest = *path;
		}
		path++;
		dest++;
	}
}

void GetFileName(const TCHAR* path,TCHAR*name,int namesize,TCHAR* extname,int extsize)
{
	TCHAR szFileName[MAX_PATH] = {0};
	TCHAR szWinPath[MAX_PATH] = {0};
	ConverToWindowPath(path,szWinPath);
	TCHAR* pos = _tcsrchr(szWinPath,TEXT('\\'));
	if(pos == NULL)
	{
		_tcscpy(szFileName,szWinPath);
	}
	else
	{
		_tcscpy(szFileName,pos + 1);
	}
	
	TCHAR* pos1 = _tcsrchr(szFileName,TEXT('.'));
	if(pos1 == NULL)
	{
		_tcsncpy(name,szFileName,namesize - 1);
		*extname = 0;
	}
	else
	{
		*pos1 = 0;
		_tcsncpy(name,szFileName,namesize - 1);
		_tcsncpy(extname,pos1 + 1,extsize - 1);
	}
}

bool TraversalFiles(const TCHAR* path,std::vector<STDSTRING>& Files,TCHAR* Folder ,const TCHAR* ext,int rettype )
{
	TCHAR szFind[MAX_PATH];
	WIN32_FIND_DATA FindFileData;
	_tcscpy(szFind,path);
	_tcscat(szFind,TEXT("\\*.*"));

	HANDLE hFind=::FindFirstFile(szFind,&FindFileData);
	if(INVALID_HANDLE_VALUE == hFind)
		return false;

	while(true)
	{
		if(!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if(ext == NULL || *ext == 0 || *ext == TEXT('*'))
			{
				TCHAR szFilePathName[MAX_PATH] = {0};
				if (rettype == 2)
				{
					_tcscpy(szFilePathName,path);
					_tcscat(szFilePathName,TEXT("\\"));
					_tcscat(szFilePathName,FindFileData.cFileName);
				}
				else if(rettype == 1)
				{
					if (Folder)
					{
						_tcscpy(szFilePathName,Folder);
					}
					_tcscat(szFilePathName,FindFileData.cFileName);
				}
				else 
				{
					_tcscat(szFilePathName,FindFileData.cFileName);
				}
				Files.push_back(szFilePathName);
			}
			else
			{
				TCHAR szName[MAX_PATH] = {0};
				TCHAR szExt[MAX_PATH] = {0};
				GetFileName(FindFileData.cFileName,szName,MAX_PATH,szExt,MAX_PATH);
				if(_tcscmp(szExt,ext) == 0)
				{
					TCHAR szFilePathName[MAX_PATH] = {0};
					if (rettype == 2)
					{
						_tcscpy(szFilePathName,path);
						_tcscat(szFilePathName,TEXT("\\"));
						_tcscat(szFilePathName,FindFileData.cFileName);
					}
					else if(rettype == 1)
					{
						if (Folder)
						{
							_tcscpy(szFilePathName,Folder);
						}
						_tcscat(szFilePathName,FindFileData.cFileName);
					}
					else 
					{
						_tcscat(szFilePathName,FindFileData.cFileName);
					}
					Files.push_back(szFilePathName);
				}
			}
		}
		else
		{			
			if(FindFileData.cFileName[0]!='.')
			{
				TCHAR szSubPath[MAX_PATH] = {0};
				TCHAR szSubFolder[MAX_PATH] = {0};
				if (Folder) _tcscpy(szSubFolder, Folder);
				_tcscat(szSubFolder, FindFileData.cFileName);
				_tcscat(szSubFolder,TEXT("\\"));
				_tcscpy(szSubPath,path);
				_tcscat(szSubPath, FindFileData.cFileName);
				_tcscat(szSubPath,TEXT("\\"));

				TraversalFiles(szSubPath,Files,szSubFolder,ext,rettype);
			}
		}
		if(!FindNextFile(hFind,&FindFileData))
			break;
	}
	FindClose(hFind);
	return true;
}


#define BETWEEN(x, a, b)	((x) >= (a) && (x) <= (b))
char* GetFirstLetter(const char* s, char* buf, int len)
{
	int pos = 0;
	const char* p = s;
	while (p && *p && pos < len)
	{
		unsigned int ch = (unsigned char)*p;
		if (ch > 0xA0)	//汉字
		{
			ch = (ch << 8) + (unsigned char)p[1];
			if (BETWEEN(ch, 0xA3B0, 0xA3B9))	//中文数字0-9
			{
				buf[pos++] = '0' + ch - 0xA3B0;
			}else if (BETWEEN(ch, 0xA3C1, 0xA3DA))	//中文字母A-Z
			{
				buf[pos++] = 'A' + ch - 0xA3C1;
			}else if (BETWEEN(ch, 0xA3E1, 0xA3FA))	//中文字母a-z
			{
				buf[pos++] = 'A' + ch - 0xA3E1;
			}else if (BETWEEN(ch, 0xB0A1, 0xB0C4))	//A
			{
				buf[pos++] = 'A';
			}else if (BETWEEN(ch, 0xB0C5, 0xB2C0))	//B
			{
				buf[pos++] = 'B';
			}else if (BETWEEN(ch, 0xB2C1, 0xB4ED))	//C
			{
				buf[pos++] = 'C';
				//if (BETWEEN(ch, 0xB2E5, 0xB4C2))	//CH
				//	buf[pos++] = 'H';
			}else if (BETWEEN(ch, 0xB4EE, 0xB6E9))	//D
			{
				buf[pos++] = 'D';
			}else if (BETWEEN(ch, 0xB6EA, 0xB7A1))	//E
			{
				buf[pos++] = 'E';
			}else if (BETWEEN(ch, 0xB7A2, 0xB8C0))	//F
			{
				buf[pos++] = 'F';
			}else if (BETWEEN(ch, 0xB8C1, 0xB9FD))	//G
			{
				buf[pos++] = 'G';
			}else if (BETWEEN(ch, 0xB9FE, 0xBBF6))	//H
			{
				buf[pos++] = 'H';
			}else if (BETWEEN(ch, 0xBBF7, 0xBFA5))	//J
			{
				buf[pos++] = 'J';
			}else if (BETWEEN(ch, 0xBFA6, 0xC0AB))	//K
			{
				buf[pos++] = 'K';
			}else if (BETWEEN(ch, 0xC0AC, 0xC2E7))	//L
			{
				buf[pos++] = 'L';
			}else if (BETWEEN(ch, 0xC2E8, 0xC4C2))	//M
			{
				buf[pos++] = 'M';
			}else if (BETWEEN(ch, 0xC4C3, 0xC5B5))	//N
			{
				buf[pos++] = 'N';
			}else if (BETWEEN(ch, 0xC5B6, 0xC5BD))	//O
			{
				buf[pos++] = 'O';
			}else if (BETWEEN(ch, 0xC5BE, 0xC6D9))	//P
			{
				buf[pos++] = 'P';
			}else if (BETWEEN(ch, 0xC6DA, 0xC8BA))	//Q
			{
				buf[pos++] = 'Q';
			}else if (BETWEEN(ch, 0xC8BB, 0xC8F5))	//R
			{
				buf[pos++] = 'R';
			}else if (BETWEEN(ch, 0xC8F6, 0xCBF9))	//S
			{
				buf[pos++] = 'S';
				//if (BETWEEN(ch, 0xC9AF, 0xCBB8))	//SH
				//	buf[pos++] = 'H';
			}else if (BETWEEN(ch, 0xCBFA, 0xCDD9))	//T
			{
				buf[pos++] = 'T';
			}else if (BETWEEN(ch, 0xCDDA, 0xCEF3))	//W
			{
				buf[pos++] = 'W';
			}else if (BETWEEN(ch, 0xCEF4, 0xD1B8))	//X
			{
				buf[pos++] = 'X';
			}else if (BETWEEN(ch, 0xD1B9, 0xD4D0))	//Y
			{
				buf[pos++] = 'Y';
			}else if (BETWEEN(ch, 0xD4D1, 0xD7F9))	//Z
			{
				buf[pos++] = 'Z';
				//if (BETWEEN(ch, 0xD4FA, 0xD7C7))	//ZH
				//	buf[pos++] = 'H';
			}else
			{
				memcpy(buf + pos, p, 2);
				pos += 2;
			}
			p += 2;
		}
		else
		{
			buf[pos++] = *p++;
		}
	}
	buf[pos] = 0;
	return buf;
}


TCHAR* FormatInteger(TCHAR* buf, int value, const TCHAR* defaultIfZero /*= NULL*/)
{
	if (!value)
	{
		return _tcscpy(buf, defaultIfZero ? defaultIfZero : TEXT(""));
	}

	_stprintf(buf, TEXT("%d"), value);
	return buf;
}


TCHAR* FormatFloat(TCHAR* buf, double value, int nDec, bool bTrimZero /*= true*/, TCHAR cSplit /*= 0*/, const TCHAR* defaultIfZero /*= NULL*/)
{
	if (!value)
	{
		return _tcscpy(buf, defaultIfZero ? defaultIfZero : TEXT(""));
	}

	TCHAR fmt[16];
	_stprintf(fmt, TEXT("%%.%df"), nDec);
	int len = _stprintf(buf, fmt, value);
	if (bTrimZero)
	{
		TCHAR* p = buf + len - 1;
		while (p > buf && *p == TEXT('0')) p--;
		if (*p == TEXT('.'))
			*p = 0;
		else
			*(p + 1) = 0;
	}
	if (cSplit)
	{
		TCHAR* p = _tcschr(buf, TEXT('.'));
		if (!p)
			p = buf + len - 4;
		else
			p -= 3;
		while (p > buf)
		{
			memmove(p + 1, p, sizeof(TCHAR) * (len + buf - p + 1));
			*p = cSplit;
			p -= 3;
			len++;
		}
	}
	return buf;
}

bool IsFileExist(TCHAR* path)
{
	return ::GetFileAttributes(path) != DWORD(-1);
}

typedef   struct   _IDSECTOREX   {   
	USHORT   wGenConfig;   
	USHORT   wNumCyls;   
	USHORT   wReserved;   
	USHORT   wNumHeads;   
	USHORT   wBytesPerTrack;   
	USHORT   wBytesPerSector;   
	USHORT   wSectorsPerTrack;   
	USHORT   wVendorUnique[3];   
	CHAR   sSerialNumber[20];   
	USHORT   wBufferType;   
	USHORT   wBufferSize;   
	USHORT   wECCSize;   
	CHAR   sFirmwareRev[8];   
	CHAR   sModelNumber[40];   
	USHORT   wMoreVendorUnique;   
	USHORT   wDoubleWordIO;   
	USHORT   wCapabilities;   
	USHORT   wReserved1;   
	USHORT   wPIOTiming;   
	USHORT   wDMATiming;   
	USHORT   wBS;   
	USHORT   wNumCurrentCyls;   
	USHORT   wNumCurrentHeads;   
	USHORT   wNumCurrentSectorsPerTrack;   
	ULONG   ulCurrentSectorCapacity;   
	USHORT   wMultSectorStuff;   
	ULONG   ulTotalAddressableSectors;   
	USHORT   wSingleWordDMA;   
	USHORT   wMultiWordDMA;   
	BYTE   bReserved[128];   
}IDSECTOREX;

bool GetHardDiskNum(char* szDiskNo)
{
	DWORD size = 0;
	GETVERSIONINPARAMS vers;
	TCHAR path[MAX_PATH];
	for (int i = 0; i < 4; i++)
	{
		_stprintf(path, TEXT("\\\\.\\PHYSICALDRIVE%d"), i);
		HANDLE hFile = CreateFile(path,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,0,0);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			continue;
		}
		ZeroMemory(&vers,sizeof(vers));
		if   (!DeviceIoControl(hFile,0x00074080,NULL,0,&vers,sizeof(vers),&size,NULL))
		{
			CloseHandle(hFile);
			continue;
		}
		//If   IDE   identify   command   not   supported,   fails
		if   (!(vers.fCapabilities&1))
		{
			TRACE("Error:   IDE   identify   command   not   supported. ");
			CloseHandle(hFile);
			return false;
		}   
		//Identify   the   IDE   drives
		SENDCMDINPARAMS inPara = {0};
		if (i & 1)
		{
			inPara.irDriveRegs.bDriveHeadReg=0xb0;
		}else
		{
			inPara.irDriveRegs.bDriveHeadReg=0xa0;
		}
		if (vers.fCapabilities&(16>> i))//We   don 't   detect   a   ATAPI   device.
		{
			TRACE("Drive   %d   is   a   ATAPI   device,   we   don 't   detect   it \n", i + 1);
			CloseHandle(hFile);
			continue;
		}else
		{
			inPara.irDriveRegs.bCommandReg=0xec;
		}
		inPara.bDriveNumber=i;
		inPara.irDriveRegs.bSectorCountReg=1;
		inPara.irDriveRegs.bSectorNumberReg=1;
		SENDCMDOUTPARAMS* out = (SENDCMDOUTPARAMS*)new char[sizeof(SENDCMDOUTPARAMS) + 512];
		ZeroMemory(out,sizeof(SENDCMDOUTPARAMS) + 512);
		inPara.cBufferSize=512;
		if   (!DeviceIoControl(hFile,0x0007c088 ,&inPara,sizeof(inPara),out,sizeof(SENDCMDOUTPARAMS)+512,&size,NULL))
		{
			TRACE("DeviceIoControl   failed:DFP_RECEIVE_DRIVE_DATA\n");
			delete[] (char*)out;
			CloseHandle(hFile);
			return false;
		}
		CloseHandle(hFile);
		IDSECTOREX* phdinfo=(IDSECTOREX*)out->bBuffer;
		for (char* p = phdinfo->sSerialNumber + sizeof(phdinfo->sSerialNumber) - 1; p >= phdinfo->sSerialNumber; p--)
		{
			if (*p == ' ') continue;
			*szDiskNo++ = *p;
		}
		*szDiskNo = 0;
		delete[] (char*)out;
		return true;
	}
	return false;
}

bool GetMacAddress(char* szMac)
{
	bool bRet = false;
	ULONG size = 0;
	ULONG ret = ::GetAdaptersInfo(NULL, &size);//第一次是取缓存大小
	if (ret == ERROR_BUFFER_OVERFLOW)
	{
		PIP_ADAPTER_INFO pInfo = (PIP_ADAPTER_INFO)new char[size];
		if (!GetAdaptersInfo(pInfo, &size))		//第二次才取实际信息
		{
			sprintf(szMac, "%02X%02X%02X%02X%02X%02X",
				pInfo->Address[0],
				pInfo->Address[1],
				pInfo->Address[2],
				pInfo->Address[3],
				pInfo->Address[4],
				pInfo->Address[5]);
			bRet = true;
		}
		delete[] (char*)pInfo;
	}
	return bRet;
}

bool GetCpuName(char* szCpu, int len)
{
	HKEY hKey = NULL;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		DWORD dwSize = len;
		LSTATUS s = RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, (byte*)szCpu, &dwSize);
		RegCloseKey(hKey);
		return s == ERROR_SUCCESS;
	}
	return false;
}

bool GetFileVersion(const TCHAR* file, TCHAR* version, int size)
{
	bool bRet = false;
	DWORD dwSize = GetFileVersionInfoSize(file, NULL);
	if (!dwSize) return bRet;

	char* szVersion = new char[dwSize];
	if (GetFileVersionInfo(file, 0, dwSize, szVersion))
	{
		DWORD* lang = NULL;
		UINT len = 0;
		if (VerQueryValue(szVersion, TEXT("\\VarFileInfo\\Translation"), (void**)&lang, &len) && len >= sizeof(DWORD))
		{
			TCHAR* szVer = NULL;
			TCHAR szBlock[128];
			_stprintf(szBlock, TEXT("\\StringFileInfo\\%04x%04x\\ProductVersion"), LOWORD(*lang), HIWORD(*lang));
			if(VerQueryValue(szVersion, szBlock, (void**)&szVer, &len))     
			{
				_tcsncpy(version, szVer, size);
				bRet = true;
			}
		}
	}
	delete[] szVersion;
	return bRet;
}


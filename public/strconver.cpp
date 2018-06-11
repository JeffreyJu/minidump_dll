#include "stdafx.h"
#include "strconver.h"
#include <atlbase.h>  
#include <time.h>

//#define ToWideChar(A,B) ToWideChar(A,B,_countof(B)-1)

bool StrIsDecNum(const TCHAR* strNum, int size)
{
	if(strNum == NULL)
		return FALSE;
	for(int i=0;i<size;i++)
	{
		if(strNum[i] == '\0')
			return TRUE;
		if(strNum[i]>'9' || strNum[i]<'0')
			return FALSE;
	}
	return true;
}

bool   StrIsDecNumA(const char* strNum, int size)
{
	if(strNum == NULL)
		return FALSE;
	for(int i=0;i<size;i++)
	{
		if(strNum[i] == '\0')
			return TRUE;
		if(strNum[i]>'9' || strNum[i]<'0')
			return FALSE;
	}
	return true;
}

bool StrIsDecFloat( const TCHAR* strNum, int size )
{
	unsigned int len = 0;
	if (strNum == NULL)
	{
		return false;
	}
	for (int i=0;i<size;i++)
	{
		if (strNum[i] == '\0')
		{
			return true;
		}
		if (strNum[i] == '.')
		{
			len++;
		}
		if ((strNum[i]<'0' || strNum[i]>'9' || len>1) && strNum[i] != '.')
		{
			return false;
		}
	}
	return true;
}


bool StrIsDecFloatA( const TCHAR* strNum, int size )
{
	unsigned int len = 0;
	if (strNum == NULL)
	{
		return false;
	}
	for (int i=0;i<size;i++)
	{
		if (strNum[i] == '\0')
		{
			return true;
		}
		if (strNum[i] == '.')
		{
			len++;
		}
		if ((strNum[i]<'0' || strNum[i]>'9' || len>1) && strNum[i] != '.')
		{
			return false;
		}
	}
	return true;
}



char *ToMultiByte(const wchar_t* src,char *dest,INT32 destsize)
{
	int size=WideCharToMultiByte(CP_ACP,0,src,-1,NULL,0,NULL,NULL);
	char* temp = new char[size + 1];
	temp[size] = 0;
	if(!WideCharToMultiByte(CP_ACP,0,src,-1,temp,size,NULL,NULL))
	{
		delete[] temp;
		return NULL;
	}
	strncpy(dest,temp,destsize);
	delete[] temp;
	return dest;
}

wchar_t *ToWideChar(const char* src,wchar_t* dest,INT32 destsize)
{
	int size = MultiByteToWideChar(CP_ACP,0, src, -1, NULL, 0);
	wchar_t* wtemp = new wchar_t[size + 1];
	wtemp[size] = 0;
	if(!MultiByteToWideChar(CP_ACP,0, src, -1, wtemp, size))
	{
		delete[] wtemp;
		return NULL;
	}
	_tcsncpy(dest,wtemp,destsize);
	delete[] wtemp;
	return dest;
}

const wchar_t* time2Unicode(__time32_t time,wchar_t* szTime,INT32 nsize)
{
	tm tmtime;
	_localtime32_s(&tmtime,&time);
	_sntprintf(szTime,nsize -1,TEXT("%d/%d/%d %d:%d:%d"),tmtime.tm_year + 1900,tmtime.tm_mon + 1,tmtime.tm_mday,tmtime.tm_hour,tmtime.tm_min,tmtime.tm_sec);
	return szTime;
}

const char* time2Ansi(__time32_t time,char* szTime,INT32 nsize)
{
	tm tmtime;
	_localtime32_s(&tmtime,&time);
	_snprintf(szTime,nsize -1,"%d/%d/%d %d:%d:%d",tmtime.tm_year + 1900,tmtime.tm_mon + 1,tmtime.tm_mday,tmtime.tm_hour,tmtime.tm_min,tmtime.tm_sec);
	return szTime;
}

void UTF8ToUnicode(const char *pIn,WCHAR* pOut)
{
	char* uchar		= (char *)pOut;	         
	uchar[1]		= ((pIn[0] & 0x0F) << 4) + ((pIn[1] >> 2) & 0x0F);
	uchar[0]		= ((pIn[1] & 0x03) << 6) + (pIn[2] & 0x3F);
}	

void UTF8ToUnicode(const char *lpIn,const int nLen, WCHAR *lpOut)
{
	int i	=	0;
	int j	=	0;
	while( i < nLen )
	{
		WCHAR unicode = 0;
		if( lpIn[i] > 0 )
		{
			*((char *)&unicode) = lpIn[i++];                       
		}
		else                 
		{
			UTF8ToUnicode(lpIn + i,&unicode);
			i += 3;    
		}
		lpOut[j++] = unicode;
	}
	lpOut[j] = '\0';	
}

void UTF8ToUnicode(const std::string sIn, std::wstring& sOut)
{
	WCHAR wsOut[1024*8];
	UTF8ToUnicode((char*)sIn.c_str(), sIn.length(), wsOut);
	sOut	=	std::wstring(wsOut);
}
	
int UnicodeToUTF8(const WCHAR* wstrIn,char* buff,int vLen)
{
	int wLen	=	wcslen(wstrIn);
	int i=0;
	#define putchar(a) buff[i++] = (BYTE)a;
	for(int j=0;(DWORD)j<wcslen(wstrIn);j++)
	{			
		WCHAR c = wstrIn[j];
		if(c<0x80)
		{
			putchar(c);
		}
		else if(c<0x800)
		{
			putchar (0xC0 | c>>6);
			putchar (0x80 | c & 0x3F);
		}
		else if(c<0x10000)
		{
			putchar (0xE0 | c>>12);
			putchar (0x80 | c>>6 & 0x3F);
			putchar (0x80 | c & 0x3F);
		}
		else if(c<0x200000)
		{
			putchar (0xF0 );
			putchar (0x80 | c>>12 & 0x3F);
			putchar (0x80 | c>>6 & 0x3F);
			putchar (0x80 | c & 0x3F);
		}
	}
	buff[i++] = '\0';
	return i*2;
}

const char * UTF82ASC(const char *srcbuf,char * outbuf,int outlen)
{
	int len=MultiByteToWideChar(CP_UTF8,0,srcbuf,-1,NULL,NULL);
	if ( len <= 0 )
		return outbuf;

	unsigned short * wszASC = new unsigned short[len+1];
	memset(wszASC, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8,0,srcbuf,-1,(wchar_t*)wszASC,len);

	len=WideCharToMultiByte(CP_ACP,0,(wchar_t*)wszASC,-1,NULL,0,NULL,NULL);
	char * szASC=new char[len+1];
	memset(szASC,0,len+1);
	WideCharToMultiByte(CP_ACP,0,(wchar_t*)wszASC,-1,szASC,len,NULL,NULL);

	strncpy(outbuf,szASC,outlen-1);

	delete [] wszASC;
	delete [] szASC;

	return outbuf;
}

const char * ASC2UTF8(const char* srcbuf,char * outbuf,int outlen)
{
	int len=MultiByteToWideChar(CP_ACP, 0, srcbuf, -1, NULL,0);
	if ( len <= 0 )
		return outbuf;

	unsigned short * wszUtf8 = new unsigned short[len+1];
	memset(wszUtf8, 0, len * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, srcbuf, -1, (wchar_t*)wszUtf8, len);

	len = WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)wszUtf8, -1, NULL, 0, NULL, NULL); 
	char *szUtf8=new char[len + 1];
	memset(szUtf8, 0, len + 1);
	WideCharToMultiByte (CP_UTF8, 0, (wchar_t*)wszUtf8, -1, szUtf8, len, NULL,NULL);

	strncpy(outbuf,szUtf8,outlen-1);

	delete[] szUtf8;
	delete[] wszUtf8;
	return outbuf;
}

void Gb2312ToUtf8(const char *pcGb2312, char* Utf8,int& Len)
{
	int nUnicodeLen = MultiByteToWideChar(CP_ACP, 0, pcGb2312, -1, NULL, 0);
	wchar_t * pcUnicode = new wchar_t[nUnicodeLen+1]; 
	memset(pcUnicode, 0, nUnicodeLen * 2 + 2); 
	MultiByteToWideChar(CP_ACP, 0, pcGb2312, -1, pcUnicode, nUnicodeLen);
	int nUtf8Len = WideCharToMultiByte(CP_UTF8, 0, pcUnicode, -1, NULL, 0, NULL, NULL);
	char *pcUtf8=new char[nUtf8Len + 1]; 
	memset(pcUtf8, 0, nUtf8Len + 1);   
	WideCharToMultiByte(CP_UTF8, 0, pcUnicode, -1, pcUtf8, nUtf8Len, NULL, NULL);
	memset(Utf8,0,Len);
	if(nUtf8Len < Len)
		Len = nUtf8Len;
	strncpy(Utf8,pcUtf8,Len);
	delete[] pcUtf8; 
	delete[] pcUnicode; 
}

void Utf8ToGb2312(const char *pcUtf8, char* GB2312,int Len)
{
	int nUnicodeLen = MultiByteToWideChar(CP_UTF8, 0, pcUtf8, -1, NULL, 0);
	wchar_t * pcUnicode = new wchar_t[nUnicodeLen+1]; 
	memset(pcUnicode, 0, nUnicodeLen * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, pcUtf8, -1, pcUnicode, nUnicodeLen);
	int nGb2312Len = WideCharToMultiByte(CP_ACP, 0, pcUnicode, -1, NULL, 0, NULL, NULL);
	char *pcGb2312=new char[nGb2312Len + 1]; 
	memset(pcGb2312, 0, nGb2312Len + 1);
	WideCharToMultiByte(CP_ACP, 0, pcUnicode, -1, pcGb2312, nGb2312Len, NULL, NULL);
	memset(GB2312,0,Len);
	if(nGb2312Len < Len)
		Len = nGb2312Len;
	strncpy(GB2312,pcGb2312,Len);
	delete[] pcGb2312; 
	delete[] pcUnicode;
}

static UINT32 CRC32[256];

//初始化表
void crc32init_table()
{
	INT32   i,j;
	UINT32   crc;
	for(i = 0;i < 256;i++)
	{
		crc = i;
		for(j = 0;j < 8;j++)
		{
			if(crc & 1)
			{
				crc = (crc >> 1) ^ 0xEDB88320;
			}
			else
			{
				crc = crc >> 1;
			}
		}
		CRC32[i] = crc;
	}
}

//crc32实现函数
UINT32 crc32( BYTE *buf, INT32 len)
{
	UINT32 ret = 0xFFFFFFFF;
	INT32   i;
	for(i = 0; i < len;i++)
	{
		ret = CRC32[((ret & 0xFF) ^ buf[i])] ^ (ret >> 8);
	}
	ret = ~ret;
	return ret;
}

void SplitString(char* str, char split, std::vector<char*>& vecString)
{
	char* p = str;
	vecString.clear();
	for (char* q = strchr(p, split); q; p = q + 1, q = strchr(p, split))
	{
		*q = 0; //强制写入一个\0 危险
		vecString.push_back(p);
	}

	if(*p) vecString.push_back(p);;
}

const char* GetRootPath()
{
	static char s_szMoundlePath[MAX_PATH] = {0};
	if(s_szMoundlePath[0] == 0)
	{
		GetModuleFileNameA(AfxGetApp()->m_hInstance,s_szMoundlePath,sizeof(s_szMoundlePath) - 1);
		char* pos = strrchr(s_szMoundlePath,'\\');
		*pos = 0;
	}
	return s_szMoundlePath;
}

const wchar_t* GetRootPathW()
{
	static WCHAR s_wszMoundlePath[MAX_PATH] = {0};
	if(s_wszMoundlePath[0] == 0)
	{
		GetModuleFileNameW(AfxGetApp()->m_hInstance,s_wszMoundlePath,_countof(s_wszMoundlePath));
		wchar_t* pos = wcsrchr(s_wszMoundlePath,L'\\');
		*pos = 0;
	}
	return s_wszMoundlePath;
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

void GetAbsoluteFileName(const TCHAR* path,TCHAR*name,int namesize,TCHAR* extname,int extsize)
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

TCHAR* GetModuleFileVersion(HMODULE hModule, TCHAR* szVersion, INT32 nBuffLen)
{
	TCHAR path[MAX_PATH] = {0};
	::GetModuleFileName(hModule, path, sizeof(path) - 1);
	DWORD reserved = 0;
	DWORD size = ::GetFileVersionInfoSize(path, &reserved);
	char* szBuffer = new char[size];
	*szVersion = 0;
	if (::GetFileVersionInfo(path, 0, size, szBuffer))
	{
		VS_FIXEDFILEINFO *pVi = NULL;
		UINT len = 0;
		if (::VerQueryValue(szBuffer, TEXT("\\"), (void**)&pVi, &len))
		{
			_stprintf_s(szVersion, nBuffLen, TEXT("%d.%d.%d.%d"), HIWORD(pVi->dwFileVersionMS), LOWORD(pVi->dwFileVersionMS), HIWORD(pVi->dwFileVersionLS), LOWORD(pVi->dwFileVersionLS));
		}
	}
	delete[] szBuffer;
	return szVersion;
}

bool CreateDir(const char* name)
{
	WIN32_FIND_DATAA   FileData; 
	HANDLE hand = FindFirstFileA(name, &FileData);
	if (hand != INVALID_HANDLE_VALUE && FileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
	{
		FindClose(hand);
		return true;
	}

	SECURITY_ATTRIBUTES attrib;
	attrib.bInheritHandle = FALSE;
	attrib.lpSecurityDescriptor = NULL;
	attrib.nLength = sizeof(SECURITY_ATTRIBUTES);

	BOOL bRet = CreateDirectoryA(name, &attrib);
	FindClose(hand);
	return bRet!= 0;
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
				GetAbsoluteFileName(FindFileData.cFileName,szName,MAX_PATH,szExt,MAX_PATH);
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

BOOL IniMd5ToFile(const char* FileName, std::string& strMD5)
{
	using namespace std;
	ifstream in(FileName, ios::binary);  
	if (!in)
	{
		return false;
	}
	MD5 md5;  
	std::streamsize length;  
	char buffer[1024];
	while (!in.eof()) {
		in.read(buffer, 1024);  
		length = in.gcount();  
		if (length > 0)  
			md5.update(buffer, length);  
	}
	in.close();  
	strMD5 =  md5.toString();
	return true;
}

bool Md5FileVerify(const char *FileName, const char* md5Dst)
{
	if(FileName==NULL || FileName[0]==0)
		return false;

	std::string md5Src;
	if(!IniMd5ToFile(FileName, md5Src))
		return false;
	if(0 != strcmp(md5Src.c_str(), md5Dst))
		return false;
	return true;
}

UINT32 unencry( const char* userid, const INT32 size, const UINT32 accredit )
{
	//根据用户名获取密钥
	UINT32 key = 0;
	int i=0;
	while(userid[i]!=0 && i<=size)
	{
		key += userid[i];
		i++;
	}
	//解密权限
	return key^accredit;
}

time_t String2Time(const char* szOut)
{
	tm local_time;
	INT32 nyear,nmonth,nday,nhour,nmin,nsec;
	sscanf(szOut,"%d/%d/%d %d:%d:%d",&nyear,&nmonth,&nday,&nhour,&nmin,&nsec);
	local_time.tm_year = nyear - 1900;
	local_time.tm_mon = nmonth - 1;
	local_time.tm_mday = nday;
	local_time.tm_hour = nhour;
	local_time.tm_min = nmin;
	local_time.tm_sec = nsec;
	return mktime(&local_time);
}

__time32_t String2Time32(const char* szOut)
{
	tm local_time;
	INT32 nyear,nmonth,nday,nhour,nmin,nsec;
	sscanf(szOut,"%d/%d/%d %d:%d:%d",&nyear,&nmonth,&nday,&nhour,&nmin,&nsec);
	local_time.tm_year = nyear - 1900;
	local_time.tm_mon = nmonth - 1;
	local_time.tm_mday = nday;
	local_time.tm_hour = nhour;
	local_time.tm_min = nmin;
	local_time.tm_sec = nsec;
	return _mktime32(&local_time);
}

void String2Time(const char* szTime,unsigned long & hour,unsigned long  & min,unsigned long & sec)
{
	sscanf(szTime,"%lu:%lu:%lu",&hour,&min,&sec);
	return ;
}

void String2DateTime(const char* szTime,unsigned long & year,unsigned long & month,unsigned long & day,unsigned long & hour,unsigned long & min,unsigned long & sec)
{
	sscanf(szTime,"%lu-%lu-%u %lu:%lu:%lu",&year,&month,&day,&hour,&min,&sec);
	return ;
}


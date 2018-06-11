#pragma once
#include <string>
#include <vector>
#include "md5.h"

#pragma comment(lib,"Version.lib")
#ifdef _UNICODE
typedef std::wstring STDSTRING;
#else
typedef std::string STDSTRING;
#endif
	
	bool   StrIsDecNum(const TCHAR* strNum, int size);
	bool   StrIsDecNumA(const char* strNum, int size);

	bool   StrIsDecFloat(const TCHAR* strNum, int size);
	bool   StrIsDecFloatA(const char* strNum, int size);

	bool   GetFileVersion(const TCHAR* file, TCHAR* version, int size);
	TCHAR* GetModuleFileVersion(HMODULE hModule, TCHAR* szVersion, INT32 nBuffLen);

	unsigned short * _wcsu16_wcs2utf16(wchar_t *wcs, unsigned short * utf16, int len);

	char *ToMultiByte(const wchar_t* src,char *dest,INT32 destsize);
	wchar_t *ToWideChar(const char* src,wchar_t* dest,INT32 destsize);

	void UTF8ToUnicode(const char *lpIn,const int nLen, wchar_t *lpOut);
	void UTF8ToUnicode(const std::string sIn, std::wstring& sOut);

	const char * UTF82ASC(const char *srcbuf,char * outbuf,int outlen);
	const char * ASC2UTF8(const char* srcbuf,char * outbuf,int outlen);
	
	//void UnicodeToUTF8(std::wstring wsIn,std::string& sOut);
	int  UnicodeToUTF8(const wchar_t* wstrIn,char* buff,int vLen);

	UINT32 crc32( BYTE *buf, INT32 len);
	void crc32init_table();
	
	void Gb2312ToUtf8(const char *pcGb2312, char* Utf8, int& Len);
	void Utf8ToGb2312(const char *pcUtf8, char* GB2312, int Len);

	void SplitString(char* str, char split, std::vector<char*>& vecString);

	const char* GetRootPath();
	const wchar_t* GetRootPathW();

	bool CreateDir(const char* name);

	const wchar_t* time2Unicode(__time32_t time,wchar_t* szTime,INT32 nsize);
	const char* time2Ansi(__time32_t time,char* szTime,INT32 nsize);
	time_t String2Time(const char* szOut);
	__time32_t String2Time32(const char* szOut);
	void String2Time(const char* szTime,unsigned long & hour,unsigned long  & min,unsigned long & sec);

	void String2DateTime(const char* szTime,unsigned long & year,unsigned long & month,unsigned long & day,unsigned long & hour,unsigned long & min,unsigned long & sec);

	void GetAbsoluteFileName(const TCHAR* path,TCHAR*name,int namesize,TCHAR* extname,int extsize);
	bool TraversalFiles(const TCHAR* path,std::vector<STDSTRING>& Files,TCHAR* Folder ,const TCHAR* ext,int rettype );
	//解密授权
	UINT32  unencry(const char* userid, const INT32 size, const UINT32 accredit);
#ifdef MD5_H
	//根据文件生成，校验md5，依赖md5.h
	BOOL IniMd5ToFile(const char* FileName, std::string& strMD5);
	bool Md5FileVerify(const char *FileName, const char* md5Dst);
#endif
// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../MiniDump/MiniDump.h"

#ifdef _DEBUG
#pragma comment(lib, "..//bin\\MiniDump.lib")
#else 
#pragma comment(lib, "..//bin\\MiniDump_D.lib")
#endif


int _tmain(int argc, _TCHAR* argv[])
{
	MiniDumpStartUp();
	//MiniDumpSetMailSender(_T("smtp.chinatransinfo.com"), _T("wangxulin@chinatransinfo.com"), _T("qwer1234"));
	//MiniDumpAddMailReceiver(_T("linlin-ring@163.com"));
	int ni = 112;
	char *p = NULL;
	*p = ni;
	return 0;
}


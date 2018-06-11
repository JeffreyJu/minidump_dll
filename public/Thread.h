//----------------------------------------------------------------------//
//                                                                      //
// MyJingle is a GoogleTalk compatible VoIP Client based on Jingle      //
//                                                                      //
// Copyright (C) 2006  bluehands GmbH & Co.mmunication KG               //
//                                                                      //
// This program is free software; you can redistribute it and/or modify //
// it under the terms of the GNU General Public License as published by //
// the Free Software Foundation; either version 2 of the License, or    //
// (at your option) any later version.                                  //
//                                                                      //
// This program is distributed in the hope that it will be useful, but  //
// WITHOUT ANY WARRANTY; without even the implied warranty of           //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     //
// General Public License for more details.                             //
//                                                                      //
// You should have received a copy of the GNU General Public License    //
// along with this program; if not, write to the Free Software          //
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA //
//                                                                      //
//----------------------------------------------------------------------//

#if !defined(Thread_h_INCLUDED)
#define Thread_h_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#define INITGUID
#include <winsock2.h>
#include "windows.h"
class  CVThread
{
public:
	CVThread(void);
	virtual ~CVThread(void);
	void Run();		//开启一个线程
	DWORD virtual Stop(INT32 time = -1);	//停止线程
	enum Prio{ LOWEST, LOW, NORMAL, HIGH, HIGHEST, REALTIME };
	INT32 SetPriority(INT32 prio);
	virtual void Svc();	//线程的执行函数

	//获 得线程句柄
	HANDLE		GetHandle() const;

	//等待
	void		Wait() const;
	INT32		Wait(DWORD timeoutM) const;

	//开始线程
	void		Start();

	//终止
	void		Terminate(DWORD exitCode = 0);

	void		Close();
protected:
	bool m_Stop; 
	HANDLE m_ThreadHandle;
	static DWORD WINAPI ThreadJumpIn(LPVOID threadParam);
};

class Mutex
{
public:
	Mutex();
	~Mutex();
	void Init();
	void Exit();
	void Aquire();
	void Release();
protected:
	CRITICAL_SECTION m_Mutex;
private:
	bool m_Initialized;
};


#endif // !defined(Thread_h_INCLUDED)

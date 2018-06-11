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
#include "stdafx.h"
#include "Thread.h"


CVThread::CVThread(void):m_Stop(true),m_ThreadHandle(NULL)
{
}

CVThread::~CVThread(void)
{
}

void CVThread::Run()
{
	if(m_ThreadHandle == NULL)
	{
		m_Stop = false;
		m_ThreadHandle = CreateThread(
			0,				//LPSECURITY_ATTRIBUTES lpThreadAttributes
			0,				//SIZE_T dwStackSize
			ThreadJumpIn,	//LPTHREAD_START_ROUTINE lpStartAddress
			this,			//LPVOID lpParameter
			0,				//DWORD dwCreationFlags
			0				//LPDWORD lpThreadId
			);
	}
}


DWORD WINAPI CVThread::ThreadJumpIn(LPVOID threadParam)
{
	((CVThread *) threadParam)->Svc();
	return 0;
}

DWORD CVThread::Stop(INT32 time)
{
	DWORD ret = 0;
	m_Stop = true;
	if(m_ThreadHandle != 0)
	{
		ret = WaitForSingleObject(m_ThreadHandle, time);	//等待一段时间，或者线程有通知
		CloseHandle(m_ThreadHandle);
		m_ThreadHandle = 0;
	}
	
	return ret;
}

void CVThread::Svc()
{
	Sleep(1000);
}

HANDLE CVThread::GetHandle() const
{
	return m_ThreadHandle;
}

void CVThread::Wait() const   
{
	if (!Wait(INFINITE))
	{
	//	TRACE(_T("Unexpected timeOut on infinite wait"));
	}
}

INT32 CVThread::Wait(DWORD timeoutM) const
{
	//BOOL			Ok;

	DWORD result = ::WaitForSingleObject(m_ThreadHandle, timeoutM);

	/*
	if (result == WAIT_TIMEOUT)			Ok = TRUE;
	else if (result == WAIT_OBJECT_0)	Ok = TRUE;
	else Ok = FALSE;
	*/

	return result;
}

void CVThread::Terminate(DWORD exitCode /* = 0 */)
{
	if(m_ThreadHandle != 0 )
	{
		if (!::TerminateThread(m_ThreadHandle, exitCode))
		{
		
		}
	}

}

void CVThread::Close()
{
	if(m_ThreadHandle != 0 )
	{
		CloseHandle(m_ThreadHandle);
		m_ThreadHandle = 0;
	}
}

INT32 CVThread::SetPriority(INT32 prio)
{
	return SetThreadPriority(m_ThreadHandle, THREAD_PRIORITY_IDLE);
}


Mutex::Mutex():
m_Initialized(false)
{
}

Mutex::~Mutex()
{
}

void Mutex::Init()
{
#if defined (WIN32)
	InitializeCriticalSection(&m_Mutex);
#else
	pthread_mutex_init(&m_Mutex,0);
#endif
	m_Initialized = true;
}

void Mutex::Exit()
{
	Aquire();
	m_Initialized = false;
	Release();
#if defined (WIN32)
	DeleteCriticalSection(&m_Mutex);
#else
	pthread_mutex_destroy(&m_Mutex);
#endif
}

void Mutex::Aquire()
{
	if(!m_Initialized)
	{
		return;
	}

	EnterCriticalSection(&m_Mutex);

}

void Mutex::Release()
{
	LeaveCriticalSection(&m_Mutex);
}

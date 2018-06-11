#ifndef _LOCK_H_
#define _LOCK_H_

class CLock
{
public:
	CLock();
	~CLock();
	void Lock();
	void Unlock();
private:
	CRITICAL_SECTION m_cs;
};

#endif
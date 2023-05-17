//***************************************************************************//
//* 版权所有  www.mediapro.cc
//*
//* 内容摘要：跨平台的Event实现
//*	
//* 当前版本：V1.0		
//* 作    者：mediapro
//* 完成日期：2015-12-26
//**************************************************************************//
#if !defined(SDEVENT_H)
#define SDEVENT_H

#ifndef WIN32
#include <pthread.h>
#endif

class CSDEvent
{
public:
	CSDEvent(const char *strName = NULL);
	virtual ~CSDEvent();

public:
	BOOL CreateSdEvent();
	void ReleaseSdEvent();

public:
	BOOL wait();
	BOOL waittime(int dwMilliseconds);
	BOOL post();
	void Reset();
private:
	void  *m_pEvent;
	char  m_strEventName[64];
#ifndef WIN32
	pthread_mutex_t m_event_mutex;
	pthread_cond_t m_event_cond;
	bool m_event_status;
#endif
};

#endif // !defined(SDEVENT_H)

//***************************************************************************//
//* 版权所有  www.mediapro.cc
//*
//* 内容摘要：跨平台Mutex实现
//*	
//* 当前版本：V1.0		
//* 作    者：mediapro
//* 完成日期：2015-12-26
//**************************************************************************//

#if !defined(SDMUTEX_H)
#define SDMUTEX_H

class CSDMutex  
{
public:
	CSDMutex(void *cs);
	virtual ~CSDMutex();

public:
	static void *CreateObject();
	static void RealseObject(void*m);
#ifdef ANDROID
	bool lock();
	bool Unlock();
#endif // ANDROID
private:
	void *m_cs;
};

class CSDMutexX
{
public:
	CSDMutexX();
	virtual ~CSDMutexX();
	
public:
	bool lock();
	bool Unlock();
	
private:
	void *m_cs;
};




#endif // !defined(SDMUTEX_H)

//***************************************************************************//
//* 版权所有  www.mediapro.cc
//*
//* 内容摘要：H264裸文件帧级读取
//*	
//* 当前版本：V1.0		
//* 作    者：mediapro
//* 完成日期：2020-03-13
//**************************************************************************//
#ifndef __SD_H264_FILE_PARSE_H__
#define __SD_H264_FILE_PARSE_H__
#include <sys/types.h>
#include <sys/stat.h>
#include "SDLog.h"

class CSDH264FilePase
{
public:
	CSDH264FilePase();
	virtual ~CSDH264FilePase(void);

	BOOL Start(const char* strH264FileUrl, unsigned int unFps, BOOL bRepet);
	void Stop();

	unsigned int ReadH264RawFrame(unsigned char* pucFrame, unsigned int unSize, unsigned int* punDts, unsigned int* punPts);


private:
	void* m_pcsExe;
	char* m_pH264TotalFileData;
	char* m_pH264CurrFrame;
	char  m_Sps[1024];
	int	  m_nSpsLen;
	char  m_Pps[1024];
	int	  m_nPpsLen;
	char  m_Sei[1024];
	int	  m_nSeiLen;
	BOOL  m_bGotSps;
	BOOL  m_bGotPps;
	int	  m_nFileId;
	off_t m_nFileSize;
	unsigned int m_unFps;
	unsigned int m_unDts;
	unsigned int m_unPts;
	BOOL  m_bRepet;
	BOOL  m_bStoped;
};

#endif	// __SD_H264_FILE_PARSE_H__
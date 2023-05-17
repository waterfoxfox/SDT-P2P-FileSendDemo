//***************************************************************************//
//* 版权所有  www.mediapro.cc
//*
//* 内容摘要：H264裸文件帧级读取，目前只支持单帧单Slice
//*	
//* 当前版本：V1.0		
//* 作    者：mediapro
//* 完成日期：2020-03-13
//**************************************************************************//
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "SDLog.h" 
#include "SDMutex.h"
#include "SDH264FileParse.h"
#include "SDFileParseCommon.h"

static int read_h264_frame_nalu(char* data, int size, char** pp, int* pnb_start_code, char** frame, int* frame_size)
{
	char* p = *pp;

	if (!srs_h264_startswith_annexb(p, size - (p - data), pnb_start_code)) 
	{
		SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_ERROR, "h264 file maybe invalid, not start with nalu start code");
		return -1;
	}

	*frame = p;
	p += *pnb_start_code;

	for (;p < data + size; p++) 
	{
		if (srs_h264_startswith_annexb(p, size - (p - data), NULL)) 
		{
			break;
		}
	}

	*pp = p;
	*frame_size = p - *frame;
	if (*frame_size <= 0) 
	{
		SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_ERROR, "h264 raw data invalid");
		return -1;
	}

	return 0;
}



CSDH264FilePase::CSDH264FilePase()
{
	m_pcsExe = CSDMutex::CreateObject();
	m_bStoped = TRUE;
	m_nFileId = -1;
	m_nFileSize = 0;
	m_unFps = 25;
	m_unDts = 0;
	m_unPts = 0;
	m_pH264TotalFileData = NULL;
	m_pH264CurrFrame = NULL;
	m_nSpsLen = 0;
	m_nPpsLen = 0;
	m_nSeiLen = 0;
}

CSDH264FilePase::~CSDH264FilePase()
{
	m_bStoped = TRUE;
	if (m_pH264TotalFileData)
	{
		free(m_pH264TotalFileData);
		m_pH264TotalFileData = NULL;
	}

	if (m_nFileId >= 0)
	{
		close(m_nFileId);
		m_nFileId = -1;
	}

	if (m_pcsExe)
	{
		CSDMutex::RealseObject(m_pcsExe);
		m_pcsExe = NULL;
	}
}

BOOL CSDH264FilePase::Start(const char* strH264FileUrl, unsigned int unFps, BOOL bRepet)
{
	CSDMutex csExe(m_pcsExe);
	if ((unFps == 0) || (strH264FileUrl == NULL))
	{
		SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_ERROR, "Start H264 file parse failed with fps:%d url:%p!", unFps, strH264FileUrl);
		return FALSE;
	}

	m_bStoped = TRUE;
	m_bRepet = bRepet;
	m_unFps = unFps;
	m_unDts = 0;
	m_unPts = 0;
	m_nSpsLen = 0;
	m_nPpsLen = 0;
	m_nSeiLen = 0;

#ifndef O_BINARY
#define O_BINARY 0 
#endif	

	m_nFileId = open(strH264FileUrl, O_RDONLY|O_BINARY);
	if (m_nFileId < 0) 
	{
		SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_ERROR, "file: %s open failed!", strH264FileUrl);
		return FALSE;
	}

	m_nFileSize = lseek(m_nFileId, 0, SEEK_END);
	if (m_nFileSize <= 0) 
	{
		SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_ERROR, "file: %s is empty!", strH264FileUrl);
		close(m_nFileId);
		m_nFileId = -1;
		return FALSE;
	}

	m_pH264TotalFileData = (char*)malloc(m_nFileSize);
	if (!m_pH264TotalFileData) 
	{
		SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_ERROR, "alloc for h264 file failed with size:%d!", (int)m_nFileSize);
		close(m_nFileId);
		m_nFileId = -1;
		return FALSE;
	}
	m_pH264CurrFrame = m_pH264TotalFileData;

	lseek(m_nFileId, 0, SEEK_SET);
	size_t nb_read = 0;
	if ((nb_read = read(m_nFileId, m_pH264TotalFileData, m_nFileSize)) != m_nFileSize) 
	{
		SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_ERROR, "read h264 file: %s failed with size:%d!", strH264FileUrl, (int)m_nFileSize);
		close(m_nFileId);
		m_nFileId = -1;
		free(m_pH264TotalFileData);
		m_pH264TotalFileData = NULL;
		return FALSE;
	}

	SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_INFO, "file: %s  fps:%d  repet:%d  open success!", strH264FileUrl, unFps, bRepet);
	m_bStoped = FALSE;
	return TRUE;
}


void CSDH264FilePase::Stop()
{
	CSDMutex csExe(m_pcsExe);
	m_bStoped = TRUE;

	if (m_pH264TotalFileData)
	{
		free(m_pH264TotalFileData);
		m_pH264TotalFileData = NULL;
	}

	if (m_nFileId >= 0)
	{
		close(m_nFileId);
		m_nFileId = -1;
	}
	m_nFileSize = 0;
	m_nSpsLen = 0;
	m_nPpsLen = 0;
	m_nSeiLen = 0;
}

unsigned int CSDH264FilePase::ReadH264RawFrame(unsigned char* pucFrame, unsigned int unSize, unsigned int* punDts, unsigned int* punPts)
{
	CSDMutex csExe(m_pcsExe);
	if (m_bStoped == TRUE)
	{
		return 0;
	}

	if ((pucFrame == NULL) || (unSize == 0) || (punDts == NULL) || (punPts == NULL))
	{
		SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_ERROR, "ReadH264RawFrame failed with:%p %d %p %p!", pucFrame, unSize, punDts, punPts);
		return 0;
	}

	*punDts = m_unDts;
	*punPts = m_unPts;
	while (1)
	{
		//已经读到尾部视情况重新开始
		if (m_pH264CurrFrame >= m_pH264TotalFileData + m_nFileSize)
		{
			if (m_bRepet == TRUE)
			{
				m_pH264CurrFrame = m_pH264TotalFileData;
			}
			else
			{
				SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_WARNING, "file: read to end then call ReadH264RawFrame!");
				return 0;
			}
		}

		char* data = NULL;
		int size = 0;
		int nb_start_code = 0;
		if (read_h264_frame_nalu(m_pH264TotalFileData, (int)m_nFileSize, &m_pH264CurrFrame, &nb_start_code, &data, &size) >= 0) 
		{

			// nalu type.
			// 7: SPS, 8: PPS, 5: I Frame, 1: P Frame, 9: AUD, 6: SEI
			char nalu_type = (char)data[nb_start_code] & 0x1f;

			//SPS PPS SEI NALU存下来等待下一帧数据一并送出
			if (nalu_type == 7)
			{
				if (size <= sizeof(m_Sps))
				{
					memcpy(m_Sps, data, size);
					m_nSpsLen = size;
				}
				else
				{
					SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_ERROR, "sps is too large:%d!", size);
					return 0;
				}
			}
			else if (nalu_type == 8)
			{
				if (size <= sizeof(m_Pps))
				{
					memcpy(m_Pps, data, size);
					m_nPpsLen = size;
				}
				else
				{
					SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_ERROR, "pps is too large:%d!", size);
					return 0;
				}
			}
			else if (nalu_type == 6)
			{
				if (size <= sizeof(m_Sei))
				{
					memcpy(m_Sei, data, size);
					m_nSeiLen = size;
				}
				else
				{
					SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_ERROR, "sei is too large:%d, will dropped!", size);
				}
			}
			else if ((nalu_type == 5) || (nalu_type == 1))
			{
				//确保从IDR开始输出
				if ((m_nSpsLen != 0) && (m_nPpsLen != 0))
				{
					//根据帧率模拟PTS DTS生成
					m_unDts += 1000 / m_unFps;
					m_unPts = m_unDts;

					if (nalu_type == 5)
					{
						int nTotal = m_nSpsLen + m_nPpsLen + m_nSeiLen + size;
						if ((int)unSize >= nTotal)
						{
							//IDR帧时连同SPS PPS SEI一起输出
							memcpy(pucFrame, m_Sps, m_nSpsLen);
							memcpy(pucFrame + m_nSpsLen, m_Pps, m_nPpsLen);
							if (m_nSeiLen)
							{
								memcpy(pucFrame + m_nSpsLen + m_nPpsLen, m_Sei, m_nSeiLen);
							}
							memcpy(pucFrame + m_nSpsLen + m_nPpsLen + m_nSeiLen, data, size);
							return nTotal;
						}
						else
						{
							//丢弃到下一个IDR
							SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_ERROR, "I frame is too large:%d, will dropped!", nTotal);
							m_nSpsLen = 0;
							m_nPpsLen = 0;
						}

					}
					else
					{
						if ((int)unSize >= size)
						{
							memcpy(pucFrame, data, size);
							return size;
						}
						else
						{
							//丢弃到下一个IDR
							SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_ERROR, "P frame is too large:%d, will dropped!", size);
							m_nSpsLen = 0;
							m_nPpsLen = 0;
						}
					}
						
				}
			}
			else if (nalu_type == 9)
			{
				//drop aud
			}
			else
			{
				SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_ERROR, "not support nalu type:%d size:%d will dropped!", nalu_type, size);
			}
		}
		else
		{
			SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_ERROR, "read_h264_frame failed!");
			return 0;
		}
	}

	return 0;

}




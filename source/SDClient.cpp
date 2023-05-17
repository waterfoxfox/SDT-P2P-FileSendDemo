////////////////////////////////////////////////////////////////////////////////
//  File Name: CSDClient.cpp
//
//  Functions:
//      客户端对象类.
//
//  History:
//  Date        Modified_By  Reason  Description	
//
////////////////////////////////////////////////////////////////////////////////

#include "SDClient.h"


extern char		g_acH264FileUrl[1024];
extern int		g_nH264Fps;
extern FEC_REDUN_TYPE_P2P g_eRedunMethod;
extern UINT		g_unRedunRatio;
extern UINT		g_unFecMinGroupSize;
extern UINT		g_unFecMaxGroupSize;
extern BOOL		g_bEnableAck;
extern BOOL		g_bSendNullData;
extern UINT		g_unNullDataBitrateKbps;

#define MAX_PAYLOAD_LEN_AUDIO	2048

//虚拟数据时附带的头部，用于绕开数据合法性检测
static unsigned char 	g_byTestStreamHead[] 	= {0x00, 0x00, 0x00, 0x01, 0x67, 0x64, 0x00, 0x28, 0xAC, 0x4C, 0x22, 0x07, 0x80, 0x8B, 0xF7, 0x08, 0x00, 0x00, 0x03, 0x00, 0x08, 0x00, 0x00, 0x03, 0x01, 0xE4, 0x78, 0xC1, 0x90, 0x8C, 0x00, 0x00, 0x00, 0x01, 0x68, 0xEE, 0x32, 0xC8, 0xB0, 0x00, 0x00, 0x00, 0x01, 0x65};

CSDClient::CSDClient()
{
	m_pSendThread = NULL;
	//客户端SDK环境初始化
	SDTerminalP2P_Enviroment_Init("./log", P2P_LOG_OUTPUT_LEVEL_INFO);
	m_pTerminal = SDTerminalP2P_Create();
	m_bClosed = TRUE;
}

CSDClient::~CSDClient()
{
	SDTerminalP2P_Delete(&m_pTerminal);
	SDTerminalP2P_Enviroment_Free();
}


BOOL CSDClient::Start(char* strRemoteIp, UINT unRemotePort, CLIENT_USER_TYPE_P2P eUserType)
{
	if ((g_acH264FileUrl[0]) && (g_bSendNullData == FALSE))
	{
		m_H264File.Start(g_acH264FileUrl, g_nH264Fps, TRUE);
	}
	else
	{
		SDLOG_PRINTF(m_strTag, SD_LOG_LEVEL_INFO, "File Video test is disable!");	
	}

	m_bClosed = FALSE;


	//设置传输相关参数
	SDTerminalP2P_SetVideoFrameRateForSmoother(m_pTerminal, g_nH264Fps);

	UINT unJitterBuffDelay = 150;
	SDTerminalP2P_SetTransParams(m_pTerminal, unJitterBuffDelay, g_eRedunMethod, g_unRedunRatio, g_unFecMinGroupSize, g_unFecMaxGroupSize, g_bEnableAck);

	//设置回调接口
	SDTerminalP2P_SetAutoBitrateNotifyCallback(m_pTerminal, P2P_AB_TYPE_ADJUST_BITRATE_FIRST, P2PAutoBitrateNotifCallback, this);
	SDTerminalP2P_SetRemoteIdrRequestNotifyCallback(m_pTerminal, P2PRemoteIdrRequestNotifyCallback, this);

	

	//获得本地IP地址(若存在多个本地IP，需选择可访问远端IP的本地IP地址)
	char strLocalIp[64];
	memset(strLocalIp, 0x0, sizeof(strLocalIp));
	char *strIp = SD_GetExportIp(strRemoteIp);
	if (strIp != NULL)
	{
		strcpy(strLocalIp, strIp);
	}

	int nRet = SDTerminalP2P_Online(m_pTerminal, strLocalIp, 0, strRemoteIp, (unsigned short)unRemotePort, eUserType);
	if (nRet < 0)
	{
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_ERROR, "SDTerminalP2P_Online Failed return:%d!", nRet);
		m_bClosed = TRUE;
		m_H264File.Stop();
		return FALSE;
	}

	//创建发送线程
	char acThreadName[128];
	sprintf(acThreadName, "SendThread");
	m_pSendThread = new CSDThread(acThreadName);

	if (!m_pSendThread->CreateThread(SendThread, SendThreadClose, (void*)this))
	{
		SDLOG_PRINTF(m_strTag, SD_LOG_LEVEL_ERROR, "CreateThread Failed for Send Thread!");
		m_bClosed = TRUE;
		m_H264File.Stop();
		SDTerminalP2P_Offline(m_pTerminal);
		delete m_pSendThread;
		m_pSendThread = NULL;
		return FALSE;
	}
	return TRUE;
}


void CSDClient::Close()
{
	m_bClosed = TRUE;
	if (m_pSendThread)
	{
		m_pSendThread->CloseThread();
		delete m_pSendThread;
		m_pSendThread = NULL;
	}

	SDTerminalP2P_Offline(m_pTerminal);
	m_H264File.Stop();
}


int CSDClient::SendThreadClose(void* pParam1)
{
	CSDClient* pMain = (CSDClient*) pParam1;

	return 0;
}

#define USE_H264_FILE	1
#define MAX_FRAME_SIZE	(1920*1080)

int CSDClient::SendThread(void *pObject1)
{
	CSDClient* pClient = (CSDClient*)pObject1;

	BYTE* pFrame = (BYTE*)calloc(MAX_FRAME_SIZE, sizeof(BYTE));
	if (pFrame == NULL)
	{
		SDLOG_PRINTF(m_strTag, SD_LOG_LEVEL_ERROR, "malloc frame buff failed!");
		return 0;
	}

	UINT unFrameSizeByte = 0;
	if (g_bSendNullData == TRUE)
	{
		//计算每帧数据的大小
		unFrameSizeByte = ((g_unNullDataBitrateKbps / 8) / g_nH264Fps) * 1000;
		if (unFrameSizeByte > MAX_FRAME_SIZE)
		{
			SDLOG_PRINTF(m_strTag, SD_LOG_LEVEL_WARNING, "Null data bitrate:%d is too large!", g_unNullDataBitrateKbps);
			unFrameSizeByte = MAX_FRAME_SIZE;

		}
		
		memcpy(pFrame, g_byTestStreamHead, sizeof(g_byTestStreamHead));
	}

	int nFrameInterval = 1000 / g_nH264Fps;
	UINT unFrameNo = 0;
	long nPrevSendTime = 0;
	UINT unDts = 0;
	UINT unPts = 0;

	while(pClient->m_bClosed == FALSE)
	{
		long nStartTime = SD_GetTickCount();
		
		if (g_bSendNullData == FALSE)
		{
			UINT unFrameLen = pClient->m_H264File.ReadH264RawFrame(pFrame, MAX_FRAME_SIZE, &unDts, &unPts);
			if (unFrameLen)
			{
				SDTerminalP2P_SendVideoData(pClient->m_pTerminal, pFrame, unFrameLen, 0, FALSE);
			}
		}
		else
		{
			SDTerminalP2P_SendVideoData(pClient->m_pTerminal, pFrame, unFrameSizeByte, 0, FALSE);
		}

		unFrameNo++;

		long nCurrTime = SD_GetTickCount();
		long nUsedTime = nCurrTime - nStartTime;

		long nSleepTime = nFrameInterval - nUsedTime;
		if (nSleepTime < 0)
		{
			nSleepTime = 0;
		}
		
		//SDLOG_PRINTF(m_strTag, SD_LOG_LEVEL_INFO, "Send a frme:%d!  send interval:%d", unFrameNo, nCurrTime - nPrevSendTime);
		nPrevSendTime = nCurrTime;
		SD_Sleep(nSleepTime);
	}

	if (pFrame)
	{
		free(pFrame);
	}

	return 0;
}

BOOL CSDClient::GetVideoAudioUpDownBitrate(float *pfVideoUpRate, float *pfVideoDownRate, float *pfAudioUpRate, float *pfAudioDownRate)
{
	if (m_bClosed == FALSE)
	{
		SDTerminalP2P_GetVideoAudioUpDownBitrate(m_pTerminal, pfVideoUpRate, pfVideoDownRate, pfAudioUpRate, pfAudioDownRate);
		return TRUE;
	}
	return FALSE;
}

BOOL CSDClient::GetVideoAudioUpDownLostRatio(float *pfVideoUpLostRatio, float *pfVideoDownLostRatio, float *pfAudioUpLostRatio, float *pfAudioDownLostRatio)
{
	if (m_bClosed == FALSE)
	{
		SDTerminalP2P_GetVideoAudioUpDownLostRatio(m_pTerminal, pfVideoUpLostRatio, pfVideoDownLostRatio, pfAudioUpLostRatio, pfAudioDownLostRatio);
		return TRUE;
	}
	return FALSE;
}

// SDK回调接口实现
BOOL CSDClient::P2PAutoBitrateNotifCallback(void* pObject, BOOL bSelectOrDropFrame, unsigned int unFrameSelectOrDropInterval, float fBitrateRatio)
{
	return FALSE;
}

BOOL P2PDropNextFrameNotifyFunc(void* pObject, unsigned int unDropFrames)
{
	return FALSE;
}

BOOL CSDClient::P2PRemoteIdrRequestNotifyCallback(void* pObject)
{
	return FALSE;
}



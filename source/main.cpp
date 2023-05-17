////////////////////////////////////////////////////////////////////////////////
//  File Name: main.cpp
//
//  Functions:
//      SDK����DEMO.
//
//  History:
//  Date        Modified_By  Reason  Description	
//
////////////////////////////////////////////////////////////////////////////////
#include "SDCommon.h"
#include "SDLog.h"
#include "SDIniFile.h"
#include "SDClient.h"
#include "SDConsoleIFace.h"
#include "SDTerminalP2PSdk.h"

#if defined(WIN32) && defined(_DEBUG_)
#include <afx.h>
#endif

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>
#endif

#define SECTION_CONFIG					"Config"
#define KEY_NAME_REMOTE_IP				"RemoteIp"			//Զ��IP
#define KEY_NAME_REMOTE_PORT			"RemotePort"		//Զ�˶˿�
#define KEY_NAME_H264_FILE				"H264FileUrl"		//�䵱�������ݵ�H264�ļ�URL	
#define KEY_NAME_H264_FPS				"H264FileFps"		//�䵱�������ݵ�H264�ļ�֡��
#define KEY_NAME_FEC_REDUN_METHOD		"FecRedunMethod"	//FEC���෽��
#define KEY_NAME_FEC_REDUN_RATIO		"FecRedunRatio"		//FEC����ȱ���
#define KEY_NAME_FEC_MIN_GROUP_SIZE		"FecMinGroupSize"	//FEC Min Group size
#define KEY_NAME_FEC_MAX_GROUP_SIZE		"FecMaxGroupSize"	//FEC Max Group size
#define KEY_NAME_FEC_ENABLE_ACK			"FecEnableAck"		//�Ƿ�֧��������ACK����
#define KEY_NAME_SEND_NULL_DATA			"SendNullDataEnable"		//�Ƿ����������ݣ����Ƕ�ȡ����
#define KEY_NAME_NULL_DATA_BITRATE		"NullDataBitrateKbps"		//�����������ݵ�����



CSDClient	g_Client;
char		g_acH264FileUrl[1024];
int			g_nH264Fps					= H264_FILE_FPS_DEF;
FEC_REDUN_TYPE_P2P g_eRedunMethod		= P2P_FEC_AUTO_REDUN_TYPE;
UINT		g_unRedunRatio				= 30;
UINT		g_unFecMinGroupSize			= 16;
UINT		g_unFecMaxGroupSize			= 64;
BOOL		g_bEnableAck				= FALSE;
BOOL		g_bSendNullData				= FALSE;
UINT		g_unNullDataBitrateKbps		= 2000;

//������
int main(int argc, char *argv[])
{
	int nRet = 0;

	//��ʼ��DEMO��־ģ��
	char strIniFileName[MAX_PATH];
	GetSameExeFile(strIniFileName, ".ini");
	SDLOG_INIT("./log", SD_LOG_LEVEL_INFO, strIniFileName);


	//��ȡ����
	CSDIniFile *pIniFile = new CSDIniFile;
	pIniFile->ReadIniFile(strIniFileName);

	//Զ��IP
	char strRemoteIp[64];
	memset(strRemoteIp, 0x0, sizeof(strRemoteIp));
	pIniFile->SDGetProfileString(SECTION_CONFIG, KEY_NAME_REMOTE_IP, strRemoteIp, 64);

	//�û�����
	CLIENT_USER_TYPE_P2P eUserType = CLIENT_USER_TYPE_AV_SEND_ONLY;

	//Զ��PORT
	UINT unRemotePort = (UINT)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_REMOTE_PORT, 4000);


	//�䵱�������ݵ�H264�ļ�·��
	memset(g_acH264FileUrl, 0x0, sizeof(g_acH264FileUrl));
	pIniFile->SDGetProfileString(SECTION_CONFIG, KEY_NAME_H264_FILE, g_acH264FileUrl, sizeof(g_acH264FileUrl));
	g_nH264Fps = pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_H264_FPS, H264_FILE_FPS_DEF);
	if ((g_nH264Fps <= 0) || (g_nH264Fps > 60))
	{
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_WARNING, "Please setup right h264 fps(%d), will use default:%d!", g_nH264Fps, H264_FILE_FPS_DEF);
		g_nH264Fps = H264_FILE_FPS_DEF;
	}

	//����FEC������ȡ
	//FEC����
	UINT unFecRedunMethod = (UINT)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_FEC_REDUN_METHOD, P2P_FEC_AUTO_REDUN_TYPE);
	g_eRedunMethod = unFecRedunMethod != 0 ? P2P_FEC_FIX_REDUN_TYPE : P2P_FEC_AUTO_REDUN_TYPE;

	//FEC�������
	UINT unFecRedundancy = (UINT)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_FEC_REDUN_RATIO, 30);
	if (unFecRedundancy > 100)
	{
		unFecRedundancy = 100;
	}
	g_unRedunRatio = unFecRedundancy;

	//FEC Min Group��С 
	UINT unFecMinGroupSize = (UINT)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_FEC_MIN_GROUP_SIZE, 16);
	if (unFecMinGroupSize > 64)
	{
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_WARNING, "Fec Group min size:%d invalid [8, 64]!", unFecMinGroupSize);
		unFecMinGroupSize = 64;
	}

	if (unFecMinGroupSize < 8)
	{
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_WARNING, "Fec Group min size:%d invalid [8, 64]!", unFecMinGroupSize);
		unFecMinGroupSize = 8;
	}
	g_unFecMinGroupSize = unFecMinGroupSize;


	//FEC Max Group��С 
	UINT unFecMaxGroupSize = (UINT)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_FEC_MAX_GROUP_SIZE, 64);
	if (unFecMaxGroupSize > 64)
	{
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_WARNING, "Fec Group max size:%d invalid [8, 64]!", unFecMaxGroupSize);
		unFecMaxGroupSize = 64;
	}

	if (unFecMaxGroupSize < 8)
	{
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_WARNING, "Fec Group max size:%d invalid [8, 64]!", unFecMaxGroupSize);
		unFecMaxGroupSize = 8;
	}
	g_unFecMaxGroupSize = unFecMaxGroupSize;

	if (g_unFecMaxGroupSize < g_unFecMinGroupSize)
	{
		delete pIniFile;
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_ERROR, "Fec Group min size:%d  max size:%d  invalid!", g_unFecMinGroupSize, g_unFecMaxGroupSize);
		SDLOG_CLOSE();
		return FALSE;
	}

	//NACK����
	UINT unEnableNack = (UINT)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_FEC_ENABLE_ACK, FALSE);
	if (unEnableNack != 0)
	{
		g_bEnableAck = TRUE;
	}
	else
	{
		g_bEnableAck = FALSE;
	}

	//�Ƿ�����������
	UINT unSendNullData = (UINT)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_SEND_NULL_DATA, FALSE);
	if (unSendNullData != 0)
	{
		g_bSendNullData = TRUE;
		//������������
		g_unNullDataBitrateKbps = (UINT)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_NULL_DATA_BITRATE, 8000);

		SDLOG_PRINTF("CAVProcess", SD_LOG_LEVEL_INFO, "Will Send NULL data instead! bitrate:%d kbps", g_unNullDataBitrateKbps);
	}
	else
	{
		g_bSendNullData = FALSE;
	}


	SDLOG_PRINTF("CAVProcess", SD_LOG_LEVEL_INFO, "FecMethod:%s  RedunRatio:%d  MinGroup:%d  MaxGroup:%d  AckEnable:%s!", g_eRedunMethod == P2P_FEC_AUTO_REDUN_TYPE ? "Auto":"Fix", 
				g_unRedunRatio, g_unFecMinGroupSize, g_unFecMaxGroupSize, g_bEnableAck == TRUE ? "Y":"N");

	delete pIniFile;
	
	//������Ч��У��
	if (strRemoteIp[0] == 0)
	{
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_ERROR, "Please setup right remote ip!");
		SDLOG_CLOSE();
		return FALSE;
	}

	SDLOG_PRINTF("Test", SD_LOG_LEVEL_INFO, "RemoteIp:%s RemotePort:%d!", strRemoteIp, unRemotePort);
	SDLOG_PRINTF("Test", SD_LOG_LEVEL_INFO, "H264 File: %s  Fps:%d!", g_acH264FileUrl, g_nH264Fps);


	//�������Է���
	if (!g_Client.Start(strRemoteIp, unRemotePort, eUserType))
	{
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_ERROR, "Test start fail...");
		SDLOG_CLOSE();
		return FALSE;
	}
	else
	{
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_INFO, "sdk test start success...");
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_INFO, "input exit will end test process!");
	}

	//ѭ��
	CSDConsleIFace::RunCommandLine("sdk_test");

	g_Client.Close();
	

	SDLOG_PRINTF("Test", SD_LOG_LEVEL_INFO, "sdk test over success...");
	SDLOG_CLOSE();

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////
//  File Name: CSDClient.h
//
//  Functions:
//      客户端对象类.
//
//  History:
//  Date        Modified_By  Reason  Description	
//
////////////////////////////////////////////////////////////////////////////////

#if !defined(_SDCLIENT_H)
#define _SDCLIENT_H

#include "SDCommon.h"
#include "SDThread.h"
#include "SDTerminalP2PSdk.h"
#include "SDH264FileParse.h"

#define H264_FILE_FPS_DEF		25


class CSDClient
{
public:
	CSDClient();
	virtual ~CSDClient();

public:
	BOOL Start(char* strRemoteIp, UINT unRemotePort, CLIENT_USER_TYPE_P2P eUserType);
	void Close();

	BOOL GetVideoAudioUpDownBitrate(float *pfVideoUpRate, float *pfVideoDownRate, 
									float *pfAudioUpRate, float *pfAudioDownRate);
	
	BOOL GetVideoAudioUpDownLostRatio(float *pfVideoUpLostRatio, float *pfVideoDownLostRatio, 
									  float *pfAudioUpLostRatio, float *pfAudioDownLostRatio);
private:
	static int SendThread(void *pParam1);
	static int SendThreadClose(void *pParam1);


	// SDK回调接口实现
	static BOOL P2PAutoBitrateNotifCallback(void* pObject, BOOL bSelectOrDropFrame, unsigned int unFrameSelectOrDropInterval, float fBitrateRatio);
	static BOOL P2PDropNextFrameNotifyFunc(void* pObject, unsigned int unDropFrames);
	static BOOL P2PRemoteIdrRequestNotifyCallback(void* pObject);

private:
	void*				 m_pTerminal;

	CSDThread*			 m_pSendThread;
	BOOL				 m_bClosed;
	CSDH264FilePase		 m_H264File;	
	FILE*				 m_pfRecvH264File;

};

#endif // !defined(_SDCLIENT_H)

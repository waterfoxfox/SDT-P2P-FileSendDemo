//***************************************************************************//
//* 版权所有  www.mediapro.cc
//*
//* 内容摘要：客户端对外P2P SDK DLL接口
//*	
//* 当前版本：V1.0		
//* 作    者：mediapro
//* 完成日期：2020-4-26
//**************************************************************************//

#ifndef _SD_TERMINALP2P_SDK_H_
#define _SD_TERMINALP2P_SDK_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined _WIN32 || defined __CYGWIN__
  #ifdef DLL_EXPORTS
    #ifdef __GNUC__
      #define DLLIMPORT_P2P_SDK __attribute__ ((dllexport))
    #else
      #define DLLIMPORT_P2P_SDK __declspec(dllexport) 
    #endif
  #else
    #ifdef __GNUC__
      #define DLLIMPORT_P2P_SDK 
    #else
      #define DLLIMPORT_P2P_SDK 
    #endif
  #endif
#else
  #if __GNUC__ >= 4
    #define DLLIMPORT_P2P_SDK __attribute__ ((visibility ("default")))
  #else
    #define DLLIMPORT_P2P_SDK
  #endif
#endif

#ifdef __APPLE__
#ifndef OBJC_BOOL_DEFINED
typedef int BOOL;
#endif 
#else
#ifndef BOOL
typedef int BOOL;
#endif
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

//日志输出的级别
typedef enum LOG_OUTPUT_LEVEL_P2P
{
	P2P_LOG_OUTPUT_LEVEL_DEBUG = 1,
	P2P_LOG_OUTPUT_LEVEL_INFO,
	P2P_LOG_OUTPUT_LEVEL_WARNING,
	P2P_LOG_OUTPUT_LEVEL_ERROR,
	P2P_LOG_OUTPUT_LEVEL_ALARM,
	P2P_LOG_OUTPUT_LEVEL_FATAL,
	P2P_LOG_OUTPUT_LEVEL_NONE
} LOG_OUTPUT_LEVEL_P2P;


//客户端类型
typedef enum CLIENT_USER_TYPE_P2P
{
	//非法类型
	CLIENT_USER_TYPE_OTHER = 0,
	//音视频收发类型
	CLIENT_USER_TYPE_AV_SEND_RECV = 1,
	//仅接收音视频类型
	CLIENT_USER_TYPE_AV_RECV_ONLY = 2,
	//仅发送音视频类型
	CLIENT_USER_TYPE_AV_SEND_ONLY = 3
} CLIENT_USER_TYPE_P2P;


//FEC冗余方法
typedef enum FEC_REDUN_TYPE_P2P
{
	//自动冗余度
	P2P_FEC_AUTO_REDUN_TYPE = 0,
	//固定冗余度
	P2P_FEC_FIX_REDUN_TYPE = 1
} FEC_REDUN_TYPE_P2P;


//码率自适应模式
typedef enum AUTO_BITRATE_TYPE_P2P
{
	//关闭码率自适应(默认)
	P2P_AB_TYPE_DISABLE = 0,
	//优先降低帧率，其次降低码率
	P2P_AB_TYPE_ADJUST_FRAME_FIRST = 1,
	//优先降低码率，其次降低帧率
	P2P_AB_TYPE_ADJUST_BITRATE_FIRST = 2
} AUTO_BITRATE_TYPE_P2P;



//送外层视频码流时附带的信息
typedef struct VideoFrameInforP2P
{
	unsigned int unWidth;
	unsigned int unHeight;
	unsigned int unFps;
	BOOL bPacketLost;
	BOOL bKeyFrame;
	BOOL bInfoUpdated;
	BOOL bIsHevc;
	//注意VPS\SPS\PPS中不含起始码
	unsigned char byVps[512];
	unsigned int unVpsSize;
	unsigned char bySps[512];
	unsigned int unSpsSize;
	unsigned char byPps[512];
	unsigned int unPpsSize;
}VideoFrameInforP2P;


//送外层音频码流时附带的信息
typedef struct AudioFrameInforP2P
{
	unsigned int unCodecType;
	unsigned int unSampleRate;
	unsigned int unChannelNum;
	unsigned int unFrameNo;
	BOOL bInfoUpdated;
}AudioFrameInforP2P;



//回调函数
// 【注意事项】
//	1、通知型回调函数中应尽可能快的退出，不进行耗时操作，不调用SDTerminal系列API接口。
//  2、数据型回调函数中允许进行解码处理

// 收到服务器发来的视频
typedef  void (*RecvP2PRemoteVideoFunc)(void* pObject, unsigned char* data, unsigned int unLen, unsigned int unPTS, VideoFrameInforP2P* pFrameInfo);

// 收到服务器发来的音频
typedef  void (*RecvP2PRemoteAudioFunc)(void* pObject, unsigned char* data, unsigned int unLen, unsigned int unPTS, AudioFrameInforP2P* pFrameInfo);

//当使用模块的码率自适应评估时，评估结果由本接口送出，外层负责具体的实施
//bSelectOrDropFrame为TRUE时，每N帧“取”一帧编码发送。为FALSE时，每N帧“丢”一帧。
//unFrameSelectOrDropInterval为N
//比如bSelectOrDropFrame为FALSE，unFrameSelectOrDropInterval=2，表示每2帧丢1帧。
//比如bSelectOrDropFrame为TRUE，unFrameSelectOrDropInterval=3，表示每3帧取1帧。
//比如fBitrateRatio=0.8表示需要将码率降低为原始码率的0.8倍
//外层需同时响应帧率调整和码率调整
//当外层执行了码率自适应动作时，返回TRUE
typedef BOOL (*P2PAutoBitrateNotifyFunc)(void* pObject, BOOL bSelectOrDropFrame, unsigned int unFrameSelectOrDropInterval, float fBitrateRatio);


//告知外层，请求丢弃接下来的一帧，不送编码器
//当外层执行了丢弃动作时，返回TRUE，否则返回FALSE
typedef BOOL (*DropNextFrameNotifyFunc)(void* pObject, unsigned int unDropFrames);


//响应远端的IDR请求，注意本接口为同步调用方式，因此外层不应在其中执行耗时操作，应尽快返回
typedef BOOL (*P2PRemoteIdrRequestNotifyFunc)(void* pObject);



///////////////////////////////////////////////////////////////////////////////////////////////////////
//									SDTerminal SDK主动型接口
///////////////////////////////////////////////////////////////////////////////////////////////////////


/***
* 环境初始化，系统只需调用一次，主要用于SDK环境以及日志模块的初始化
* @param: outputPath表示日志存放路径，支持相对路径和绝对路径，若目录不存在将自动创建
* @param: outputLevel表示日志输出的级别，只有等于或者高于该级别的日志输出到文件，取值范围参考LOG_OUTPUT_LEVEL
* @return: 
*/
DLLIMPORT_P2P_SDK void  SDTerminalP2P_Enviroment_Init(const char* outputPath, int outputLevel);

DLLIMPORT_P2P_SDK void  SDTerminalP2P_Enviroment_Free();

/***
* 创建SDTerminal
* @return: 返回模块指针，为NULL则失败
*/
DLLIMPORT_P2P_SDK void*  SDTerminalP2P_Create();

/***
* 销毁SDTerminal，并设置指针为NULL，使用者应该做好与其他API之间的互斥保护
* @param ppTerminal: 模块指针的指针
* @return:
*/
DLLIMPORT_P2P_SDK void  SDTerminalP2P_Delete(void** ppTerminal);


/***
* 准备会话
* @param pTerminal: 模块指针
* @param strLocalIP: 绑定的本地IP地址;
*                    当设置为NULL或''时且strRemoteIP为NULL或''时，内部将使用INADDR_ANY，交由操作系统选择一个网卡IP,多IP时系统选择不一定符合预期。
*                    当设置为NULL或''时且strRemoteIP为有效值时，内部将根据strRemoteIP选择同网段网卡IP。
* @param shLocalPort: 绑定的本地端口号，允许设置本地端口为0，由操作系统选择一个当前可用的端口。作为服务端(播放端)上使用时，需要指定本地监听的端口。
* @param strRemoteIP: 远端IP地址。
*					  服务端(播放端)上设置NULL即可，作为服务端(播放端)，一般是不知道客户端(发送端)的IP和端口的，内部将在收到远端数据后自动翻转IP和端口，从而获得可用于向远端发送数据的IP和端口。
*					  当用户指定了合法的远端IP时，SDK仅会接收来自该IP的数据，丢弃来自其他IP的数据	
* @param shRemotePort: 远端端口号。服务端(播放端)上设置远端端口号为0即可。
* @param eUserType: 本客户端类型：收发一体、纯发送、纯接收，按需设置类型可以实现最小的资源开销。
* @return: <0为失败错误码，>=0为成功
*/
DLLIMPORT_P2P_SDK int  SDTerminalP2P_Online(void* pTerminal, const char *strLocalIP, unsigned short shLocalPort, const char *strRemoteIP, unsigned short shRemotePort, CLIENT_USER_TYPE_P2P eUserType);

 
    
/***
* 结束会话
* @param pTerminal: 模块指针
* @return:
*/
DLLIMPORT_P2P_SDK void  SDTerminalP2P_Offline(void* pTerminal);



/***
* 发送音频数据
* @param pTerminal: 模块指针
* @param byBuf: 发送已编码的一帧音频码流【必须输入ADTS流】
* @param unLen: 数据长度
* @param unDts: SDTerminalP2P_SetUseInternalTimeStamp指定为使用外部时间戳模式时，本参数供用户传入时间戳。默认为内部时间戳模式，本参数被忽略，可传入0
* @return: 
*/
DLLIMPORT_P2P_SDK void  SDTerminalP2P_SendAudioData(void* pTerminal, unsigned char *byBuf, unsigned int unLen, unsigned int unDts);



/***
* 发送视频数据
* @param pTerminal: 模块指针
* @param byBuf: 发送已编码的一帧视频码流，内部自带拆分功能【必须输入带起始码的码流】
* @param unLen: 数据长度
* @param unDts: SDTerminalP2P_SetUseInternalTimeStamp指定为使用外部时间戳模式时，本参数供用户传入时间戳。默认为内部时间戳模式，本参数被忽略，可传入0
* @param bIsHevc: 当前码流是否为HEVC（H265）码流，是则设置为TRUE，H264码流设置为FALSE
* @return: 
*/
DLLIMPORT_P2P_SDK void  SDTerminalP2P_SendVideoData(void* pTerminal, unsigned char *byBuf, unsigned int unLen, unsigned int unDts, BOOL bIsHevc);


/***
* 设置上行传输参数，若不调用本API将使用默认传输参数【必须Online之前调用】
* @param pTerminal: 模块指针
* @param unJitterBuffDelay: 本客户端接收码流时的内部JitterBuff缓存时间（毫秒）初始值,范围0~600。设置为0时，将关闭内部接收JitterBuff功能，此时可以获得最低延时。
* @param eFecRedunMethod: 为上行FEC冗余度方法，包括AUTO_REDUN自动冗余度、FIX_REDUN固定冗余度。自动冗余度将根据网络情况自行调整冗余。固定冗余度则全程使用固定值。
* @param unFecRedunRatio: FEC 固定冗余度时对应的上行冗余比率或自动冗余度时对应的冗余初始值，比如设置为30，则表示使用30%冗余。
* @param unFecMinGroupSize: 为上行FEC分组的下限值，建议设置为16。
* @param unFecMaxGroupSize: 为上行FEC分组的上限值，根据终端CPU能力而定，最大不超过72，越大FEC所消耗的CPU越高，抗丢包能力也越强，建议性能足够的设备上设置为64。
* @param bEnableAck：是否启用ACK功能，当设置为TRUE时开启ACK。当设置为FALSE时开启NACK。收发双方保持设置一致。建议设置为FALSE，使用NACK模式
* @return:
*/
DLLIMPORT_P2P_SDK void  SDTerminalP2P_SetTransParams(void* pTerminal, unsigned int unJitterBuffDelay, FEC_REDUN_TYPE_P2P eFecRedunMethod, 
                                          unsigned int unFecRedunRatio, unsigned int unFecMinGroupSize, unsigned int unFecMaxGroupSize, BOOL bEnableAck);



/***
* 设置自动冗余度模式下的冗余度上下限，未调用时默认为0~100【Online之前调用生效】
* @param pTerminal: 模块指针
* @param unAutoRedunRatioMin: 自动冗余度下限。
* @param unAutoRedunRatioMax: 自动冗余度上限。
* @return:
*/
DLLIMPORT_P2P_SDK void  SDTerminalP2P_SetAutoRedunMinMax(void* pTerminal, unsigned int unAutoRedunRatioMin, unsigned int unAutoRedunRatioMax);




/***
* 获取音视频丢包率统计信息（内部已经乘100得到百分比）
* @param pTerminal: 模块指针
* @param pfVideoUpLostRatio: 视频上行丢包率
* @param pfVideoDownLostRatio: 视频下行丢包率
* @param pfAudioUpLostRatio: 音频上行丢包率
* @param pfAudioDownLostRatio: 音频下行丢包率
* @return:
*/
DLLIMPORT_P2P_SDK void  SDTerminalP2P_GetVideoAudioUpDownLostRatio(void* pTerminal, float *pfVideoUpLostRatio, float *pfVideoDownLostRatio, 
                                                        float *pfAudioUpLostRatio, float *pfAudioDownLostRatio);



/***
* 获取音视频码率统计信息，单位Kbps
* @param pTerminal: 模块指针
* @param pfVideoUpRate: 视频上行码率
* @param pfVideoDownRate: 视频下行码率
* @param pfAudioUpRate: 音频上行码率
* @param pfAudioDownRate: 音频下行码率
* @return:
*/
DLLIMPORT_P2P_SDK void  SDTerminalP2P_GetVideoAudioUpDownBitrate(void* pTerminal, float *pfVideoUpRate, float *pfVideoDownRate, 
                                                      float *pfAudioUpRate, float *pfAudioDownRate);


/***
* 获取当前时刻RTT
* @param pTerminal: 模块指针
* @return: RTT值
*/
DLLIMPORT_P2P_SDK unsigned int  SDTerminalP2P_GetCurrentRtt(void* pTerminal);


/***
* 获取SDK版本信息
* @param pTerminal: 模块指针
* @return: 版本号
*/
DLLIMPORT_P2P_SDK unsigned int  SDTerminalP2P_GetVersion(void* pTerminal);



/***
* 设置视频帧率信息，作为发送时内部的Smoother处理参考
* 注意该帧率要符合实际帧率,可以高于实际帧率，但不能低于实际帧率，否则将导致发送速度不足。不调用本函数时，默认关闭smooth处理【Online接口前调用生效】
* @param pTerminal: 模块指针
* @param unFrameRate: 视频参考帧率
* @return:
*/
DLLIMPORT_P2P_SDK void  SDTerminalP2P_SetVideoFrameRateForSmoother(void* pTerminal, unsigned int unFrameRate);



/***
* 指定使用内部自动生成时间戳还是使用外部提供的时间戳(Send接口传入)，默认为内部时间戳。【Online接口前调用生效】
* @param pTerminal: 模块指针
* @param bUseInternalTimestamp: TRUE表示采用内部时间戳，FALSE表示由用户外层提供时间戳（Send接口时）
* @return:
*/
DLLIMPORT_P2P_SDK void  SDTerminalP2P_SetUseInternalTimeStamp(void* pTerminal, BOOL bUseInternalTimestamp);



/***
* 是否使能丢包冻结机制，在出现丢包时停止对外输出视频流，等待下一个完整关键帧到来时继续对外输出。默认开启。【Online接口前调用生效】
* @param pTerminal: 模块指针
* @param bEnable: TRUE表示启用丢包冻结机制。FALSE表示关闭丢包冻结机制，此时所有接收到的数据均返回外层，同时提供丢包标识、关键帧标识。
* @return:
*/
DLLIMPORT_P2P_SDK void  SDTerminalP2P_EnableFreezeFrameWhenLost(void* pTerminal, BOOL bEnable);




///////////////////////////////////////////////////////////////////////////////////////////////////////
//					SDTerminal SDK回调型接口，注意：回调接口设置均满足【Online接口前调用生效】
///////////////////////////////////////////////////////////////////////////////////////////////////////

/***
* 设置码率自适应开关、模式以及反馈回调函数
* @param pTerminal: 模块指针
* @param eAutoBitrateMethod: 码率自适应方法
* @param pfAutoBitrateNotifyCallback: 码率自适应反馈回调函数
* @param pObject: 透传给回调函数的用户指针
* @return: 
*/
DLLIMPORT_P2P_SDK void  SDTerminalP2P_SetAutoBitrateNotifyCallback(void* pTerminal, AUTO_BITRATE_TYPE_P2P eAutoBitrateMethod, 
														P2PAutoBitrateNotifyFunc pfAutoBitrateNotifyCallback, void* pObject);

/***
* 设置码率自适应-请求外层单次丢帧的回调函数
* @param pTerminal: 模块指针
* @param pfDropNextFrameNotifyCallback: 请求外层单次丢帧回调函数
* @param pObject: 透传给回调函数的用户指针
* @return: 
*/
DLLIMPORT_P2P_SDK void  SDTerminalP2P_SetDropNextFrameNotifyCallback(void* pTerminal, DropNextFrameNotifyFunc pfDropNextFrameNotifyCallback, void* pObject);


/***
* 设置远端IDR请求反馈回调函数
* @param pTerminal: 模块指针
* @param pfRemoteIdrRequestNotifyCallback: DR请求反馈回调函数
* @param pObject: 透传给回调函数的用户指针
* @return: 
*/
DLLIMPORT_P2P_SDK void  SDTerminalP2P_SetRemoteIdrRequestNotifyCallback(void* pTerminal, P2PRemoteIdrRequestNotifyFunc pfRemoteIdrRequestNotifyCallback, void* pObject);


/***
* 设置接收视频回调函数
* @param pTerminal: 模块指针
* @param pfRecvRemoteVideoCallback: 接收视频的回调函数指针
* @param pObject: 透传给回调函数的用户指针
* @return: 
*/
DLLIMPORT_P2P_SDK void  SDTerminalP2P_SetRecvRemoteVideoCallback(void* pTerminal, RecvP2PRemoteVideoFunc pfRecvRemoteVideoCallback, void* pObject);


/***
* 设置接收音频回调函数
* @param pTerminal: 模块指针
* @param pfRecvRemoteAudioCallback: 接收音频的回调函数指针
* @param pObject: 透传给回调函数的用户指针
* @return: 
*/
DLLIMPORT_P2P_SDK void  SDTerminalP2P_SetRecvRemoteAudioCallback(void* pTerminal, RecvP2PRemoteAudioFunc pfRecvRemoteAudioCallback, void* pObject);

#ifdef __cplusplus
}
#endif

#endif // _SD_TERMINALP2P_SDK_H_

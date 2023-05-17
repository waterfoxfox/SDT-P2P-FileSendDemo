# DEMO说明
本DEMO用于演示纯传输层SDK的使用，DEMO读取指定的H264裸码流文件，解析成帧，调用API发送给对端。<br>

可通过DEMO配置文件设置对端IP、端口，FEC相关参数，H264裸码流文件路径等。


```js
[Config]
;对端IP、端口
RemoteIp=172.31.36.87        
RemotePort=4000

;H264裸码流文件路径、帧率指定
H264FileUrl=./720-2.5M.h264
H264FileFps=25

;Fec method, 0-自动冗余度  1-固定冗余度
FecRedunMethod=0

;Fec自动冗余时的初始冗余比例或固定冗余度时的固定冗余比例
FecRedunRatio=30

;FEC group分组大小设置
FecMinGroupSize=16
FecMaxGroupSize=64

;ACK模式或NACK模式，收发两端需配置一致。建议默认使用NACK模式
FecEnableAck=0
```
<br>

SDK API的调用集中在SDClient.cpp中
<br>



ffmpeg制作任意分辨率H264测试码流命令：

```js
ffmpeg.exe -f lavfi -i testsrc=duration=100:size=1280x720:rate=25:decimals=2 -pix_fmt yuv420p -vcodec libx264  -profile:v high -x264opts force-cfr:fps=25:keyint=50:min-keyint=1:ref=1:bitrate=1600:bframes=0  -t 30  -y  output.h264
#本命令即生成1参考帧、无B帧、720P分辨率、1.6Mbps的H264测试流
```


测试工程使用VS2010或更高版本编译


---

# 相关资源
跟多文档、代码资源见：https://mediapro.apifox.cn

SDK 商用及定制化、技术支持服务可联系：[http://www.mediapro.cc/](http://www.mediapro.cc/)


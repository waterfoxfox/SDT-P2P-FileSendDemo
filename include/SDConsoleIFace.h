////////////////////////////////////////////////////////////////////////////////
//  File Name: SDConsoleIFace.h
//
//  Functions:
//      控制台的命令行接口类的头文件.
//
//
////////////////////////////////////////////////////////////////////////////////

#if !defined(SDCONSOLEIFACE_H)
#define SDCONSOLEIFACE_H

/*
 * 查看当前支持的命令
 * 查看某个命令的帮助
 *    1，支持帮助
 *    2，支持命令解析
 *    3，支持命令动态扩展
 */

#include "SDCommon.h"
#include "SDIniFile.h"


#define  CMD_NOT_EXSIT_COMMAND     "Does not exist command!!!"
#define  CMD_SYNTAX_ERROR          "Syntax error in parameters!!!"
#define  CMD_NOT_SUPPORT_COMMAND   "Does not support command!!!"


typedef struct stServerBaseRunPar ServerBaseRunPar;
class CSDConsleIFace  
{
public:
	CSDConsleIFace();
	virtual ~CSDConsleIFace();
	
public:
	// 系统命令行系统注册命令类，告诉系统自己有命令可以执行
	static void Register (CSDConsleIFace *pIFace);
	// 系统命令行命令注销类
	static void UnRegister (CSDConsleIFace *pIFace);
	// 字符串解释接口，FALSE为解释失败，否则成功
	static int  CmdInterpreter(char *strCmd);
	// 进入本地控制台方式的命令行系统接口
	static int  RunCommandLine(const char *strCommandPrompt);
	static int  RunCommandLine(void *callprintf);


	// 返回-1，表示获取失败，如果大于0表示返回占用的字节总数
	// 从一个字符串中根据命令行规范获取一个数字，nValue为返回的数字值，nValidDataLength为值所占用字节数
	int FromStringGetMumbers(char* strCmd, int &nValue, int &nValidDataLength);
	// 从一个字符串中根据命令行规范获取一个子字符串，strValue为返回的子字符串，nValidDataLength为值所占用字节数
	int FromStringGetSubString(char* strCmd, char* strValue, int &nValidDataLength);
	
public:
	// 需要子类实现的命令帮助接口
	virtual void Help() = 0;
	// 需要子类实现的命令解释接口，返回TRUE表示解释成功，否则失败
	virtual int Interpreter(char *strCmd) = 0;
	
public:
	// 查找是否为当前命令
	int   Equal(char *strName);
	// 或者命令行名字
	char  *GetCmdName() { return m_strCmd; }
	
protected:
	char   m_strCmd[32];
};

#endif // !defined(SDCONSOLEIFACE_H)
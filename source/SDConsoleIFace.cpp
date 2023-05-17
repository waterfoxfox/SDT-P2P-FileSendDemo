////////////////////////////////////////////////////////////////////////////////
//  File Name: SDConsoleIFace.cpp
//
//  Functions:
//      控制台的命令行接口类.
//
//
////////////////////////////////////////////////////////////////////////////////

#include "SDConsoleIFace.h"
#include "SDClient.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
static vector<CSDConsleIFace*>     lConsleMap;
extern CSDClient	g_Client;

CSDConsleIFace::CSDConsleIFace()
{
}

CSDConsleIFace::~CSDConsleIFace()
{
	
}

void CSDConsleIFace::Register(CSDConsleIFace *pIFace)
{
	vector<CSDConsleIFace*>::iterator p;
	for (p=lConsleMap.begin(); p != lConsleMap.end(); p++ )
	{
		if (*p == pIFace)
		{
			return;
		}
	}

	lConsleMap.push_back(pIFace);

}

int CSDConsleIFace::Equal(char *strName) { return !strcmp(m_strCmd, strName); }

void CSDConsleIFace::UnRegister(CSDConsleIFace *pIFace)
{

	vector<CSDConsleIFace*>::iterator p;
	for (p=lConsleMap.begin(); p != lConsleMap.end(); p++ )
	{
		if (*p == pIFace)
		{
			lConsleMap.erase(p);
			return;
		}
	}

}

int CSDConsleIFace::CmdInterpreter(char *strCmd)
{

	char *strParlist = NULL;
	char strCmdTag[64];
	vector<CSDConsleIFace*>::iterator p;
    BYTE unIndex = 0;
		
	if (strlen(strCmd) < 1)
	{
		return TRUE;
	}

	while(unIndex < 64 && strCmd[unIndex] != ' ' && strCmd[unIndex] != '\0')
	{
		strCmdTag[unIndex] = strCmd[unIndex];
		unIndex++;
	}
	strCmdTag[unIndex] = '\0';
	if (unIndex > 0 && unIndex < 64)
	{
		if (strcmp(strCmdTag, "help") == 0 || strcmp(strCmdTag, "?") == 0)
		{
			strParlist = strCmd+unIndex;
			while (*strParlist == ' ') strParlist++;
			if (*strParlist == '\0')
			{
				for (p=lConsleMap.begin(); p != lConsleMap.end(); p++ )
				{
					printf("\n\n********* %s *********\n", (*p)->GetCmdName());
					(*p)->Help();
				}
				return TRUE;
			}
			else
			{
				unIndex = 0;
				while (strParlist[unIndex] != '\0')
				{
					if ( !((strParlist[unIndex] >= 'a' && strParlist[unIndex] <= 'z')
						 || (strParlist[unIndex] >= 'A' && strParlist[unIndex] <= 'Z')
						 || (strParlist[unIndex] >= '0' && strParlist[unIndex] <= '9')) )
					{
						printf(CMD_SYNTAX_ERROR"\n");
						return FALSE;
					}
					unIndex++;
				}

				for (p=lConsleMap.begin(); p != lConsleMap.end(); p++ )
				{
					if ((*p)->Equal(strParlist))
					{
						(*p)->Help();
						return TRUE;
					}
				}
			}
		}
		else
		{
			for (p=lConsleMap.begin(); p != lConsleMap.end(); p++ )
			{
				if ((*p)->Equal(strCmdTag))
				{
					return (*p)->Interpreter(strCmd + unIndex);
				}
			}
		}
	}
	
	printf(CMD_NOT_EXSIT_COMMAND"\n");

	return FALSE;
}

int  CSDConsleIFace::RunCommandLine(void *callprintf)
{
	return 0;
}

int CSDConsleIFace::RunCommandLine(const char *strCommandPrompt)
{
	int   nRet = 0;
	char  strcmd[128];
	char  strShowCmdPrompt[32];

	// 提取命令行提示符
	while (nRet < 30)
	{
		if ( (strCommandPrompt[nRet] >= 'a' && strCommandPrompt[nRet] <= 'z')
			|| (strCommandPrompt[nRet] >= 'A' && strCommandPrompt[nRet] <= 'Z')
			|| (strCommandPrompt[nRet] >= '0' && strCommandPrompt[nRet] <= '9'))
		{
			strShowCmdPrompt[nRet] = strCommandPrompt[nRet];
		}
		else
		{
			break;
		}
		nRet++;
	}
	strShowCmdPrompt[nRet] = '/';
	strShowCmdPrompt[nRet+1] = '\0';

	while (1)
	{
		memset(strcmd, 0, 128);
		printf("%s", strShowCmdPrompt);

		if ( fgets(strcmd, sizeof(strcmd), stdin) != NULL )
		{
			//过滤空格
			int i = 0;
			BOOL bEmpty = TRUE;
			for (i = 0; i < (int)strlen(strcmd); i++)
			{
				if ((strcmd[i] != ' ') && (strcmd[i] != '\n'))
				{
					bEmpty = FALSE;
					break;
				}
			}

			if (bEmpty == TRUE)
			{
				float fVideoUpRate, fVideoDownRate, fAudioUpRate, fAudioDownRate;
				g_Client.GetVideoAudioUpDownBitrate(&fVideoUpRate, &fVideoDownRate, &fAudioUpRate, &fAudioDownRate);

				float fVideoUpLostRatio, fVideoDownLostRatio, fAudioUpLostRatio, fAudioDownLostRatio;
				g_Client.GetVideoAudioUpDownLostRatio(&fVideoUpLostRatio, &fVideoDownLostRatio, &fAudioUpLostRatio, &fAudioDownLostRatio);

				printf("Video Up bitrate:%.2f kbps (%.2f)    Down bitrate:%.2f kbps (%.2f)\n ", fVideoUpRate, fVideoUpLostRatio, fVideoDownRate, fVideoDownLostRatio);
				continue;
			}

			for (i = 0; i < (int)strlen(strcmd); i++)
			{
				if (strcmd[i] == '\n')
				{
					strcmd[i] = 0;
				}
			}

			// 解释命令
			if (strcmp(strcmd, "exit") == 0)
			{
				return TRUE;
			}
			CSDConsleIFace::CmdInterpreter(strcmd);
		}
		else
		{
			break;
		}
	}

	return FALSE;
}

int CSDConsleIFace::FromStringGetMumbers(char* strCmd, int &nValue, int &nValidDataLength)
{
	char* strCurPosi = strCmd;
	int nCount = 0;
	nValidDataLength = 0;
	while( *strCurPosi == ' ' )
	{
		nCount++;
		strCurPosi++;
	}
	char strTemp[64];
	BYTE byIndex = 0;
	memset(strTemp, 0, sizeof(strTemp));
	while (*strCurPosi >= '0' && *strCurPosi <= '9' && byIndex < sizeof(strTemp)-1)
	{
		strTemp[byIndex] = *strCurPosi;
		strCurPosi++;
		byIndex++;
		nCount++;
		nValidDataLength++;
	}
	if (*strCurPosi == '\0' || *strCurPosi == '\r' || *strCurPosi == '\n' || *strCurPosi == ' ')
	{
		nValue = atoi(strTemp);
		return nCount;
	}
	else
	{
		nValidDataLength = 0;
		return -1;
	}
}


int CSDConsleIFace::FromStringGetSubString(char* strCmd, char* strValue, int &nValidDataLength)
{
	int nCount = 0;
	char* strCurPosi = strCmd;
	nValidDataLength = 0;
	while( *strCurPosi == ' ' )
	{
		nCount++;
		strCurPosi++;
	}

	while (*strCurPosi != '\0' && *strCurPosi != '\r' && *strCurPosi != '\n' && *strCurPosi != ' ')
	{
		*strValue = *strCurPosi;
		strValue++;
		strCurPosi++;
		nValidDataLength++;
	}
	*strValue = '\0';
	return (int)strlen(strValue);
}

#if 1//def CONFIG_CMDLINE

#include "Shell_CmdLine.h"

/*
支持的命令格式如�?
Test(1,2,3,4,5) �?多支持输�?个参�?
Test(1,2,0x3,4,"str")
Test 1
Test 1 "str"
支持�?写输�?
*/
static SCmdLine Sg_CmdLine;

#if 1
#define CmdLine_Strtok strtok
#else
char* CmdLine_Strtok(char* pSrc, const char* delim)
{
	static char* p = 0;
	char* pRet = Null;

	if(pSrc)
	{
		p = pSrc;
	}

	if(*p == 0)
	{
		pRet = Null;
		goto End;
	}

	pRet = p;

	//优先把字符串找出�?
	if(*p == '"')	//第一�?"'
	{
		p++;
		while(*p++ != '"')
		{
			if(*p == 0)
			{
				pRet = Null;
				goto End;
			}
		}

		if(*p != 0)
		{
			*p++ = 0;
		}
		goto End;
	}

	while(*p)
	{
		const char* pByte = delim;
		for(pByte = delim; *pByte != 0; pByte++)
		{
			if(*p == *pByte)
			{
				*p++ = 0;
				goto End;
			}
		}
		p++;
	}

End:
	return pRet;
}

#endif

/******************************************************************************
* Function: Convert HEX to INT                               
*
* Description: Convert HEX data that is input through terminal to INT data
*
* Input      : String contains HEX data that is input through terminal          
*                   
* Output:    : None
*
* Return     : INT data converted from input
*
* Others:         
******************************************************************************/
int SCmdLine_htoi(const char *s)
{
	int n = 0;

	if( !s ) return 0;

	if( *s == '0' )
	{
		s++;
		if( *s == 'x' || *s == 'X' )s++;
	}

	while(*s != '\0')
	{
		n <<= 4;
		if( *s <= '9' )
			n |= ( *s & 0xf );
		else
			n |= ( (*s & 0xf) + 9 );
		s++;
	}
	return n;
}

/******************************************************************************
* Function: Print Format                               
*
* Description:  Place string formated in buffer, then send out through terminal
*
* Input      :  lpszFormat: pointer to string that specifies print format        
*               ...:variable argument that will be printed
*                   
* Output:    : None
*
* Return     : Length of the formated string
*
* Others:         
******************************************************************************/
int SCmdLine_Printf(const char* lpszFormat, ...)
{
	int nLen = 0;
	va_list ptr;
	char g_Pfbuffer[128];
    #ifndef UART_DEBUG
    return 0;
    #endif
	//LOCK();

	memset(g_Pfbuffer, 0, sizeof(g_Pfbuffer));
	va_start(ptr, lpszFormat);
	nLen = vsnprintf(g_Pfbuffer, sizeof(g_Pfbuffer), lpszFormat, ptr);
	va_end(ptr);
	
	if(Sg_CmdLine.printf) Sg_CmdLine.printf(g_Pfbuffer);
	
	//UNLOCK();

	return nLen;
}

/******************************************************************************
* Function: Command help                               
*
* Description:  Print the command prototype
*
* Input      :  None     
*                   
* Output:    : None
*
* Return     : None
*
* Others:         
******************************************************************************/
void SCmdLine_Help()
{
	int i = 0;
	const SFnDef* pFnDef = Sg_CmdLine.m_FnArray;
	
	for(i = 0; i < Sg_CmdLine.m_FnCount; i++, pFnDef++)
	{		
		SCmdLine_Printf("\t %s\n", pFnDef->m_Title);
	}
}

/******************************************************************************
* Function: Get argument's type                            
*
* Description:  Get argument's type that is string, decimal or hexadecimal
*
* Input      :  pointer to argument that needs to be checked       
*                   
* Output:    : None
*
* Return     : Type(string, decimal or hexadecimal)
*
* Others:         
******************************************************************************/
Shell_ArgType SCmdLine_GetArgType(const char* argStr)
{
	int nLen = strlen(argStr);

	if(Null == argStr) return SARGT_NONE;

	if(*argStr == '\"')
	{
		if('\"' == argStr[nLen-1])
		{
			return SARGT_STR;
		}
		else
		{
			return SARGT_ERROR;
		}
	}
	else if(argStr[0] == '0' && (argStr[1] == 'x' || argStr[1] == 'X'))
	{
		return SARGT_HEX;
	}
	else
	{
		return SARGT_DEC;
	}
}

/******************************************************************************
* Function: Parse the command                      
*
* Description:  Get command name and arguments  
*               
* Input      :  cmdLineStr:pointer to command/function string    
*               argCount:pointer to the count of argument    
*
* Output:    :  pFnName:pointer to command/function name  
*               pArgs[]:pointer to arguments    
*
* Return     :  Parsing state
*
* Others:         
******************************************************************************/
Bool SCmdLine_Parse(char* cmdLineStr, char** pFnName, char* pArgs[], int* argCount)
{
	int maxArgCount = *argCount;
	char *token;
	char fnNameseps[]   = " (\n";
	char argSeps[]   = ", )\n";

	//Find function name
	token = CmdLine_Strtok(cmdLineStr, fnNameseps);
	if(Null == token) return False;
	*pFnName = token;

	*argCount= 0;
	token = CmdLine_Strtok( NULL, argSeps);
	while( token != NULL )
	{
		pArgs[(*argCount)++] = token;
		if((*argCount) > maxArgCount)
		{
			SCmdLine_Printf("PF_ERROR: Arg count is too many\n");
			return False;
		}
		token = CmdLine_Strtok( NULL, argSeps);
	}

	return True;
}

/******************************************************************************
* Function: Convert arguments' format                      
*
* Description:  Convert arguments' format(string, decimal or hexadecimal) to uint32
*               
* Input      :  pArgs[]:pointer to arguments    
*               argCount:pointer to the count of argument    
*
* Output:    :  arg[]:store the data converted from arguments 
*
* Return     :  Converting state
*
* Others:         
******************************************************************************/
Bool SCmdLine_ArgConvert(char* pArgs[], int argCount, uint32 arg[])
{
	int i = 0;
	Shell_ArgType at = SARGT_NONE;
	char* pChar = Null;

	for(i = 0; i < argCount; i++)
	{
		at = SCmdLine_GetArgType(pArgs[i]);
		if(SARGT_DEC == at)
		{
			arg[i] = atoi(pArgs[i]);
		}
		else if(SARGT_HEX == at)
		{
			arg[i] = SCmdLine_htoi(pArgs[i]);
		}
		else if(SARGT_STR == at)
		{
			pChar = pArgs[i];
			pChar[strlen(pChar) - 1] = 0;
			pChar++;
			arg[i] = (uint32)pChar;
		}
		else
		{
			SCmdLine_Printf("\tArg[%d](%s) error. \n", i+1, pArgs[i]);
			return False;
		}
	}
	return True;
}

/******************************************************************************
* Function: Execute command                    
*
* Description:  Execute command  
*               
* Input      :  pCmdLine:contains all the commands and related info  
*               pFnName:function/command name   
*               arg:arguments 
*               argCount:count of the arguments     
*
* Output:    :  None
*
* Return     :  None
*
* Others:         
******************************************************************************/
void SCmdLine_Exe(SCmdLine* pCmdLine, const char* pFnName, uint32 arg[], int argCount)
{
	Bool isFind = 0;
	int i = 0;
	const SFnDef* pFnEntry = pCmdLine->m_FnArray;
	const SFnDef* pFoundEntry = Null;
	
	#define FUN(n, funType, args) if(n == pFoundEntry->m_ArgCount)	\
		{	\
			((funType)pFoundEntry->pFn) args;	\
			return;	\
		}

	for(i = 0; i < pCmdLine->m_FnCount; i++, pFnEntry++)
	{
		if(strcmp(pFnName, "?") == 0)
		{
			SCmdLine_Help();
			return;
		}

		//和函数名部分比较
		if(strstr(pFnEntry->m_Title, pFnName) == pFnEntry->m_Title)
		{
			char* str;
			
			isFind++;
			
			if(Null == pFoundEntry)
				pFoundEntry = pFnEntry;
			//查找函数�?
			str	= strchr(pFnEntry->m_Title, '(');
			if(Null == str)
				str	= strchr(pFnEntry->m_Title, ' ');
			
			if(Null == str) continue;
			//和函数名完全比较
			if(memcmp(pFnEntry->m_Title, pFnName, str - pFnEntry->m_Title) == 0)
			{
				isFind = 1;
				pFoundEntry = pFnEntry;
				break;
			}
		}
	}

	if(0 == isFind)
	{
		SCmdLine_Printf("Unknown: %s\n", pFnName);
		return;
	}
	else if(isFind > 1)
	{
		//如果找出的函数名多于�?个，则打印所有的部分比较正确的函数名
		pFnEntry = pCmdLine->m_FnArray;
		for(i = 0; i < pCmdLine->m_FnCount; i++, pFnEntry++)
		{
			if(strstr(pFnEntry->m_Title, pFnName) == pFnEntry->m_Title)
			{
				SCmdLine_Printf("%s\n", pFnEntry->m_Title);
			}
		}
		return;
	}

	FUN(0, FnArg0, ());
	FUN(1, FnArg01, (arg[0]));
	FUN(2, FnArg02, (arg[0], arg[1]));
	FUN(3, FnArg03, (arg[0], arg[1], arg[2]));
	FUN(4, FnArg04, (arg[0], arg[1], arg[2], arg[3]));
	FUN(5, FnArg05, (arg[0], arg[1], arg[2], arg[3], arg[4]));
}

/******************************************************************************
* Function: Get the count of arguments                    
*
* Description:  Get the count of arguments of the functions
*               
* Input      :  str:pointer to the string of the function prototype
*
* Output:    :  None
*
* Return     :  The count of arguments, -1 indicates error  
*
* Others:         
******************************************************************************/
int SCmdLine_GetArgCount(const char* str)
{
	Bool bFlag = False;
	int nArgCount = 0;
	
	str	= strchr(str, '(');

	if(Null == str)
	{
		return 0;
	}
	
	while(*(++str) != '\0')
	{
		if(')' == *str)
		{
			break;
		}
		else if(!bFlag)
		{
			if(' ' != *str)
			{
				bFlag = True;
				nArgCount++;

				if(',' == *str)
				{
					nArgCount++;
				}
			}
		}
		else if(',' == *str)
		{
			nArgCount++;
		}
	}
	
	return *str == ')' ? nArgCount : -1;
}

void SCmdLine_Reset(SCmdLine* pCmdLine)
{
	if(pCmdLine->m_isEcho)
		SCmdLine_Printf("->");
	
	memset(pCmdLine->m_CmdLineStr, 0, sizeof(pCmdLine->m_CmdLineStr));
	pCmdLine->m_CmdLineStrLen = 0;
}

void SCmdLine_AddStr(const char* str)
{
	Shell_CmdLine_AddStrEx(str, strlen(str));
}

/******************************************************************************
* Function: Add string and execute                    
*
* Description:  Receive command from terminal and execute it
*               
* Input      :  str:pointer to the string of the function prototype received 
*               len:length of the string 
*
* Output:    :  None
*
* Return     :  None 
*
* Others:         
******************************************************************************/
void Shell_CmdLine_AddStrEx(const char* str, int len)
{
	int i = 0;
	SCmdLine* pCmdLine = &Sg_CmdLine;
	char* pBuf = pCmdLine->m_CmdLineStr;

	for(i = 0; i < len; i++, str++)
	{
		if(pCmdLine->m_CmdLineStrLen >= MAX_CMDLINE_LEN)
		{
			SCmdLine_Reset(pCmdLine);
		}
		
		if(pCmdLine->m_isEcho)
		{
			SCmdLine_Printf("%c", *str);
		}
		
		//if(*str != KEY_CR && *str != KEY_LF)
        if(*str != KEY_ENTER)            
		{
			pBuf[pCmdLine->m_CmdLineStrLen++] = *str;
		}
		//if(KEY_CR == *str)// || ')' == *str)
        if(KEY_ENTER == *str)// || ')' == *str)		
		{
			char* pFnName = Null;
			char* argStr[MAX_ARG_COUNT] = {0};
			int argCount = MAX_ARG_COUNT;
			
			if(('\r' == pBuf[0] && pCmdLine->m_CmdLineStrLen == 1) || 0 == pCmdLine->m_CmdLineStrLen)
			{
				SCmdLine_Reset(pCmdLine);
				return;
			}

			if(SCmdLine_Parse(pBuf, &pFnName, argStr, &argCount))
			{
				uint32 arg[MAX_ARG_COUNT] = {0};

				if(SCmdLine_ArgConvert(argStr, argCount, arg))
				{
					SCmdLine_Exe(pCmdLine, pFnName, arg, argCount);
				}
			}
			SCmdLine_Reset(pCmdLine);
	   	}
	}
}

/******************************************************************************
* Function: Command line initiation                   
*
* Description:  Initiate Sg_CmdLine
*               
* Input      :  pCmdTable:pointer to the command table
*               cmdTableCount:command count 
*               isEcho:indicate if it echo 
*               printf:terminal driver
*
* Output:    :  None
*
* Return     :  None 
*
* Others:         
******************************************************************************/
void SCmdLine_Init(SFnDef* pCmdTable, uint8 cmdTableCount, Bool isEcho, OutPutFun printf)
{
	int i = 0;
	SFnDef* pFnEntry = pCmdTable;
	
	memset(&Sg_CmdLine, 0, sizeof(SCmdLine));
	
	Sg_CmdLine.m_isEcho = isEcho;

	Sg_CmdLine.m_FnArray = pCmdTable;
	Sg_CmdLine.m_FnCount = cmdTableCount;
	Sg_CmdLine.printf = printf;

	for(i = 0; i < cmdTableCount; i++, pFnEntry++)
	{
		int argCount = SCmdLine_GetArgCount(pFnEntry->m_Title);
		if(argCount < 0 || argCount > MAX_ARG_COUNT)
		{
			SCmdLine_Printf("[%s] error, get arg count[%d] error.\n", pFnEntry->m_Title, pFnEntry->m_ArgCount);
		}
		
		pFnEntry->m_ArgCount = (int8)argCount;
	}
}

#endif


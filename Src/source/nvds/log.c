/*
 * File      : log.c
 * This file is part of FW_G2_CC
 * Copyright (C) 2020 by IMMOTOR
 *
 * SPI FLASH
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-14     lane      first implementation
 */
#include "common.h"
#include "Log.h"
#include "nvds.h"
#include "record.h"

/*******************************
函数功能：大括号字符换内容combo string处理，例如braceCheck("0-title1|1-title2", 1, buf, 0) => "0:title1"
参数说明：
	string：待处理的字符串，格式为 "0-title1|1-title2"
	value:  字符串的值域。
	buf:    输出缓冲区
	offset: 输出缓冲区保存数据的偏移值
返回值：buf缓冲区保存数据之后的偏移值
***********************************/
int braceProc(const char* str, int value, char* buf, int offset)
{
	Bool isMatch = False;
	int comboStrVal = 0;
	while (str)
	{
		int ret = sscanf(str, "%d-", &comboStrVal);
		if (sscanf(str, "%d-", &comboStrVal) == 1 && comboStrVal == value)
		{
			//互联":"之前的数值
			//str = strchr(str, ':') + 1;
			while ((*str != '|') && (*str != 0))
			{
				isMatch = True;
				buf[offset++] = *str;
				str++;
			}
			break;
		}
		str = strchr(str, '|');
		if (str) str += 1;	//越过字符'|'
	}

	if (!isMatch)
	{
		offset += sprintf(&buf[offset], "%d-unknown", value);
	}

	return offset;
}

/************************************************************
函数功能：32位整数按照BIT格式化输出。
参数说明：
	buf：输出缓冲区。
	val：输出的目标值。
	fmt：格式化内容，支持以下格式：
		"%1B"		//输出val.BIT[0], 数值显示为10进制
		"%1BX"		//输出和上面一样
		"%0-1B"		//输出val.BIT[0-1], 数值显示为10进制
		"%2-6BX"	//输出val.BIT[2-6], 数值显示为16进制
		"%7-10B{1:Title1|2:Title2|...}"//输出val.BIT[7-10]，数值使用大括号内的字符串代替,大括号的字符串长度不能超过128个字节。
		"%7-10BX{1:Title1|2:Title2|...}"//输出和上面一样。
	numberOfBit：根据输出格式fmt计算的val的有效Bit总数。如果为Null，表示忽略这个参数。
返回值：buf的长度
************************************************************/
int SprintfBit(char buf[], const char* fmt, uint32 val, uint8* numberOfBit)
{
	Bool isMatch = False;
	uint8 byte, byte1, byte2 = 0;
	uint8 bits = 0;
	int v1, v2, len = 0;
	const char* s = fmt;
	const char* p = strstr(fmt, "%");
	char brace[128];

	while (p)
	{
		static int8 strFmt[] = { '%', 'X', 0 };
		memcpy(&buf[len], s, (uint32)p - (uint32)s);
		len += (uint32)p - (uint32)s;
		s = p;

		p++;

		if ((3 <= sscanf(p, "%d%c%c%c", &v1, &byte, &byte1, &byte2)) && (byte == 'B' || byte == 'b'))	//Search string "%0B" or "%0BX"
		{
			isMatch = True;

			//获取字符串的{}之间的字符 x#${0:title1|1:title2}=>0:title1|1:title2"
			if ((byte1 == '{' || byte2 == '{') && sscanf(p, "%*[^{]{%[^}]", brace) == 1)
			{
				//大括号内的COMBO字符处理
				len = braceProc(brace, GET_BITS(val, v1, v1), buf, len);
				s = strstr(p, "}") + 1;
			}
			else
			{
				len += sprintf(&buf[len], "%d", GET_BITS(val, v1, v1));
				if (byte1 == 'X' || byte1 == 'x')
					s = strchr(p, byte1) + 1;
				else
					s = strstr(p, "B") + 1;
			}

			bits = MAX(bits, v1);
		}
		else
		{
			int ret = sscanf(p, "%d-%d%c%c%c", &v1, &v2, &byte, &byte1, &byte2);

			if (ret >= 3 && (byte == 'B' || byte == 'b')) //%1-7B
			{
				isMatch = True;

				v1 = MIN(v1, v2);
				v2 = MAX(v1, v2);

				if (ret >= 4)	//%1-7BX
				{
					strFmt[1] = (byte1 == 'X' || byte1 == 'x') ? byte1 : 'd';
				}
				if ((byte1 == '{' || byte2 == '{') && sscanf(p, "%*[^{]{%[^}]", brace) == 1)	//%1-7BX{...}
				{
					//大括号内的COMBO字符处理
					len = braceProc(brace, GET_BITS(val, v1, v2), buf, len);
					s = strstr(p, "}") + 1;
				}
				else
				{
					len += sprintf(&buf[len], strFmt, GET_BITS(val, v1, v2));
					if (byte1 == 'X' || byte1 == 'x')
						s = strchr(p, byte1) + 1;
					else
						s = strstr(p, "B") + 1;
				}
				bits = MAX(bits, v1);
				bits = MAX(bits, v2);
			}
		}

		//Search next "%..B"
		p = strstr(p, "%");
	}

	//Copy tail of string to buff
	p = (char*)(fmt + strlen(fmt));
	memcpy(&buf[len], s, (uint32)p - (uint32)s);
	len += (uint32)p - (uint32)s;
	buf[len++] = 0;

	if (numberOfBit) * numberOfBit = bits;
	return isMatch ? len : 0;
}

void Log_Init(LogItem* item, uint8 moduleId, uint8 catId, uint8 subId, LogType logType, uint8 eventId, uint32 val)
{
	item->dateTime = DateTime_GetSeconds(Null);
	item->moduleId		= moduleId;
	item->subId	= subId;
	item->logType		= logType;
	item->catId			= catId;

	item->eventId = eventId;
	memcpy(item->data, &val, 4);
}

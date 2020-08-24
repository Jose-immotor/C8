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
�������ܣ��������ַ�������combo string��������braceCheck("0-title1|1-title2", 1, buf, 0) => "0:title1"
����˵����
	string����������ַ�������ʽΪ "0-title1|1-title2"
	value:  �ַ�����ֵ��
	buf:    ���������
	offset: ����������������ݵ�ƫ��ֵ
����ֵ��buf��������������֮���ƫ��ֵ
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
			//����":"֮ǰ����ֵ
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
		if (str) str += 1;	//Խ���ַ�'|'
	}

	if (!isMatch)
	{
		offset += sprintf(&buf[offset], "%d-unknown", value);
	}

	return offset;
}

/************************************************************
�������ܣ�32λ��������BIT��ʽ�������
����˵����
	buf�������������
	val�������Ŀ��ֵ��
	fmt����ʽ�����ݣ�֧�����¸�ʽ��
		"%1B"		//���val.BIT[0], ��ֵ��ʾΪ10����
		"%1BX"		//���������һ��
		"%0-1B"		//���val.BIT[0-1], ��ֵ��ʾΪ10����
		"%2-6BX"	//���val.BIT[2-6], ��ֵ��ʾΪ16����
		"%7-10B{1:Title1|2:Title2|...}"//���val.BIT[7-10]����ֵʹ�ô������ڵ��ַ�������,�����ŵ��ַ������Ȳ��ܳ���128���ֽڡ�
		"%7-10BX{1:Title1|2:Title2|...}"//���������һ����
	numberOfBit�����������ʽfmt�����val����ЧBit���������ΪNull����ʾ�������������
����ֵ��buf�ĳ���
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

			//��ȡ�ַ�����{}֮����ַ� x#${0:title1|1:title2}=>0:title1|1:title2"
			if ((byte1 == '{' || byte2 == '{') && sscanf(p, "%*[^{]{%[^}]", brace) == 1)
			{
				//�������ڵ�COMBO�ַ�����
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
					//�������ڵ�COMBO�ַ�����
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

/*
 * File      : smart_shell.c
 * This file is part of FW_G2_CC
 * Copyright (C) 2020 by IMMOTOR
 *
 * ���ں�shell.c��ͻ����������smart_shell.c
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-25     lane      first implementation
 */
 
 #include <stdio.h>
//#include "ntag_app.h"
////#include "battery_process.h"
#include <stdio.h>
#include <stdint.h>

#include "gd32f403.h"
#if 0
/*!
 * \brief ��ӡ״̬��Ϣ
 *		  
 * \param  argc:��ӡ��־
 *
 * \return NONE 
 */
static void Dump(int argc, char**argv)
{
	int ind = 0;

	sscanf(&(*argv[1]), "%d", &ind);

	//if(7 == ind || 0 == ind) BatteryDump(Null);
}
MSH_CMD_EXPORT(Dump, Dump sample: Dump <uint8_t ind>);

/*!
 * \brief �Լ죬����·��Ӳ��
 *		  
 * \param   NONE 
 *
 * \return NONE 
 */
static void SelfTest(void)
{
//	extern void ErrList_Dump(void);
	__IO uint32_t sn0=*(__IO uint32_t*)(0x1FFFF7E8);
	__IO uint32_t sn1=*(__IO uint32_t*)(0x1FFFF7EC);
	__IO uint32_t sn2=*(__IO uint32_t*)(0x1FFFF7F0);
	
	Printf("\r\nsID: %X%X%X\r\n",sn2,sn1,sn0);

}
MSH_CMD_EXPORT(SelfTest , SelfTest board);
#endif

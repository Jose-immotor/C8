/*
 * File      : smart_shell.h
 * This file is part of FW_G2_CC
 * Copyright (C) 2020 by IMMOTOR
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-08     lane      first implementation
 */

#ifndef __SMART_SHELL_H__
#define __SMART_SHELL_H__

#include <rtthread.h>
#include "common.h"
//#include "safebuf.h"

//typedef struct{
////    rt_device_t usart_dev;
//	SafeBuf     shell_SafeBuf;
//}smart_shell_param_t;
//extern smart_shell_param_t smart_shell_param;

void Smart_shell_init();
	
#endif

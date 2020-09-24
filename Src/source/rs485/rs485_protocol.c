
#include "rs485_protocol.h"
#include "common.h"
#include "drv_usart.h"

void RS485_Run(void)
{
	char data;
	
	data = rs485_getchar();
	if (data == 0xff)
	{
		return;
	}
	else
		Printf("rs485 receive:%d",data);
}

void RS485_Init(void)
{
	const static Obj obj = {
	.name = "RS485",
	.Run = RS485_Run,
	};

	ObjList_add(&obj);
}

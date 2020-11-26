/*
一线通的控制逻辑:
	先发送低位。发码头码前拉低4ms~6ms。时序范围为400us：1200us到1ms：3ms之间。建议参考时序800us：2400us；
	频繁发送时，数据与数据之间大于100ms间隔。
	BUSY是语音播放忙信号，用户可以根据情况是否使用。
	上电需要等待100ms后才可以给语音芯片发码。

	BIT-0: 高电平:低电平 = 1:3
	BIT-1: 高电平:低电平 = 3:1
*/
#include "common.h"
#include "Sif.h"
#include "nvc.h"

void SIF_ISR_ENABLE(void)
{
	timer_counter_value_config(TIMER3,0);
	timer_enable(TIMER3);
} 
void SIF_ISR_DISABLE(void)
{
	timer_disable(TIMER3);
} 

#define SIF_LOW() {pSif->initTicks = g_SifTicks; SIF_IO_LOW();}
#define SIF_HIGH(){pSif->initTicks = g_SifTicks; SIF_IO_HIGH();}
#define SIF_LOW_FOR(us) {SIF_LOW();  pSif->lowTicks = us;}
#define SIF_HIGH_FOR(us){SIF_HIGH(); pSif->highTicks = us;}

#define SIF_TX_BIT_0() {pSif->highTicks = 800 ; pSif->lowTicks = 2400; SIF_HIGH();}
#define SIF_TX_BIT_1() {pSif->highTicks = 2400; pSif->lowTicks = 800 ; SIF_HIGH();}

static uint32 g_SifTicks = 1;

static Bool Sif_isTimerOut(uint32 initTicks, uint32 timeOutTicks)
{
	uint32 totalTicks = 0;
	
	if(g_SifTicks < initTicks)
	{
		//tick counter overflow
		totalTicks = 0xFFFFFFFF - initTicks + g_SifTicks;
	}
	else
	{
		totalTicks = g_SifTicks - initTicks;
	}

	return (totalTicks >= timeOutTicks);
}

void Sif_Isr(Sif* pSif, uint32 us)
{
	g_SifTicks += us;

	if(pSif->isDone) return;

	if(pSif->initTicks)
	{
		if(pSif->highTicks) 
		{
			if(!Sif_isTimerOut(pSif->initTicks, pSif->highTicks)) return;

			pSif->highTicks = 0;
			pSif->initTicks = 0;
			if(pSif->lowTicks)
			{
				SIF_LOW();
				return;
			}
		}
		
		if(pSif->lowTicks)
		{
			if(!Sif_isTimerOut(pSif->initTicks, pSif->lowTicks)) return;

			pSif->lowTicks = 0;
			pSif->initTicks = 0;
			SIF_IO_HIGH();
		}
	}

	//数据是否发送完毕
	if(pSif->txByteCount == pSif->txByteInd)
	{
		SIF_ISR_DISABLE();
		if(!pSif->isDone)
		{
			pSif->Done(pSif);
		}
		pSif->isDone = True;
		SIF_IO_HIGH();	// 默认保持拉高
		return;
	}
	
	//判断8个BIT是否发送完毕
	if(pSif->txBitInd >= 8)
	{
		pSif->txBitInd = 0;
		
		//判断所有数据是否发送完毕
		if(++pSif->txByteInd >= pSif->txByteCount)
		{
			SIF_HIGH_FOR(100000);	//结束后拉高并保持为100MS，才能发送下一帧数据
			return;
		}
	}
	
	if((pSif->buff[pSif->txByteInd] >> pSif->txBitInd) & 0x01)
	{
		SIF_TX_BIT_1();
	}
	else
	{
		SIF_TX_BIT_0();
	}
	
	pSif->txBitInd++;
}

void Sif_Tx(Sif* pSif)
{
	Printf("Sif Tx:0x%x\n", pSif->buff[0]);
	Sif_Reset(pSif);

	//发送Frame head, 拉低4毫秒	
	SIF_LOW_FOR(4000);

	pSif->isDone = False;
	SIF_ISR_ENABLE();
}

void Sif_TxByte(Sif* pSif, uint8 byte)
{
	pSif->buff[0] = byte;
	pSif->txByteCount = 1;
	Sif_Tx(pSif);
}

void Sif_TxBytes(Sif* pSif, const void* pData, int len)
{
	memcpy(pSif->buff, pData, len);
	pSif->txByteCount = len;
	Sif_Tx(pSif);
}

void Sif_Reset(Sif* pSif)
{
	pSif->txByteInd = 0;
	pSif->txBitInd  = 0;
	pSif->initTicks = 0;
	pSif->isDone    = True;
	pSif->lowTicks  = 0;
	pSif->highTicks = 0;
	
	SIF_ISR_DISABLE();
	SIF_IO_HIGH();
}

void Sif_Init(Sif* pSif, SifDoneFn done)
{
	memset(pSif, 0, sizeof(Sif));
	pSif->Done = done;
	pSif->isDone = True;
	SIF_ISR_DISABLE();
	//平时保持为HIGH
	SIF_IO_HIGH();
}

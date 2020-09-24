/*
һ��ͨ�Ŀ����߼�:
	�ȷ��͵�λ������ͷ��ǰ����4ms~6ms��ʱ��ΧΪ400us��1200us��1ms��3ms֮�䡣����ο�ʱ��800us��2400us��
	Ƶ������ʱ������������֮�����100ms�����
	BUSY����������æ�źţ��û����Ը�������Ƿ�ʹ�á�
	�ϵ���Ҫ�ȴ�100ms��ſ��Ը�����оƬ���롣

	BIT-0: �ߵ�ƽ:�͵�ƽ = 1:3
	BIT-1: �ߵ�ƽ:�͵�ƽ = 3:1
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

	//�����Ƿ������
	if(pSif->txByteCount == pSif->txByteInd)
	{
		SIF_ISR_DISABLE();
		if(!pSif->isDone)
		{
			pSif->Done(pSif);
		}
		pSif->isDone = True;
		SIF_IO_HIGH();	// Ĭ�ϱ�������
		return;
	}
	
	//�ж�8��BIT�Ƿ������
	if(pSif->txBitInd >= 8)
	{
		pSif->txBitInd = 0;
		
		//�ж����������Ƿ������
		if(++pSif->txByteInd >= pSif->txByteCount)
		{
			SIF_HIGH_FOR(100000);	//���������߲�����Ϊ100MS�����ܷ�����һ֡����
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

	//����Frame head, ����4����	
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
	//ƽʱ����ΪHIGH
	SIF_IO_HIGH();
}

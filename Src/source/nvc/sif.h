#ifndef __SIF_H_
#define __SIF_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "gd32e10x.h"

#define NVC_PWR_PORT	GPIOC
#define NVC_PWR_PIN		GPIO_PIN_4

#define NVC_SIF_PORT	GPIOB
#define NVC_SIF_PIN		GPIO_PIN_0

#define SIF_IO_HIGH()	gpio_bit_set(NVC_SIF_PORT,NVC_SIF_PIN)
#define SIF_IO_LOW()	gpio_bit_reset(NVC_SIF_PORT,NVC_SIF_PIN)

#define SIF_BUF_SIZE 2

typedef enum _SifState
{
	SIF_INIT = 0
	,SIF_TX_HEAD
	,SIF_TX_DATA
	,SIF_WAIT_TIMEOUT
}SifState;

struct _Sif;
typedef void (*SifDoneFn)(struct _Sif* pSif);
typedef struct _Sif
{
	uint8 txByteInd;
	uint8 txBitInd;
	uint8 txByteCount;
	uint8 buff[SIF_BUF_SIZE];

	uint8  isDone;
	uint32 initTicks;
	uint32 lowTicks;
	uint32 highTicks;

	SifDoneFn Done;
}Sif;

inline Bool Sif_isDone(Sif* pSif){return pSif->isDone;};

void Sif_Init(Sif* pSif, SifDoneFn done);
void Sif_TxByte(Sif* pSif, uint8 byte);
void Sif_Reset(Sif* pSif);
void Sif_Isr(Sif* pSif, uint32 us);

#ifdef __cplusplus
}
#endif

#endif



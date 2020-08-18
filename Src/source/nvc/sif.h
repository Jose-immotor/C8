#ifndef __SIF_H_
#define __SIF_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "gd32f403.h"

#define NVC_SIF_PORT	GPIOC
#define NVC_SIF_PIN		GPIO_PIN_6
#define NVC_BSY_PORT	GPIOC
#define NVC_BSY_PIN		GPIO_PIN_7
#define NVC_PWR_PORT	GPIOC
#define NVC_PWR_PIN		GPIO_PIN_8

#define NVC_PWR_ON()	gpio_bit_set(NVC_PWR_PORT,NVC_PWR_PIN)
#define NVC_PWR_OFF()	gpio_bit_reset(NVC_PWR_PORT,NVC_PWR_PIN)

#define NVC_IS_BSY() 	(gpio_input_bit_get(NVC_BSY_PORT,NVC_BSY_PIN) == RESET)

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



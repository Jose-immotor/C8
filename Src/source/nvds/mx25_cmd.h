/*
 * COPYRIGHT (c) 2010-2014 MACRONIX INTERNATIONAL CO., LTD
 * SPI Flash Low Level Driver (LLD) Sample Code
 *
 * SPI interface command hex code, type definition and function prototype.
 *
 * $Id: MX25_CMD.h,v 1.20 2013/11/08 01:41:48 modelqa Exp $
 */
#ifndef    __MX25_CMD_H__
#define    __MX25_CMD_H__

#include "typedef.h"
#include <stdint.h>

#define    FlashID          0xc22015

// Return Message
typedef enum {
    FlashOperationSuccess,
    FlashWriteRegFailed,
    FlashTimeOut,
    FlashIsBusy,
    FlashQuadNotEnable,
    FlashAddressInvalid
}ReturnMsg;

// Flash status structure define
struct sFlashStatus{
    /* Mode Register:
     * Bit  Description
     * -------------------------
     *  7   RYBY enable
     *  6   Reserved
     *  5   Reserved
     *  4   Reserved
     *  3   Reserved
     *  2   Reserved
     *  1   Parallel mode enable
     *  0   QPI mode enable
    */
    uint8    mode_reg;
    BOOL     ArrangeOpt;
};

typedef struct sFlashStatus FlashStatus;

/* Basic functions */
void Mx25_Init(void);
void Mx25_Start(void);
void Mx25_CsHigh(void);
void Mx25_CsLow(void);

/* Utility functions */
void Initial_Spi(void);
BOOL Mx25_WaitReady( uint32 ExpectTime );
BOOL Mx25_WaitRYBYReady( uint32 ExpectTime );
BOOL Mx25_IsBusy( void );
BOOL Mx25_IsQIO( void );

/* Flash commands */
ReturnMsg Mx25_RDID( uint32 *Identification );
ReturnMsg Mx25_RES( uint8 *ElectricIdentification );
ReturnMsg Mx25_REMS( uint16 *REMS_Identification, FlashStatus *fsptr );

ReturnMsg Mx25_RDSR( uint8 *StatusReg );
#ifdef SUPPORT_WRSR_CR
   ReturnMsg Mx25_WRSR( uint16 UpdateValue );
#else
   ReturnMsg Mx25_WRSR( uint8 UpdateValue );
#endif
ReturnMsg Mx25_RDSCUR( uint8 *SecurityReg );
ReturnMsg Mx25_WRSCUR( void );

ReturnMsg Mx25_Read( uint32 flash_address, uint8 *target_address, uint32 byte_length  );
ReturnMsg Mx25_FASTREAD( uint32 flash_address, uint8 *target_address, uint32 byte_length );
ReturnMsg Mx25_DREAD( uint32 flash_address, uint8 *target_address, uint32 byte_length );
ReturnMsg Mx25_RDSFDP( uint32 flash_address, uint8 *target_address, uint32 byte_length );

ReturnMsg Mx25_WREN( void );
ReturnMsg Mx25_WRDI( void );
ReturnMsg Mx25_WritePage( uint32 flash_address, const uint8 *source_address, uint32 byte_length );

ReturnMsg Mx25_EraseSector( uint32 flash_address );
ReturnMsg Mx25_EraseBlock( uint32 flash_address );
ReturnMsg Mx25_EraseChip( void );

ReturnMsg Mx25_DP( void );
ReturnMsg Mx25_RDP( void );
ReturnMsg Mx25_ENSO( void );
ReturnMsg Mx25_EXSO( void );

uint8 Mx25_ReadIdTest( void );
uint8 Mx25_ReadWriteTest( void );
Bool Mx25_Write(uint32 offset, const void* pData, int len);

#endif    /* __MX25_CMD_H__ */

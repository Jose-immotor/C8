/*
 * COPYRIGHT (c) 2010-2014 MACRONIX INTERNATIONAL CO., LTD
 * SPI Flash Low Level Driver (LLD) Sample Code
 *
 * SPI interface command set
 *
 * $Id: MX25_CMD.c,v 1.29 2013/08/12 02:56:37 mxclldb1 Exp $
 */

#include "mx25_cmd.h"
#include "drv_spi.h"

/*
 * Function:       Mx25_Read
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 * Description:    The READ instruction is for reading data out.
 * Return Message: FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg Mx25_Read( uint32 flash_address, uint8 *target_address, uint32 byte_length )
{
	spi_flash_buffer_read(target_address,flash_address,byte_length);
    return FlashOperationSuccess;
}

/*
 * Function:       Mx25_EraseSector
 * Arguments:      flash_address, 32 bit flash memory address
 * Description:    The SE instruction is for erasing the data
 *                 of the chosen sector (4KB) to be "1".
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
ReturnMsg Mx25_EraseSector( uint32 flash_address )
{	
	spi_flash_sector_erase(flash_address);
    return FlashOperationSuccess;
}

//写数据进Flash,
//注意:调用该函数之前必须确保擦除操作已经完成
Bool Mx25_Write(uint32 offset, const void* pData, int len)
{
	spi_flash_buffer_write((uint8_t*)pData,offset,len);
    return True;
}

/*
 * Simple flash ID test
 */
uint8 Mx25_ReadIdTest( void )
{
	#define  Error_inc(x)  x = x + 1;
    uint32  flash_id = 0;
    uint16  error_cnt = 0;

//    ReturnMsg  msg;
	flash_id = spi_flash_read_id();

    /* Compare to expected value */
    if( flash_id != FlashID )
    {
		//Printf("Flash ID test failed, 0x%08x != 0x%08x.\n", flash_id, FlashID);
        Error_inc( error_cnt );
		goto End ;
    }

End:
//	Printf("Flash ID test %s.\n", (error_cnt == 0)? "passed" : "failed");
	return (error_cnt == 0);
}

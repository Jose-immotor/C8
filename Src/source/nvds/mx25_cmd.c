/*
 * COPYRIGHT (c) 2010-2014 MACRONIX INTERNATIONAL CO., LTD
 * SPI Flash Low Level Driver (LLD) Sample Code
 *
 * SPI interface command set
 *
 * $Id: MX25_CMD.c,v 1.29 2013/08/12 02:56:37 mxclldb1 Exp $
 */

#include "common.h"
#include "mx25_cmd.h"
#include "drv_spi.h"
//#include "page.h"
#include <rtthread.h>




void Mx25_Init()
{



}

void Mx25_Start()
{

}

/*
 * ID Command
 */

/*
 * Function:       Mx25_RDID
 * Arguments:      Identification, 32 bit buffer to store id
 * Description:    The RDID instruction is to read the manufacturer ID
 *                 of 1-byte and followed by Device ID of 2-byte.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg Mx25_RDID( uint32 *Identification )
{

    return FlashOperationSuccess;
}

/*
 * Function:       Mx25_RES
 * Arguments:      ElectricIdentification, 8 bit buffer to store electric id
 * Description:    The RES instruction is to read the Device
 *                 electric identification of 1-byte.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg Mx25_RES( uint8 *ElectricIdentification )
{


    return FlashOperationSuccess;
}

/*
 * Function:       Mx25_REMS
 * Arguments:      REMS_Identification, 16 bit buffer to store id
 *                 fsptr, pointer of flash status structure
 * Description:    The REMS instruction is to read the Device
 *                 manufacturer ID and electric ID of 1-byte.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg Mx25_REMS( uint16 *REMS_Identification, FlashStatus *fsptr )
{
 

    return FlashOperationSuccess;
}


/*
 * Register  Command
 */

/*
 * Function:       Mx25_RDSR
 * Arguments:      StatusReg, 8 bit buffer to store status register value
 * Description:    The RDSR instruction is for reading Status Register Bits.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg Mx25_RDSR( uint8 *StatusReg )
{
    

    return FlashOperationSuccess;
}


/*
 * Function:       Mx25_WRSR
 * Arguments:      UpdateValue, 8/16 bit status register value to updata
 * Description:    The WRSR instruction is for changing the values of
 *                 Status Register Bits (and configuration register)
 * Return Message: FlashIsBusy, FlashTimeOut, FlashOperationSuccess
 */
ReturnMsg Mx25_WRSR( uint8 UpdateValue )
{
   
        return FlashOperationSuccess;


}

/*
 * Function:       Mx25_RDSCUR
 * Arguments:      SecurityReg, 8 bit buffer to store security register value
 * Description:    The RDSCUR instruction is for reading the value of
 *                 Security Register bits.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg Mx25_RDSCUR( uint8 *SecurityReg )
{


    return FlashOperationSuccess;

}


/*
 * Function:       Mx25_DREAD
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 * Description:    The DREAD instruction enable double throughput of Serial
 *                 Flash in read mode
 * Return Message: FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg Mx25_DREAD( uint32 flash_address, uint8 *target_address, uint32 byte_length )
{


    return FlashOperationSuccess;
}


/*
 * Function:       Mx25_FASTREAD
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 * Description:    The FASTREAD instruction is for quickly reading data out.
 * Return Message: FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg Mx25_FASTREAD( uint32 flash_address, uint8 *target_address, uint32 byte_length )
{
 

    return FlashOperationSuccess;
}


/*
 * Function:       Mx25_RDSFDP
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 * Description:    RDSFDP can retrieve the operating characteristics, structure
 *                 and vendor-specified information such as identifying information,
 *                 memory size, operating voltages and timinginformation of device
 * Return Message: FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg Mx25_RDSFDP( uint32 flash_address, uint8 *target_address, uint32 byte_length )
{
  

    return FlashOperationSuccess;
}
/*
 * Program Command
 */

/*
 * Function:       Mx25_WREN
 * Arguments:      None.
 * Description:    The WREN instruction is for setting
 *                 Write Enable Latch (WEL) bit.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg Mx25_WREN( void )
{
  

    return FlashOperationSuccess;
}

/*
 * Function:       Mx25_WRDI
 * Arguments:      None.
 * Description:    The WRDI instruction is to reset
 *                 Write Enable Latch (WEL) bit.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg Mx25_WRDI( void )
{
  

    return FlashOperationSuccess;
}

/*
 * Function:       Mx25_WRSCUR
 * Arguments:      None.
 * Description:    The WRSCUR instruction is for changing the values of
 *                 Security Register Bits.
 * Return Message: FlashIsBusy, FlashOperationSuccess, FlashWriteRegFailed,
 *                 FlashTimeOut
 */
ReturnMsg Mx25_WRSCUR( void )
{
 
                return FlashWriteRegFailed;


}


/*
 * Read Command
 */

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
 * Function:       Mx25_WritePage
 * Arguments:      flash_address, 32 bit flash memory address
 *                 source_address, buffer address of source data to program
 *                 byte_length, byte length of data to programm
 * Description:    The PP instruction is for programming
 *                 the memory to be "0".
 *                 The device only accept the last 256 byte ( or 32 byte ) to program.
 *                 If the page address ( flash_address[7:0] ) reach 0xFF, it will
 *                 program next at 0x00 of the same page.
 *                 Some products have smaller page size ( 32 byte )
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
ReturnMsg Mx25_WritePage( uint32 flash_address, const uint8 *source_address, uint32 byte_length )
{
   
        return FlashOperationSuccess;

}


/*
 * Erase Command
 */

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


/*
 * Function:       Mx25_EraseBlock
 * Arguments:      flash_address, 32 bit flash memory address
 * Description:    The BE instruction is for erasing the data
 *                 of the chosen sector (64KB) to be "1".
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
ReturnMsg Mx25_EraseBlock( uint32 flash_address )
{
   
        return FlashOperationSuccess;

}

/*
 * Function:       Mx25_EraseChip
 * Arguments:      None.
 * Description:    The CE instruction is for erasing the data
 *                 of the whole chip to be "1".
 * Return Message: FlashIsBusy, FlashOperationSuccess, FlashTimeOut
 */
ReturnMsg Mx25_EraseChip( void )
{
	return FlashOperationSuccess;
}

/*
 * Function:       Mx25_DP
 * Arguments:      None.
 * Description:    The DP instruction is for setting the
 *                 device on the minimizing the power consumption.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg Mx25_DP( void )
{
    return FlashOperationSuccess;
}

/*
 * Function:       Mx25_RDP
 * Arguments:      None.
 * Description:    The Release from RDP instruction is
 *                 putting the device in the Stand-by Power mode.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg Mx25_RDP( void )
{


    return FlashOperationSuccess;
}

/*
 * Function:       Mx25_ENSO
 * Arguments:      None.
 * Description:    The ENSO instruction is for entering the secured OTP mode.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg Mx25_ENSO( void )
{

    return FlashOperationSuccess;
}

/*
 * Function:       Mx25_EXSO
 * Arguments:      None.
 * Description:    The EXSO instruction is for exiting the secured OTP mode.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg Mx25_EXSO( void )
{


    return FlashOperationSuccess;
}

//写数据进Flash,
//注意:调用该函数之前必须确保擦除操作已经完成
Bool Mx25_Write(uint32 offset, const void* pData, int len)
{
	spi_flash_buffer_write((uint8_t*)pData,offset,len);
    return True;
}

/*(
 * Reset setting Command
 */

/*
 * Security Command
 */

/*
 * Suspend/Resume Command
 */


#define TEST_FUNCTION

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
	Printf("Flash ID test %s.\n", (error_cnt == 0)? "passed" : "failed");
	return (error_cnt == 0);
}

/*
 * Simple flash read/write test
 */
uint8 Mx25_ReadWriteTest( void )
{
////    FlashStatus  flash_state = {0};
//#define  TEST_COUNT 		64
//#define  TRANS_LENGTH  		512
//#define  RANDOM_SEED   		106
//#define  FLASH_TARGET_ADDR  0x00020000

//    uint32  flash_addr;
//    uint32  trans_len = 0;
//    uint16  i=0, error_cnt = 0;
//    uint16  seed = 0;
////    uint8   st_reg = 0;
//    uint8   memory_addr[TRANS_LENGTH] = {0};
//    uint8   memory_addr_cmp[TRANS_LENGTH] = {0};

//    /* Assign initial condition */
//    flash_addr = FLASH_TARGET_ADDR;
//    trans_len = TRANS_LENGTH;
//    seed = RANDOM_SEED;

//    /* Prepare data to transfer */
//    srand( seed );
//    for( i=0; i< trans_len; i=i+1 ){
//        memory_addr[i] = rand()%256;   // generate random byte data
//    }
//		
//    /* Erase 4K sector of flash memory
//       Note: It needs to erase dirty sector before program */
//    Mx25_EraseSector( flash_addr );

//    /* Program data to flash memory */
//    Mx25_Write( flash_addr, memory_addr, trans_len );

//    /* Read flash memory data to memory buffer */
//    Mx25_Read( flash_addr, memory_addr_cmp, trans_len );

//    /* Compare original data and flash data */
//    for( i=0; i < (trans_len); i=i+1 )
//    {
//        if( memory_addr[i] != memory_addr_cmp[i] )
//		{
//            error_cnt++;
//			DUMP_BYTE(memory_addr, trans_len);
//			Printf("ReadWriteTest.\n");
//			DUMP_BYTE(memory_addr_cmp, trans_len);
//			break;
//		}
//    }
//#if 0
//	flash_addr += trans_len;
//	
//    /* Program data to flash memory */
//    Mx25_WritePage( flash_addr, memory_addr, trans_len );
//	memset(memory_addr_cmp, 0, trans_len);
//    Mx25_Read( 0, memory_addr_cmp, trans_len );
//	memset(memory_addr_cmp, 0, trans_len);
//    Mx25_Read( flash_addr, memory_addr_cmp, trans_len );
//    /* Compare original data and flash data */
//    for( i=0; i < (trans_len); i=i+1 )
//    {
//        if( memory_addr[i] != memory_addr_cmp[i] )
//		{
//            Error_inc( error_cnt );
//			break;
//		}
//    }
//#endif
//    /* Erase 4K sector of flash memory */
//    Mx25_EraseSector( flash_addr );
// 
//	Printf("Mx25_ReadWriteTest: %s\r\n", (error_cnt == 0 ) ? "OK" : "FAILED");
//    return ( error_cnt == 0 );

}

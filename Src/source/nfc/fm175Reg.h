#if 1

#ifndef FM175XX_REG_H
#define FM175XX_REG_H

#include "typedef.h"

#define NFC_I2C		I2C1

#define CommandReg	0x01
#define ComIEnReg	0x02
#define DivIEnReg	0x03
#define ComIrqReg	0x04
#define DivIrqReg	0x05
#define ErrorReg	0x06
#define Status1Reg	0x07
#define Status2Reg	0x08
#define FIFODataReg 0x09
#define FIFOLevelReg 0x0A
#define WaterLevelReg 0x0B
#define ControlReg	0x0C
#define BitFramingReg 0x0D
#define CollReg	0x0E
#define ModeReg 0x11
#define TxModeReg 0x12
#define RxModeReg 0x13
#define TxControlReg 0x14
#define TxAutoReg 0x15
#define TxSelReg 0x16
#define RxSelReg 0x17
#define RxThresholdReg 0x18
#define DemodReg 0x19
#define MfTxReg 0x1C
#define MfRxReg 0x1D
#define SerialSpeedReg 0x1F
#define CRCMSBReg 0x21
#define CRCLSBReg 0x22
#define ModWidthReg 0x24
#define GsNOffReg 0x23
#define TxBitPhaseReg 0x25
#define RFCfgReg 0x26
#define GsNReg 0x27
#define CWGsPReg 0x28
#define ModGsPReg 0x29
#define TModeReg 0x2A
#define TPrescalerReg 0x2B
#define TReloadMSBReg 0x2C
#define TReloadLSBReg 0x2D
#define TCounterValMSBReg 0x2E
#define TCounterValLSBReg 0x2F
#define TestSel1Reg 0x31
#define TestSel2Reg 0x32
#define TestPinEnReg 0x33
#define TestPinValueReg 0x34
#define TestBusReg 0x35
#define AutoTestReg 0x36
#define VersionReg 0x37
#define AnalogTestReg 0x38
#define TestDAC1Reg 0x39
#define TestDAC2Reg 0x3A
#define TestADCReg 0x3B

#define Idle	0x00
#define Mem		0x01
#define RandomID	0x02
#define CalcCRC	0x03
#define Transmit	0x04
#define NoCmdChange	0x07
#define Receive	0x08
#define Transceive	0x0C
#define MFAuthent	0x0E
#define SoftReset	0x0F

//FM17522芯片IIC地址1
#define FM17522_I2C_ADDR  0x28
//FM17522芯片IIC地址2
#define FM17522_I2C_ADDR1 0x29

typedef struct _IICReg
{
	uint8 dev_addr;
	int latestErr;
}IICReg;

Bool IICReg_readByte(IICReg* reg, uint8 reg_addr, uint8* value);
Bool IICReg_readBytes(IICReg* reg, uint8 reg_addr, uint8* buf, int bufSize);
Bool IICReg_writeByte(IICReg* reg, uint8 reg_addr, uint8 value);
Bool IICReg_writeBytes(IICReg* reg, uint8 reg_addr, const uint8* buf, int bufSize);
Bool IICReg_readFifo(IICReg* reg, uint8* buf, int bufSize);
Bool IICReg_writeFifo(IICReg* reg, const uint8* buf, int bufSize);

/*************************************************************/
/*函数名：	    Set_BitMask									 */
/*功能：	    置位寄存器操作								 */
/*输入参数：	reg_add，寄存器地址；mask，寄存器置位		 */
/*返回值：	    OK											 */
/*				ERROR										 */
/*************************************************************/
Bool IICReg_SetBitMask(IICReg* reg, unsigned char reg_addr, unsigned char mask);

/*********************************************/
/*函数名：	    Clear_BitMask   */
/*功能：	    清除位寄存器操作    */

/*输入参数：	reg_add，寄存器地址；mask，寄存器清除位 */
/*返回值：	    OK
				ERROR   */
/*********************************************/
unsigned char IICReg_clearBitMask(IICReg* reg, unsigned char reg_addr, unsigned char mask);
Bool IICReg_clearFIFO(IICReg* reg);

#define IIC_REG_ERR_RETURN_FALSE(x) do{if(!(x)) return False;}while(0)
#define IIC_REG_ERR_RETURN(x)       do{if(!(x)) return ;}while(0)

#endif

#endif



#if 1

#ifndef FM175XX_H
#define FM175XX_H

#include "drv_i2c.h"
#include "typedef.h"

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

#ifdef GLOBAL_FM17522
#undef GLOBAL_FM17522
#endif

#ifndef FM17522_C
#define GLOBAL_FM17522 extern 
#else
#define GLOBAL_FM17522 
#endif

#define NFC_I2C		I2C1

//定义读卡器的数量
#define NFC_READER_COUNT_MAX 1//单仓
//FM17522芯片I2C地址
#define FM17522_I2C_ADDR  0x28
//NFC1
#define FM17522_I2C_ADDR1 0x29

unsigned char Fm17522_get_slave_addr(void);
void Pcd_Comm_timer_cb(void);

unsigned char Read_Reg(unsigned char reg_addr);
unsigned char Write_Reg(unsigned char reg_addr, unsigned char reg_value);
unsigned char Read_FIFO(unsigned char length, unsigned char *fifo_data);
unsigned char Write_FIFO(unsigned char length, unsigned char *fifo_data);
unsigned char Clear_FIFO(void);
unsigned char Set_BitMask(unsigned char reg_addr, unsigned char mask);
unsigned char Clear_BitMask(unsigned char reg_addr, unsigned char mask);
unsigned char Set_Rf(unsigned char mode);
//void nfc_intisr_cb(unsigned char *pInData, unsigned char InLenByte,
//					unsigned char *pOutData, unsigned int *pOutLenBit);
unsigned char nfc_frame_tx(unsigned char *pInData, unsigned char InLenByte);
unsigned char Pcd_Comm(unsigned char Command, unsigned char *pInData, unsigned char InLenByte, unsigned char *pOutData, unsigned int *pOutLenBit);
unsigned char Pcd_SetTimer(unsigned long delaytime);
unsigned char Pcd_ConfigISOType(unsigned char type);
unsigned char FM175XX_SoftReset(void);
unsigned char FM175XX_HardReset(void);
unsigned char FM175XX_SoftPowerdown(void);
unsigned char Read_Ext_Reg(unsigned char reg_add);
unsigned char Write_Ext_Reg(unsigned char reg_add, unsigned char reg_value);
void FM17522_Delayms(unsigned int delayms);
GLOBAL_FM17522 unsigned char i2c_fm17522_addr_init(void);
GLOBAL_FM17522 unsigned char FM175XX_HardPowerdown(void);

unsigned char FM175XX_switchPort(unsigned char port);
void FM17522_Init(void);

#endif

#endif



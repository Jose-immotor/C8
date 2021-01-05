/*
 * File      : fm175xx.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 *  
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-15     kim 		   the first version
 * ...
 */
/*************************************************************/
#include "Common.h"
#include "fm175Drv.h"
#include "SwTimer.h"
#include "_Macro.h"
#include "Fm175Drv.h"
#include "drv_gpio.h"
#include "drv_i2c.h"


//static Fm175Drv g_fmDrv;

static FmFsmFn fm175Drv_findFsm(uint8 state);
static void fm175Drv_switchState(Fm175Drv* pDrv, uint8 state);
void fm175Drv_irq_timeOut(Fm175Drv* pDrv);
void fm175Drv_irq_tx(Fm175Drv* pDrv);
Bool fm175Drv_transStart(Fm175Drv* pDrv, FM17522_CMD cmd, uint32 timeOutMs);

#if 1

static DrvIo* g_pNfcNpdAIO = Null;
static DrvIo* g_pNfcNpdBIO = Null;
static DrvIo* g_pNfcPwrOffIO = Null;

#define FM17522_NPD_HIGHT 	PortPin_Set(g_pNfcNpdAIO->periph, g_pNfcNpdAIO->pin, 1)
#define FM17522_NPD_LOW		PortPin_Set(g_pNfcNpdAIO->periph, g_pNfcNpdAIO->pin, 0)

#define FM17522B_NPD_HIGHT 	PortPin_Set(g_pNfcNpdBIO->periph, g_pNfcNpdBIO->pin, 1)
#define FM17522B_NPD_LOW	PortPin_Set(g_pNfcNpdBIO->periph, g_pNfcNpdBIO->pin, 0)

/*********************************************/
/*函数名：	    Pcd_ConfigISOType    */
/*功能：	    设置操作协议    */
/*输入参数：	type 0，ISO14443A协议；    */
/*					 1，ISO14443B协议；    */
/*返回值：	    OK    */
/*********************************************/
Bool Fm175Drv_ConfigISOType(Fm175Drv* pDrv, NFC_ISO_TYPE type)
{
	if (type == NFC_ISO_TYPE_A)                     //ISO14443_A
	{
		IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, ControlReg, 0x10)); //ControlReg 0x0C 设置reader模式// chenke, FM17522 的bit4是RFT预留       
		IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TxAutoReg, 0x40)); //TxASKReg 0x15 设置100%ASK有效        
		IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, TxModeReg, 0x00));  //TxModeReg 0x12 设置TX CRC无效，TX FRAMING =TYPE A// chenke, TX CRC无效, TXSPEED 106Kbit        
		IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, RxModeReg, 0x00)); //RxModeReg 0x13 设置RX CRC无效，RX FRAMING =TYPE A// chenke, RX CRC无效, RXSPEED 106Kbit, RxNoErr = 0, RxMultipl = 0, 
	}
	else if (type == NFC_ISO_TYPE_B)                     //ISO14443_B
	{
		IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, ControlReg, 0x10)); //ControlReg 0x0C 设置reader模式        
		IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, TxModeReg, 0x83)); //TxModeReg 0x12 设置TX CRC有效，TX FRAMING =TYPE B        
		IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, RxModeReg, 0x83)); //RxModeReg 0x13 设置RX CRC有效，RX FRAMING =TYPE B        
		IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, GsNReg, 0xF4)); //GsNReg 0x27 设置ON电导        
		IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, GsNOffReg, 0xF4)); //GsNOffReg 0x23 设置OFF电导        
		IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, TxAutoReg, 0x00));// TxASKReg 0x15 设置100%ASK无效        
	}
	return True;
}

/****************************************************************************************/
/*名称：Fm175Drv_Request																	*/
/*功能：Fm175Drv_Request卡片寻卡															*/
/*输入：																				*/
/*输出：																			 	*/
/*	       	pTagType[0],pTagType[1] =ATQA                                         		*/
/*       	OK: 应答正确                                                              	*/
/*	 		ERROR: 应答错误																*/
/****************************************************************************************/

Bool Fm175Drv_Request(Fm175Drv* pDrv, unsigned char* pTagType)
{
	unsigned char result;
	unsigned char send_buff[1];
	unsigned char rece_buff[2];
	int rece_bitlen = sizeof(rece_buff);

	IIC_REG_ERR_RETURN_FALSE(IICReg_clearBitMask(&pDrv->iicReg, TxModeReg, 0x80));//关闭TX CRC

	IIC_REG_ERR_RETURN_FALSE(IICReg_clearBitMask(&pDrv->iicReg, RxModeReg, 0x80));//关闭RX CRC

	IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, RxModeReg, 0x08));//关闭位接收//chenke, RxNoErr = 1, 接收到无效数据流会被忽略，同时接收器保持激活状态

	IIC_REG_ERR_RETURN_FALSE(IICReg_clearBitMask(&pDrv->iicReg, Status2Reg, 0x08));//chenke, Crypto1On = 0, 该位只在M1卡的读写模式有效。

	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, BitFramingReg, 0x07));//chenke, TxLastBit = 7, 最后一个字节需要被发送的位数。

	send_buff[0] = 0x26;// chenke, 0x26是REQA命令，由PCD发出，以探测用于类型A PICC的工作场

	result = fm175Drv_SyncTransfer(pDrv, Transceive, send_buff, 1, rece_buff, &rece_bitlen, 1);
	if (result && rece_bitlen == 2)
	{
		*pTagType = rece_buff[0];
		*(pTagType + 1) = rece_buff[1];
	}
	return result;
}

/****************************************************************************************/
/*名称：Fm175Drv_Anticollision														*/
/*功能：Fm175Drv_Anticollision卡片防冲突												*/
/*输入：selcode =0x93，0x95，0x97														*/
/*输出：																			 	*/
/*	       	pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID                            		*/
/****************************************************************************************/
Bool Fm175Drv_Anticollision(Fm175Drv* pDrv, unsigned char selcode,unsigned char* pSnr)
{
	unsigned char result;
	unsigned char i;
	unsigned char send_buff[2];
	unsigned char rece_buff[5];
	int rece_bitlen = sizeof(rece_buff);

	IIC_REG_ERR_RETURN_FALSE(IICReg_clearBitMask(&pDrv->iicReg, TxModeReg, 0x80));

	IIC_REG_ERR_RETURN_FALSE(IICReg_clearBitMask(&pDrv->iicReg, RxModeReg, 0x80));

	IIC_REG_ERR_RETURN_FALSE(IICReg_clearBitMask(&pDrv->iicReg, Status2Reg, 0x08));

	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, BitFramingReg, 0x00));

	IIC_REG_ERR_RETURN_FALSE(IICReg_clearBitMask(&pDrv->iicReg, CollReg, 0x80));

	send_buff[0] = selcode;
	send_buff[1] = 0x20;

	result = fm175Drv_SyncTransfer(pDrv, Transceive, send_buff, 2, rece_buff, &rece_bitlen, 1);
	
	PFL(DL_NFC,"NFC[%02X] Antico:%02X-%02X:%02X%02X%02X%02X%02X\n",
		pDrv->iicReg.dev_addr,result,rece_bitlen,
		rece_buff[0],rece_buff[1],rece_buff[2],rece_buff[3],rece_buff[4]);
		
	if (result && rece_bitlen == sizeof(rece_buff))
	{
		for (i = 0; i < 5; i++)
			* (pSnr + i) = rece_buff[i];
		if (pSnr[4] != (pSnr[0] ^ pSnr[1] ^ pSnr[2] ^ pSnr[3]))
		{
			result = False;
		}
	}
	else
	{
		result = False;
	}
	return result;
}

/****************************************************************************************/
/*名称：Fm175Drv_Select																	*/
/*功能：Fm175Drv_Select卡片选卡															*/
/*输入：selcode =0x93，0x95，0x97														*/
/*      pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID 			    			     	*/
/*输出：																			 	*/
/*	       	pSak[0],pSak[1],pSak[2] =SAK			                            		*/
/*       	OK: 应答正确                                                              	*/
/*	 		ERROR: 应答错误																*/
/****************************************************************************************/
Bool Fm175Drv_Select(Fm175Drv* pDrv, unsigned char selcode, unsigned char* pSnr, unsigned char* pSak)
{
	unsigned char result;
	unsigned char i;
	unsigned char send_buff[7];
	unsigned char rece_buff[5];
	int rece_bitlen = sizeof(rece_buff);

	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, BitFramingReg, 0x00));
	IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TxModeReg, 0x80));
	IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, RxModeReg, 0x80));
	IIC_REG_ERR_RETURN_FALSE(IICReg_clearBitMask(&pDrv->iicReg, Status2Reg, 0x08));

	send_buff[0] = selcode;
	send_buff[1] = 0x70;

	for (i = 0; i < 5; i++)
		send_buff[i + 2] = *(pSnr + i);

	result = fm175Drv_SyncTransfer(pDrv, Transceive, send_buff, sizeof(send_buff), rece_buff, &rece_bitlen, 1);
	if (result)
	{
		*pSak = rece_buff[0];
	}
	return result;
}

/****************************************************************/
/*名称: TypeA_RATS 											 */
/*功能: 该函数实现ISO14443A-4协议中的RATS处理			     */
/*输入: param byte bit8-bit5：FSDI,bit4-bit0 ：CID			 */
/*       ats 用于保存RATS命令返回的数据						 */
/*输出:														 */
/* OK: 应答正确												 */
/* ERROR: 应答错误											 */
/****************************************************************/
Bool Fm175Drv_RATS(Fm175Drv* pDrv, unsigned char param)
{
	unsigned char send_buff[2];
	unsigned char rece_buff[14];
	int rece_bitlen = sizeof(rece_buff);

	send_buff[0] = 0xE0;
	send_buff[1] = param;                                //
	//CID = param & 0x0f;
	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, BitFramingReg, 0x00));
	IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TxModeReg, 0x80));                         //设置发送CRC
	IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, RxModeReg, 0x80));                         //设置接收CRC
	IIC_REG_ERR_RETURN_FALSE(IICReg_clearBitMask(&pDrv->iicReg, Status2Reg, 0x08));

	return fm175Drv_SyncTransfer(pDrv, Transceive, send_buff, sizeof(send_buff), rece_buff, &rece_bitlen, 5);
}

/****************************************************************************************/
/*名称：Fm175Drv_CardActivate														*/
/*功能：Fm175Drv_CardActivate卡片激活												*/
/*输入：																				*/
/*       			    			     												*/
/*	       								 												*/
/*输出：	pTagType[0],pTagType[1] =ATQA 											 	*/
/*	       	pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID 		                   		*/
/*	       	pSak[0],pSak[1],pSak[2] =SAK			                            		*/
/*       	OK: 应答正确                                                              	*/
/*	 		ERROR: 应答错误																*/
/****************************************************************************************/
Bool Fm175Drv_CardActivate(Fm175Drv* pDrv)
{
	unsigned char pTagType[2];
	unsigned char pSnr[15];
	unsigned char pSak[3];

	if (!Fm175Drv_Request(pDrv, pTagType)) return False; //寻卡 Standard	 send request command Standard mode
	
	PFL(DL_NFC,"NFC[%02X] Request:%02X%02X\n",
		pDrv->iicReg.dev_addr,pTagType[0],pTagType[1]);
	
	if ((pTagType[0] & 0xC0) == 0x00)	//一重UID
	{
		if (!Fm175Drv_Anticollision(pDrv, 0x93, pSnr))				return False; //1级防冲突
		if (!Fm175Drv_Select       (pDrv, 0x93, pSnr, pSak))			return False; //1级选卡
	}

	if ((pTagType[0] & 0xC0) == 0x40)	//二重UID
	{
		if (!Fm175Drv_Anticollision(pDrv, 0x93, pSnr))				return False; //1级防冲突
		if (!Fm175Drv_Select       (pDrv, 0x93, pSnr, pSak))			return False; //1级选卡
		if (!Fm175Drv_Anticollision(pDrv, 0x95, pSnr + 5 ))			return False; //2级防冲突
		if (!Fm175Drv_Select       (pDrv, 0x95, pSnr + 5, pSak+1))	return False; //2级选卡
	}

	if ((pTagType[0] & 0xC0) == 0x80)	//三重UID
	{
		if (!Fm175Drv_Anticollision(pDrv, 0x93, pSnr))				return False; //1级防冲突
		if (!Fm175Drv_Select       (pDrv, 0x93, pSnr, pSak))			return False; //1级选卡
		if (!Fm175Drv_Anticollision(pDrv, 0x95, pSnr + 5 ))			return False; //2级防冲突
		if (!Fm175Drv_Select       (pDrv, 0x95, pSnr + 5, pSak+1))	return False; //2级选卡
		if (!Fm175Drv_Anticollision(pDrv, 0x97, pSnr + 10 ))			return False; //3级防冲突
		if (!Fm175Drv_Select       (pDrv, 0x97, pSnr + 10, pSak+2))	return False; //3级选卡
	}

	return True;
}

/*********************************************/
/*函数名：	    FM175XX_SoftPowerdown    */
/*功能：	    软件低功耗操作    */
/*输入参数：	    */
/*返回值：	    OK，进入低功耗模式；    */
/*				ERROR，退出低功耗模式；    */
/*********************************************/
Bool Fm175Drv_SoftPowerdown(Fm175Drv* pDrv)
{
	//unsigned char result;
	////    result = 
	//IICReg_readByte(pDrv, CommandReg, &result);

	return IICReg_SetBitMask(&pDrv->iicReg, CommandReg, 0x10);//进入低功耗模式
}

/*********************************************/
/*函数名：	    Set_RF  */
/*功能：	    设置射频输出    */

/*输入参数：	mode，射频输出模式
				0，关闭输出
				1,仅打开TX1输出
				2,仅打开TX2输出
				3，TX1，TX2打开输出，TX2为反向输出  */
				/*返回值：	    OK
								ERROR   */
/*********************************************/
Bool Fm175Drv_setRf(Fm175Drv* pDrv, unsigned char mode)
{
	unsigned char result;

	IIC_REG_ERR_RETURN_FALSE(IICReg_readByte(&pDrv->iicReg, TxControlReg, &result));
	if ((result & 0x03) == mode) return True;

	if (mode == 0) IIC_REG_ERR_RETURN_FALSE(IICReg_clearBitMask(&pDrv->iicReg, TxControlReg, 0x03));	 //关闭TX1，TX2输出
	if (mode == 1) IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TxControlReg, mode));	 //仅打开TX1输出
	if (mode == 2) IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TxControlReg, mode));	 //仅打开TX2输出
	if (mode == 3) IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TxControlReg, mode));	 //打开TX1，TX2输出

	return True;
}

void fm175Drv_event(Fm175Drv* pDrv, TRANS_EVENT evt, TRANSFER_RESULT res)
{
	pDrv->latestErr = res;
	if (pDrv->Event) pDrv->Event(pDrv->obj, evt);

	if (evt == TRANS_FAILED || evt == TRANS_SUCCESS)
	{
		pDrv->transParam.txBufSize = 0;	//取消传输

		IIC_REG_ERR_RETURN(IICReg_SetBitMask(&pDrv->iicReg, ControlReg, 0x80));           // stop timer now
		IIC_REG_ERR_RETURN(IICReg_writeByte(&pDrv->iicReg, CommandReg, Idle));
		IIC_REG_ERR_RETURN(IICReg_clearBitMask(&pDrv->iicReg, BitFramingReg, 0x80));//关闭发送
		IIC_REG_ERR_RETURN(IICReg_writeByte(&pDrv->iicReg, WaterLevelReg, 0));//设置WaterLeve

		SwTimer_Stop(&pDrv->timer);
		pDrv->transStatus = TRANSFER_STATUS_IDLE;
	}

	if (evt == TRANS_FAILED && pDrv->state == FM_STATE_TRANSFER)
	{
		//重新搜卡
		fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
	}
}	

void fm175Drv_irq_rx(Fm175Drv* pDrv)
{
	TransParam* param = &pDrv->transParam;
	uint8 bytesInFifo = 0;

	if (pDrv->transStatus != TRANSFER_STATUS_RX) return;
	IIC_REG_ERR_RETURN(IICReg_readByte(&pDrv->iicReg, FIFOLevelReg, &bytesInFifo));

	//循环读取所有FIFO的内容
	while (bytesInFifo)
	{
		int readLen = MIN(param->rxBufSize - param->transBufOffset, bytesInFifo);
		if (readLen == 0)
		{
			//用户应该在此事件中，处理已经接收到的数据param->rxBuf中，并且设置param->rxBufSize的值
			fm175Drv_event(pDrv, TRANS_RX_BUF_FULL, TRANS_RESULT_SUCCESS);
			param->transBufOffset = 0;
		}
		else
		{
			IIC_REG_ERR_RETURN(IICReg_readFifo(&pDrv->iicReg, &param->rxBuf[param->transBufOffset], readLen)); //读出FIFO内容
			param->transBufOffset += readLen;
			param->offset += readLen;
			IIC_REG_ERR_RETURN(IICReg_readByte(&pDrv->iicReg, FIFOLevelReg, & bytesInFifo));
		}
	}
	param->totalLen = param->offset;
}

void fm175Drv_irq_tx(Fm175Drv* pDrv)
{
	TransParam* param = &pDrv->transParam;
	const TransProtocolCfg* cfg = pDrv->cfg;
	int remainLen = 0;

	if (pDrv->transStatus != TRANSFER_STATUS_TX) return;

	//当前还有多少未传输的数据在FIFO
	uint8 bytesInFifo = 0;

	//如果水位为0，说明传输已经完成，FIFO中可能有接收到的数据，不能够读取bytesInFifo查看FIFO中还有多少个数据没被发送走。
	//如果水位不为0，说明传输未完成，通过读取bytesInFifo查看FIFO中还有多少个数据没被发送走。
	if (pDrv->waterLevel != 0)
	{
		IIC_REG_ERR_RETURN(IICReg_readByte(&pDrv->iicReg, FIFOLevelReg, &bytesInFifo));
	}

	//计算实际已经传输的数据长度
	int sentLen = param->putBytesInTxFifo - bytesInFifo;

	param->offset += sentLen;
	param->transBufOffset += sentLen;

	remainLen = param->totalLen - param->offset;
	if (remainLen == 0)
	{
		PFL(DL_NFC, "NFC[%X] TX(%d) Done.\n",pDrv->iicReg.dev_addr, param->transBufOffset);
		param->putBytesInTxFifo = 0;
		goto TxDone;
	}
	else
	{
		//如果剩余的FIFO空间可以存放全部数据，则设水位为0，否则设置FIFO深度的一半。
		uint8 waterLevel = (remainLen <= pDrv->cfg->fifoDeepth - bytesInFifo) ? 0 : (pDrv->cfg->fifoDeepth >> 1);
		if (pDrv->waterLevel != waterLevel)
		{
			pDrv->waterLevel = waterLevel;
			IIC_REG_ERR_RETURN(IICReg_writeByte(&pDrv->iicReg, WaterLevelReg, pDrv->waterLevel));//设置WaterLeve
		}
	}

	if (param->transBufOffset >= param->txBufSize)
	{
		//用户应该在此事件中，复制要传输的数据到pDrv->txBuf中，并且设置pDrv->transMgr.txBufSize的值
		fm175Drv_event(pDrv, TRANS_TX_BUF_EMPTY, TRANS_RESULT_SUCCESS);
		param->transBufOffset = 0;
	}

	//计算需要放到TX FIFO中的字节数
	param->putBytesInTxFifo = MIN(param->txBufSize - param->transBufOffset, cfg->fifoDeepth - bytesInFifo);

	if (param->putBytesInTxFifo)
	{
		PFL(DL_NFC, "NFC[%X] TX(%d/%d,%d):",pDrv->iicReg.dev_addr, param->transBufOffset, param->totalLen, param->putBytesInTxFifo);
		DUMP_BYTE_LEVEL(DL_NFC, & param->txBuf[param->transBufOffset], param->putBytesInTxFifo);
		PFL(DL_NFC, "\n");
		
		IIC_REG_ERR_RETURN(IICReg_writeFifo(&pDrv->iicReg, &param->txBuf[param->transBufOffset], param->putBytesInTxFifo));
		IIC_REG_ERR_RETURN(IICReg_SetBitMask(&pDrv->iicReg, BitFramingReg, 0x80));	//启动发送
		return;
	}
	
TxDone:
	//是否需要接收数据
	if (param->rxBufSize == 0)
	{
		fm175Drv_event(pDrv, TRANS_SUCCESS, TRANS_RESULT_SUCCESS);
	}
	else
	{
		param->offset = 0;
		param->transBufOffset = 0;
		pDrv->transStatus = TRANSFER_STATUS_RX;
		param->totalLen = 0;

		pDrv->waterLevel = (param->rxBufSize <= pDrv->cfg->fifoDeepth) ? pDrv->cfg->fifoDeepth : (pDrv->cfg->fifoDeepth >> 1);
		IIC_REG_ERR_RETURN(IICReg_writeByte(&pDrv->iicReg, WaterLevelReg, pDrv->waterLevel));//设置接收FIFO的WaterLeve
	}
}

void fm175Drv_irq_idle(Fm175Drv* pDrv)
{
}
void fm175Drv_irq_hiAlert(Fm175Drv* pDrv)
{
	fm175Drv_irq_rx(pDrv);
}

void fm175Drv_irq_rxDone(Fm175Drv* pDrv)
{
	fm175Drv_irq_rx(pDrv);
	fm175Drv_event(pDrv, TRANS_SUCCESS, TRANS_RESULT_SUCCESS);
}

void fm175Drv_irq_loAlert(Fm175Drv* pDrv)
{
	fm175Drv_irq_tx(pDrv);
}
void fm175Drv_irq_err(Fm175Drv* pDrv)
{
	//不处理，Mifare卡片的写操作时会返回4个BIT,需要屏蔽错误检测
}
void fm175Drv_irq_timeOut(Fm175Drv* pDrv)
{
	if (pDrv->transStatus == TRANSFER_STATUS_IDLE) return;

	fm175Drv_event(pDrv, TRANS_FAILED, TRANS_RESULT_TIMEOUT);
}

//获取中断处理函数
FmIrqFn fm175Drv_getIrqHandler(Fm175Drv * pDrv, uint8 irq)
{
	struct
	{
		uint8 irq;
		FmIrqFn handler;
	}
	const static g_irqHandler[] =
	{
		{REG_IRQ_MASK_TX		, (FmIrqFn)fm175Drv_irq_tx},
		{REG_IRQ_MASK_RX		, (FmIrqFn)fm175Drv_irq_rxDone},
		{REG_IRQ_MASK_IDLE		, (FmIrqFn)fm175Drv_irq_idle},
		{REG_IRQ_MASK_HI_ALERT	, (FmIrqFn)fm175Drv_irq_hiAlert},
		{REG_IRQ_MASK_LO_ALERT	, (FmIrqFn)fm175Drv_irq_loAlert},
		{REG_IRQ_MASK_ERR		, (FmIrqFn)fm175Drv_irq_err},
		{REG_IRQ_MASK_TIMEOUT	, (FmIrqFn)fm175Drv_irq_timeOut},
	};

	for (int i = 0; i < GET_ELEMENT_COUNT(g_irqHandler); i++)
	{
		if (irq & g_irqHandler[i].irq) return g_irqHandler[i].handler;
	}

	return Null;
}

void fm175Drv_irq(Fm175Drv * pDrv, uint8 irq)
{
#if 0
	//查看中断位
	char buf[128]={0};
	SprintfBit(buf, "TxDone[%6B],RxDone[%5B],Idle[%4B],RxFifoHi[%3B],TxFifoLo[%2B],Err[%1B],Timeout[%0B]", irq, Null);
	PFL(DL_NFC, "irq=0x%02X, [%s]\n", irq, buf);
#endif
	for (int i = 0; i < 8; i++)
	{
		if (irq & BIT(i))
		{
			FmIrqFn handler = fm175Drv_getIrqHandler(pDrv, BIT(i));
			if (handler)
			{
				handler(pDrv);
			}

			//清中断
			IIC_REG_ERR_RETURN(IICReg_writeByte(&pDrv->iicReg, ComIrqReg, BIT(i)));
		}
	}
}

static void fm175Drv_switchState(Fm175Drv* pDrv, uint8 state)
{
//	uint8 rc = 0;
	if (pDrv->state == state) return;

	PFL(DL_NFC, "NFC[%X] switch %d to %d\n",pDrv->iicReg.dev_addr,pDrv->state , state);

	if (state == FM_STATE_INIT)
	{
	}
	else if (state == FM_STATE_NPD_LOW)
	{
		
		if(pDrv->iicReg.dev_addr == FM17522_I2C_ADDR)
			FM17522_NPD_LOW;
		else if(pDrv->iicReg.dev_addr == FM17522_I2C_ADDR1)
			FM17522B_NPD_LOW;
		SwTimer_Start(&pDrv->timer, 10, 0);
	}
	else if (state == FM_STATE_NPD_HIGH)
	{
		if(pDrv->iicReg.dev_addr == FM17522_I2C_ADDR)
			FM17522_NPD_HIGHT;
		else if(pDrv->iicReg.dev_addr == FM17522_I2C_ADDR1)
			FM17522B_NPD_HIGHT;
		SwTimer_Start(&pDrv->timer, 10, 0);
	}
	else if (state == FM_STATE_SEARCH_CARD)
	{
		Fm175Drv_setRf(pDrv, ((Battery*)pDrv->obj)->cfg->antselct);
//		Fm175Drv_setRf(pDrv, pDrv->antPort);
		//打开TX输出后需要延时等待天线载波信号稳定
		SwTimer_Start(&pDrv->timer, 1, 0);
	}
	else if (state == FM_STATE_SLEEP)
	{
		SwTimer_Stop(&pDrv->timer);
		if(pDrv->iicReg.dev_addr == FM17522_I2C_ADDR)
			FM17522_NPD_LOW;
		else if(pDrv->iicReg.dev_addr == FM17522_I2C_ADDR1)
			FM17522B_NPD_LOW;
		//进入低功耗
		Fm175Drv_SoftPowerdown(pDrv);
		((Battery*)pDrv->obj)->cfg->antselct = 1;
//		pDrv->antPort = 1;
	}
	else if (state == FM_STATE_TRANSFER)
	{
		SwTimer_Start(&pDrv->sleepWdTimer, 60000, 0);
		fm175Drv_event(pDrv, SEARCH_CARD_DONE, TRANS_RESULT_SUCCESS);
	}

	pDrv->state = state;
	pDrv->fsm = fm175Drv_findFsm(state);
}

void fm175Drv_fsmInit(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param)
{
	if (msg == FM_MSG_RUN)
	{
		fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
	}
	else if(msg == FM_MSG_SWITCH_NFC)
	{
		pDrv->iicReg.dev_addr = (uint8)param;
	}
}

void fm175Drv_fsmNpdLow(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param)
{
	if (msg == FM_MSG_RUN)
	{
		if (SwTimer_isTimerOut(&pDrv->timer))
		{
			fm175Drv_switchState(pDrv, FM_STATE_NPD_HIGH);
		}
	}
	else if (msg == FM_MSG_SWITCH_NFC)
	{
		pDrv->iicReg.dev_addr = (uint8)param;
	}
}

void fm175Drv_fsmNpdLHigh(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param)
{
	if (msg == FM_MSG_RUN)
	{
		if (SwTimer_isTimerOut(&pDrv->timer))
		{
			fm175Drv_switchState(pDrv, FM_STATE_SEARCH_CARD);
		}
	}
	else if (msg == FM_MSG_SWITCH_NFC)
	{
		pDrv->iicReg.dev_addr = (uint8)param;
	}
}

void fm175Drv_fsmSearchCard(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param)
{
	if (msg == FM_MSG_RUN)
	{
		if (SwTimer_isTimerOut(&pDrv->timer))
		{
			Bool isOk = Fm175Drv_ConfigISOType(pDrv, NFC_ISO_TYPE_A);
			if (isOk) isOk = Fm175Drv_CardActivate(pDrv);
			if (isOk) isOk = Fm175Drv_RATS(pDrv, 0x20);

			pDrv->cardIsExist = isOk;

			if (pDrv->cardIsExist)
			{
				fm175Drv_switchState(pDrv, FM_STATE_TRANSFER);
			}
//			else if (pDrv->antPort == 1)
//			{
//				pDrv->antPort = 2;
////				fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
//			}
			else if (((Battery*)pDrv->obj)->cfg->antselct == 1)
			{
				((Battery*)pDrv->obj)->cfg->antselct = 2;
				fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
			}
//			else if (((Battery*)pDrv->obj)->cfg->antselct == 2)
//			{
//				((Battery*)pDrv->obj)->cfg->antselct = 1;
//				fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
//			}
			else
			{
				fm175Drv_event(pDrv, TRANS_FAILED, TRANS_RESULT_SEARCH_CARD_FAILED);
				fm175Drv_switchState(pDrv, FM_STATE_SLEEP);
			}
		}
	}
	if (msg == FM_MSG_SWITCH_NFC)
	{
		pDrv->iicReg.dev_addr = (uint8)param;
		fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
	}
}

void fm175Drv_fsmSleep(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param)
{
	if (msg == FM_MSG_RUN)
	{
		//PortPin_Set(g_pNfcPwrOffIO->periph, g_pNfcPwrOffIO->pin, False);
		if(Fsm_ReadActiveFlag() & AF_PMS)
		{
			if (pDrv->transParam.txBufSize && pDrv->transStatus == TRANSFER_STATUS_PENDING_TX)
			{
				fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
			}
		}
	}
	else if (msg == FM_MSG_SWITCH_NFC)
	{
		//PortPin_Set(g_pNfcPwrOffIO->periph, g_pNfcPwrOffIO->pin, False);
		pDrv->iicReg.dev_addr = (uint8)param;
		//if(Fsm_ReadActiveFlag() & AF_PMS)
		fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
	}
	else if (msg == FM_MSG_STOP)
	{
		//PortPin_Set(g_pNfcPwrOffIO->periph, g_pNfcPwrOffIO->pin, True);
		Fsm_SetActiveFlag(AF_NFC, False);
	}
}

void fm175Drv_waitTransDone(Fm175Drv* pDrv)
{
	//同步发送方式，发完数据等待接收
	uint8 irq = 0;
	while (pDrv->transStatus != TRANSFER_STATUS_IDLE && !SwTimer_isTimerOut(&pDrv->timer))
	{
		if (IICReg_readByte(&pDrv->iicReg, ComIrqReg, &irq))//查询中断标志	
		{
			fm175Drv_irq(pDrv, irq);
		}
	}
}

void fm175Drv_fsmTransfer(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param)
{
	//是否发送命令超时，没有接收到响应
	if (msg == FM_MSG_RUN)
	{
		if (SwTimer_isTimerOut(&pDrv->timer))
		{
			fm175Drv_irq_timeOut(pDrv);
		}
		else if (pDrv->transParam.txBufSize && pDrv->transStatus == TRANSFER_STATUS_PENDING_TX)
		{
			SwTimer_ReStart(&pDrv->sleepWdTimer);
			if (!fm175Drv_transStart(pDrv, Transceive, 200))
			{
				fm175Drv_event(pDrv, TRANS_FAILED, TRANS_RESULT_FAILED);
			}
		}
		else if (SwTimer_isTimerOut(&pDrv->sleepWdTimer))
		{
			fm175Drv_switchState(pDrv, FM_STATE_SLEEP);
		}
		else if (pDrv->transStatus == TRANSFER_STATUS_TX || pDrv->transStatus == TRANSFER_STATUS_RX)
		{
			uint8 irq = 0;
			if (IICReg_readByte(&pDrv->iicReg, ComIrqReg, &irq))//查询中断标志	
			{
				fm175Drv_irq(pDrv, irq);
			}
		}
		/*
		else if (pDrv->transParam.txBufSize == 0)//lane 20201117新加，不加的话读取电池2后回到读电池1，txBufSize会为0，直接就读不到电池了
		{
			fm175Drv_event(pDrv, TRANS_FAILED, TRANS_RESULT_FAILED);
		}
		*/
	}
	else if (msg == FM_MSG_SWITCH_NFC)
	{
		if (pDrv->transStatus == TRANSFER_STATUS_IDLE)
		{
			pDrv->iicReg.dev_addr = (uint8)param;
			fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
		}
	}
}

static FmFsmFn fm175Drv_findFsm(uint8 state)
{
	struct
	{
		uint8 state;
		FmFsmFn fsm;
	}
	static const fsmDispatch[] =
	{
		{FM_STATE_INIT			, fm175Drv_fsmInit},
		{FM_STATE_NPD_LOW		, fm175Drv_fsmNpdLow},
		{FM_STATE_NPD_HIGH		, fm175Drv_fsmNpdLHigh},
		{FM_STATE_SEARCH_CARD	, fm175Drv_fsmSearchCard},
		{FM_STATE_SLEEP			, fm175Drv_fsmSleep},
		{FM_STATE_TRANSFER		, fm175Drv_fsmTransfer},
	};
	for (int i = 0; i < GET_ELEMENT_COUNT(fsmDispatch); i++)
	{
		if (state == fsmDispatch[i].state)
		{
			return fsmDispatch[i].fsm;
		}
	}

	//程序不可能运行到这里
	Assert(False);
	return Null;
}

void fm175Drv_fsm(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param)
{
	pDrv->fsm(pDrv, msg, param);
}

#endif

/*********************************************/
/*函数名：	    fm175Drv_setTimer    */
/*功能：	    设置接收延时    */
/*输入参数：	delaytime，延时时间（单位为毫秒）    */
/*返回值：	    OK    */
/*********************************************/
Bool fm175Drv_setTimer(Fm175Drv* pDrv, unsigned long delaytime)//设定超时时间（ms）
{
	unsigned long TimeReload;
	unsigned int Prescaler;

	Prescaler = 0;
	TimeReload = 0;
	while (Prescaler < 0xfff)
	{
		TimeReload = ((delaytime * (long)13560) - 1) / (Prescaler * 2 + 1);
		if (TimeReload < 0xffff)
			break;
		Prescaler++;
	}
	TimeReload = TimeReload & 0xFFFF;
	IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TModeReg, Prescaler >> 8));
	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, TPrescalerReg, Prescaler & 0xFF));
	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, TReloadMSBReg, TimeReload >> 8));
	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, TReloadLSBReg, TimeReload & 0xFF));
	
	return True;
}

/***************************************************************
函数功能：传输初始化,如果是大块数据传输，需要分批填充txBuf,可在设置该函数中的参数txBuf=Null。
参数说明：
	pDrv：确定程序对象。
	txBuf： 发送数据指针,如果为NULL，表示在此不指定传输数据(传输大块数据)，需要再TRANS_TX_BUF_EMPTY中填充发送数据
	txBufSize： 发送数据长度
	rxBuf：		接收数据指针
	rxBufSize： 接收数据长度
返回值：TRUE-传输初始化成功，FALSE-传输初始化失败
***************************************************************/
Bool fm175Drv_transferInit(Fm175Drv* pDrv, const void* txBuf, int txBufSize, void* rxBuf, int rxBufSize, void* pObj)
{
	TransParam* param = &pDrv->transParam;
	//有数据正在传输
	if (pDrv->transStatus != TRANSFER_STATUS_IDLE) return False;
	pDrv->transStatus = TRANSFER_STATUS_PENDING_TX;
	
	param->txBuf		= txBuf;
	param->txBufSize = txBufSize;

	param->rxBuf		= rxBuf;
	param->rxBufSize = rxBufSize;
	pDrv->obj = pObj;

	return True;
}


/***************************************************************
函数功能：传输启动，启动后会触发传输事件，通过调用事件回调函数通知传输结果，成功或者失败，参考事件定义FM_EVENT。
参数说明：
	pDrv：确定程序对象。
	cmd：传输命令
	timeOutMs： 超时时间
返回值：TRUE-传输启动成功，FALSE-传输启动失败
***************************************************************/
Bool fm175Drv_transStart(Fm175Drv* pDrv, FM17522_CMD cmd, uint32 timeOutMs)
{
	//timeOutMs = 200;
	TransParam* param = &pDrv->transParam;

	if (pDrv->transStatus != TRANSFER_STATUS_PENDING_TX) return False;

	pDrv->cmd = cmd;
	//pDrv->obj = cbObj;

	param->totalLen = pDrv->transParam.txBufSize;
	param->offset = 0;

	//如果txBuf为NULL，表示不指定传输数据，需要在TRANS_TX_BUF_EMPTY中填充发送数据
	param->txBufSize = (param->txBuf) ? pDrv->transParam.txBufSize : 0;

	param->transBufOffset = 0;
	param->putBytesInTxFifo = 0;

	//pDrv->txBuf = txBuf;
	//pDrv->transMgr.txBufSize = txBufSize;
	//pDrv->rxBuf = rxBuf;
	//pDrv->rxBufSize = rxBufSize;

	IIC_REG_ERR_RETURN_FALSE(IICReg_clearFIFO(&pDrv->iicReg));

	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, CommandReg, Idle));
	//    rx_temp = IICReg_readByte(&pDrv->iicReg, CommandReg);

	pDrv->waterLevel = 0;
	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, WaterLevelReg, pDrv->waterLevel));//设置WaterLeve
//   	rx_temp = IICReg_readByte(&pDrv->iicReg, WaterLevelReg);

	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, ComIrqReg, 0x7F));//清除IRQ标志
//	rx_temp = IICReg_readByte(&pDrv->iicReg, ComIrqReg);

	IIC_REG_ERR_RETURN_FALSE(fm175Drv_setTimer(pDrv, timeOutMs));
	IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TModeReg, 0x80));//自动启动定时器
	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, CommandReg, cmd));

	pDrv->transStatus = TRANSFER_STATUS_TX;
	SwTimer_Start(&pDrv->timer, timeOutMs + 10, 0);

	fm175Drv_event(pDrv, TRANS_TX_START, TRANS_RESULT_SUCCESS);

	fm175Drv_irq_tx(pDrv);
	return True;
}

//同步发送数据
Bool fm175Drv_SyncTransfer(Fm175Drv* pDrv
	, FM17522_CMD cmd
	, const void* txBuf, int txBufSize
	, void* rxBuf, int* rxBufSize
	, uint32 timeOutMs
)
{
	TransParam* param = &pDrv->transParam;
	//如果设备状态state是激活，
	if (pDrv->state == FM_STATE_TRANSFER)
	{
		//传输状态transStatus为非TRANSFER_STATUS_IDLE，返回失败
		if (pDrv->transStatus != TRANSFER_STATUS_IDLE) return False;
	}
	else if (pDrv->state == FM_STATE_SEARCH_CARD)
	{
		//如果设备状态state是搜卡，优先级最高，传输状态transStatus为任何状态都可以发送
	}
	else //其他状态，返回失败
	{
		return False;
	}

	int rc = False;

	//临时保存传输参数，同步传输结束后恢复**************************
	TransEventFn	_evtFn		= pDrv->Event	;
	FM17522_CMD		_cmd		= pDrv->cmd		;
	const void*		_txBuf		= param->txBuf	;
	int				_txBufSize  = pDrv->transParam.txBufSize;
	void*			_rxBuf      = param->rxBuf	;
	int				_rxBufSize  = param->rxBufSize;
	TRANSFER_STATUS _transStatus = pDrv->transStatus;
	/**************************************************************/

	pDrv->Event = Null;	//事件函数指针置空,同步传输不需要Event调用
	pDrv->transStatus = TRANSFER_STATUS_IDLE;

	//启动传输并等待结果
	fm175Drv_transferInit(pDrv, txBuf, txBufSize, rxBuf, *rxBufSize, pDrv->obj);
	if (fm175Drv_transStart(pDrv, cmd, timeOutMs))
	{
		fm175Drv_waitTransDone(pDrv);
	}
	else
	{
		pDrv->latestErr = TRANS_RESULT_FAILED;
		rc = False;
		goto End; 
	}


	if (pDrv->latestErr == TRANS_RESULT_SUCCESS)
	{
		*rxBufSize = pDrv->transParam.totalLen;
		rc = True;
		goto End;
	}
End:
	//恢复传输参数
	pDrv->Event		= _evtFn		;
	pDrv->cmd		= _cmd		;
	param->txBuf		= _txBuf		;
	pDrv->transParam.txBufSize	= _txBufSize	;
	param->rxBuf		= _rxBuf		;
	param->rxBufSize = _rxBufSize;
	pDrv->transStatus = _transStatus;
	return rc;
}

Bool fm175Drv_switchNfc(Fm175Drv* pDrv, uint8 iicAddr)
{
	if (pDrv->iicReg.dev_addr == iicAddr) return True;

	fm175Drv_fsm(pDrv, FM_MSG_SWITCH_NFC, iicAddr);

	return (pDrv->iicReg.dev_addr == iicAddr);
}

void fm175Drv_stop(Fm175Drv* pDrv)
{
	fm175Drv_switchState(pDrv, FM_STATE_SLEEP);
	fm175Drv_fsm(pDrv, FM_MSG_STOP, 0);
}

void fm175Drv_run(Fm175Drv* pDrv)
{
	fm175Drv_fsm(pDrv, FM_MSG_RUN, 0);
}

void fm175Drv_start(Fm175Drv* pDrv)
{
	fm175Drv_switchState(pDrv, FM_STATE_SLEEP);
	Fsm_SetActiveFlag(AF_NFC, True);
}

void fm175Drv_init(Fm175Drv* pDrv, uint8 iicAddr, const TransProtocolCfg* cfg, TransEventFn	Event)
{
	memset(pDrv, 0, sizeof(Fm175Drv));

	pDrv->cfg = cfg;
	pDrv->transStatus = TRANSFER_STATUS_IDLE;
	pDrv->antPort = 1;
	pDrv->Event = Event;

	pDrv->iicReg.dev_addr = iicAddr;
	
	rt_hw_i2c_init(NFC_I2C);
	g_pNfcNpdAIO = IO_Get(IO_NFC_NPD_A);
	g_pNfcNpdBIO = IO_Get(IO_NFC_NPD_B);
	g_pNfcPwrOffIO=IO_Get(IO_NFC_PWR_OFF);
	PortPin_Set(g_pNfcNpdAIO->periph, g_pNfcNpdAIO->pin, False);
	PortPin_Set(g_pNfcPwrOffIO->periph, g_pNfcPwrOffIO->pin, False);
}

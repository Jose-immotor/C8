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
static DrvIo* g_pNfcPwrOffIO = Null;

#define FM17522_NPD_HIGHT 	PortPin_Set(g_pNfcNpdAIO->periph, g_pNfcNpdAIO->pin, 1)
#define FM17522_NPD_LOW		PortPin_Set(g_pNfcNpdAIO->periph, g_pNfcNpdAIO->pin, 0)

/*********************************************/
/*��������	    Pcd_ConfigISOType    */
/*���ܣ�	    ���ò���Э��    */
/*���������	type 0��ISO14443AЭ�飻    */
/*					 1��ISO14443BЭ�飻    */
/*����ֵ��	    OK    */
/*********************************************/
Bool Fm175Drv_ConfigISOType(Fm175Drv* pDrv, NFC_ISO_TYPE type)
{
	if (type == NFC_ISO_TYPE_A)                     //ISO14443_A
	{
		IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, ControlReg, 0x10)); //ControlReg 0x0C ����readerģʽ// chenke, FM17522 ��bit4��RFTԤ��       
		IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TxAutoReg, 0x40)); //TxASKReg 0x15 ����100%ASK��Ч        
		IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, TxModeReg, 0x00));  //TxModeReg 0x12 ����TX CRC��Ч��TX FRAMING =TYPE A// chenke, TX CRC��Ч, TXSPEED 106Kbit        
		IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, RxModeReg, 0x00)); //RxModeReg 0x13 ����RX CRC��Ч��RX FRAMING =TYPE A// chenke, RX CRC��Ч, RXSPEED 106Kbit, RxNoErr = 0, RxMultipl = 0, 
	}
	else if (type == NFC_ISO_TYPE_B)                     //ISO14443_B
	{
		IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, ControlReg, 0x10)); //ControlReg 0x0C ����readerģʽ        
		IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, TxModeReg, 0x83)); //TxModeReg 0x12 ����TX CRC��Ч��TX FRAMING =TYPE B        
		IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, RxModeReg, 0x83)); //RxModeReg 0x13 ����RX CRC��Ч��RX FRAMING =TYPE B        
		IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, GsNReg, 0xF4)); //GsNReg 0x27 ����ON�絼        
		IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, GsNOffReg, 0xF4)); //GsNOffReg 0x23 ����OFF�絼        
		IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, TxAutoReg, 0x00));// TxASKReg 0x15 ����100%ASK��Ч        
	}
	return True;
}

/****************************************************************************************/
/*���ƣ�Fm175Drv_Request																	*/
/*���ܣ�Fm175Drv_Request��ƬѰ��															*/
/*���룺																				*/
/*�����																			 	*/
/*	       	pTagType[0],pTagType[1] =ATQA                                         		*/
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/

Bool Fm175Drv_Request(Fm175Drv* pDrv, unsigned char* pTagType)
{
	unsigned char result;
	unsigned char send_buff[1];
	unsigned char rece_buff[2];
	int rece_bitlen = sizeof(rece_buff);

	IIC_REG_ERR_RETURN_FALSE(IICReg_clearBitMask(&pDrv->iicReg, TxModeReg, 0x80));//�ر�TX CRC

	IIC_REG_ERR_RETURN_FALSE(IICReg_clearBitMask(&pDrv->iicReg, RxModeReg, 0x80));//�ر�RX CRC

	IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, RxModeReg, 0x08));//�ر�λ����//chenke, RxNoErr = 1, ���յ���Ч�������ᱻ���ԣ�ͬʱ���������ּ���״̬

	IIC_REG_ERR_RETURN_FALSE(IICReg_clearBitMask(&pDrv->iicReg, Status2Reg, 0x08));//chenke, Crypto1On = 0, ��λֻ��M1���Ķ�дģʽ��Ч��

	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, BitFramingReg, 0x07));//chenke, TxLastBit = 7, ���һ���ֽ���Ҫ�����͵�λ����

	send_buff[0] = 0x26;// chenke, 0x26��REQA�����PCD��������̽����������A PICC�Ĺ�����

	result = fm175Drv_SyncTransfer(pDrv, Transceive, send_buff, 1, rece_buff, &rece_bitlen, 1);
	if (result && rece_bitlen == 2)
	{
		*pTagType = rece_buff[0];
		*(pTagType + 1) = rece_buff[1];
	}
	return result;
}

/****************************************************************************************/
/*���ƣ�Fm175Drv_Anticollision														*/
/*���ܣ�Fm175Drv_Anticollision��Ƭ����ͻ												*/
/*���룺selcode =0x93��0x95��0x97														*/
/*�����																			 	*/
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
/*���ƣ�Fm175Drv_Select																	*/
/*���ܣ�Fm175Drv_Select��Ƭѡ��															*/
/*���룺selcode =0x93��0x95��0x97														*/
/*      pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID 			    			     	*/
/*�����																			 	*/
/*	       	pSak[0],pSak[1],pSak[2] =SAK			                            		*/
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
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
/*����: TypeA_RATS 											 */
/*����: �ú���ʵ��ISO14443A-4Э���е�RATS����			     */
/*����: param byte bit8-bit5��FSDI,bit4-bit0 ��CID			 */
/*       ats ���ڱ���RATS����ص�����						 */
/*���:														 */
/* OK: Ӧ����ȷ												 */
/* ERROR: Ӧ�����											 */
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
	IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TxModeReg, 0x80));                         //���÷���CRC
	IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, RxModeReg, 0x80));                         //���ý���CRC
	IIC_REG_ERR_RETURN_FALSE(IICReg_clearBitMask(&pDrv->iicReg, Status2Reg, 0x08));

	return fm175Drv_SyncTransfer(pDrv, Transceive, send_buff, sizeof(send_buff), rece_buff, &rece_bitlen, 5);
}

/****************************************************************************************/
/*���ƣ�Fm175Drv_CardActivate														*/
/*���ܣ�Fm175Drv_CardActivate��Ƭ����												*/
/*���룺																				*/
/*       			    			     												*/
/*	       								 												*/
/*�����	pTagType[0],pTagType[1] =ATQA 											 	*/
/*	       	pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID 		                   		*/
/*	       	pSak[0],pSak[1],pSak[2] =SAK			                            		*/
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/
Bool Fm175Drv_CardActivate(Fm175Drv* pDrv)
{
	unsigned char pTagType[2];
	unsigned char pSnr[15];
	unsigned char pSak[3];

	if (!Fm175Drv_Request(pDrv, pTagType)) return False; //Ѱ�� Standard	 send request command Standard mode

	if ((pTagType[0] & 0xC0) == 0x00)	//һ��UID
	{
		if (!Fm175Drv_Anticollision(pDrv, 0x93, pSnr))				return False; //1������ͻ
		if (!Fm175Drv_Select       (pDrv, 0x93, pSnr, pSak))			return False; //1��ѡ��
	}

	if ((pTagType[0] & 0xC0) == 0x40)	//����UID
	{
		if (!Fm175Drv_Anticollision(pDrv, 0x93, pSnr))				return False; //1������ͻ
		if (!Fm175Drv_Select       (pDrv, 0x93, pSnr, pSak))			return False; //1��ѡ��
		if (!Fm175Drv_Anticollision(pDrv, 0x95, pSnr + 5 ))			return False; //2������ͻ
		if (!Fm175Drv_Select       (pDrv, 0x95, pSnr + 5, pSak+1))	return False; //2��ѡ��
	}

	if ((pTagType[0] & 0xC0) == 0x80)	//����UID
	{
		if (!Fm175Drv_Anticollision(pDrv, 0x93, pSnr))				return False; //1������ͻ
		if (!Fm175Drv_Select       (pDrv, 0x93, pSnr, pSak))			return False; //1��ѡ��
		if (!Fm175Drv_Anticollision(pDrv, 0x95, pSnr + 5 ))			return False; //2������ͻ
		if (!Fm175Drv_Select       (pDrv, 0x95, pSnr + 5, pSak+1))	return False; //2��ѡ��
		if (!Fm175Drv_Anticollision(pDrv, 0x97, pSnr + 10 ))			return False; //3������ͻ
		if (!Fm175Drv_Select       (pDrv, 0x97, pSnr + 10, pSak+2))	return False; //3��ѡ��
	}

	return True;
}

/*********************************************/
/*��������	    FM175XX_SoftPowerdown    */
/*���ܣ�	    ����͹��Ĳ���    */
/*���������	    */
/*����ֵ��	    OK������͹���ģʽ��    */
/*				ERROR���˳��͹���ģʽ��    */
/*********************************************/
Bool Fm175Drv_SoftPowerdown(Fm175Drv* pDrv)
{
	//unsigned char result;
	////    result = 
	//IICReg_readByte(pDrv, CommandReg, &result);

	return IICReg_SetBitMask(&pDrv->iicReg, CommandReg, 0x10);//����͹���ģʽ
}

/*********************************************/
/*��������	    Set_RF  */
/*���ܣ�	    ������Ƶ���    */

/*���������	mode����Ƶ���ģʽ
				0���ر����
				1,����TX1���
				2,����TX2���
				3��TX1��TX2�������TX2Ϊ�������  */
				/*����ֵ��	    OK
								ERROR   */
/*********************************************/
Bool Fm175Drv_setRf(Fm175Drv* pDrv, unsigned char mode)
{
	unsigned char result;

	IIC_REG_ERR_RETURN_FALSE(IICReg_readByte(&pDrv->iicReg, TxControlReg, &result));
	if ((result & 0x03) == mode) return True;

	if (mode == 0) IIC_REG_ERR_RETURN_FALSE(IICReg_clearBitMask(&pDrv->iicReg, TxControlReg, 0x03));	 //�ر�TX1��TX2���
	if (mode == 1) IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TxControlReg, mode));	 //����TX1���
	if (mode == 2) IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TxControlReg, mode));	 //����TX2���
	if (mode == 3) IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TxControlReg, mode));	 //��TX1��TX2���

	return True;
}

void fm175Drv_event(Fm175Drv* pDrv, TRANS_EVENT evt, TRANSFER_RESULT res)
{
	pDrv->latestErr = res;
	if (pDrv->Event) pDrv->Event(pDrv->obj, evt);

	if (evt == TRANS_FAILED || evt == TRANS_SUCCESS)
	{
		pDrv->transParam.txBufSize = 0;	//ȡ������

		IIC_REG_ERR_RETURN(IICReg_SetBitMask(&pDrv->iicReg, ControlReg, 0x80));           // stop timer now
		IIC_REG_ERR_RETURN(IICReg_writeByte(&pDrv->iicReg, CommandReg, Idle));
		IIC_REG_ERR_RETURN(IICReg_clearBitMask(&pDrv->iicReg, BitFramingReg, 0x80));//�رշ���
		IIC_REG_ERR_RETURN(IICReg_writeByte(&pDrv->iicReg, WaterLevelReg, 0));//����WaterLeve

		SwTimer_Stop(&pDrv->timer);
		pDrv->transStatus = TRANSFER_STATUS_IDLE;
	}

	if (evt == TRANS_FAILED && pDrv->state == FM_STATE_TRANSFER)
	{
		//�����ѿ�
		fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
	}
}	

void fm175Drv_irq_rx(Fm175Drv* pDrv)
{
	TransParam* param = &pDrv->transParam;
	uint8 bytesInFifo = 0;

	if (pDrv->transStatus != TRANSFER_STATUS_RX) return;
	IIC_REG_ERR_RETURN(IICReg_readByte(&pDrv->iicReg, FIFOLevelReg, &bytesInFifo));

	//ѭ����ȡ����FIFO������
	while (bytesInFifo)
	{
		int readLen = MIN(param->rxBufSize - param->transBufOffset, bytesInFifo);
		if (readLen == 0)
		{
			//�û�Ӧ���ڴ��¼��У������Ѿ����յ�������param->rxBuf�У���������param->rxBufSize��ֵ
			fm175Drv_event(pDrv, TRANS_RX_BUF_FULL, TRANS_RESULT_SUCCESS);
			param->transBufOffset = 0;
		}
		else
		{
			IIC_REG_ERR_RETURN(IICReg_readFifo(&pDrv->iicReg, &param->rxBuf[param->transBufOffset], readLen)); //����FIFO����
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

	//��ǰ���ж���δ�����������FIFO
	uint8 bytesInFifo = 0;

	//���ˮλΪ0��˵�������Ѿ���ɣ�FIFO�п����н��յ������ݣ����ܹ���ȡbytesInFifo�鿴FIFO�л��ж��ٸ�����û�������ߡ�
	//���ˮλ��Ϊ0��˵������δ��ɣ�ͨ����ȡbytesInFifo�鿴FIFO�л��ж��ٸ�����û�������ߡ�
	if (pDrv->waterLevel != 0)
	{
		IIC_REG_ERR_RETURN(IICReg_readByte(&pDrv->iicReg, FIFOLevelReg, &bytesInFifo));
	}

	//����ʵ���Ѿ���������ݳ���
	int sentLen = param->putBytesInTxFifo - bytesInFifo;

	param->offset += sentLen;
	param->transBufOffset += sentLen;

	remainLen = param->totalLen - param->offset;
	if (remainLen == 0)
	{
		PFL(DL_NFC, "NFC TX(%d) Done.\n", param->transBufOffset);
		param->putBytesInTxFifo = 0;
		goto TxDone;
	}
	else
	{
		//���ʣ���FIFO�ռ���Դ��ȫ�����ݣ�����ˮλΪ0����������FIFO��ȵ�һ�롣
		uint8 waterLevel = (remainLen <= pDrv->cfg->fifoDeepth - bytesInFifo) ? 0 : (pDrv->cfg->fifoDeepth >> 1);
		if (pDrv->waterLevel != waterLevel)
		{
			pDrv->waterLevel = waterLevel;
			IIC_REG_ERR_RETURN(IICReg_writeByte(&pDrv->iicReg, WaterLevelReg, pDrv->waterLevel));//����WaterLeve
		}
	}

	if (param->transBufOffset >= param->txBufSize)
	{
		//�û�Ӧ���ڴ��¼��У�����Ҫ��������ݵ�pDrv->txBuf�У���������pDrv->transMgr.txBufSize��ֵ
		fm175Drv_event(pDrv, TRANS_TX_BUF_EMPTY, TRANS_RESULT_SUCCESS);
		param->transBufOffset = 0;
	}

	//������Ҫ�ŵ�TX FIFO�е��ֽ���
	param->putBytesInTxFifo = MIN(param->txBufSize - param->transBufOffset, cfg->fifoDeepth - bytesInFifo);

	if (param->putBytesInTxFifo)
	{
		PFL(DL_NFC, "NFC TX(%d/%d,%d):", param->transBufOffset, param->totalLen, param->putBytesInTxFifo);
		DUMP_BYTE_LEVEL(DL_NFC, & param->txBuf[param->transBufOffset], param->putBytesInTxFifo);
		PFL(DL_NFC, "\n");
		
		IIC_REG_ERR_RETURN(IICReg_writeFifo(&pDrv->iicReg, &param->txBuf[param->transBufOffset], param->putBytesInTxFifo));
		IIC_REG_ERR_RETURN(IICReg_SetBitMask(&pDrv->iicReg, BitFramingReg, 0x80));	//��������
		return;
	}
	
TxDone:
	//�Ƿ���Ҫ��������
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
		IIC_REG_ERR_RETURN(IICReg_writeByte(&pDrv->iicReg, WaterLevelReg, pDrv->waterLevel));//���ý���FIFO��WaterLeve
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
	//������Mifare��Ƭ��д����ʱ�᷵��4��BIT,��Ҫ���δ�����
}
void fm175Drv_irq_timeOut(Fm175Drv* pDrv)
{
	if (pDrv->transStatus == TRANSFER_STATUS_IDLE) return;

	fm175Drv_event(pDrv, TRANS_FAILED, TRANS_RESULT_TIMEOUT);
}

//��ȡ�жϴ�����
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
	//�鿴�ж�λ
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

			//���ж�
			IIC_REG_ERR_RETURN(IICReg_writeByte(&pDrv->iicReg, ComIrqReg, BIT(i)));
		}
	}
}

static void fm175Drv_switchState(Fm175Drv* pDrv, uint8 state)
{
//	uint8 rc = 0;
	if (pDrv->state == state) return;

	if (state == FM_STATE_INIT)
	{
	}
	else if (state == FM_STATE_NPD_LOW)
	{
		FM17522_NPD_LOW;
		SwTimer_Start(&pDrv->timer, 10, 0);
	}
	else if (state == FM_STATE_NPD_HIGH)
	{
		FM17522_NPD_HIGHT;
		SwTimer_Start(&pDrv->timer, 10, 0);
	}
	else if (state == FM_STATE_SEARCH_CARD)
	{
		Fm175Drv_setRf(pDrv, pDrv->antPort);

		//��TX�������Ҫ��ʱ�ȴ������ز��ź��ȶ�
		SwTimer_Start(&pDrv->timer, 1, 0);
	}
	else if (state == FM_STATE_SLEEP)
	{
		//����͹���
		Fm175Drv_SoftPowerdown(pDrv);
		pDrv->antPort = 1;
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
			else if (pDrv->antPort == 1)
			{
				pDrv->antPort = 2;
				fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
			}
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
		if (pDrv->transParam.txBufSize && pDrv->transStatus == TRANSFER_STATUS_PENDING_TX)
		{
			fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
		}
	}
	else if (msg == FM_MSG_SWITCH_NFC)
	{
		pDrv->iicReg.dev_addr = (uint8)param;
		fm175Drv_switchState(pDrv, FM_STATE_NPD_LOW);
	}
}

void fm175Drv_waitTransDone(Fm175Drv* pDrv)
{
	//ͬ�����ͷ�ʽ���������ݵȴ�����
	uint8 irq = 0;
	while (pDrv->transStatus != TRANSFER_STATUS_IDLE && !SwTimer_isTimerOut(&pDrv->timer))
	{
		if (IICReg_readByte(&pDrv->iicReg, ComIrqReg, &irq))//��ѯ�жϱ�־	
		{
			fm175Drv_irq(pDrv, irq);
		}
	}
}

void fm175Drv_fsmTransfer(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param)
{
	//�Ƿ������ʱ��û�н��յ���Ӧ
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
			if (IICReg_readByte(&pDrv->iicReg, ComIrqReg, &irq))//��ѯ�жϱ�־	
			{
				fm175Drv_irq(pDrv, irq);
			}
		}
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

	//���򲻿������е�����
	Assert(False);
	return Null;
}

void fm175Drv_fsm(Fm175Drv* pDrv, FM17522_MSG msg, uint32 param)
{
	pDrv->fsm(pDrv, msg, param);
}

#endif

/*********************************************/
/*��������	    fm175Drv_setTimer    */
/*���ܣ�	    ���ý�����ʱ    */
/*���������	delaytime����ʱʱ�䣨��λΪ���룩    */
/*����ֵ��	    OK    */
/*********************************************/
Bool fm175Drv_setTimer(Fm175Drv* pDrv, unsigned long delaytime)//�趨��ʱʱ�䣨ms��
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
�������ܣ������ʼ��,����Ǵ�����ݴ��䣬��Ҫ�������txBuf,�������øú����еĲ���txBuf=Null��
����˵����
	pDrv��ȷ���������
	txBuf�� ��������ָ��,���ΪNULL����ʾ�ڴ˲�ָ����������(����������)����Ҫ��TRANS_TX_BUF_EMPTY����䷢������
	txBufSize�� �������ݳ���
	rxBuf��		��������ָ��
	rxBufSize�� �������ݳ���
����ֵ��TRUE-�����ʼ���ɹ���FALSE-�����ʼ��ʧ��
***************************************************************/
Bool fm175Drv_transferInit(Fm175Drv* pDrv, const void* txBuf, int txBufSize, void* rxBuf, int rxBufSize, void* pObj)
{
	TransParam* param = &pDrv->transParam;
	//���������ڴ���
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
�������ܣ�����������������ᴥ�������¼���ͨ�������¼��ص�����֪ͨ���������ɹ�����ʧ�ܣ��ο��¼�����FM_EVENT��
����˵����
	pDrv��ȷ���������
	cmd����������
	timeOutMs�� ��ʱʱ��
����ֵ��TRUE-���������ɹ���FALSE-��������ʧ��
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

	//���txBufΪNULL����ʾ��ָ���������ݣ���Ҫ��TRANS_TX_BUF_EMPTY����䷢������
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
	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, WaterLevelReg, pDrv->waterLevel));//����WaterLeve
//   	rx_temp = IICReg_readByte(&pDrv->iicReg, WaterLevelReg);

	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, ComIrqReg, 0x7F));//���IRQ��־
//	rx_temp = IICReg_readByte(&pDrv->iicReg, ComIrqReg);

	IIC_REG_ERR_RETURN_FALSE(fm175Drv_setTimer(pDrv, timeOutMs));
	IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TModeReg, 0x80));//�Զ�������ʱ��
	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(&pDrv->iicReg, CommandReg, cmd));

	pDrv->transStatus = TRANSFER_STATUS_TX;
	SwTimer_Start(&pDrv->timer, timeOutMs + 10, 0);

	fm175Drv_event(pDrv, TRANS_TX_START, TRANS_RESULT_SUCCESS);

	fm175Drv_irq_tx(pDrv);
	return True;
}

//ͬ����������
Bool fm175Drv_SyncTransfer(Fm175Drv* pDrv
	, FM17522_CMD cmd
	, const void* txBuf, int txBufSize
	, void* rxBuf, int* rxBufSize
	, uint32 timeOutMs
)
{
	TransParam* param = &pDrv->transParam;
	//����豸״̬state�Ǽ��
	if (pDrv->state == FM_STATE_TRANSFER)
	{
		//����״̬transStatusΪ��TRANSFER_STATUS_IDLE������ʧ��
		if (pDrv->transStatus != TRANSFER_STATUS_IDLE) return False;
	}
	else if (pDrv->state == FM_STATE_SEARCH_CARD)
	{
		//����豸״̬state���ѿ������ȼ���ߣ�����״̬transStatusΪ�κ�״̬�����Է���
	}
	else //����״̬������ʧ��
	{
		return False;
	}

	int rc = False;

	//��ʱ���洫�������ͬ�����������ָ�**************************
	TransEventFn	_evtFn		= pDrv->Event	;
	FM17522_CMD		_cmd		= pDrv->cmd		;
	const void*		_txBuf		= param->txBuf	;
	int				_txBufSize  = pDrv->transParam.txBufSize;
	void*			_rxBuf      = param->rxBuf	;
	int				_rxBufSize  = param->rxBufSize;
	TRANSFER_STATUS _transStatus = pDrv->transStatus;
	/**************************************************************/

	pDrv->Event = Null;	//�¼�����ָ���ÿ�,ͬ�����䲻��ҪEvent����
	pDrv->transStatus = TRANSFER_STATUS_IDLE;

	//�������䲢�ȴ����
	fm175Drv_transferInit(pDrv, txBuf, txBufSize, rxBuf, *rxBufSize, Null);
	if (fm175Drv_transStart(pDrv, cmd, timeOutMs))
	{
		fm175Drv_waitTransDone(pDrv);
	}
	else
	{
		pDrv->latestErr == TRANS_RESULT_FAILED;
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
	//�ָ��������
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

void fm175Drv_run(Fm175Drv* pDrv)
{
	fm175Drv_fsm(pDrv, FM_MSG_RUN, 0);
}

void fm175Drv_start(Fm175Drv* pDrv)
{
	fm175Drv_switchState(pDrv, FM_STATE_SLEEP);
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
	g_pNfcPwrOffIO=IO_Get(IO_NFC_PWR_OFF);
	PortPin_Set(g_pNfcPwrOffIO->periph, g_pNfcPwrOffIO->pin, False);
}

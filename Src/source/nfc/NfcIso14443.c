
#include "NfcIso14443.h"
#include "fm175Drv.h"


/*********************************************/
/*函数名：	    Pcd_ConfigISOType    */
/*功能：	    设置操作协议    */
/*输入参数：	type 0，ISO14443A协议；    */
/*					 1，ISO14443B协议；    */
/*返回值：	    OK    */
/*********************************************/
Bool NfcIso14443_ConfigISOType(Fm175Drv* pDrv, NFC_ISO_TYPE type)
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
/*名称：NfcIso14443_Request																	*/
/*功能：NfcIso14443_Request卡片寻卡															*/
/*输入：																				*/
/*输出：																			 	*/
/*	       	pTagType[0],pTagType[1] =ATQA                                         		*/
/*       	OK: 应答正确                                                              	*/
/*	 		ERROR: 应答错误																*/
/****************************************************************************************/

Bool NfcIso14443_Request(Fm175Drv* pDrv, unsigned char* pTagType)
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
/*名称：NfcIso14443_Anticollision														*/
/*功能：NfcIso14443_Anticollision卡片防冲突												*/
/*输入：selcode =0x93，0x95，0x97														*/
/*输出：																			 	*/
/*	       	pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID                            		*/
/****************************************************************************************/
Bool NfcIso14443_Anticollision(Fm175Drv* pDrv, unsigned char selcode,unsigned char* pSnr)
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
/*名称：NfcIso14443_Select																	*/
/*功能：NfcIso14443_Select卡片选卡															*/
/*输入：selcode =0x93，0x95，0x97														*/
/*      pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID 			    			     	*/
/*输出：																			 	*/
/*	       	pSak[0],pSak[1],pSak[2] =SAK			                            		*/
/*       	OK: 应答正确                                                              	*/
/*	 		ERROR: 应答错误																*/
/****************************************************************************************/
Bool NfcIso14443_Select(Fm175Drv* pDrv, unsigned char selcode, unsigned char* pSnr, unsigned char* pSak)
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
Bool NfcIso14443_RATS(Fm175Drv* pDrv, unsigned char param)
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
/*名称：NfcIso14443_CardActivate														*/
/*功能：NfcIso14443_CardActivate卡片激活												*/
/*输入：																				*/
/*       			    			     												*/
/*	       								 												*/
/*输出：	pTagType[0],pTagType[1] =ATQA 											 	*/
/*	       	pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID 		                   		*/
/*	       	pSak[0],pSak[1],pSak[2] =SAK			                            		*/
/*       	OK: 应答正确                                                              	*/
/*	 		ERROR: 应答错误																*/
/****************************************************************************************/
Bool NfcIso14443_CardActivate(Fm175Drv* pDrv)
{
	unsigned char pTagType[2];
	unsigned char pSnr[15];
	unsigned char pSak[3];

	if (!NfcIso14443_Request(pDrv, pTagType)) return False; //寻卡 Standard	 send request command Standard mode

	if ((pTagType[0] & 0xC0) == 0x00)	//一重UID
	{
		if (!NfcIso14443_Anticollision(pDrv, 0x93, pSnr))				return False; //1级防冲突
		if (!NfcIso14443_Select       (pDrv, 0x93, pSnr, pSak))			return False; //1级选卡
	}

	if ((pTagType[0] & 0xC0) == 0x40)	//二重UID
	{
		if (!NfcIso14443_Anticollision(pDrv, 0x93, pSnr))				return False; //1级防冲突
		if (!NfcIso14443_Select       (pDrv, 0x93, pSnr, pSak))			return False; //1级选卡
		if (!NfcIso14443_Anticollision(pDrv, 0x95, pSnr + 5 ))			return False; //2级防冲突
		if (!NfcIso14443_Select       (pDrv, 0x95, pSnr + 5, pSak+1))	return False; //2级选卡
	}

	if ((pTagType[0] & 0xC0) == 0x80)	//三重UID
	{
		if (!NfcIso14443_Anticollision(pDrv, 0x93, pSnr))				return False; //1级防冲突
		if (!NfcIso14443_Select       (pDrv, 0x93, pSnr, pSak))			return False; //1级选卡
		if (!NfcIso14443_Anticollision(pDrv, 0x95, pSnr + 5 ))			return False; //2级防冲突
		if (!NfcIso14443_Select       (pDrv, 0x95, pSnr + 5, pSak+1))	return False; //2级选卡
		if (!NfcIso14443_Anticollision(pDrv, 0x97, pSnr + 10 ))			return False; //3级防冲突
		if (!NfcIso14443_Select       (pDrv, 0x97, pSnr + 10, pSak+2))	return False; //3级选卡
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
Bool NfcIso14443_SoftPowerdown(Fm175Drv* pDrv)
{
	//unsigned char result;
	////    result = 
	//IICReg_readByte(pDrv, CommandReg, &result);

	return IICReg_SetBitMask(&pDrv->iicReg, CommandReg, 0x10);//进入低功耗模式
}

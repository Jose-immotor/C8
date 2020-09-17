
#include "NfcIso14443.h"
#include "fm175Drv.h"


/*********************************************/
/*��������	    Pcd_ConfigISOType    */
/*���ܣ�	    ���ò���Э��    */
/*���������	type 0��ISO14443AЭ�飻    */
/*					 1��ISO14443BЭ�飻    */
/*����ֵ��	    OK    */
/*********************************************/
Bool NfcIso14443_ConfigISOType(Fm175Drv* pDrv, NFC_ISO_TYPE type)
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
/*���ƣ�NfcIso14443_Request																	*/
/*���ܣ�NfcIso14443_Request��ƬѰ��															*/
/*���룺																				*/
/*�����																			 	*/
/*	       	pTagType[0],pTagType[1] =ATQA                                         		*/
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/

Bool NfcIso14443_Request(Fm175Drv* pDrv, unsigned char* pTagType)
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
/*���ƣ�NfcIso14443_Anticollision														*/
/*���ܣ�NfcIso14443_Anticollision��Ƭ����ͻ												*/
/*���룺selcode =0x93��0x95��0x97														*/
/*�����																			 	*/
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
/*���ƣ�NfcIso14443_Select																	*/
/*���ܣ�NfcIso14443_Select��Ƭѡ��															*/
/*���룺selcode =0x93��0x95��0x97														*/
/*      pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID 			    			     	*/
/*�����																			 	*/
/*	       	pSak[0],pSak[1],pSak[2] =SAK			                            		*/
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
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
/*����: TypeA_RATS 											 */
/*����: �ú���ʵ��ISO14443A-4Э���е�RATS����			     */
/*����: param byte bit8-bit5��FSDI,bit4-bit0 ��CID			 */
/*       ats ���ڱ���RATS����ص�����						 */
/*���:														 */
/* OK: Ӧ����ȷ												 */
/* ERROR: Ӧ�����											 */
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
	IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, TxModeReg, 0x80));                         //���÷���CRC
	IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(&pDrv->iicReg, RxModeReg, 0x80));                         //���ý���CRC
	IIC_REG_ERR_RETURN_FALSE(IICReg_clearBitMask(&pDrv->iicReg, Status2Reg, 0x08));

	return fm175Drv_SyncTransfer(pDrv, Transceive, send_buff, sizeof(send_buff), rece_buff, &rece_bitlen, 5);
}

/****************************************************************************************/
/*���ƣ�NfcIso14443_CardActivate														*/
/*���ܣ�NfcIso14443_CardActivate��Ƭ����												*/
/*���룺																				*/
/*       			    			     												*/
/*	       								 												*/
/*�����	pTagType[0],pTagType[1] =ATQA 											 	*/
/*	       	pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID 		                   		*/
/*	       	pSak[0],pSak[1],pSak[2] =SAK			                            		*/
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/
Bool NfcIso14443_CardActivate(Fm175Drv* pDrv)
{
	unsigned char pTagType[2];
	unsigned char pSnr[15];
	unsigned char pSak[3];

	if (!NfcIso14443_Request(pDrv, pTagType)) return False; //Ѱ�� Standard	 send request command Standard mode

	if ((pTagType[0] & 0xC0) == 0x00)	//һ��UID
	{
		if (!NfcIso14443_Anticollision(pDrv, 0x93, pSnr))				return False; //1������ͻ
		if (!NfcIso14443_Select       (pDrv, 0x93, pSnr, pSak))			return False; //1��ѡ��
	}

	if ((pTagType[0] & 0xC0) == 0x40)	//����UID
	{
		if (!NfcIso14443_Anticollision(pDrv, 0x93, pSnr))				return False; //1������ͻ
		if (!NfcIso14443_Select       (pDrv, 0x93, pSnr, pSak))			return False; //1��ѡ��
		if (!NfcIso14443_Anticollision(pDrv, 0x95, pSnr + 5 ))			return False; //2������ͻ
		if (!NfcIso14443_Select       (pDrv, 0x95, pSnr + 5, pSak+1))	return False; //2��ѡ��
	}

	if ((pTagType[0] & 0xC0) == 0x80)	//����UID
	{
		if (!NfcIso14443_Anticollision(pDrv, 0x93, pSnr))				return False; //1������ͻ
		if (!NfcIso14443_Select       (pDrv, 0x93, pSnr, pSak))			return False; //1��ѡ��
		if (!NfcIso14443_Anticollision(pDrv, 0x95, pSnr + 5 ))			return False; //2������ͻ
		if (!NfcIso14443_Select       (pDrv, 0x95, pSnr + 5, pSak+1))	return False; //2��ѡ��
		if (!NfcIso14443_Anticollision(pDrv, 0x97, pSnr + 10 ))			return False; //3������ͻ
		if (!NfcIso14443_Select       (pDrv, 0x97, pSnr + 10, pSak+2))	return False; //3��ѡ��
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
Bool NfcIso14443_SoftPowerdown(Fm175Drv* pDrv)
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
Bool NfcIso14443_setRf(Fm175Drv* pDrv, unsigned char mode)
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

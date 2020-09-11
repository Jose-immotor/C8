#if 1

/*************************************************************/
//2014.07.15�޸İ�
/*************************************************************/
#define TYPE_A_C
#include "FM175XX.h"
#include "type_a.h"

#define ERROR		1
#define	OK			0
  
/****************************************************************************************/
/*���ƣ�TypeA_Request																	*/
/*���ܣ�TypeA_Request��ƬѰ��															*/
/*���룺																				*/
/*       			    			     												*/
/*	       								 												*/
/*�����																			 	*/
/*	       	pTagType[0],pTagType[1] =ATQA                                         		*/					
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/
unsigned char TypeA_Request(unsigned char *pTagType)
{
    unsigned char result;
    unsigned char send_buff[1];
    unsigned char rece_buff[2];
	unsigned int rece_bitlen;
    
	Clear_BitMask(TxModeReg,0x80);//�ر�TX CRC
        
	Clear_BitMask(RxModeReg,0x80);//�ر�RX CRC
        
	Set_BitMask(RxModeReg, 0x08);//�ر�λ����//chenke, RxNoErr = 1, ���յ���Ч�������ᱻ���ԣ�ͬʱ���������ּ���״̬
    
	Clear_BitMask(Status2Reg,0x08);//chenke, Crypto1On = 0, ��λֻ��M1���Ķ�дģʽ��Ч��
    
	Write_Reg(BitFramingReg,0x07);//chenke, TxLastBit = 7, ���һ���ֽ���Ҫ�����͵�λ����
    
	send_buff[0] = 0x26;// chenke, 0x26��REQA�����PCD��������̽����������A PICC�Ĺ�����
   	Pcd_SetTimer(1);//chenke, ��FM17522оƬ�����ö�ʱ���ӳ�ʱ��(�������ﻹû��ʹ�ܸö�ʱ��?)��
	result = Pcd_Comm(Transceive,send_buff,1,rece_buff,&rece_bitlen);
	if ((result == OK) && (rece_bitlen == 2*8))
	{    
		*pTagType     = rece_buff[0];
		*(pTagType+1) = rece_buff[1];
	}
 	return result;
}

/****************************************************************************************/
/*���ƣ�TypeA_WakeUp																	*/
/*���ܣ�TypeA_WakeUp��ƬѰ��															*/
/*���룺																				*/
/*       			    			     												*/
/*	       								 												*/
/*�����																			 	*/
/*	       	pTagType[0],pTagType[1] =ATQA                                         		*/					
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/

unsigned char TypeA_WakeUp(unsigned char *pTagType)
{    
    unsigned char result;
    unsigned char send_buff[1];
    unsigned char rece_buff[2];
	unsigned int rece_bitlen;
    
	Clear_BitMask(TxModeReg,0x80);//�ر�TX CRC
	Clear_BitMask(RxModeReg,0x80);//�ر�RX CRC
	Set_BitMask(RxModeReg, 0x08);//�ر�λ����
    
	Clear_BitMask(Status2Reg,0x08);
	Write_Reg(BitFramingReg,0x07);
    
 	send_buff[0] = 0x52;
   	Pcd_SetTimer(1);
    
	result = Pcd_Comm(Transceive,send_buff,1,rece_buff,&rece_bitlen);

	if ((result == OK) && (rece_bitlen == 2*8))
	{    
		*pTagType     = rece_buff[0];
		*(pTagType + 1) = rece_buff[1];
	}
	return result;
}


/****************************************************************************************/
/*���ƣ�TypeA_Anticollision																*/
/*���ܣ�TypeA_Anticollision��Ƭ����ͻ													*/
/*���룺selcode =0x93��0x95��0x97														*/
/*       			    			     												*/
/*	       								 												*/
/*�����																			 	*/
/*	       	pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID                            		*/					
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/



unsigned char TypeA_Anticollision(unsigned char selcode,
    unsigned char *pSnr)
{    
    unsigned char result;
    unsigned char i;
    unsigned char send_buff[2];
    unsigned char rece_buff[5];
    unsigned int rece_bitlen;
    
	Clear_BitMask(TxModeReg,0x80);
    
	Clear_BitMask(RxModeReg,0x80);
    
    Clear_BitMask(Status2Reg,0x08);
    
    Write_Reg(BitFramingReg,0x00);
    
    Clear_BitMask(CollReg,0x80);
    
    send_buff[0] = selcode;
    send_buff[1] = 0x20;
    Pcd_SetTimer(1);
    
    result = Pcd_Comm(Transceive,send_buff,2,rece_buff,&rece_bitlen);
    
    if (result == OK)
    {
    	 for (i = 0; i < 5; i++)
         	*(pSnr + i)  = rece_buff[i];
         if (pSnr[4] != (pSnr[0]^pSnr[1]^pSnr[2]^pSnr[3]))
        {
    		result = ERROR;    
        }
    }
  return result;
}


/****************************************************************************************/
/*���ƣ�TypeA_Select																	*/
/*���ܣ�TypeA_Select��Ƭѡ��															*/
/*���룺selcode =0x93��0x95��0x97														*/
/*      pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID 			    			     	*/
/*	       								 												*/
/*�����																			 	*/
/*	       	pSak[0],pSak[1],pSak[2] =SAK			                            		*/					
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/


unsigned char TypeA_Select(unsigned char selcode,
    unsigned char *pSnr,
    unsigned char *pSak)
{    
    unsigned char result;
    unsigned char i;
    unsigned char send_buff[7];
    unsigned char rece_buff[5];
    unsigned int rece_bitlen;
    
	Write_Reg(BitFramingReg,0x00);
  	Set_BitMask(TxModeReg,0x80);    
	Set_BitMask(RxModeReg,0x80);
    Clear_BitMask(Status2Reg,0x08);
    
	send_buff[0] = selcode;
    send_buff[1] = 0x70;
    
    for (i = 0; i < 5; i++)
       	send_buff[i + 2] = *(pSnr + i);
    
   	Pcd_SetTimer(1);    
    result = Pcd_Comm(Transceive,send_buff,7,rece_buff,&rece_bitlen);
    if (result == OK)
    	*pSak = rece_buff[0]; 
	return result;
}



/****************************************************************************************/
/*���ƣ�TypeA_Halt																		*/
/*���ܣ�TypeA_Halt��Ƭֹͣ																*/
/*���룺																				*/
/*       			    			     												*/
/*	       								 												*/
/*�����																			 	*/
/*	       											                            		*/					
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/

unsigned char TypeA_Halt(void)
{
    unsigned char result;
    unsigned char send_buff[2];
    unsigned char rece_buff[1];
	unsigned int rece_bitlen;
    
    send_buff[0] = 0x50;
    send_buff[1] = 0x00;
   
   	Write_Reg(BitFramingReg,0x00);
  	Set_BitMask(TxModeReg,0x80);
	Set_BitMask(RxModeReg,0x80);
    Clear_BitMask(Status2Reg,0x08);
	Pcd_SetTimer(1);    
    result = Pcd_Comm(Transmit,send_buff,2,rece_buff,&rece_bitlen);

    return result;
}


/****************************************************************************************/
/*���ƣ�TypeA_CardActivate																*/
/*���ܣ�TypeA_CardActivate��Ƭ����														*/
/*���룺																				*/
/*       			    			     												*/
/*	       								 												*/
/*�����	pTagType[0],pTagType[1] =ATQA 											 	*/
/*	       	pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID 		                   		*/
/*	       	pSak[0],pSak[1],pSak[2] =SAK			                            		*/					
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/ 
unsigned char TypeA_CardActivate(unsigned char *pTagType,
    unsigned char *pSnr,
    unsigned char *pSak)
{
    unsigned char result;
    result=TypeA_Request(pTagType);//Ѱ�� Standard	 send request command Standard mode

    if (result == ERROR)
    {
        return ERROR;
    }
    
    if 	((pTagType[0]&0xC0) == 0x00)	//һ��UID
    {
        result=TypeA_Anticollision(0x93,pSnr);//1������ͻ
        if (result == ERROR)
        {
            return ERROR;
        }
        
        result=TypeA_Select(0x93,pSnr,pSak);//1��ѡ��
        if (result == ERROR)
        {
            return ERROR;
        }
    }

    if 	((pTagType[0] & 0xC0) == 0x40)	//����UID
    {
        result=TypeA_Anticollision(0x93,pSnr);//1������ͻ
        if (result == ERROR)
        {
            return ERROR;
        }

        result=TypeA_Select(0x93,pSnr,pSak);//1��ѡ��
        if (result == ERROR)
        {
            return ERROR;
        }
        result=TypeA_Anticollision(0x95,pSnr+5);//2������ͻ
        if (result == ERROR)
        {
            return ERROR;
        }

        result=TypeA_Select(0x95,pSnr+5,pSak);//2��ѡ��
        if (result == ERROR)
        {
            return ERROR;
        }
    }

    if 	((pTagType[0]&0xC0) == 0x80)	//����UID
    {
        result=TypeA_Anticollision(0x93,pSnr);//1������ͻ
        if (result==ERROR)
        {
            return ERROR;
        }

        result=TypeA_Select(0x93,pSnr,pSak);//1��ѡ��
        if (result == ERROR)
        {
            return ERROR;
        }

        result=TypeA_Anticollision(0x95,pSnr+5);//2������ͻ
        if (result == ERROR)
        {
            return ERROR;
        }

        result=TypeA_Select(0x95,pSnr+5,pSak+1);//2��ѡ��
        if (result == ERROR)
        {
            return ERROR;
        }

        result=TypeA_Anticollision(0x97,pSnr+10);//3������ͻ
        if (result == ERROR)
        {
            return ERROR;
        }

        result=TypeA_Select(0x97,pSnr+10,pSak+2);//3��ѡ��
        if (result == ERROR)
        {
            return ERROR;
        }
    }

    return result;
}

/****************************************************************/
/*����: TypeA_RATS 													           */
/*����: �ú���ʵ��ISO14443A-4Э���е�RATS����			     */
/*����: param byte bit8-bit5��FSDI,bit4-bit0 ��CID     */
/*       ats ���ڱ���RATS����ص�����                */
/*���:															                   */
/* OK: Ӧ����ȷ												                 */
/* ERROR: Ӧ�����												             */
/****************************************************************/
unsigned char TypeA_RATS(unsigned char param, unsigned char* ats)
{
	unsigned char send_buff[4];
	unsigned char result;
	unsigned int rece_bitlen;
	//unsigned long CID = 0;

	send_buff[0] = 0xE0;
	send_buff[1] = param;                                //
	//CID = param & 0x0f;
	Write_Reg(BitFramingReg, 0x00);
	Set_BitMask(TxModeReg, 0x80);                         //���÷���CRC
	Set_BitMask(RxModeReg, 0x80);                         //���ý���CRC
	Clear_BitMask(Status2Reg, 0x08);
	Pcd_SetTimer(5);
	Clear_FIFO();
	result = Pcd_Comm(Transceive, send_buff, 2, ats, &rece_bitlen);
	return result;
}
#endif

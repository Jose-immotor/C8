#if 1

/*************************************************************/
//2014.07.15修改版
/*************************************************************/
#define TYPE_A_C
#include "FM175XX.h"
#include "type_a.h"

#define ERROR		1
#define	OK			0
  
/****************************************************************************************/
/*名称：TypeA_Request																	*/
/*功能：TypeA_Request卡片寻卡															*/
/*输入：																				*/
/*       			    			     												*/
/*	       								 												*/
/*输出：																			 	*/
/*	       	pTagType[0],pTagType[1] =ATQA                                         		*/					
/*       	OK: 应答正确                                                              	*/
/*	 		ERROR: 应答错误																*/
/****************************************************************************************/
unsigned char TypeA_Request(unsigned char *pTagType)
{
    unsigned char result;
    unsigned char send_buff[1];
    unsigned char rece_buff[2];
	unsigned int rece_bitlen;
    
	Clear_BitMask(TxModeReg,0x80);//关闭TX CRC
        
	Clear_BitMask(RxModeReg,0x80);//关闭RX CRC
        
	Set_BitMask(RxModeReg, 0x08);//关闭位接收//chenke, RxNoErr = 1, 接收到无效数据流会被忽略，同时接收器保持激活状态
    
	Clear_BitMask(Status2Reg,0x08);//chenke, Crypto1On = 0, 该位只在M1卡的读写模式有效。
    
	Write_Reg(BitFramingReg,0x07);//chenke, TxLastBit = 7, 最后一个字节需要被发送的位数。
    
	send_buff[0] = 0x26;// chenke, 0x26是REQA命令，由PCD发出，以探测用于类型A PICC的工作场
   	Pcd_SetTimer(1);//chenke, 在FM17522芯片上设置定时器延迟时间(可能这里还没有使能该定时器?)，
	result = Pcd_Comm(Transceive,send_buff,1,rece_buff,&rece_bitlen);
	if ((result == OK) && (rece_bitlen == 2*8))
	{    
		*pTagType     = rece_buff[0];
		*(pTagType+1) = rece_buff[1];
	}
 	return result;
}

/****************************************************************************************/
/*名称：TypeA_WakeUp																	*/
/*功能：TypeA_WakeUp卡片寻卡															*/
/*输入：																				*/
/*       			    			     												*/
/*	       								 												*/
/*输出：																			 	*/
/*	       	pTagType[0],pTagType[1] =ATQA                                         		*/					
/*       	OK: 应答正确                                                              	*/
/*	 		ERROR: 应答错误																*/
/****************************************************************************************/

unsigned char TypeA_WakeUp(unsigned char *pTagType)
{    
    unsigned char result;
    unsigned char send_buff[1];
    unsigned char rece_buff[2];
	unsigned int rece_bitlen;
    
	Clear_BitMask(TxModeReg,0x80);//关闭TX CRC
	Clear_BitMask(RxModeReg,0x80);//关闭RX CRC
	Set_BitMask(RxModeReg, 0x08);//关闭位接收
    
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
/*名称：TypeA_Anticollision																*/
/*功能：TypeA_Anticollision卡片防冲突													*/
/*输入：selcode =0x93，0x95，0x97														*/
/*       			    			     												*/
/*	       								 												*/
/*输出：																			 	*/
/*	       	pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID                            		*/					
/*       	OK: 应答正确                                                              	*/
/*	 		ERROR: 应答错误																*/
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
/*名称：TypeA_Select																	*/
/*功能：TypeA_Select卡片选卡															*/
/*输入：selcode =0x93，0x95，0x97														*/
/*      pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID 			    			     	*/
/*	       								 												*/
/*输出：																			 	*/
/*	       	pSak[0],pSak[1],pSak[2] =SAK			                            		*/					
/*       	OK: 应答正确                                                              	*/
/*	 		ERROR: 应答错误																*/
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
/*名称：TypeA_Halt																		*/
/*功能：TypeA_Halt卡片停止																*/
/*输入：																				*/
/*       			    			     												*/
/*	       								 												*/
/*输出：																			 	*/
/*	       											                            		*/					
/*       	OK: 应答正确                                                              	*/
/*	 		ERROR: 应答错误																*/
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
/*名称：TypeA_CardActivate																*/
/*功能：TypeA_CardActivate卡片激活														*/
/*输入：																				*/
/*       			    			     												*/
/*	       								 												*/
/*输出：	pTagType[0],pTagType[1] =ATQA 											 	*/
/*	       	pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID 		                   		*/
/*	       	pSak[0],pSak[1],pSak[2] =SAK			                            		*/					
/*       	OK: 应答正确                                                              	*/
/*	 		ERROR: 应答错误																*/
/****************************************************************************************/ 
unsigned char TypeA_CardActivate(unsigned char *pTagType,
    unsigned char *pSnr,
    unsigned char *pSak)
{
    unsigned char result;
    result=TypeA_Request(pTagType);//寻卡 Standard	 send request command Standard mode

    if (result == ERROR)
    {
        return ERROR;
    }
    
    if 	((pTagType[0]&0xC0) == 0x00)	//一重UID
    {
        result=TypeA_Anticollision(0x93,pSnr);//1级防冲突
        if (result == ERROR)
        {
            return ERROR;
        }
        
        result=TypeA_Select(0x93,pSnr,pSak);//1级选卡
        if (result == ERROR)
        {
            return ERROR;
        }
    }

    if 	((pTagType[0] & 0xC0) == 0x40)	//二重UID
    {
        result=TypeA_Anticollision(0x93,pSnr);//1级防冲突
        if (result == ERROR)
        {
            return ERROR;
        }

        result=TypeA_Select(0x93,pSnr,pSak);//1级选卡
        if (result == ERROR)
        {
            return ERROR;
        }
        result=TypeA_Anticollision(0x95,pSnr+5);//2级防冲突
        if (result == ERROR)
        {
            return ERROR;
        }

        result=TypeA_Select(0x95,pSnr+5,pSak);//2级选卡
        if (result == ERROR)
        {
            return ERROR;
        }
    }

    if 	((pTagType[0]&0xC0) == 0x80)	//三重UID
    {
        result=TypeA_Anticollision(0x93,pSnr);//1级防冲突
        if (result==ERROR)
        {
            return ERROR;
        }

        result=TypeA_Select(0x93,pSnr,pSak);//1级选卡
        if (result == ERROR)
        {
            return ERROR;
        }

        result=TypeA_Anticollision(0x95,pSnr+5);//2级防冲突
        if (result == ERROR)
        {
            return ERROR;
        }

        result=TypeA_Select(0x95,pSnr+5,pSak+1);//2级选卡
        if (result == ERROR)
        {
            return ERROR;
        }

        result=TypeA_Anticollision(0x97,pSnr+10);//3级防冲突
        if (result == ERROR)
        {
            return ERROR;
        }

        result=TypeA_Select(0x97,pSnr+10,pSak+2);//3级选卡
        if (result == ERROR)
        {
            return ERROR;
        }
    }

    return result;
}

/****************************************************************/
/*名称: TypeA_RATS 													           */
/*功能: 该函数实现ISO14443A-4协议中的RATS处理			     */
/*输入: param byte bit8-bit5：FSDI,bit4-bit0 ：CID     */
/*       ats 用于保存RATS命令返回的数据                */
/*输出:															                   */
/* OK: 应答正确												                 */
/* ERROR: 应答错误												             */
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
	Set_BitMask(TxModeReg, 0x80);                         //设置发送CRC
	Set_BitMask(RxModeReg, 0x80);                         //设置接收CRC
	Clear_BitMask(Status2Reg, 0x08);
	Pcd_SetTimer(5);
	Clear_FIFO();
	result = Pcd_Comm(Transceive, send_buff, 2, ats, &rece_bitlen);
	return result;
}
#endif

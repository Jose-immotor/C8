

#include "common.h"
#include "drv_usart.h"
#include "Rs485Modbus.h"
#include "rs485_protocol.h"

#if 1
//static uint8_t g_Rs485modBusTxBuf[128];
static uint8_t g_Rs485modBusRxBuf[128];

static uint8_t g_Rs485RxBuf[96];
static uint8_t g_Rs485TxBuf[192];

CirBuff gRs485rxBuf = {0x00};
//CirBuff gTx485txBuf = {0x00};

extern Battery g_Bat[MAX_BAT_COUNT];


void usart_push_data( uint8_t data )
{
	CirBuffPush(&gRs485rxBuf, &data, 1 );
}

static uint8_t _get_CheckSum( uint16_t *value , uint8_t *data , uint16_t size )
{
	uint32_t checksum = 0 ;
	uint16_t i = 0  ;
	if( !value || !data || !size ) return 0;
	for( i = 0 ; i < size - 1 ; i += 2 )
	{
		checksum += ( ( data[i+1] << 8 ) | data[i+0] );
	}
	if( i < size ) checksum += data[i];
	
	while( checksum >> 16 )
	{
		checksum = (checksum&0xFFFF)+(checksum>>16);
	}
	*value = (uint16_t)~checksum ;
	return 1 ;
}


int Rs485_Tx(const uint8_t* pData, int len)
{
	int i;
	DrvIo* g_Rs485DirCtrlIO = Null;
	
	//CirBuffPush( &gTx485txBuf, pData, len);
		
	g_Rs485DirCtrlIO = IO_Get(IO_DIR485_CTRL);
	PortPin_Set(g_Rs485DirCtrlIO->periph, g_Rs485DirCtrlIO->pin, True);
	for(i=0;i<len;i++)
	{
		uart4_put_byte(*pData++);
	}
	PortPin_Set(g_Rs485DirCtrlIO->periph, g_Rs485DirCtrlIO->pin, False);
	
	return 0;
}
/*
�Ǳ�����֡��ʽ��
[֡ͷ:F5 F5 F5 F5] + [֡����:1Byte��֡ͷ<64] + [�豸��ַ0x41] + [������1Byte] + [����] + [У�����ۼӺ�2Byte]

������֡��ʽ��
[֡ͷ:FE FE FE FE] + [֡����:1Byte��֡ͷ<64] + [�豸��ַ0x40] + [������1Byte] + [����] + [У�����ۼӺ�2Byte]

�������֡��ʽ��
[֡����:1Byte��֡ͷ<64] + [�豸��ַ0x30/0x31] + [Э������'B'] + [������1Byte] + [У����2Byte] + [�������� 0x16] + [�����״̬ 0] + [����]

A5 A5 A5 A5 0D A0 FF FF 01 00 00 14 A6

FE FE FE FE 0D 40 01 53 01 01 01 00 A4 
FE FE FE FE 1B 40 03 40 01 00 00 00 00 02 4C 42 04 48 4B 32 41 14 13 06 0C 02 72

len + 0x30/0x31 + 'B' + 0x10 + sum_0 + sum_1 + 0x16 + 0x00

*/
#if 1

static uint16_t _Rs485Dcode(uint8_t *poutbuff , uint16_t Osize )
{
	uint16_t pos = 0 ;
	uint16_t len = 0 ;
	if( !poutbuff || !Osize || !gRs485rxBuf.mpBuff ) return 0 ;
	//
_RS485_DCODE:	
	pos = gRs485rxBuf.miHead ;
	len = 0 ;
	while( pos != gRs485rxBuf.miTail && len < Osize )
	{
		poutbuff[len++] = gRs485rxBuf.mpBuff[pos];
		_CIR_LOOP_ADD(pos, 1, gRs485rxBuf.miSize );
		//
		if( len > 7 )
		{
			if( /*&& ( poutbuff[1] == 0x31 || poutbuff[1] == 0x32 ) */
				poutbuff[2] == 'B' && poutbuff[3] == 0x10 
			 	&& poutbuff[6] == 0x16 && poutbuff[7] == 0x00 )
			{
				if( len == poutbuff[0]) 
				{
					gRs485rxBuf.miHead = pos ;
					return len ;
				}
				else if( poutbuff[0] < len )
				{
					//gRs485rxBuf.miHead = pos ;
					_CIR_LOOP_ADD( gRs485rxBuf.miHead, 1, gRs485rxBuf.miSize );
					goto _RS485_DCODE ; 
				}
			}
			else
			{
				//gRs485rxBuf.miHead = pos ;
				_CIR_LOOP_ADD( gRs485rxBuf.miHead, 1, gRs485rxBuf.miSize );
				goto _RS485_DCODE ; 
			}
		}	
	}

	if( len >= Osize )
	{
		_CIR_LOOP_ADD( gRs485rxBuf.miHead, 1, gRs485rxBuf.miSize );
		goto _RS485_DCODE ;
	}
	return 0 ;
	
}





#else
static uint16_t _Rs485Dcode(uint8_t *poutbuff , uint16_t Osize )
{
	uint16_t pos = 0 ;
	uint16_t len = 0 ;
	if( !poutbuff || !Osize || !gRs485rxBuf.mpBuff ) return 0 ;

_RS485_DCODE:
	pos = gRs485rxBuf.miHead;
	len = 0 ;
	
	while( pos != gRs485rxBuf.miTail && len < Osize )
	{
		poutbuff[len++] = gRs485rxBuf.mpBuff[pos];
		_CIR_LOOP_ADD(pos, 1, gRs485rxBuf.miSize );
		//
		if( len == 8 )
		{
			if( poutbuff[0] == 0xF5 && poutbuff[1] == 0xF5 && 
				poutbuff[2] == 0xF5 && poutbuff[3] == 0xF5 )		// �Ǳ�����
			{
				if( !( poutbuff[4] < 255 && poutbuff[5] == 0x41 ) )	// �������ַ����,����Ϊ����
				{
					_CIR_LOOP_ADD( gRs485rxBuf.miHead, 1, gRs485rxBuf.miSize );
					goto _RS485_DCODE ;
				}
			}
			else if( poutbuff[0] == 0xFE && poutbuff[1] == 0xFE && 
				poutbuff[2] == 0xFE && poutbuff[3] == 0xFE )		// ��������
			{
				if( !( poutbuff[4] < 255 && poutbuff[5] == 0x40 ) )	// �������ַ����,����Ϊ����
				{
					_CIR_LOOP_ADD( gRs485rxBuf.miSize, 1, gRs485rxBuf.miSize );
					goto _RS485_DCODE ;
				}
			}
			else if( poutbuff[0] == 0xA5 && poutbuff[1] == 0xA5 && 
				poutbuff[2] == 0xA5 && poutbuff[3] == 0xA5 )		// ��������
			{
				if( !( poutbuff[4] < 255 && poutbuff[5] == 0x40 ) )	// �������ַ����,����Ϊ����
				{
					_CIR_LOOP_ADD( gRs485rxBuf.miSize, 1, gRs485rxBuf.miSize );
					goto _RS485_DCODE ;
				}
			}
			else													// �������
			{
				if( ! ( poutbuff[0] < 255 /*&& ( poutbuff[1] == 0x31 || poutbuff[1] == 0x32 ) */
					&& poutbuff[2] == 'B' && poutbuff[6] == 0x16 && poutbuff[7] == 0x00 ) )
				{
					_CIR_LOOP_ADD( gRs485rxBuf.miHead, 1, gRs485rxBuf.miSize );
					goto _RS485_DCODE ;
				}
			}
		}

		if( len >= 8 )
		{
			if( poutbuff[0] == 0xF5 && len == poutbuff[4] ) // �Ǳ�����
			{
				gRs485rxBuf.miHead = pos ;
				return len ;
			}
			if( poutbuff[0] == 0xFE && len == poutbuff[4] ) // ��������
			{
				gRs485rxBuf.miHead = pos ;
				return len ;
			}
			if( poutbuff[0] == len )						// �������
			{
				gRs485rxBuf.miHead = pos ;
				return len ;
			}
		}
	}
	
	if( len >= Osize )
	{
		_CIR_LOOP_ADD( gRs485rxBuf.miHead, 1, gRs485rxBuf.miSize );
		goto _RS485_DCODE ;
	}
	return 0 ;
}

#endif //

/*
�ϱ����ݵ�����
1����ش���
2��˫�綼��ʱ�Ҿ��ŵ�ʱ���������һ����طŵ� < 15A ����Ϊ��Ӵ�����
--�������һ������С,����Ϊ�Ӵ�����--


3��


*/


#if 1


static Bool _BatteryIsValid( uint8_t bat )
{
	int16_t curr_0 = 0 , curr_1 = 0;
	// �����ز�����,����Ҫ�ϱ�
	if( g_Bat[bat].presentStatus != BAT_IN ) return False ;
	//
	// ������,��������ؾ��ڷŵ� 
	if( g_Bat[0].presentStatus  == BAT_IN && g_Bat[1].presentStatus == BAT_IN )
	{
		// �����ؾ��ŵ�,����һ�ϵ�طŵ���� > 15A
		if( g_Bat[0].bmsInfo.state&0x0200 && g_Bat[1].bmsInfo.state&0x0200 )
		{
			curr_0 = SWAP16(g_Bat[0].bmsInfo.tcurr) - 30000;
			curr_1 = SWAP16(g_Bat[1].bmsInfo.tcurr) - 30000;
			//
			if( curr_0 < -1500 )		// ��� bat 0 �ŵ����15A����˵��bat 1 �Ӵ�����
			{
				if( bat == 1 ) return False ;
			}
			if( curr_1 < -1500 )		// 
			{
				if( bat == 0 ) return False ;
			}
				
		}
	}
	return True ;
}


#else

#define		_SUB(a,b)			( (a) > (b) ? ( (a) - (b) ) : ( (b) - (a)) )

static Bool _BatteryIsValid( uint8_t bat )
{
	uint16_t sub_curr = 0;
 	int16_t curr_0 = 0 , curr_1 = 0;
	
	// �����ز�����,����Ҫ�ϱ�
	if( g_Bat[bat].presentStatus != BAT_IN ) return False ;
	// ������,��������ؾ��ڷŵ� 
	if( g_Bat[0].presentStatus  == BAT_IN && g_Bat[1].presentStatus == BAT_IN )
	{
		// �����ؾ��ŵ�,��������
		if( g_Bat[0].bmsInfo.state&0x0200 && g_Bat[1].bmsInfo.state&0x0200 )
		{
			curr_0 = SWAP16(g_Bat[0].bmsInfo.tcurr);
			curr_1 = SWAP16(g_Bat[1].bmsInfo.tcurr);
			sub_curr = _SUB( curr_0, curr_1 );
			PFL(DL_485,"Bat0:%d,Bat1:%d\n",curr_0,curr_1);
			if( sub_curr > 100 )	// ���������ش���1A,�����С��Ϊû�Ӵ���
			{
				if( g_Bat[0].bmsInfo.tcurr < g_Bat[1].bmsInfo.tcurr )
				{
					if ( bat == 0 ) return False ;
				}
				else
				{
					if( bat == 1 ) return False ;
				}
			}
		
		}
	}
	return True ;
}

#endif 	


//���� 5����ȡ��Ϣ ����Ϣ���38 �ֽ� 18+20 SN+VER
static uint16_t _Batter_Cmd05( Battery *pBat , uint8_t * pout)
{
	uint16_t len = 0 ;
	if( !pout || !pBat) return 0 ;
	// SN 18Byte
	
	//pout[len++] = pBat->bmsID.sn12 & 0xFF ;
	//pout[len++] = pBat->bmsID.sn12 >> 8 ;
	pout[len++] = pBat->bmsID.sn34 & 0xFF ;
	pout[len++] = pBat->bmsID.sn34 >> 8 ;
	pout[len++] = pBat->bmsID.sn56 & 0xFF ;
	pout[len++] = pBat->bmsID.sn56 >> 8 ;
	pout[len++] = pBat->bmsID.sn78 & 0xFF ;
	pout[len++] = pBat->bmsID.sn78 >> 8 ;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	//
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	// VER 20Byte
	pout[len++] = pBat->bmsID.prver & 0xFF ;
	pout[len++] = pBat->bmsID.prver >> 8 ;
	
	pout[len++] = pBat->bmsID.hwver & 0xFF ;
	pout[len++] = pBat->bmsID.hwver >> 8 ;
	
	pout[len++] = pBat->bmsID.blver & 0xFF ;
	pout[len++] = pBat->bmsID.blver >> 8 ;

	pout[len++] = pBat->bmsID.fwmsv & 0xFF ;
	pout[len++] = pBat->bmsID.fwmsv >> 8 ;

	pout[len++] = pBat->bmsID.fwrev & 0xFF ;
	pout[len++] = pBat->bmsID.fwrev >> 8 ;

	pout[len++] = pBat->bmsID.fwbnh & 0xFF ;
	pout[len++] = pBat->bmsID.fwbnh >> 8 ;

	pout[len++] = pBat->bmsID.fwbnl & 0xFF ;
	pout[len++] = pBat->bmsID.fwbnl >> 8 ;
	
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	return len ;
}

//���� 6������ش���
static uint16_t _Batter_Cmd06( Battery *pBat , uint8_t * pout)
{
	uint16_t len = 0 ;
//	uint16_t temp_u16 = 0;
	if( !pout || !pBat) return 0 ;
	// SOH
	pout[len++] = pBat->bmsInfo.soh & 0xFF ;
	pout[len++] = pBat->bmsInfo.soh >> 8 ;
	
	//������
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = pBat->bmsInfo.cycle & 0xFF ;
	pout[len++] = pBat->bmsInfo.cycle >> 8 ;
	
	//����ѹ����
	//���ѹ����
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	//�Ź�ѹ����
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	//��������� 
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	//�Ź������� 
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	//��·����
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	//����´���
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	//�ŵ��´���
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	//����´���
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	//�Ÿ��´���
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	return len ;
}


//���� 7�������ѹ
static uint16_t _Batter_Cmd07( Battery *pBat , uint8_t * pout)
{
	uint16_t len = 0 , i = 0 ;
//	uint8_t bat_num = 0 ;
	if( !pout || !pBat) return 0 ;
	
	//bat_num = pBat->bmsID.bvolt/3600 ;	// ʵ�ʵ�ش���

	pout[len++] = 0x10 ;		// �����ؽ���
	// 1 ��ص�ѹ
	for( i = 0 ; i < 16 ; i++ )
	{
		pout[len++] = pBat->bmsInfo.bvolt[i] & 0xFF;
		pout[len++] = pBat->bmsInfo.bvolt[i] >> 8;
	}
	return len ;
}




static const BatteryCmd gBatteryCmdList[] = 
{
	{0x05,_Batter_Cmd05},	// ��ȡ��Ϣ
	{0x06,_Batter_Cmd06},	// ��ȡ��ش���
	{0x07,_Batter_Cmd07},	// ��ȡ�����ѹ
};

static uint16_t _getCenterCtlOperation( Battery *pBat , uint8_t cmd , uint8_t *pOutBuff )
{
	uint8_t i = 0 ;
	for( i = 0 ; i < sizeof(gBatteryCmdList)/sizeof(gBatteryCmdList[0]) ; i++ )
	{
		if( cmd == gBatteryCmdList[i].miCmd )
		{
			PFL(DL_485,"Rs485 ExCmd:0x%02X\n", cmd );
			if( gBatteryCmdList[i].mpCmdCB )
			{
				return gBatteryCmdList[i].mpCmdCB( pBat, pOutBuff );
			}
			break ;
		}
	}
	return 0 ;
}




// 486 ָ���б�
static uint8_t _last_addr = 0 ;
static void _Rs485_Cmd10( uint8_t *pinbuff , uint16_t len )
{
	//�ȼ���ַ�ǲ��Ǹ���ص�
	uint16_t tx_len = 0 ;
	Battery *pCurBat = NULL ;
	uint16_t sum = 0 ;
	uint32_t tempu32 = 0;
	
	pCenterCtrlData pCenterCtrl = (pCenterCtrlData) ( pinbuff + 8 );
	pBatteryRespond pBatterRes = (pBatteryRespond)(g_Rs485TxBuf+8);

	pCurBat = pinbuff[1] == 0x32 ? &g_Bat[0] : &g_Bat[1] ;
	
	// 31 ��Ӧ���1
	// 32 ��Ӧ���0
	switch( pinbuff[1] )
	{
		case 0x31 :
		case 0x32 :
			g_Rs485TxBuf[tx_len++] = 0;		// ����
			g_Rs485TxBuf[tx_len++] = pinbuff[1];	// ��ذ汾
			g_Rs485TxBuf[tx_len++] = 'B';
			g_Rs485TxBuf[tx_len++] = 0x10;
			g_Rs485TxBuf[tx_len++] = 0x00;	// У��
			g_Rs485TxBuf[tx_len++] = 0x00;	// У��
			g_Rs485TxBuf[tx_len++] = 0x16;	// ��������
			g_Rs485TxBuf[tx_len++] = 0x00;	// �����״̬

			//if(  _BatteryIsValid( pinbuff[1] == 0x31 ? 0x01 : 0x00 ) )
			if( pCurBat->presentStatus == BAT_IN )
			{
				g_Rs485TxBuf[tx_len++] = 0x00 ;	// Ԥ�ƿ���ʻ���� 
				g_Rs485TxBuf[tx_len++] = bigendian16_get((uint8_t*)&pCurBat->bmsInfo.soc)/10 ;	// ���� 
				g_Rs485TxBuf[tx_len++] = 0x00 ;	// Ԥ�Ƴ�����ʱ��
				
				PFL(DL_485,"Bat State:%X,%d,%d\n",
					pCurBat->bmsInfo.state,pCurBat->bmsInfo.tcurr,pCurBat->bmsInfo.tvolt);
				
				g_Rs485TxBuf[tx_len++] = pCurBat->bmsInfo.state&0x0100 ? 0x01 : 0x00;	// 1����У�0:δ���

				tempu32 = bigendian16_get((uint8_t*)&pCurBat->bmsInfo.tcurr);
				tempu32 -= 30000 ;
				tempu32 *= 10;
				g_Rs485TxBuf[tx_len++] = tempu32 & 0xFF ;	// ���� ma
				g_Rs485TxBuf[tx_len++] = tempu32 >> 8;	// ����
				g_Rs485TxBuf[tx_len++] = tempu32 >> 16 ;	// ����
				g_Rs485TxBuf[tx_len++] = tempu32 >> 24 ;	// ����
				//
				tempu32 = bigendian16_get((uint8_t*)&pCurBat->bmsInfo.tvolt);
				tempu32 *= 10 ;
				g_Rs485TxBuf[tx_len++] = tempu32 & 0xFF ;	// ��ѹ mv
				g_Rs485TxBuf[tx_len++] = tempu32 >> 8 ;	// ��ѹ
				g_Rs485TxBuf[tx_len++] = tempu32 >> 16 ;		// ��ѹ
				g_Rs485TxBuf[tx_len++] = tempu32 >> 24 ;	// ��ѹ
				//
				g_Rs485TxBuf[tx_len++] = bigendian16_get((uint8_t*)&pCurBat->bmsInfo.dmost)/10 - 40 ;	// �¶�

				// ����״̬
				g_Rs485TxBuf[tx_len] = 0x00;
				if( bigendian16_get((uint8_t*)&pCurBat->bmsInfo.opft1) &(~((1<<15)|(1<<14)|(1<<0)|(1<<3)|(1<<6)|(1<<8)|(1<<10))))
				{
					g_Rs485TxBuf[tx_len] = 0x01 ;
				}
				if( bigendian16_get((uint8_t*)&pCurBat->bmsInfo.opft1) &(~(1<<2)))
				{
					g_Rs485TxBuf[tx_len] = 0x01 ;
				}
				if( bigendian16_get((uint8_t*)&pCurBat->bmsInfo.devft1)&(~((1<<7)|(1<<8))))
				{
					g_Rs485TxBuf[tx_len] = 0x01 ;
				}
				if(bigendian16_get((uint8_t*)&pCurBat->bmsInfo.devft2))
				{
					g_Rs485TxBuf[tx_len] = 0x01 ;
				}
				g_Rs485TxBuf[tx_len] <<= 2 ;
				tx_len++;
				g_Rs485TxBuf[tx_len++] = 0x00;
				// ƽ���еĵ���
				g_Rs485TxBuf[tx_len++] = 0x00;
				g_Rs485TxBuf[tx_len++] = 0x00;
				g_Rs485TxBuf[tx_len++] = 0x00;//pCurBat->bmsInfo.balasta >> 8;
				g_Rs485TxBuf[tx_len++] = 0x00;//pCurBat->bmsInfo.balasta & 0xFF;
				/*
				��״̬
				//Bit0��1���Ѱ� 0��δ��
				//Bit1��1���ŵ翪 0:�ŵ��
				//Bit2��1����翪 0:����
				//Bit3��1��С������ 0:С������
				//Bit4: 1:���� 0��δ��
				//Bit5: 1:���� ,0:�����
				*/
				g_Rs485TxBuf[tx_len] = 0x01;
				
				tempu32 = bigendian16_get((uint8_t*)&pCurBat->bmsInfo.state);
				if(   tempu32& 0x04 )	// Ԥ��
				{
					g_Rs485TxBuf[tx_len] |= 0x08;
				}
				if( tempu32 & 0x02 )	// �ŵ� -- �������һ����طŵ� > 15A��������һ��Ϊ�Ӵ�����
				{
					//g_Rs485TxBuf[tx_len] |= 0x02;
					g_Rs485TxBuf[tx_len] |= 
						_BatteryIsValid( pinbuff[1] == 0x31 ? 0x01 : 0x00 ) ? 0x20 : 0x00 ;
				}
				// ����������Ϊ���,<0���Ϸŵ�
				tempu32 = bigendian16_get((uint8_t*)&pCurBat->bmsInfo.tcurr);

				if( tempu32 > 30000 ) // > 30000 ���
				{
					g_Rs485TxBuf[tx_len] |= 0x04;
				}
				else				// < �ŵ�
				{
					g_Rs485TxBuf[tx_len] |= 0x02;
				}
				
				//if( tempu32 & 0x01 )	// ���
				//{
				//	g_Rs485TxBuf[tx_len] |= 0x04;
				//}
				tx_len++;
				// ������� 
				g_Rs485TxBuf[tx_len++] = ( pCenterCtrl->miOperation << 4 ) | 0x01 ;
				// ��������
				tx_len += _getCenterCtlOperation( pCurBat , pCenterCtrl->miOperation , g_Rs485TxBuf + tx_len );

				// sum
			}
			else
			{
				//��ز����� or �Ӵ����� ---����������
			}
			g_Rs485TxBuf[0] = tx_len ;
			_get_CheckSum(&sum, g_Rs485TxBuf, tx_len);
			g_Rs485TxBuf[4] = sum & 0xFF ;
			g_Rs485TxBuf[5] = sum >> 8 ;

			// ������
			_last_addr |= ( pinbuff[1] - 0x30 ) ;
			if( _last_addr == 3 )
			g_pdoInfo.isRs485Ok = 1;
			
			break ;
		default :
			tx_len = 0x00 ;
			break ;
	}
	//
	if( tx_len )
	{
		Rs485_Tx( g_Rs485TxBuf, tx_len );
		tx_len = 0 ;
	}
	return ;
}



static const Rs485Cmd gRs485CmdList[] = 
{
	{0x10,_Rs485_Cmd10},
};




static void RS485_Stop(void)
{
	//
}

/*
uint8_t buff[] = { 0xFE,0xFE,0xFE,0xFE,0x0D,0x40,0x01,0x01,0x01,0x01,0x01,0x00,0x52,
0xFE,0xFE,0xFE,0xFE,0x1A,0x40,0x03,0xFF,0xFF,0x00,0xFF,0x00,0xFF,0x02,0x47,0x42,0x03,0x48,0x4C,0x32,0x14,0x11,0x06,0x09,0x05,0xE1
,0x17,0x31,0x42,0x10,0x0B,0x03,0x16,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x05,0x00,0xC0,0x5B,0xC0,0x5B,0x02,0x02,0xFE
,0x17,0x32,0x42,0x10,0xC5,0x88,0x16,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x05,0x00,0x94,0xD5,0xC0,0x5B,0x02,0x02,0x6F};
*/
static void RS485_Run(void)
{
	//���,��������
	uint16_t rlen = 0 ;
	uint8_t cmd = 0, i = 0 ;
	uint16_t sum = 0 , data_sum = 0 ;
	//PFL(DL_485,"RS485:%d-%d\n",gRs485rxBuf.miHead,gRs485rxBuf.miTail);
	
	while( rlen = _Rs485Dcode( g_Rs485RxBuf, sizeof(g_Rs485RxBuf) ) )
	{
		PFL(DL_485, "485 RX(%d):",rlen);
		DUMP_BYTE_LEVEL(DL_485, g_Rs485RxBuf , rlen );
		PFL(DL_485, "\n");
			
		//if( g_Rs485RxBuf[0] == 0xFE || g_Rs485RxBuf[0] == 0xF5 )
		//{
			//data_sum = ( g_Rs485RxBuf[rlen-1] << 8 ) | g_Rs485RxBuf[rlen-2] ;
			//g_Rs485RxBuf[rlen-1] = 0x00 ;
			//g_Rs485RxBuf[rlen-2] = 0x00 ;
			//_get_CheckSum(&sum, g_Rs485RxBuf, rlen);
		//	cmd = g_Rs485RxBuf[6];
		//	continue ;
		//}
		//else
		{
			data_sum = ( g_Rs485RxBuf[4] << 8 ) | g_Rs485RxBuf[5] ;
			g_Rs485RxBuf[4] = 0x00 ;
			g_Rs485RxBuf[5] = 0x00 ;
			_get_CheckSum(&sum, g_Rs485RxBuf, rlen);
			cmd = g_Rs485RxBuf[3];
		}
		sum = Dt_convertToU16(&sum,DT_UINT16);
		
		if( data_sum != sum )
		{
			PFL_WARNING("485 sum error:[%04X:%04X]\n", data_sum , sum );
			continue ;
		}
		PFL(DL_485,"485 Rev Cmd:0x%02X,Len:%d\n",cmd , rlen );
		
		for( i = 0 ; i < sizeof(gRs485CmdList)/sizeof(gRs485CmdList[0]) ;i++ )
		{
			if( cmd == gRs485CmdList[i].micmd )
			{
				if( gRs485CmdList[i].mpCmdCB ) gRs485CmdList[i].mpCmdCB(g_Rs485RxBuf,rlen);
				break ;
			}
		}
	}
}



static void RS485_Start(void)
{
	CirBuffInit( &gRs485rxBuf , g_Rs485modBusRxBuf , sizeof(g_Rs485modBusRxBuf));
	//CirBuffInit( &gTx485txBuf , g_Rs485modBusTxBuf , sizeof(g_Rs485modBusTxBuf));
}


void RS485_Init(void)
{
	CirBuffInit( &gRs485rxBuf , g_Rs485modBusRxBuf , sizeof(g_Rs485modBusRxBuf));
	//CirBuffInit( &gTx485txBuf , g_Rs485modBusTxBuf , sizeof(g_Rs485modBusTxBuf));
	const static Obj obj = {
		.name = "RS485",
		.Start = RS485_Start,
		.Run = RS485_Run,
		.Stop = RS485_Stop,
	};

	ObjList_add(&obj);
}




#else

Mod g_Rs485ModBus;
uint32_t Rs485RunTicks;
uint8_t Rs485TestSendFlag =0;
uint8_t Rs485TestRxBuff[10];
uint8_t Rs485TestTxBuff[5]={0x01,0x02,0x03,0x04,0x05};

//���е�BMS����������¼��ص�����
MOD_EVENT_RC Rs485Test_event(Battery* pBat, const ModCmd* pCmd, MOD_TXF_EVENT ev)
{
	if (ev == MOD_REQ_SUCCESS)
	{
		if((pCmd->cmd == 1)&&( pCmd->pExt->transferLen == sizeof(Rs485TestTxBuff)))
		{
			if (memcmp(pCmd->pStorage, pCmd->pExt->transferData, pCmd->pExt->transferLen) == 0)
			{
				g_pdoInfo.isRs485Ok = 1;
				Rs485TestSendFlag =1;
			}
		}
	}
	else if (ev == MOD_REQ_FAILED)
	{
		g_pdoInfo.isRs485Ok = 0;
		Rs485TestSendFlag =0;
	}

	return MOD_EVENT_RC_SUCCESS;
}


int Rs485_Tx(const uint8_t* pData, int len)
{
	int i;
	DrvIo* g_Rs485DirCtrlIO = Null;
	
	g_Rs485DirCtrlIO = IO_Get(IO_DIR485_CTRL);
	PortPin_Set(g_Rs485DirCtrlIO->periph, g_Rs485DirCtrlIO->pin, True);
	for(i=0;i<len;i++)
	{
		uart4_put_byte(*pData++);
	}
	PortPin_Set(g_Rs485DirCtrlIO->periph, g_Rs485DirCtrlIO->pin, False);
	
	return 0;
}

int Rs485RxData(void)
{
	char data;
	
	data = rs485_getchar();
	if (data == 0xff)
	{

	}
	else
	{
		Rs485Mod_RxData(&g_Rs485ModBus,&data,1);
	}
}

void RS485_Run(void)
{
	const uint16_t Rs485Sendms = 1000;
	Rs485RxData();
	Rs485Mod_Run(&g_Rs485ModBus);
	if((SwTimer_isTimerOutEx(Rs485RunTicks,Rs485Sendms))&&
		(Rs485TestSendFlag == 0))
	{
		Rs485RunTicks = GET_TICKS();
		Mod_SendCmd(g_Rs485ModBus.cfg, 1);
	}
}

void RS485_Start(void)
{
	Rs485RunTicks = GET_TICKS();
	Rs485TestSendFlag =0;
}

void RS485_Init(void)
{
	#define RS485_CMD_COUNT 2
	static ModCmdEx g_BmsCmdEx[RS485_CMD_COUNT];
	//ModCmdEx g_BmsCmdEx[BMS_CMD_COUNT];
	/*��ز�λ0���������ñ�*******************************************************************/
	const static ModCmd g_Rs4850Cmds[RS485_CMD_COUNT] =
	{
		{&g_BmsCmdEx[0], 1 ,MOD_READ, MOD_READ_HOLDING_REG, "Rs485Test" , Rs485TestRxBuff , sizeof(Rs485TestRxBuff) ,
        	Rs485TestTxBuff, sizeof(Rs485TestTxBuff), Null,(ModEventFn)Rs485Test_event},
		
	};

	const static ModCfg Rs485Cfg =
	{
		.port = 0,
		.cmdCount = RS485_CMD_COUNT,
		.cmdArray = g_Rs4850Cmds,
//		.pCbObj = &g_Bat[0],
//		.TresferEvent = (ModEventFn)Bat_event,
		.TxFn = Rs485_Tx,
	};
	/*Modbus����Э��֡����*****************************************************/
	static uint8_t g_Rs485modBusTxBuf[128];
	static uint8_t g_Rs485modBusRxBuf[128];
	const static ModFrameCfg g_Rs485frameCfg =
	{
		.txBuf = g_Rs485modBusTxBuf,
		.txBufLen = sizeof(g_Rs485modBusTxBuf),
		.rxBuf = g_Rs485modBusRxBuf,
		.rxBufLen = sizeof(g_Rs485modBusRxBuf),

		.waitRspMsDefault = 1000,
		.rxIntervalMs = 1,
		.sendCmdIntervalMs = 100,
	};
	
	const static Obj obj = {
	.name = "RS485",
	.Start = RS485_Start,
	.Run = RS485_Run,
	};

	ObjList_add(&obj);
	
	Rs485Mod_Init(&g_Rs485ModBus, &Rs485Cfg, &g_Rs485frameCfg);
}

#endif // 



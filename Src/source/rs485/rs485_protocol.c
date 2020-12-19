

#include "common.h"
#include "drv_usart.h"
#include "Rs485Modbus.h"
#include "rs485_protocol.h"

#if 1
//static uint8_t g_Rs485modBusTxBuf[128];
static uint8_t g_Rs485modBusRxBuf[256];

static uint8_t g_Rs485RxBuf[96];
static uint8_t g_Rs485TxBuf[128];

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
仪表数据帧格式：
[帧头:F5 F5 F5 F5] + [帧长度:1Byte含帧头<64] + [设备地址0x41] + [命令字1Byte] + [数据] + [校验字累加和2Byte]

控制器帧格式：
[帧头:FE FE FE FE] + [帧长度:1Byte含帧头<64] + [设备地址0x40] + [命令字1Byte] + [数据] + [校验字累加和2Byte]

电池数据帧格式：
[帧长度:1Byte含帧头<64] + [设备地址0x30/0x31] + [协议类型'B'] + [命令字1Byte] + [校验字2Byte] + [命令处理调用 0x16] + [命令返回状态 0] + [数据]

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
				poutbuff[2] == 0xF5 && poutbuff[3] == 0xF5 )		// 仪表数据
			{
				if( !( poutbuff[4] < 255 && poutbuff[5] == 0x41 ) )	// 长度与地址不对,则认为有误
				{
					_CIR_LOOP_ADD( gRs485rxBuf.miHead, 1, gRs485rxBuf.miSize );
					goto _RS485_DCODE ;
				}
			}
			else if( poutbuff[0] == 0xFE && poutbuff[1] == 0xFE && 
				poutbuff[2] == 0xFE && poutbuff[3] == 0xFE )		// 控制数据
			{
				if( !( poutbuff[4] < 255 && poutbuff[5] == 0x40 ) )	// 长度与地址不对,则认为有误
				{
					_CIR_LOOP_ADD( gRs485rxBuf.miSize, 1, gRs485rxBuf.miSize );
					goto _RS485_DCODE ;
				}
			}
			else if( poutbuff[0] == 0xA5 && poutbuff[1] == 0xA5 && 
				poutbuff[2] == 0xA5 && poutbuff[3] == 0xA5 )		// 控制数据
			{
				if( !( poutbuff[4] < 255 && poutbuff[5] == 0x40 ) )	// 长度与地址不对,则认为有误
				{
					_CIR_LOOP_ADD( gRs485rxBuf.miSize, 1, gRs485rxBuf.miSize );
					goto _RS485_DCODE ;
				}
			}
			else													// 电池数据
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
			if( poutbuff[0] == 0xF5 && len == poutbuff[4] ) // 仪表数据
			{
				gRs485rxBuf.miHead = pos ;
				return len ;
			}
			if( poutbuff[0] == 0xFE && len == poutbuff[4] ) // 控制数据
			{
				gRs485rxBuf.miHead = pos ;
				return len ;
			}
			if( poutbuff[0] == len )						// 电池数据
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
上报数据的条件
1、电池存在
2、双电都在时且均放电时，如果其中一个电池放电 < 15A 则认为其接触不良
--如果其中一个电流小,则认为接触不良--


3、


*/


/*

1、当两电池均放电时，如果一个电池 > 15A，则认为另外一个电池接触不良
2、

*/
/*
static Bool _BatteryIsValid( uint8_t bat )
{
	int16_t curr_0 = 0 , curr_1 = 0;
	// 如果电池不存在,则不需要上报
	if( g_Bat[bat].presentStatus != BAT_IN ) return False ;
	//
	// 如果电池,且两个电池均在放电 
	if( g_Bat[0].presentStatus == BAT_IN && g_Bat[1].presentStatus == BAT_IN )
	{
		// 如果电池均放电,但有一上电池放电电流 > 15A
		if( g_Bat[0].bmsInfo.state&0x0200 && g_Bat[1].bmsInfo.state&0x0200 )
		{
			curr_0 = SWAP16(g_Bat[0].bmsInfo.tcurr) - 30000;
			curr_1 = SWAP16(g_Bat[1].bmsInfo.tcurr) - 30000;
			//
			if( curr_0 < -1500 )		// 如果 bat 0 放电大于15A，则说明bat 1 接触不良
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
*/





//命令 5：读取信息 读信息命令：38 字节 18+20 SN+VER
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

//命令 6：读相关次数
static uint16_t _Batter_Cmd06( Battery *pBat , uint8_t * pout)
{
	uint16_t len = 0 ;
//	uint16_t temp_u16 = 0;
	if( !pout || !pBat) return 0 ;
	// SOH
	pout[len++] = pBat->bmsInfo.soh & 0xFF ;
	pout[len++] = pBat->bmsInfo.soh >> 8 ;
	
	//充电次数
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = pBat->bmsInfo.cycle & 0xFF ;
	pout[len++] = pBat->bmsInfo.cycle >> 8 ;
	
	//充电过压次数
	//充过压次数
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	//放过压次数
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	//充过流次数 
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	//放过流次数 
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	//短路次数
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	//充低温次数
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	//放低温次数
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	//充高温次数
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	//放高温次数
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	pout[len++] = 0x00;
	return len ;
}


//命令 7：单体电压
static uint16_t _Batter_Cmd07( Battery *pBat , uint8_t * pout)
{
	uint16_t len = 0 , i = 0 ;
//	uint8_t bat_num = 0 ;
	if( !pout || !pBat) return 0 ;
	
	//bat_num = pBat->bmsID.bvolt/3600 ;	// 实际电池串数

	pout[len++] = 0x10 ;		// 单体电池节数
	// 1 电池电压
	for( i = 0 ; i < 16 ; i++ )
	{
		pout[len++] = pBat->bmsInfo.bvolt[i] & 0xFF;
		pout[len++] = pBat->bmsInfo.bvolt[i] >> 8;
	}
	return len ;
}




static const BatteryCmd gBatteryCmdList[] = 
{
	{0x05,_Batter_Cmd05},	// 读取信息
	{0x06,_Batter_Cmd06},	// 读取相关次数
	{0x07,_Batter_Cmd07},	// 读取单体电压
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




// 486 指令列表
static uint8_t _last_addr = 0 ;

extern uint8_t g_Bat0_State ;
extern uint8_t g_Bat1_State ;


static void _Rs485_Cmd10( uint8_t *pinbuff , uint16_t len )
{
	//先检测地址是不是给电池的
	uint16_t tx_len = 0 ;
	Battery *pCurBat = NULL ;
	uint16_t sum = 0 ;
	uint32_t tempu32 = 0;
	
	pCenterCtrlData pCenterCtrl = (pCenterCtrlData) ( pinbuff + 8 );
	pBatteryRespond pBatterRes = (pBatteryRespond)(g_Rs485TxBuf+8);

	pCurBat = pinbuff[1] == 0x32 ? &g_Bat[0] : &g_Bat[1] ;		// 0x32-Bat0
	
	// 31 对应电池1
	// 32 对应电池0
	// 如果电池没接 or 电池没放电 or 电池 接触不良，则不要上报此电池
	switch( pinbuff[1] )
	{
		case 0x31 :
		case 0x32 :
			g_Rs485TxBuf[tx_len++] = 0;		// 长度
			g_Rs485TxBuf[tx_len++] = pinbuff[1];	// 电池版本
			g_Rs485TxBuf[tx_len++] = 'B';
			g_Rs485TxBuf[tx_len++] = 0x10;
			g_Rs485TxBuf[tx_len++] = 0x00;	// 校验
			g_Rs485TxBuf[tx_len++] = 0x00;	// 校验
			g_Rs485TxBuf[tx_len++] = 0x16;	// 命令处理调用
			g_Rs485TxBuf[tx_len++] = 0x00;	// 命令返回状态
			
			if( pCurBat->presentStatus == BAT_IN )	// 如果存在
			{
				// 1、如果些电池没有放电 
				if( !( pCurBat->bmsInfo.state & 0x0200 ) )
				{
					break ;
				}
				if( pinbuff[1] == 0x31 )		// Bat1
				{
					if( !g_Bat1_State ) break ;
				}
				else if( pinbuff[1] == 0x32 )	// Bat0
				{
					if( !g_Bat0_State ) break ;
				}
				
				g_Rs485TxBuf[tx_len++] = 0x00 ;	// 预计可行驶距离 
				g_Rs485TxBuf[tx_len++] = bigendian16_get((uint8_t*)&pCurBat->bmsInfo.soc)/10 ;	// 电量 
				g_Rs485TxBuf[tx_len++] = 0x00 ;	// 预计充电完成时间
				
				PFL(DL_485,"Bat State:%X,%d,%d\n",
					pCurBat->bmsInfo.state,pCurBat->bmsInfo.tcurr,pCurBat->bmsInfo.tvolt);
				
				g_Rs485TxBuf[tx_len++] = pCurBat->bmsInfo.state&0x0100 ? 0x01 : 0x00;	// 1充电中，0:未充电

				tempu32 = bigendian16_get((uint8_t*)&pCurBat->bmsInfo.tcurr);
				tempu32 -= 30000 ;
				tempu32 *= 10;
				g_Rs485TxBuf[tx_len++] = tempu32 & 0xFF ;	// 电流 ma
				g_Rs485TxBuf[tx_len++] = tempu32 >> 8;	// 电流
				g_Rs485TxBuf[tx_len++] = tempu32 >> 16 ;	// 电流
				g_Rs485TxBuf[tx_len++] = tempu32 >> 24 ;	// 电流
				//
				tempu32 = bigendian16_get((uint8_t*)&pCurBat->bmsInfo.tvolt);
				tempu32 *= 10 ;
				g_Rs485TxBuf[tx_len++] = tempu32 & 0xFF ;	// 电压 mv
				g_Rs485TxBuf[tx_len++] = tempu32 >> 8 ;	// 电压
				g_Rs485TxBuf[tx_len++] = tempu32 >> 16 ;		// 电压
				g_Rs485TxBuf[tx_len++] = tempu32 >> 24 ;	// 电压
				//
				g_Rs485TxBuf[tx_len++] = bigendian16_get((uint8_t*)&pCurBat->bmsInfo.dmost)/10 - 40 ;	// 温度

				// 运行状态
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
				// 平衡中的单体
				g_Rs485TxBuf[tx_len++] = 0x00;
				g_Rs485TxBuf[tx_len++] = 0x00;
				g_Rs485TxBuf[tx_len++] = 0x00;//pCurBat->bmsInfo.balasta >> 8;
				g_Rs485TxBuf[tx_len++] = 0x00;//pCurBat->bmsInfo.balasta & 0xFF;
				/*
				绑定状态
				//Bit0：1：已绑定 0：未绑定
				//Bit1：1：放电开 0:放电关
				//Bit2：1：充电开 0:充电关
				//Bit3：1：小电流开 0:小电流关
				//Bit4: 1:充满 0：未满
				//Bit5: 1:并联 ,0:单电池
				*/
				g_Rs485TxBuf[tx_len] = 0x01;
				
				tempu32 = bigendian16_get((uint8_t*)&pCurBat->bmsInfo.state);
				if(   tempu32& 0x04 )	// 预放
				{
					g_Rs485TxBuf[tx_len] |= 0x08;
				}
				if( tempu32 & 0x02 )	// 放电 -- 如果其中一个电池放电 > 15A，则另外一个为接触不良
				{
					g_Rs485TxBuf[tx_len] |= 0x02;
					//g_Rs485TxBuf[tx_len] |= 
					//	_BatteryIsValid( pinbuff[1] == 0x31 ? 0x01 : 0x00 ) ? 0x20 : 0x00 ;
				}
				// 电流大则认为充电,<0则认放电
				tempu32 = bigendian16_get((uint8_t*)&pCurBat->bmsInfo.tcurr);

				if( tempu32 > 30000 ) // > 30000 充电
				{
					g_Rs485TxBuf[tx_len] |= 0x04;
				}
				else				// < 放电
				{
					g_Rs485TxBuf[tx_len] |= 0x02;
				}
				
				//if( tempu32 & 0x01 )	// 充电
				//{
				//	g_Rs485TxBuf[tx_len] |= 0x04;
				//}
				tx_len++;
				// 操作结果 
				g_Rs485TxBuf[tx_len++] = ( pCenterCtrl->miOperation << 4 ) | 0x01 ;
				// 附加数据
				tx_len += _getCenterCtlOperation( pCurBat , pCenterCtrl->miOperation , g_Rs485TxBuf + tx_len );
			}
			else
			{
				//电池不存在 or 接触不良 ---不处理数据
			}
			break ;
		default :
			tx_len = 0x00 ;
			break ;
	}
	
		
	// 测试用
	_last_addr |= ( pinbuff[1] - 0x30 ) ;
	if( _last_addr == 3 )
	g_pdoInfo.isRs485Ok = 1;
			
	if( tx_len )
	{
		// sum
		g_Rs485TxBuf[0] = tx_len ;
		_get_CheckSum(&sum, g_Rs485TxBuf, tx_len);
		g_Rs485TxBuf[4] = sum & 0xFF ;
		g_Rs485TxBuf[5] = sum >> 8 ;
	
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
	//解包,发送数据
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

//所有的BMS单个命令的事件回调函数
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
	/*电池槽位0的命令配置表*******************************************************************/
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
	/*Modbus传输协议帧配置*****************************************************/
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



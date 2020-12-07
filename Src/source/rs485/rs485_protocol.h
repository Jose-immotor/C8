
#ifndef __RS485_PROTOCOL_H__
#define __RS485_PROTOCOL_H__


void RS485_Init(void);


// 指令处理
typedef void (*_Rs485_RxCmd)(uint8_t *,uint16_t);

typedef struct
{
	uint8_t 		micmd;
	_Rs485_RxCmd	mpCmdCB;
}Rs485Cmd;


// BAT数据
typedef uint16_t (*_Rs485_BatteryCmd)( Battery* ,uint8_t*);

typedef struct
{
	uint8_t				miCmd;
	_Rs485_BatteryCmd	mpCmdCB ;
}BatteryCmd;





#pragma pack(1)

// 0x10 指令 中控下发数据
typedef struct 
{
	uint8_t 	miSpeed ;			// 实际转速
	uint32_t	miCurOnceDistance;	// 本次电门打开行驶的距离 
	uint8_t		miSwitch;			// 电门开关信号
	uint8_t		miOperation;		// 操作
	uint8_t 	miDouElect;			// 双电
	uint16_t 	miCheckCode;		// 对码
	uint16_t	miUpCheckCode;		// 更新对码
	// 可选区域1
	uint8_t		mpExtData[1];
}CenterCtrlData , *pCenterCtrlData;

// 可选区域1 :



// 0x10 指令，电流回应数据
typedef struct
{
	uint8_t		miEstiDriDistance;	// 预计可行驶距离 
	uint8_t		miBatSoc;			// 电池电量 
	uint8_t 	miEstChgTime;		// 预计充电完成时间
	int32_t 	miDischgCurrent;	// 放电电流 ma
	uint32_t 	miVoltage ;			// 电压 mv
	uint8_t		miBatTemp;			// 温度
	uint16_t 	miBMSState;			// bms 状态
	uint32_t	miMomEquum;			// 平衡中的单体
	/*
	Bit0 : 1已经绑定,0未绑定
	Bit1 : 1放电开,0放电关
	Bit2 : 1充电开,0充电关
	Bit3 : 1小电流开,0小电流关
	Bit4 : 1充满,0未满
	Bit5 : 1关联,0单电
	*/
	uint8_t		miBindState;		// 绑定状态 
	/*
		Bit7~Bit4 : 对应操作命令
		Bit3~Bit0 : 对应操作结果 1:成功,0:失败
	*/
	uint8_t 	miOperation;		// 操作结果 
	uint8_t		mpExtData[1];		// 可选
}BatteryRespond ,*pBatteryRespond ;





#pragma pack()







#endif

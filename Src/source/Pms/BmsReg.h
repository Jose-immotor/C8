//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: MBFindMap.h
//创建人  	: Handry
//创建日期	: 
//描述	    : Modbus从机地址映射处理头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	: Allen
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

#ifndef _MB_FINDMAP_H
#define _MB_FINDMAP_H

#include "typedef.h"
//=============================================================================================
//宏定义
#define BMS_ID_SIZE     6

//=============================================================================================

typedef struct _BmsRegCtrl
{
	uint16_t reserved0 : 8;  //BIT[8-15]:保留
	uint16_t chgEn : 1;  //BIT[0]:充电使能
	uint16_t dischgEn : 1;  //BIT[1]:放电使能
	uint16_t preDischg : 1;  //BIT[2]:预防电使能
	uint16_t reserved1 : 2;  //BIT[3-4]:保留
	uint16_t sleepEn : 1;  //BIT[5]:浅随眠使能
	uint16_t deepSleepEn : 1; //BIT[6]:深睡眠使能
	uint16_t chgrIn : 1;  //BIT[7]:充电器插入
	
}BmsRegCtrl;

// 1字节对齐
#pragma pack(1)

//BMS设备信息寄存器
typedef struct
{
	/*
	Addr 0 ~ 49
	*/
	uint16_t prver;		// 协议版本号
	uint16_t hwver;		// 硬件版本号
	uint16_t blver;		// Boot 版本号
	uint16_t fwmsv;		// 固件版本---主次版本号
	uint16_t fwrev;		// 固件版本---修正版本号
	uint16_t fwbnh;		// 固件版本---编译版本高字
	uint16_t fwbnl;		// 固件版本---编译版本低字
	uint16_t mcun12;	// MCU ID 
	uint16_t mcun34;
	uint16_t mcun56;
	uint16_t mcun78;
	uint16_t btype;		// 电池类型
	uint16_t bvolt;		// 额定电压
	uint16_t bcap;		// 额定电流
	uint16_t sn12;		// 序列号
	uint16_t sn34;
	uint16_t sn56;
	uint16_t sn78;
	uint16_t erech;		// 最早记录号 高字
	uint16_t erecl;		// 最早记录号 低字
	uint16_t lrech;		// 最近记录号 高字
	uint16_t lrecl;		// 最近记录号 低字
	uint16_t ltsta;		// 历史状态
	uint16_t ltmaxv;	// 历史最高单体电压
	uint16_t ltmaxvn;	// 历史最高单体电压编号 
	uint16_t ltminv;	// 历史最低单体电压
	uint16_t ltminvn;	// 历史最低单体电压编号
	uint16_t ltmaxt;	// 历史最高温度
	uint16_t ltmaxtn;	// 历史最低温度编号
	uint16_t ltmint;	// 历史最低温度
	uint16_t ltmintn;	// 历史最低温度编号
	uint16_t ltmaxcc;	// 历史最大充电电流
	uint16_t ltmaxdc;	// 历史最大放电电流
	uint16_t df_ver;	// DF版本
	// 34 - 49 预留
}BmsReg_info;

//BMS只读寄存器
typedef struct
{
	/*
	Addr:256 ~ 363
	*/
	uint16_t userId[4];		//BID 256,257,258,259
	/*
		Bit0 : CHG_MOS : 1 :充电MOS打开,0:充电MOS关闭
		Bit1 ：DIS_MOS ：1：放电MOS打开,0:放电MOS关闭
		Bit2 : PRE_CHG_MOS : 1 预放电MOS打开,0：关闭
		Bit3 : G2_SAT : 1:G2有效，0:G2无效
		Bit4 : BALANCE_STA : 1:电池组正在均衡()
		Bit5 : DEV_FAULT: 1:设备故障
		Bit6 : OP_FAULT 1:运行故障
		Bit7 : OP_WARNING 1:运行告警
		Bit8 : CERTIFI : BMS验证HOST 1：需要验证，0：验证通过
		Bit9 : DISASSEN : 1:光敏有效,0:光敏无效
		Bit10 : EXTERN MODULE :1:外置模块在位，0：外置模块不在位
		Bit11 ~ Bit13 : 保留
		BIt14 : CHG_FULL 1:充电满
		Bit15 :保留
	*/
	uint16_t state;			//STATE BMS 状态 260
	uint16_t soc;			//SOC	0.1 电池级荷电状态 261
	uint16_t tvolt;			//总电压	262
	uint16_t tcurr;			//总电流,放电为负,充电为正,0.01A单位 + 30000偏移 263
	uint16_t htemp;			//最高温度 264
	uint16_t htnum;			//最高温度编号 265
	uint16_t ltemp;			//最低温度 266 
	uint16_t ltnum;			//最低温度编号 267
	uint16_t hvolt;			//最高单体电压 268
	uint16_t hvnum;			//最高单体电压编号 269
	uint16_t lvolt;			//电低单体电压 270
	uint16_t lvnum;			//电低单体电压编号 271
	uint16_t dsop;			//10s内最大放电电流   272
	uint16_t csop;			//10s内最大充电电流 273
	uint16_t soh;			//SOH 274
	uint16_t cycle;			//循环次数 275
	uint16_t rcap;			//剩余容量 276
	uint16_t fcap;			//充满容量 277
	uint16_t fctime;		//充满时间 278
	uint16_t prow;			//剩余能量 279
	uint16_t devft1;		//设备故障1 280
	uint16_t devft2;		//设备故障2 281
	uint16_t opft1;			//运行故障1 282
	uint16_t opft2;			//运行故障2 283
	uint16_t opwarn1;		//运行告警1 284
	uint16_t opwarn2;		//运行告警2 285
	//
	uint16_t cmost;			//充电MOS温度 286
	uint16_t dmost;			//放电MOS温度
	uint16_t pret;			//电量计温度
	uint16_t cont;			//连接器温度
	uint16_t btemp[2];		//温度1&2
	uint16_t bvolt[16];		//单体电压
	uint16_t balasta;		// 均衡值
	//uint16_t ignore[6];		// 加速度x,y,z,MCU3.3,预放电过载检测,电量计电流
	//uint16_t maxcvolt;		//最大充电电压
	//uint16_t pretvolt;		//电量计电压
	//uint16_t tvstemp;		//TVS管温度
	//uint16_t battempsp1;	//电池1温升速度
	//uint16_t battempsp2;	//电池2温升速度
	//uint16_t pretempsp;		//电量计温升速度
	//uint16_t 					//电芯电压最大下降速度
	//最近最大充电电流
	//最近最大放电电流
	//保险丝温度
	//Curr_DET_T Bit3-Bit0:大放电电流持续时间,Bit9~Bit4:10s最大放电电流,Bit15~bit10:10s内平均电流
	// 327~338 预留
	// 339~342 外置模块数据
	// 343 -- 单电池17电压
	// 344 -- AFE采样决电压
	// 235 -- AFE采样PACK端电压
	// 246 -- 第17电池均衡状态
	// 247 -- 电量计 SOC
	// 248 -- 电量计 SOH
	// 249 -- 电量计--循环次数
	// 250 -- 电量计--剩余容量
	// 251 -- 电量计--充满容量
	// 252 -- SOX 历史充电容量高位(mAH)
	// 253 -- SOX 历史充电容量低位(mAH)
	// 254 -- SOX 历史放电容量高位(mAH)
	// 255 -- SOX 历史放电容量低位(mAH)
	// 256 -- 外围模块状态--- Bit0 :1定位,0未定位,Bit1:0未连接服务器,1已连接服务器,Bit2:0无短信,1有短信
	// 257 - 358 :经度
	// 359 - 360 :伟度
	// 361 -- 	GPRS连接时间
	// 362 -- 	GPRS 发送到接收最长时间
	// 363 -- GPRS 发送到接收最短时间
	
	
}BmsReg_deviceInfo;

//BMS控制寄存器
typedef struct
{
	/*
	512 ~ 556
	*/
	/*
		BMS控制字
		Bit0 : R/W 	 CHG_EN 充电开关控制，1--充电导通,0--充电关闭
		Bit1 : R/W 	 DIS_EN 放电开关控制，1--放电导通,0--放电关闭
		Bit2 : R/W	 PRE_CHG_EN :预放电开关控制，1:导通，0:关闭
		Bit3,Bit4 : R/W	 保留
		Bit5 : R/W	1:浅休眠,0:电池工作
		Bit6 : R/W  1:深度休眠,0:电池工作
		Bit7 : R/W  1:充电器接入,0:充电器未接入
		Bit8 ~ Bit14:保留
		Bit15 : 使能均衡测试模式
	*/
	uint16_t ctrl;
	uint16_t bmscer[8];  // BMS 验证字，用于BMS验证Host , bmscer[7]:读写此字认为读写16个认证读写完毕
	uint16_t hostcer[8]; // Host 验证字，用于ｈｏｓｔ　验证ｂｍｓ
	uint16_t afeccal; // 校准AFE电流增益 0.01A/Bit ,W:写入当前实际电流值开始校准AFE，R：0--校准完毕,others--正在校准
	uint16_t b16vcal; // 校准第16节电池电压增益
	uint16_t yrmo;		// 年/月 BCD码
	uint16_t dthr;		// 日/时 BCD码
	uint16_t mnsc;		// 分/秒 BCD码
	uint16_t reset;		// 复位 1s后复位，不回应应答
	uint16_t record;	// 记录控制(测试)
	uint16_t balctl;    // 均衡控制(测试)
	uint16_t tctrl;		// 测试控制
	uint16_t tres;		// 测试结果 
	// 539 ~ 554 		保留
	// 555 	-- B4/B5 BMS使用
	// 556 --- 第17节电池均衡控制(测试)
}BmsReg_ctrl;

//BMS配置寄存器
typedef struct
{
	/*
		ADDR:768 ~ 4116
	*/
	/*
	ADDR:768 非易失存储器命令
	W:
	0 -- 无效
	0x1235 -- 
	*/
	uint16_t nvmcmd; 
	uint16_t rwsn[4];  // 序列号
	uint16_t sovp;		// 单体过压保护值(1s) 1mv/Bit
	uint16_t sovpr;		// 单体过压保护恢复值 
	uint16_t suvp1;		// 单体欠压保护
	uint16_t suvpt1;
	uint16_t suvpr1;
	uint16_t suvpsoc1;
	uint16_t suvpsoct1;
	uint16_t suvpsocr1;
	uint16_t suvp2;
	uint16_t suvpr2;
	uint16_t cocp;
	uint16_t cocpt;
	uint16_t docp1;
	uint16_t docpt1;
	uint16_t docp2;
	uint16_t dscp;
	uint16_t cotp;
	uint16_t cotpt;
	uint16_t cotpr;
	uint16_t dotp;
	uint16_t dotpt;
	uint16_t dotpr;
	uint16_t cutp;
	uint16_t cutpt;
	uint16_t cutpr;
	uint16_t dutp;
	uint16_t dutpt;
	uint16_t dutpr;
	uint16_t mosotp;
	uint16_t mosotpt;
	uint16_t mosotpr;
	uint16_t conotp;
	uint16_t conotpt;
	uint16_t conotpr;
	uint16_t sovw;
	uint16_t sovwt;
	uint16_t sovwr;
	uint16_t suvw;
	uint16_t suvwt;
	uint16_t suvwr;
	uint16_t cocw;
	uint16_t cocwt;
	uint16_t cocwr;
	uint16_t docw;
	uint16_t docwt;
	uint16_t docwr;
	uint16_t cotw;
	uint16_t cotwt;
	uint16_t cotwr;
	uint16_t dotw;
	uint16_t dotwt;
	uint16_t dotwr;
	uint16_t cutw;
	uint16_t cutwt;
	uint16_t cutwr;
	uint16_t dutw;
	uint16_t dutwt;
	uint16_t dutwr;
	uint16_t afecg;
	uint16_t b16vg;
	uint16_t cycles;
	uint16_t tvsotp;
	uint16_t tvsotpt;
	uint16_t tvsotpr;
}BmsReg_cfg;

//BMS用户信息区寄存器

typedef struct
{
	uint16_t udcmd;
	uint16_t userID[4];
}BmsReg_user;

#define BMS_REG_ID_ADDR 0

#define BMS_REG_INFO_ADDR_1 256
#define BMS_REG_INFO_ADDR_2 285
#define BMS_REG_INFO_END    316//322

#define BMS_REG_CTRL_ADDR   512
#define BMS_REG_CFG_ADDR    768
#define BMS_REG_USER_ADDR   4096

//寄存器地址定义
extern const BmsReg_info* g_bmsIICReg_id_addrMap;
extern const BmsReg_deviceInfo* g_bmsIICReg_Info_addrMap;
extern const BmsReg_ctrl* g_bmsIICReg_ctrl_addrMap;
extern const BmsReg_cfg* g_bmsIICReg_cfg_addrMap;
extern const BmsReg_user* g_bmsIICReg_info_addrMap;



// 认证
// bms to pms
typedef struct
{
	uint8_t		AckRandom[5];		// ack[0] + random[1~4]
}PmsBmsAuthRsq1;

typedef struct
{
	uint8_t		Ack;			// ack[0]
}PmsBmsAuthRsq2;


typedef struct
{
	uint8_t 	AckDigest[33];	// ack[0] + digest[1~32]
}BmsPmsAuthRsq;



#pragma pack()
#endif

/*****************************************end of MBFindMap.h*****************************************/

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
	uint16_t reserved0 : 6;  //BIT[8-15]:保留
	uint16_t chgrIn : 1;  //BIT[7]:充电器插入
	uint16_t deepSleepEn : 1; //BIT[6]:深睡眠使能
	uint16_t sleepEn : 1;  //BIT[5]:浅随眠使能
	uint16_t reserved1 : 2;  //BIT[3-4]:保留
	uint16_t preDischg : 1;  //BIT[2]:预防电使能
	uint16_t dischgEn : 1;  //BIT[1]:放电使能
	uint16_t chgEn : 1;  //BIT[0]:充电使能
}BmsRegCtrl;

// 1字节对齐
#pragma pack(1)

//BMS设备信息寄存器
typedef struct
{
	uint16_t prver;
	uint16_t hwver;
	uint16_t blver;
	uint16_t fwmsv;
	uint16_t fwrev;
	uint16_t fwbnh;
	uint16_t fwbnl;
	uint16_t mcun12;
	uint16_t mcun34;
	uint16_t mcun56;
	uint16_t mcun78;
	uint16_t btype;
	uint16_t bvolt;
	uint16_t bcap;
	uint16_t sn12;
	uint16_t sn34;
	uint16_t sn56;
	uint16_t sn78;
	uint16_t erech;
	uint16_t erecl;
	uint16_t lrech;
	uint16_t lrecl;
	uint16_t ltsta;
	uint16_t ltmaxv;
	uint16_t ltmaxvn;
	uint16_t ltminv;
	uint16_t ltminvn;
	uint16_t ltmaxt;
	uint16_t ltmaxtn;
	uint16_t ltmint;
	uint16_t ltmintn;
	uint16_t ltmaxcc;
	uint16_t ltmaxdc;
	uint16_t df_ver;
}BmsReg_info;

//BMS只读寄存器
typedef struct
{
	uint16_t userId[4];
	uint16_t state;
	uint16_t soc;
	uint16_t tvolt;
	uint16_t tcurr;
	uint16_t htemp;
	uint16_t htnum;
	uint16_t ltemp;
	uint16_t ltnum;
	uint16_t hvolt;
	uint16_t hvnum;
	uint16_t lvolt;
	uint16_t lvnum;
	uint16_t dsop;
	uint16_t csop;
	uint16_t soh;
	uint16_t cycle;
	uint16_t rcap;
	uint16_t fcap;
	uint16_t fctime;
	uint16_t prow;
	uint16_t devft1;
	uint16_t devft2;
	uint16_t opft1;
	uint16_t opft2;
	uint16_t opwarn1;
	uint16_t opwarn2;
	uint16_t cmost;
	uint16_t dmost;
	uint16_t pret;
	uint16_t cont;
	uint16_t btemp[2];
	uint16_t bvolt[16];
	uint16_t balasta;
}BmsReg_deviceInfo;

//BMS控制寄存器
typedef struct
{
	uint16_t ctrl;
	uint16_t bmscer[8];
	uint16_t hostcer[8];
	uint16_t afeccal;
	uint16_t b16vcal;
	uint16_t yrmo;
	uint16_t dthr;
	uint16_t mnsc;
	uint16_t reset;
	uint16_t record;
	uint16_t balctl;
	uint16_t tctrl;
	uint16_t tres;
}BmsReg_ctrl;

//BMS配置寄存器
typedef struct
{
	uint16_t nvmcmd;
	uint16_t rwsn[4];
	uint16_t sovp;
	uint16_t sovpr;
	uint16_t suvp1;
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
#define BMS_REG_INFO_END    322

#define BMS_REG_CTRL_ADDR   512
#define BMS_REG_CFG_ADDR    768
#define BMS_REG_USER_ADDR   4096

//寄存器地址定义
extern const BmsReg_info* g_bmsReg_id_addrMap;
extern const BmsReg_deviceInfo* g_bmsReg_Info_addrMap;
extern const BmsReg_ctrl* g_bmsReg_ctrl_addrMap;
extern const BmsReg_cfg* g_bmsReg_cfg_addrMap;
extern const BmsReg_user* g_bmsReg_info_addrMap;

#pragma pack()
#endif

/*****************************************end of MBFindMap.h*****************************************/

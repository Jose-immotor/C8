#ifndef __ERROR_DEF_H__
#define __ERROR_DEF_H__

//#include "Pms.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*Error list for detail*/
//#define ERR_TYPE_SYSTEM   	0x00
//#define ERR_TYPE_MOTOR		0x01
//#define ERR_TYPE_BATTERY1	0x02
//#define ERR_TYPE_BATTERY2	0x03
//#define ERR_TYPE_GPS		0x04
//#define ERR_TYPE_GPRS		0x05
//#define ERR_TYPE_BT	0x06

/*Trouble Code List For LCD or server*/
/*MCU system error*/
//#define TC_SMART_SYSTEM_LOST  			1
#define TC_CTR_485_LOST				2					
//#define TC_LCD_Driver_Sytem_LOST		3

#define TC_BRAKE_ERROR					10   
//#define TC_HANDLE_ERROR				11
#define TC_MOTOR_HOLL_ERROR			12
//#define TC_MOTOR_VOLTAGE_P			13
//#define TC_MOTOR_OVER_A				14
//#define TC_MOTOR_DRIVER_LOST			15 
#define TC_MOTOR_IVALID				16
#define TC_MOTOR_OTHER 				17

#define TC_BATTERY_ERR_HW			20	//硬件错误
#define TC_BATTERY_ERR_CHARGE		21	//充电错误
#define TC_BATTERY_ERR_DISCHARGE	22	//放电错误
#define TC_BATTERY_ERR_OTHERS		23	//其他错误

#define TC_BATTERY_ALARM_CHARGE		26	//充电告警
#define TC_BATTERY_ALARM_DISCHARGE	27	//放电告警

#define TC_GPS_INIT_FAIL					30
#define TC_GPS_START_FAIL					31
#define TC_GPS_ANTENNA_OPEN				32
#define TC_GPS_ANTENNA_SHORT				33
#define TC_GPS_NOT_FIX						34

#define TC_GPRS_INIT_FAIL					40
#define TC_GPRS_SIMCARD_INVALID			41
#define TC_GPRS_NO_SINGAL					42
#define TC_GPRS_NETWORK_REG_FAIL			43
#define TC_GPRS_CONNECT_SERVER_FAIL		44		
#define TC_GPRS_REG_DENIED				46			//过期

#define TC_BT_MODULE_INIT_FAIL			50
#define TC_BT_MODULE_FW_MATCH_FAIL		51
#define TC_GROY_ERROR					52	//异动传感器故障（读取失败）
//#define TC_LOGO_SET_FAIL

#define ALARM_CODE_OK	 	0
#define ALARM_CODE_MONTION 	1
#define ALARM_CODE_FENCE 	2


enum
{
	ERR_TYPE_SYSTEM = 0x00,
	ERR_TYPE_MOTOR1,
	ERR_TYPE_MOTOR2,
	ERR_TYPE_BATTERY1,
	ERR_TYPE_BATTERY2,
	ERR_TYPE_GPRS,
	ERR_TYPE_GPS,
	ERR_TYPE_BT,
	ERR_TYPE_GROY,
	ERR_TYPE_MAX
};


#ifdef __cplusplus
}
#endif

#endif


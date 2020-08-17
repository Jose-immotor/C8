

/**@file
 *
 * @defgroup app_fifo FIFO implementation
 * @{
 * @ingroup app_common
 *
 * @brief FIFO implementation.
 */

#ifndef __APP_TASK_BAT_H__
#define __APP_TASK_BAT_H__

#include <rtthread.h>
#include <stdbool.h>
#include "nfc_cmd_process.h"
#include "fm175xx.h"
#include "typedef.h"


#ifndef BATTERY_PROCESS_C
#define GLOBAL_BPROC extern
#else
#define GLOBAL_BPROC
#endif



typedef enum{
    ENUM_DISCHARGE,
    ENUM_CHARGE,
    ENUM_SLEEP
}ENUM_BMS_STATE;

//��������������״̬
extern unsigned char gl_hall_gpio_state;
extern unsigned char state_take_part;
enum{
    ENUM_NFC_CMD_INDEX_R_VERSION,
    ENUM_NFC_CMD_INDEX_R_INFO1,
    ENUM_NFC_CMD_INDEX_R_INFO2,
    ENUM_NFC_CMD_INDEX_R_CTRL,
    ENUM_NFC_CMD_INDEX_W_CTRL,
    ENUM_NFC_CMD_INDEX_R_PARAM1,
    ENUM_NFC_CMD_INDEX_R_PARAM2,
    ENUM_NFC_CMD_INDEX_W_PARAM,
    ENUM_NFC_CMD_INDEX_R_USERDATA,
    ENUM_NFC_CMD_INDEX_W_USERDATA,
    ENUM_NFC_CMD_INDEX_W_RESET,
    ENUM_NFC_CMD_INDEX_R_BLV,//��ѯbootloader�汾��
    ENUM_NFC_CMD_INDEX_BYPASS,// ͸��
    ENUM_NFC_CMD_INDEX_AUTHORITY, //��Ȩ����
    ENUM_NFC_CMD_INDEX_HISTORY_RECORD,
    ENUM_NFC_CMD_INDEX_MAX
};

//������־��Щ�����Ѿ���BMS�ж�ȡ��ɣ����Թ��ο�ʹ��
#define BMS_DEV_INFO_FLAG_BIT (1<<0)
#define BMS_READONLY_DATA1_FLAG_BIT (1<<1)
#define BMS_READONLY_DATA2_FLAG_BIT (1<<2)
#define BMS_CTRL_FLAG_BIT (1<<3)
#define BMS_PARAM1_FLAG_BIT (1<<4)
#define BMS_PARAM2_FLAG_BIT (1<<5)
#define BMS_USER_FLAG_BIT (1<<6) 



// �����ʾ������Ҫ����
GLOBAL_BPROC unsigned char gl_cmd_buf[NFC_READER_COUNT_MAX][ENUM_NFC_CMD_INDEX_MAX];



#define BMS_CTRL_BIT_CHG_SWITCH (1<<0)
#define BMS_CTRL_BIT_SUPPLY_SWITCH (1<<1)
#define BMS_CTRL_BIT_PRESUPPLY_SWITCH (1<<2)

#define BMS_STATE_BIT_CHG_SWITCH (1<<0)
#define BMS_STATE_BIT_SUPPLY_SWITCH (1<<1)
#define BMS_STATE_BIT_PRESUPPLY_SWITCH (1<<2)
//���������λ
#define BMS_STATE_BIT_CHG_IN (1<<7)
//��س������־λ
#define BMS_STATE_BIT_FULLY_CHARGE (1<<14)






enum{
ENUM_REG_ADDR_PRVER = 0, //Э��汾
ENUM_REG_ADDR_HWVER = 1, //Ӳ���汾
ENUM_REG_ADDR_BLVER = 2, //Bootloader�汾
ENUM_REG_ADDR_FWMSV = 3, //�̼��汾
ENUM_REG_ADDR_FWREV = 4, //�̼��汾- �����汾
ENUM_REG_ADDR_FWBNH = 5, //����汾����
ENUM_REG_ADDR_FWBNL = 6, //����汾����
    
ENUM_REG_ADDR_MCUN12 = 7,
ENUM_REG_ADDR_MCUN34 = 8,
ENUM_REG_ADDR_MCUN56 = 9,
ENUM_REG_ADDR_MCUN78 = 10,

ENUM_REG_ADDR_BVOLT = 12, //���ѹ
ENUM_REG_ADDR_BCAP = 13,//�����
ENUM_REG_ADDR_SN12 = 14,
ENUM_REG_ADDR_SN34 = 15,
ENUM_REG_ADDR_SN56 = 16,
ENUM_REG_ADDR_SN78 = 17,
ENUM_REG_ADDR_ERECH = 18, // �����¼�Ÿ���
ENUM_REG_ADDR_ERECL = 19, // �����¼�ŵ���
ENUM_REG_ADDR_LRECH = 20, // �����¼�Ÿ���
ENUM_REG_ADDR_LRECL = 21, // �����¼�ŵ���


ENUM_REG_ADDR_LTSTA = 22, //��ʷ״̬

//USER ID 
ENUM_REG_ADDR_RO_USERID12 = 256,
ENUM_REG_ADDR_RO_USERID34 = 257,
ENUM_REG_ADDR_RO_USERID56 = 258,
ENUM_REG_ADDR_RO_USERID78 = 259,

ENUM_REG_ADDR_STATE = 260,
ENUM_REG_ADDR_SOC = 261,
ENUM_REG_ADDR_TVOLT = 262, //Ŀǰ�����ѹ
ENUM_REG_ADDR_TCURR = 263, //Ŀǰ�������
ENUM_REG_ADDR_HTEMP = 264, //��ߵ���¶�
ENUM_REG_ADDR_LTEMP = 266,//��͵���¶�
ENUM_REG_ADDR_HVOLT = 268,//��ߵ����ѹ
ENUM_REG_ADDR_HVNUM = 269, // ��ߵ����ѹ��ر��
ENUM_REG_ADDR_LVOLT = 270,// ��͵����ѹ
ENUM_REG_ADDR_LVNUM = 271, // ��͵����ѹ��ر��
ENUM_REG_ADDR_DSOP = 272, // 10s�������ŵ����
ENUM_REG_ADDR_CSOP = 273,//10s������������
ENUM_REG_ADDR_SOH = 274,//         ����״̬
ENUM_REG_ADDR_CYCLE = 275,//ѭ������

ENUM_REG_ADDR_DEVFT1 = 280, //�豸������1
ENUM_REG_ADDR_DEVFT2 = 281, //�豸������1
ENUM_REG_ADDR_OPFT1 = 282, //���й�����1
ENUM_REG_ADDR_OPFT2 = 283, //���й�����2
ENUM_REG_ADDR_OPWARN1 = 284, //���и澯��1
ENUM_REG_ADDR_OPWARN2 = 285, //���и澯��2
ENUM_REG_ADDR_CMOST = 286, //���MOS�¶�
ENUM_REG_ADDR_DMOST = 287, //�ŵ�MOS�¶�
ENUM_REG_ADDR_FUELT = 288 , //�������¶�,
ENUM_REG_ADDR_CONT = 289, //�������¶�
ENUM_REG_ADDR_BTEMP1 = 290, //����¶�1
ENUM_REG_ADDR_BTEMP2 = 291, //����¶�2
ENUM_REG_ADDR_BVOLT1 = 292, // ������1��ѹ
ENUM_REG_ADDR_BVOLT2 = 293, // ������2��ѹ
ENUM_REG_ADDR_BVOLT3 = 294, // ������3��ѹ
ENUM_REG_ADDR_BVOLT4 = 295, // ������4��ѹ
ENUM_REG_ADDR_BVOLT5 = 296, // ������5��ѹ
ENUM_REG_ADDR_BVOLT6 = 297, // ������6��ѹ
ENUM_REG_ADDR_BVOLT7 = 298, // ������7��ѹ
ENUM_REG_ADDR_BVOLT8 = 299, // ������8��ѹ
ENUM_REG_ADDR_BVOLT9 = 300, // ������9��ѹ
ENUM_REG_ADDR_BVOLT10 = 301, // ������10��ѹ
ENUM_REG_ADDR_BVOLT11 = 302, // ������11��ѹ
ENUM_REG_ADDR_BVOLT12 = 303, // ������12��ѹ
ENUM_REG_ADDR_BVOLT13 = 304, // ������13��ѹ
ENUM_REG_ADDR_BVOLT14 = 305, // ������14��ѹ
ENUM_REG_ADDR_BVOLT15 = 306, // ������15��ѹ
ENUM_REG_ADDR_BVOLT16 = 307, // ������16��ѹ


ENUM_REG_ADDR_MAXCHGV = 315,//������ѹ
ENUM_REG_ADDR_TVST = 317, // TVS�¶�
ENUM_REG_ADDR_BHR1 = 318, //���1�����ٶ�
ENUM_REG_ADDR_BHR2 = 319, //���2�����ٶ�
ENUM_REG_ADDR_FGHR = 320, // �����������ٶ�
ENUM_REG_ADDR_DROCV = 321, // ��о��ѹ����½��ٶ�
ENUM_REG_ADDR_IDROCV = 322, // ��о��ѹ����½��ٶȱ��


ENUM_REG_ADDR_CTRL = 512, 
ENUM_REG_ADDR_RESET = 534,


//USER ID 
ENUM_REG_ADDR_USERID12 = 4097,
ENUM_REG_ADDR_USERID34 = 4098,
ENUM_REG_ADDR_USERID56 = 4099,
ENUM_REG_ADDR_USERID78 = 4100,

};


//ÿ����ѯ��ʱ�䳤��
#define BAT_POLL_ON_TIME_MAX 10000
// ��ѯ��ؼ��ʱ������
#define BAT_POLL_INTERVAL_MAX 60000
#define BAT_POLL_BIG_INTERVAL_MAX 86400000 // 24Сʱ
#define BAT_POLL_BIG_BIG_INTERVAL_MAX 0xFFFFFFFE//259200000 // 72Сʱ
// �����б仯ʱ��һֱ����ֱ��ѯģʽ1���ӣ������ٸ���ʵ���������ģʽ
#define BAT_POLL_DELAY_MAX 60000//1800000//360000         //��Ϊ60��
//���յ�����BMS��start �������������ѯ��ص�ʱ�䣬10����
#define BAT_POLL_BMS_UPDATE_DELAY_MAX 600000
//����͸����������ʱ��ֹͣ�����ѯ180��
#define BAT_POLL_DELAY_MAX_BY_BYPASS_CMD 180000
typedef struct{
unsigned char is_status_changed_on_door;
unsigned char is_poll_allways_en_flag; // 0, ��ʾ��ʱ��������ѯ��1����ʾһֱ��ѯ���޼��ʱ��
unsigned int poll_inerval_timer_cnt;
unsigned int poll_interval_MAX;
unsigned int poll_allways_en_delay_cnt; // �ڲ����б仯��ʱ����reset��������֤�ղ��ȥ�ĵ������ѯ������
unsigned int poll_interval_for_bms_update_cnt;
}st_bat_poll_param;
GLOBAL_BPROC st_bat_poll_param sl_bat_poll_param;


//��ز���ʱ��ʹ�ó������������Ƿ���λ
typedef struct{
unsigned char is_bat_v_valid; //������Ƿ��⵽��������ѹ
//�Ƿ���Ҫ����ص�ѹ������⵽�е�أ����SOC�����ŹرվͿ�ʼ���
unsigned char is_need_bat_v_checking; // 0�� ����Ҫ��⣻1�����ڼ�⣻2��������
unsigned short bat_v_charger_cmd_cnt; //��ʼ�򿪵��Ԥ�ź�������ͨѶ�Ĵ���
unsigned int bat_v_charger_comm_timeout_cnt; //��ʼ�򿪵��Ԥ�ź�������ͨѶ��ʱ�����
unsigned short bat_v_value; //�������⵽�ĵ�������ѹֵ����λ0.1V
unsigned short bat_v_charger_check_totol_time_cnt; //�������̵����ʱ�䡣
unsigned char bat_v_charger_charge_valid_cnt; // ������е���������ҵ���е�������Ĵ����������Ӧ��ʱ�����
}st_check_bat_v;
GLOBAL_BPROC st_check_bat_v gl_check_bat_v;



typedef struct{
//���ֹͣ����ԭ��Դ
unsigned int src_of_stop_flag;
unsigned char bat_ID[6];
unsigned char stop_flag;
unsigned int current_diff_counter;
unsigned int bat_current_out_counter; 
unsigned int hall_miss_status_clear_delay_cnt;
//ѡ����Щԭ��Դ���Ա����õ�src_of_stop_flag��
unsigned int src_of_stop_flag_disable_flag; // ��Ӧsrc_of_stop_flag�е�ÿ��Bit��0������1�ǽ�ֹ
}st_event_to_stop_chg_param;
GLOBAL_BPROC st_event_to_stop_chg_param gl_event_to_stop_chg_param;

typedef struct{
    unsigned char is_bat_chg_enable; // 0, �������磬1������ǰ��С������磬2�������������
    unsigned char is_need_to_reflash_temp; //�Ƿ���Ҫˢ�µ�ز���ʱ��ĵ���¶�ֵ
    short temp_value;//������ʱ��������¶ȣ���λ��0.1��
    unsigned int delay_counter_1;//��һ�׶μ�����
    unsigned int delay_counter_2;//�ڶ��׶μ�����
    unsigned int delay_counter_charge; //ǰ�ڳ��׶μ���
    unsigned int low_current_delay_count; //�͵�������ʱ�䳤��
}st_bat_low_temp_param;
GLOBAL_BPROC st_bat_low_temp_param gl_bat_low_temp_param;

typedef struct{
    short low_temp; // 0.1��Ϊ��λ
    unsigned int low_temp_delay_cnt; // 1mSΪ��λ
    short high_temp;// 0.1��Ϊ��λ
    unsigned int high_temp_delay_cnt;// 1mSΪ��λ
    unsigned int low_current_delay_count;// 1mSΪ��λ
}st_bat_low_temp_ctrl_param;

//������������ѹ���������⵼�������ѹƫ��ʱ���ʵ����߳���������ѹ��
typedef struct{
unsigned char reseve[3];
unsigned char voltage_offset_value; // ��λ0.1V
unsigned int debount_time_cnt;
//��ǰ��������ѹ(δ��ƫ����)����λ0.01V
unsigned short expect_setting_voltage;
//��ǰ����������(δ��ƫ����)����λ0.01A
unsigned short expect_setting_current;
}st_charger_setting_V_offset_param;
GLOBAL_BPROC st_charger_setting_V_offset_param gl_charger_setting_V_offset_param;

//�������̵Ĳ���
typedef struct{
unsigned char connector_default_state:1; //������״̬; 1, δ����; 0, ���Ӻ�
unsigned char switch_default_state:1; //΢������״̬; 1, δ��λ; 0, ��λ
unsigned char reserve_bit:6;
}st_bat_theft_prevention_param;
#ifndef BATTERY_PROCESS_C
GLOBAL_BPROC st_bat_theft_prevention_param gl_bat_theft_prevention_param;
#else
GLOBAL_BPROC st_bat_theft_prevention_param gl_bat_theft_prevention_param = {0, 0, 0};
#endif


/*
��ȡ�����ʷ��¼
*/
//��ʷ��¼��ŵķ�Χ
#define HISTORY_RECORD_INDEX_START 1
#define HISTORY_RECORD_INDEX_END 536862720

typedef struct{
//���Ͷ�ȡ��ʷ���ݵ�NFCͨѶ����ļ��ʱ��
unsigned int NFC_CMD_interval_cnt;
//��Ҫ��ȡ�ĵ�ǰ��¼��ţ�������Ҫ��ȡ���Ǵ���һ����ſ�ʼ
unsigned int history_record_begin_index;
//��Ҫ��ȡ�ļ�¼��Ŀ
unsigned int history_record_cnt;
//��ǰ�Ѿ���ȡ�ļ�¼��Ŀ
unsigned int history_record_read_cnt;
//��ǰ��������¼���
unsigned int history_record_earliest_index;
//��ǰ��������¼���
unsigned int history_record_lastest_index;
//��Ҫ��ȡ�ĵ�ǰ��¼��ŵ�ʱ�� �� BCD��
unsigned char history_record_begin_index_datetime[6];
//��ǰ��������¼��ŵ�ʱ��
unsigned char history_record_earliest_index_datetime[6];
//��ǰ��������¼��ŵ�ʱ��
unsigned char history_record_lastest_index_datetime[6];
//������ʷ����
unsigned char history_record_buf[128];
//���ID
unsigned char bat_ID[6];
//��ʼ��¼��ź���Ŀ�Ƿ���Ч
unsigned char is_index_valid:1;
//buffer�е������Ƿ���Ч
unsigned char is_buf_valid:1;
//buffer�е������Ƿ��Ѿ��ϴ�
unsigned char is_buf_updated:1;
//��ǰ��������¼��ź�ʱ���ֵ�Ƿ���Ч
unsigned char is_earliest_param_valid:1;
//��ǰ��������¼��ź�ʱ���ֵ�Ƿ���Ч
unsigned char is_lastest_param_valid:1;
unsigned char bit_reserve:4;
}st_bat_history_record_param;

GLOBAL_BPROC st_bat_history_record_param gl_bat_history_record_param;



//BAT_ERROR_CHECK_DELAY_AFTER_BAT_UPDATE
GLOBAL_BPROC unsigned int sl_bat_error_check_delay_after_bat_update_cnt;


typedef struct{
    unsigned char OD_update_file_max_subindex;
    /*�����ļ���Ӳ���汾��*/
    unsigned char OD_update_file_HW_version[2];
    /*�����ļ��Ĺ̼��汾��*/
    unsigned char OD_update_file_FW_version[7];
    /*�����ļ���buffer��ַ*/
    unsigned int OD_update_file_buff_addr;
}st_update_file_param;

GLOBAL_BPROC void Battery_clear_reg_valid_flag(unsigned char bms_index);

GLOBAL_BPROC void Battery_Process(void);
GLOBAL_BPROC unsigned short Battery_get_reg_value(unsigned char bms_index, unsigned short reg_index);
GLOBAL_BPROC unsigned short Battery_get_voltage(unsigned char bms_index);
GLOBAL_BPROC unsigned short Battery_get_SOC(unsigned char bms_index);


GLOBAL_BPROC int Battery_get_current(unsigned char bms_index);
GLOBAL_BPROC unsigned char is_battery_error(unsigned char bms_index);
GLOBAL_BPROC bool is_battery_A_V_reg_valid(unsigned char bms_index);
GLOBAL_BPROC void Battery_cmd_buf_clear(unsigned char bms_index);
GLOBAL_BPROC bool Battery_is_one_cmd_buf_empty(unsigned char bms_index);

GLOBAL_BPROC unsigned char Battery_get_reg_valid_flag(unsigned char bms_index);
GLOBAL_BPROC unsigned char Battery_send_cmd(unsigned char bms_index);

GLOBAL_BPROC unsigned short Battery_get_switch_state(unsigned char bms_index);
GLOBAL_BPROC unsigned char slave_rs485_is_bat_valid(unsigned char bms_index);
GLOBAL_BPROC void thread_battery_entry(void* parameter);
GLOBAL_BPROC void Battery_timer_count_cb(void);

#define BAT_ERR_SIZE 		6
#define SERIAL_NUM_SIZE		6


#if 1
typedef struct _Battery
{
	uint8  port; 			//
	
	uint8  isPresent:1; 	//is in port
	uint8  isCharged:1; 		
	uint8  Reserved:6; 		

	//Battery properties
	uint8  serialNum[SERIAL_NUM_SIZE];
	uint8  soc;
	uint8  soh;
	uint8  temperature;
	uint16 voltage;
	uint16 current;
	uint16 cycleCount; 	//
	uint16 capacity; 	//
	
	uint8  batteryErr[BAT_ERR_SIZE]; //������
}Battery;
#endif

void Battery_Init(Battery* pBattery, uint8 nPort);


#endif // MID_CONTROL_PROTOCOL_H__

/** @} */


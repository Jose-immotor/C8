/**@file
 *
 * @defgroup app_fifo FIFO implementation
 * @{
 * @ingroup app_common
 *
 * @brief FIFO implementation.
 */

#ifndef __BTTTERY_H__
#define __BTTTERY_H__

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

#define SERIAL_NUM_SIZE		6

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



#endif // __BTTTERY_H__

/** @} */


#ifndef NTAG_APP_H
#define NTAG_APP_H

#include "fm175xx.h"
#include "gd32f403.h"
//#include "common.h"
#ifndef NTAG_APP_C
#define GLOABL_NTAG extern
#else
#define GLOABL_NTAG 
#endif



typedef struct{
    unsigned char result;
    unsigned char vl_BMS_index; //��0��ʼ
    unsigned int vl_tmp_cnt;
    unsigned int vl_timer_cnt[NFC_READER_COUNT_MAX];
    //2018-5-31����֤����ÿ��3��Ҫ�л�һ��NFC����оƬ����ѯ���
    unsigned int vl_interval_timer_cnt;
}st_fn_NTAGTX_param;
GLOABL_NTAG st_fn_NTAGTX_param gl_NTAGTX_param;

#ifdef PMS_IIC_ERROR_COUNT_ENABLE
//˳��������ͳ�ƶ����ʧ�ܴ���
extern unsigned int gl_NTAG_read_faild_cnt;
extern unsigned int gl_NTAG_read_cnt;
//˳��������ͳ�ƶ���سɹ��Ĵ���
extern unsigned int gl_NTAG_read_sucess_cnt;
//˳�������ͳ�ƶ���ط��ص�����CRCУ�����Ĵ���
extern unsigned int gl_NTAG_read_crc_err_cnt;
#endif

#define FM17522_NPD_PORT	GPIOE
#define FM17522_NPD_PIN		GPIO_PIN_14

#define FM17522_NPD_LOW		gpio_bit_reset(FM17522_NPD_PORT,FM17522_NPD_PIN)
#define FM17522_NPD_HIGHT	gpio_bit_set(FM17522_NPD_PORT,FM17522_NPD_PIN)

#define FM17522_POWER_PORT	GPIOE
#define FM17522_POWER_PIN	GPIO_PIN_13

#define FM17522_POWER_ON	gpio_bit_reset(FM17522_POWER_PORT,FM17522_POWER_PIN)
#define FM17522_POWER_OFF	gpio_bit_set(FM17522_POWER_PORT,FM17522_POWER_PIN)


GLOABL_NTAG unsigned char TypeA_PPSS(unsigned char Pps0, unsigned char Pps1, unsigned char *PpsAck);
GLOABL_NTAG unsigned char TypeA_RATS(unsigned char param, unsigned char *ats);
GLOABL_NTAG unsigned char FM11NC08Active(unsigned char *ats);   
unsigned char NTAG_task_x(void);

GLOABL_NTAG unsigned char NTAG_task_reset(void);
GLOABL_NTAG void NTAG_task_process(void);

GLOABL_NTAG void NTAG_clear_reg_valid_flag(unsigned char bms_index);
GLOABL_NTAG unsigned char NTAG_get_nfc_working_status(void);     
GLOABL_NTAG unsigned char NTAG_task_timer_cb(void);

#endif

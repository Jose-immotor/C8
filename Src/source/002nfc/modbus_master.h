

/**@file
 *
 * @defgroup app_fifo FIFO implementation
 * @{
 * @ingroup app_common
 *
 * @brief FIFO implementation.
 */

#ifndef MODBUS_MASTER_H__
#define MODBUS_MASTER_H__

#include "fm175xx.h"

#ifndef MODBUS_MASTER_C
#define GLOBAL_MM extern
#else
#define GLOBAL_MM
#endif

//modbus״̬��
typedef enum{
    ENUM_MM_NFC_INIT,//NFC���ڳ�ʼ��״̬
    ENUM_MM_IDLE,//����״̬
    ENUM_MM_BC_WAIT,//�ȴ���ת�ӳ�
    ENUM_MM_ACK_WAIT,//�ȴ�Ӧ��
    ENUM_MM_ACK_PROCESS,//����Ӧ��
    ENUM_MM_ERR_PROCESS//������
}ENUM_MODBUS_MASTER_STATE;

//�������ݰ�����״̬��
typedef enum{
    ENUM_MD_INIT,//��ʼ״̬
    ENUM_MD_IDLE,//����״̬
    ENUM_MD_SND,//����״̬
    ENUM_MD_RCV,//����״̬
    ENUM_MD_CTRL//���ƺ͵ȴ�״̬
}ENUM_MODBUS_DATA_RCV_STATE;

#define MM_FN_CODE_READ_REG 0x03
#define MM_FN_CODE_WRITE_REG 0x06
#define MM_FN_CODE_WRITE_MULTI_REG 0x10
#define MM_FN_CODE_READ_HR_REG 0x46 // Histroy Record ��ʷ����
#define MM_FN_CODE_UPDATE_REG 0x67
#define MM_FN_CODE_AUTHORITY_REG 0x68
#define MM_FN_CODE_BYPASS 0x7F //�Զ��壬�����Լ�ʶ����͸������


//������󳤶�
#define MM_SND_BUFF_MAX 138
#define MM_RCV_BUFF_MAX 138

//��ͷ��Ϣ
#define MM_PCB_VALUE 0x02
#define MM_RESERVE_VALUE 0x00
#define MM_ADDR_VALUE 0x01

//���ݰ�����ط�����
#define MM_NFC_SND_ERR_MAX 5

//��ʱʱ��
#define MM_RSP_WAIT_TIMER_CNT_MAX 1500
#define MM_BC_WAIT_TIMER_CNT_MAX 200

// 1�ֽڶ���
//#pragma pack(1)
typedef struct{
//    ENUM_MODBUS_MASTER_STATE mm_state[NFC_READER_COUNT_MAX];
    //ENUM_MODBUS_DATA_RCV_STATE md_rcv_state[NFC_READER_COUNT_MAX];
    //unsigned char need2snd_data;//��Ҫ�������ݰ�
//    unsigned char BMS_index; //ĿǰͨѶ�ĵ�����
    unsigned char slave_addr[NFC_READER_COUNT_MAX];//����ʱ��Ĵӻ���ַ��0 ��ʾ�㲥, �ӻ���Ч��ַ1~247
    unsigned char MM_fn_code[NFC_READER_COUNT_MAX];//����Ĺ�����
    unsigned char MM_NFC_snd_buff[NFC_READER_COUNT_MAX][MM_SND_BUFF_MAX];//���ͻ���
    unsigned char MM_NFC_rcv_buff[NFC_READER_COUNT_MAX][MM_RCV_BUFF_MAX];//���ջ���
    unsigned char MM_NFC_snd_err_cnt[NFC_READER_COUNT_MAX];//���ʹ������
    unsigned char MM_NFC_nead_reset[NFC_READER_COUNT_MAX]; //����1��ʾNFC��������Ҫ����
    unsigned int MM_NFC_snd_len[NFC_READER_COUNT_MAX];//���ͻ����д��ڵ����ݳ���
    unsigned int MM_NFC_rcv_len[NFC_READER_COUNT_MAX];//���ջ����д��ڵ����ݳ���
//    unsigned int MM_IDLE_timer_cnt;
    //unsigned char MM_NFC_snd_enable;
}st_modbus_param;



#pragma pack(1)
typedef struct{

    unsigned char reg_addr_begin[2]; //��ˣ���ʼ��ַ
    unsigned char reg_cnt[2];  //��ˣ��Ĵ�������

}st_read_reg_03;

typedef struct{

    unsigned char reg_addr[2]; //��ˣ��Ĵ�����ַ
    unsigned char reg_value[2]; //��ˣ��Ĵ���ֵ

}st_write_reg_06;

typedef struct{

    unsigned char reg_addr_begin[2]; //��ˣ���ʼ��ַ
    unsigned char reg_cnt[2];  //��ˣ��Ĵ�������
    unsigned char byte_cnt;  //��ˣ��ֽڼ���
    unsigned char *byte_data; //�Ĵ���ֵ
}st_write_reg_10;
#pragma pack()






//�������
//#pragma pack()

GLOBAL_MM st_modbus_param gl_modbus_param;

GLOBAL_MM unsigned char MM_check_packet_head(unsigned char *pl_buff, unsigned char slave_addr,unsigned char fn_code);

GLOBAL_MM void MM_init(unsigned char bms_index);
GLOBAL_MM unsigned char MM_is_snd_done(void);
GLOBAL_MM unsigned char MM_is_snd_done_by_index(unsigned char bms_index);
GLOBAL_MM void MM_NFC_init_done(unsigned char bms_index);
GLOBAL_MM void MM_NFC_snd_done_cb(void);
GLOBAL_MM unsigned char MM_NFC_snd_ERR(void);
GLOBAL_MM void MM_timer_count_cb(void);
GLOBAL_MM unsigned char MM_set_BMS_index(unsigned char pl_index);
GLOBAL_MM unsigned char MM_is_bypass_fn_code_working(void);
GLOBAL_MM unsigned char MM_is_read_HR_fn_code_working(void);

GLOBAL_MM void MM_task(void);

GLOBAL_MM unsigned char MM_snd_multi_read_cmd(unsigned char slave_addr,st_read_reg_03* pl_read_reg_03);
GLOBAL_MM unsigned char MM_snd_multi_write_cmd(unsigned char slave_addr,st_write_reg_10* pl_write_reg_10);
GLOBAL_MM unsigned char MM_snd_read_history_record_cmd(unsigned char slave_addr, unsigned int record_index);
GLOBAL_MM unsigned char MM_snd_bypass(unsigned char slave_addr,unsigned char* data, unsigned int data_len);

GLOBAL_MM unsigned char MM_is_all_snd_done(void);

#endif // 
/** @} */

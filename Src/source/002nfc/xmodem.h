

#ifndef XMODEM_H__
#define XMODEM_H__

#ifndef XMODEM_C
#define GLOBAL_XMODEM extern
#else
#define GLOBAL_XMODEM
#endif

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define CTRLZ 0x1A
#define DLY_1S 1000
#define MAXRETRANS 25


typedef enum {
    ENUM_XMODEM_IDLE,
    ENUM_XMODEM_START,
    ENUM_XMODEM_START_WAIT_RCV,
    ENUM_XMODEM_TRANS_DATA,
    ENUM_XMODEM_TRANS_DATA_WAIT_RCV,
    ENUM_XMODEM_END,
    ENUM_XMODEM_END_WAIT_RCV,
    ENUM_XMODEM_REBOOT,
    ENUM_XMODEM_REBOOT_WAIT_RCV,
}enum_xmodem_state;

GLOBAL_XMODEM enum_xmodem_state gl_xmodem_state[NFC_READER_COUNT_MAX];


GLOBAL_XMODEM unsigned int gl_bms_reboot_wait_cnt;
#define BMS_XMODEM_TOTAL_TIMEOUT_MAX 26000
GLOBAL_XMODEM unsigned int gl_bms_return_to_normal_mode_cnt;



typedef enum{
ENUM_BMS_STATE_CHECK_IDLE,
ENUM_BMS_STATE_CHECK_REBOOT_BMS,
ENUM_BMS_STATE_CHECK_REBOOT_BMS_WAIT_RCV,
ENUM_BMS_STATE_CHECK_SND,
ENUM_BMS_STATE_CHECK_RCV    
}enum_bms_state_check;

GLOBAL_XMODEM enum_bms_state_check gl_bms_state_check[NFC_READER_COUNT_MAX];


typedef enum{
    ENUM_BMS_UPDATE_IDLE,
    //等待确认BMS进入bootloader升级状态
    ENUM_BMS_UPDATE_CHECK_PROCESS,
    //与BMS进入xmodem交互状态
    ENUM_BMS_UPDATE_XMODEM_PROCESS,
}enum_bms_update_state;

GLOBAL_XMODEM enum_bms_update_state gl_bms_update_state[NFC_READER_COUNT_MAX];


GLOBAL_XMODEM void bms_update_init(void);
GLOBAL_XMODEM void bms_update_process(unsigned char bms_index);
GLOBAL_XMODEM void bms_update_timer_cb(void);
GLOBAL_XMODEM void bms_xmodem_total_timeout_cnt_reload(void);



#endif // MID_CONTROL_PROTOCOL_H__

/** @} */

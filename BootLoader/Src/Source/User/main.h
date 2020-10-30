
#ifndef __MAIN_H__
#define __MAIN_H__

#include "Common.h"

#pragma pack(push)
#pragma pack(1)
typedef struct
{
    uint32_t app_crc;
    uint32_t length;
    uint8_t sw_main;
    uint8_t sw_s1;
    uint8_t sw_s2;
    uint8_t sw_build[4];//lsb first
    uint8_t hw_main;
    uint8_t hw_sub;
    uint8_t reserve[107];
    uint32_t info_crc;
}t_APP_INFO;
#pragma pack(pop)


#endif

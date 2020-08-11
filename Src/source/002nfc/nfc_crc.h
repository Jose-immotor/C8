#ifndef NFC_CRC_H
#define NFC_CRC_H

#ifndef NFC_CRC_C
#define GLOBAL_NFC_CRC extern
#else
#define GLOBAL_NFC_CRC
#endif


#define CRC_A 1
#define CRC_B 2

#include "typedef.h"
typedef  unsigned char BYTE;
//typedef  unsigned char uchar;
//typedef  unsigned char uint8;


GLOBAL_NFC_CRC void ComputeCrc(int CRCType,unsigned char *Data, unsigned int Length,unsigned char *TransmitFirst, unsigned char *TransmitSecond);





#endif



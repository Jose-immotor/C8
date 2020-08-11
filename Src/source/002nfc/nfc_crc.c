

#define NFC_CRC_C

#include "nfc_crc.h"


//typedef  unsigned char BYTE;
//typedef  unsigned char uchar;
//typedef  unsigned char uint8;

static unsigned short UpdateCrc(unsigned char ch, unsigned short *lpwCrc)
{
  ch = (ch^(unsigned char)((*lpwCrc) & 0x00FF));
  ch = (ch^(ch<<4));

  *lpwCrc = (*lpwCrc >> 8)^((unsigned short)ch << 8)^((unsigned
  short)ch<<3)^((unsigned short)ch>>4);
  return(*lpwCrc);
}

void ComputeCrc(int CRCType,unsigned char *Data, unsigned int Length,BYTE *TransmitFirst, BYTE *TransmitSecond)
{
    unsigned char chBlock;
    unsigned short wCrc;
    unsigned char * vl_data;

    if(Length == 0)
    {
        return;
    }
    vl_data = Data;
    switch(CRCType)
    {
        case CRC_A:
            wCrc = 0x6363; // ITU-V.41
        break;
        case CRC_B:
            wCrc = 0xFFFF; // ISO 3309
        break;
        default:
            return;
        break;
    }
    do {
        chBlock = *(vl_data++);
        UpdateCrc(chBlock, &wCrc);
    } while (--Length);
    if (CRCType == CRC_B)
        wCrc = ~wCrc; // ISO 3309
        
    *TransmitFirst = (BYTE) (wCrc & 0xFF);
    *TransmitSecond = (BYTE) ((wCrc >> 8) & 0xFF);

    return;
}



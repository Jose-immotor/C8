#ifndef TYPE_A_H
#define TYPE_A_H

#ifndef TYPE_A_C
#define GLOABL_TYPEA extern
#else
#define GLOABL_TYPEA 
#endif

#if 0
extern unsigned char PICC_ATQA[2],PICC_UID[15],PICC_SAK[3];

extern unsigned char TypeA_Request(unsigned char *pTagType);
extern unsigned char TypeA_WakeUp(unsigned char *pTagType);
extern unsigned char TypeA_Anticollision(unsigned char selcode,unsigned char *pSnr);
extern unsigned char TypeA_Select(unsigned char selcode,unsigned char *pSnr,unsigned char *pSak);
extern unsigned char TypeA_Halt(void);
extern unsigned char TypeA_CardActivate(unsigned char *pTagType,unsigned char *pSnr,unsigned char *pSak);
#endif

GLOABL_TYPEA unsigned char PICC_ATQA[2];
GLOABL_TYPEA unsigned char PICC_UID[15];
GLOABL_TYPEA unsigned char PICC_SAK[3];


GLOABL_TYPEA unsigned char TypeA_Request(unsigned char *pTagType);
GLOABL_TYPEA unsigned char TypeA_WakeUp(unsigned char *pTagType);
GLOABL_TYPEA unsigned char TypeA_Anticollision(unsigned char selcode, unsigned char *pSnr);
GLOABL_TYPEA unsigned char TypeA_Select(unsigned char selcode, unsigned char *pSnr, unsigned char *pSak);
GLOABL_TYPEA unsigned char TypeA_Halt(void);
GLOABL_TYPEA unsigned char TypeA_CardActivate(unsigned char *pTagType, unsigned char *pSnr, unsigned char *pSak);
unsigned char TypeA_RATS(unsigned char param, unsigned char* ats);

#endif

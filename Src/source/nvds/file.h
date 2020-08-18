#ifndef __FW_FILE__H_
#define __FW_FILE__H_

#ifdef __cplusplus
extern "C"{
#endif
#include "common.h"
#include "SectorMgr.h"

#define GETYPE_STR(pFile) ((FT_SMART == (pFile)->type) ? "SMART" : "PMS")

typedef enum _FileType
{
	 FT_SMART
	,FT_PMS
	,FT_BMS
}FileType;

typedef struct _DataRomEx
{
	uint32 m_AppFileLen;
	uint32 m_AppFileCrc;
	
	uint8 m_HwMainVer;
	uint8 m_HwSubVer;
	
	uint8  m_AppMainVer;
	uint8  m_AppSubVer;
	uint8  m_AppMinorVer;
	uint32 m_AppBuildeNum;
}DataRomEx;

typedef struct _File
{
	SectorMgr 	info;
	FileType  	type;
	uint32 		fileLen;
	DataRomEx	dataRom;
	uint32 		readOffset;
}File;

uint32 File_CalcuCrc(uint32 pAddr, int len);
Bool File_update(File* pFile, uint32 offset, const uint8* pData, uint32 len);
Bool File_updateDone(File* pFile);
File* File_Create(FileType ft, uint32 len);
void File_Seek(File* pFile, uint32 offset);
int File_IsEof(File* pFile);
int File_Read(File* pFile, void* pBuf, int bufLen);

inline uint32 File_GetOffset(File* pFile){return pFile->readOffset;};
inline uint32 File_GetLen(File* pFile){return pFile->fileLen;};
inline void File_Seek(File* pFile, uint32 offset);
inline int File_IsEof(File* pFile){return pFile->readOffset >= pFile->fileLen;};




#ifdef __cplusplus
}
#endif

#endif


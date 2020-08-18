
#include "File.h"
#include "Updater.h"
#include "crc32.h"
#include "datarom.h"
#include "smart_system.h"
#include "mx25_cmd.h"
#ifdef CFG_BMS
#include "Bms.h"
#endif
#define FILE_PRINTF Printf

uint32 File_CalcuCrc(uint32 pAddr, int len)
{
	uint16 size = BIN_SECTOR_SIZE;
	uint32 i = 0;
	uint32 crc = CRC32_CHECK_CODE;
	uint32 remainLen = len;
	
	for(i = 0; i < len; i += size)
	{
		remainLen = (len - i);
		size = (remainLen > BIN_SECTOR_SIZE) ? BIN_SECTOR_SIZE : remainLen;
		
		Mx25_Read(pAddr + i, g_CommonBuf, size);
		
		crc = Crc32_Calc(crc, g_CommonBuf, size);
		//Printf("offset=%d, crc=0x%08x\n", i, crc);
	}
	return crc;
}

File* File_Create(FileType ft, uint32 len)
{
	static File g_File;
	uint32 fileAddr = 0;

	memset(&g_File, 0, sizeof(File));
	
	if(ft == FT_SMART)
	{
		fileAddr = (uint32)&g_pNvdsMap->binFileBuf;
	}
	else if(ft == FT_PMS)
	{
		fileAddr = (uint32)&g_pNvdsMap->pmsFileBuf;
	}
	else if(ft == FT_BMS)
	{
		fileAddr = (uint32)&g_pNvdsMap->bmsFileBuf;
	}
	SectorMgr_Init(&g_File.info, fileAddr, Null, BIN_SECTOR_SIZE);

	g_File.type = ft;
	g_File.fileLen = len;
	g_File.readOffset = 0;
	
	return &g_File;
}

inline void File_Seek(File* pFile, uint32 offset)
{
	pFile->readOffset = (offset >= pFile->fileLen) ? pFile->fileLen : offset;
}

int File_Read(File* pFile, void* pBuf, int bufLen)
{	
	bufLen = MIN(pFile->fileLen - pFile->readOffset, bufLen);
	if(bufLen)
	{
		Mx25_Read(pFile->info.m_startAddr + pFile->readOffset, pBuf, bufLen);
		pFile->readOffset += bufLen;
	}
	return bufLen;
}

Bool File_Write128Bytes(File* pFile, uint32 offset, const uint8* pData)
{	
	//FILE_PRINTF("offset=%d\n", offset);
	#if 0
	//不写进Flash，检验收到的数据是否正确。
	static uint32 crc = CRC32_CHECK_CODE;
	if((offset == pFile->fileLen - BIN_SECTOR_SIZE))
	{
		CrcSector* pCrc = (CrcSector*)pData;
		DataRomEx* pDataRom = &pFile->dataRom;

		pDataRom->m_AppFileCrc 	= pCrc->m_CodeCrc;
		pDataRom->m_AppFileLen 	= pCrc->m_FwBinDataLen 	;
		
		pDataRom->m_AppMainVer 	= pCrc->m_MainVer 		;
		pDataRom->m_AppSubVer 	= pCrc->m_MinorVer 		;
		pDataRom->m_AppMinorVer = pCrc->m_RevisionNum 	;
		pDataRom->m_AppBuildeNum= pCrc->m_BuildNum		;
		
		pDataRom->m_HwMainVer 	= pCrc->m_HwMainVer 	;
		pDataRom->m_HwSubVer 	= pCrc->m_HwSubVer 		;

		Printf("File download %s\n", (crc == pDataRom->m_AppFileCrc) ? "Success" : "Failed");
	}
	else
	{
		int size = 0xF434 - offset;
		int len = MIN(size, BIN_SECTOR_SIZE);
		crc = Crc32_Calc(crc, (uint8*)pData, len);
		Printf("offset=%d, crc=0x%08x, size=%d, Len=%d\n", offset, crc, size, len);
	}
	#else
	if(offset == 0)
	{
		Mx25_EraseBlock(pFile->info.m_startAddr);
	}
	
	if((offset == pFile->fileLen - BIN_SECTOR_SIZE))
	{
		CrcSector* pCrc = (CrcSector*)pData;
		DataRomEx* pDataRom = &pFile->dataRom;

		pDataRom->m_AppFileCrc 	= pCrc->m_CodeCrc;
		pDataRom->m_AppFileLen 	= pCrc->m_FwBinDataLen 	;
		
		pDataRom->m_AppMainVer 	= pCrc->m_MainVer 		;
		pDataRom->m_AppSubVer 	= pCrc->m_MinorVer 		;
		pDataRom->m_AppMinorVer = pCrc->m_RevisionNum 	;
		pDataRom->m_AppBuildeNum= pCrc->m_BuildNum		;
		
		pDataRom->m_HwMainVer 	= pCrc->m_HwMainVer 	;
		pDataRom->m_HwSubVer 	= pCrc->m_HwSubVer 		;	
	}
	
	
	if(!Mx25_Write(pFile->info.m_startAddr + offset, pData, BIN_SECTOR_SIZE))
	{
		FILE_PRINTF("%s updated[0x%x] failed.\n", GETYPE_STR(pFile), offset);
		return False;
	}
	/*
	Mx25_Read(pFile->info.m_startAddr + offset, g_CommonBuf, BIN_SECTOR_SIZE);
	{
		if(memcmp(pData, g_CommonBuf, BIN_SECTOR_SIZE) != 0)
		{
			FILE_PRINTF("Write[0x%x] failed***********.\n", offset);
			DUMP_BYTE(pData, BIN_SECTOR_SIZE);
			FILE_PRINTF("\n");
			DUMP_BYTE(g_CommonBuf, BIN_SECTOR_SIZE);
			return False;
		}
	}
	*/
	#endif
	return True;
}

Bool File_update(File* pFile, uint32 offset, const uint8* pData, uint32 len)
{	
	int i = 0;
	Assert(len % BIN_SECTOR_SIZE == 0);
	
	for(i = 0; i < len; i += BIN_SECTOR_SIZE)
	{
		File_Write128Bytes(pFile, offset+i, &pData[i]);
		if(FT_SMART == pFile->type && g_pDataRom->m_isUpdateFw)
		{
			g_pDataRom->m_isUpdateFw = False;	
			DataRom_Write();
		}
		else if(FT_PMS == pFile->type && g_Settings.pmsFwLen)
		{
			WritePmsFileLen(0);
		}
			
	}
	return True;
}

Bool File_updateDone(File* pFile)
{
	uint32 crc = File_CalcuCrc(pFile->info.m_startAddr, pFile->dataRom.m_AppFileLen);
	if(pFile->dataRom.m_AppFileCrc != crc)
	{
		FILE_PRINTF("%s Crc Error[0x%08X][0x%08X], fileLen=%d\n", GETYPE_STR(pFile), pFile->dataRom.m_AppFileCrc, crc, pFile->fileLen);
		return False;
	}
	else
	{
		if(FT_SMART == pFile->type)
		{
			DataRomEx* pDataRom = &pFile->dataRom;
			
			g_pDataRom->m_AppFileCrc 	= pDataRom->m_AppFileCrc ;	
			g_pDataRom->m_AppFileLen 	= pDataRom->m_AppFileLen ;
			#if 1
			g_pDataRom->m_OlderAppMainVer 	= g_pDataRom->m_AppMainVer ;
			g_pDataRom->m_OlderAppSubVer 	= g_pDataRom->m_AppSubVer 	;
			g_pDataRom->m_OlderAppMinorVer 	= g_pDataRom->m_AppMinorVer ;
			g_pDataRom->m_OlderAppBuildeNum = g_pDataRom->m_AppBuildeNum;
			#endif
			g_pDataRom->m_AppMainVer 	= pDataRom->m_AppMainVer ;
			g_pDataRom->m_AppSubVer 	= pDataRom->m_AppSubVer 	;
			g_pDataRom->m_AppMinorVer 	= pDataRom->m_AppMinorVer ;
			g_pDataRom->m_AppBuildeNum 	= pDataRom->m_AppBuildeNum;
			
			g_pDataRom->m_HwMainVer 	= pDataRom->m_HwMainVer ;
			g_pDataRom->m_HwSubVer 		= pDataRom->m_HwSubVer 	;	
			g_pDataRom->m_isUpdateFw 	= True	;	
			
			DataRom_Write();
		}
		else if(FT_PMS == pFile->type)
		{
			WritePmsFileLen(pFile->fileLen);
		}
		Printf("%s(%d) updated success.\n", GETYPE_STR(pFile), pFile->fileLen);
		return True;
	}
}


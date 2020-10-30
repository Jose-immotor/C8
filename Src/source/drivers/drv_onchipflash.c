
#include "drv_onchipflash.h"


#define FMC_PAGES_PROTECTED (OB_WP_6 | OB_WP_7)

#define FMC_PAGE_SIZE           ((uint16_t)0x400U)//1KB
#define FMC_WRITE_START_ADDR    ((uint32_t)0x0801fc00U)//127页
#define FMC_WRITE_END_ADDR      ((uint32_t)0x0801ffffU)

int gd32_flash_read(uint32_t  addr, uint8_t *buf, uint16_t size)
{
    uint16_t i;

    if ((addr + size) > FMC_WRITE_END_ADDR)
    {
        return -1;
    }

    for (i = 0; i < size; i++, buf++, addr++)
    {
        *buf = *(uint8_t *) addr;
    }

    return size;
}

int gd32_flash_write(uint32_t addr, uint8_t *buf, uint16_t size)
{
	typedef enum {FAILED = 0, PASSED = !FAILED} test_state;

	uint8_t result = 0;
	uint32_t end_addr = addr + size;
	uint32_t wp_value = 0xFFFFFFFF, protected_pages = 0x0;
	uint32_t Address = 0x0;
	uint8_t *data;
	__IO fmc_state_enum fmc_state = FMC_READY;
	__IO test_state program_state = PASSED;
	
	if((addr % 4 != 0)||((end_addr) > FMC_WRITE_END_ADDR)||
		(size < 1))
        return -1;

    /* unlock the flash program/erase controller */
    fmc_unlock();
    ob_unlock();

    fmc_flag_clear(FMC_FLAG_END);
    fmc_flag_clear(FMC_FLAG_WPERR);
    fmc_flag_clear(FMC_FLAG_PGERR);

	/* Get pages write protection status */
    wp_value = ob_write_protection_get();
	
    /* The selected pages are not write protected */
    if((wp_value & FMC_PAGES_PROTECTED) != 0x00)
	{
        /* Clear All pending flags */
		fmc_flag_clear(FMC_FLAG_END);
		fmc_flag_clear(FMC_FLAG_WPERR);
		fmc_flag_clear(FMC_FLAG_PGERR);
	  
		/* FLASH Half Word program of data 0x1753 at addresses defined by BANK1_WRITE_START_ADDR and BANK1_WRITE_END_ADDR */
		Address = addr;
		data = buf;
		while((Address < end_addr) && (fmc_state == FMC_READY))
		{
			fmc_state = fmc_word_program(Address, *((uint32_t *)data));
			Address += 4;
			data  += 4;
		}
		if(fmc_state != FMC_READY)
		{
			result = -1;
		}	
		/* Check the correctness of written data */
//		Address = addr;
//		data = buf;
//		while((Address < end_addr) && (program_state != FAILED))
//		{
//			if((*(__IO uint32_t*) Address) != *((uint32_t *)buf))
//				program_state = FAILED;

//			Address += 4;
//			data  += 4;
//		}
//		if(program_state != PASSED)
//		{
//		    result = -1;
//		}		
    }
	else
		result = -1;

    /* lock the flash program/erase controller */
    fmc_lock();
    ob_lock();

    if (result != 0)
		return result;

    return size;
}

/**
 * Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
int gd32_flash_erase(uint32_t addr, uint16_t size)
{
    uint8_t result = 0;
	
	__IO fmc_state_enum fmc_state = FMC_READY;
    uint32_t FirstSector = 0, NbOfSectors = 0;
	uint32_t EraseCounter;

    if ((addr + size) > FMC_WRITE_END_ADDR)
        return -1;

    /* Unlock the Flash to enable the flash control register access */
    /* unlock the flash program/erase controller */
    fmc_unlock();
    ob_unlock();

    /* clear all pending flags */
    fmc_flag_clear(FMC_FLAG_END);
    fmc_flag_clear(FMC_FLAG_WPERR);
    fmc_flag_clear(FMC_FLAG_PGERR);

    /* Get the 1st sector to erase */
    FirstSector = (addr - FMC_WRITE_START_ADDR) / FMC_PAGE_SIZE;//GetSector(addr);
    /* Get the number of sector to erase from 1st sector*/
    NbOfSectors = (addr +size - FMC_WRITE_START_ADDR) / FMC_PAGE_SIZE;//GetSector(addr + size - 1) - FirstSector + 1;
	
    /* erase the flash pages */
    for(EraseCounter = FirstSector; (EraseCounter <= NbOfSectors)&& (fmc_state == FMC_READY); EraseCounter++)
	{
        fmc_state = fmc_page_erase(FMC_WRITE_START_ADDR + (FMC_PAGE_SIZE * EraseCounter));
		fmc_flag_clear(FMC_FLAG_END);
		fmc_flag_clear(FMC_FLAG_WPERR);
		fmc_flag_clear(FMC_FLAG_PGERR);
    }
	if(fmc_state == FMC_READY)
	{
		result = -1;
	}
	/* lock the flash program/erase controller */
    fmc_lock();
    ob_lock();

    if (result != 0)
    {
        return result;
    }

    return size;
}


/** @file Flash_W25Q64CV.c
 * Handle a Lenovo W520 BIOS flash chip.
 * @author Adrien RICCIARDI
 */
#include "Configuration.h"
#include "Flash.h"
#include "SPI.h"

#if CONFIGURATION_FLASH_SELECT_W25Q64CV

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void FlashInitialize(void)
{
	unsigned char Manufacturer_ID;
	unsigned short Device_ID;

	// Read known IDs
	FlashONFIReadID(&Manufacturer_ID, &Device_ID);
	if (Manufacturer_ID != 0xEF) while (1);
	if (Device_ID != 0x4017) while (1);
}

#endif 

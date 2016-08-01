/** @file Flash_MX25L6435E.c
 * Handle a Lenovo W520 BIOS flash chip.
 * @author Adrien RICCIARDI
 */
#include "Configuration.h"
#include "Flash.h"
#include "SPI.h"

#if CONFIGURATION_FLASH_SELECT_MX25L6435E

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void FlashInitialize(void)
{
	unsigned char Manufacturer_ID;
	unsigned short Device_ID;

	// Read known IDs
	FlashReadID(&Manufacturer_ID, &Device_ID);
	if (Manufacturer_ID != 0xC2) while (1);
	if (Device_ID != 0x2017) while (1);
}

#endif 

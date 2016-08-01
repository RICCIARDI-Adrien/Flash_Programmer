/** @file Flash.c
 * @see Flash.h for description.
 * @author Adrien RICCIARDI
 */
#include "Flash.h"
#include "SPI.h"

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void FlashONFIReadID(unsigned char *Pointer_Manufacturer_ID, unsigned short *Pointer_Device_ID)
{
	SPISetSlaveSelectState(1);

	// Send command
	SPITransferByte(0x9F);

	// Receive Manufacturer ID
	*Pointer_Manufacturer_ID = SPITransferByte(0xFF);

	// Receive Device ID
	*Pointer_Device_ID = SPITransferByte(0xFF) << 8;
	*Pointer_Device_ID |= SPITransferByte(0xFF);

	SPISetSlaveSelectState(0);
}

void FlashONFIReadBytes(unsigned long Address, unsigned short Bytes_Count, unsigned char xdata *Pointer_Buffer)
{
	SPISetSlaveSelectState(1);

	// Send the command
	SPITransferByte(0x03);

	// Send the address
	SPITransferByte(Address >> 16);
	SPITransferByte(Address >> 8);
	SPITransferByte(Address);

	// Read the data
	while (Bytes_Count > 0)
	{
		*Pointer_Buffer = SPITransferByte(0xFF);
		Pointer_Buffer++;
		Bytes_Count--;
	}

	SPISetSlaveSelectState(0);
}

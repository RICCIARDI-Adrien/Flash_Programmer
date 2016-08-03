/** @file Flash.c
 * @see Flash.h for description.
 * @author Adrien RICCIARDI
 */
#include "Flash.h"
#include "SPI.h"

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Allow the memory to be written. */
static void FlashEnableWriting(void)
{
	SPISetSlaveSelectState(1);

	// Send the command
	SPITransferByte(0x06);

	SPISetSlaveSelectState(0);
}

/** Get the Status Register value.
 * @return The Status Register.
 */
static unsigned char FlashReadStatusRegister(void)
{
	unsigned char Status_Register;

	SPISetSlaveSelectState(1);

	// Send the command
	SPITransferByte(0x05);

	// Get the status register
	Status_Register = SPITransferByte(0xFF);

	SPISetSlaveSelectState(0);

	return Status_Register;
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void FlashReadID(unsigned char *Pointer_Manufacturer_ID, unsigned short *Pointer_Device_ID)
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

void FlashReadBytes(unsigned long Address, unsigned short Bytes_Count, unsigned char xdata *Pointer_Buffer)
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

void FlashWriteBytes(unsigned long Address, unsigned short Bytes_Count, unsigned char xdata *Pointer_Buffer)
{
	unsigned short Bytes_To_Write;

	// The address is aligned on a page, so use the page as the default write unit to speed operations
	if ((Address & FLASH_PAGE_SIZE_BIT_MASK) == 0)
	{
		while (Bytes_Count > 0)
		{
			// Determine how many bytes to write
			if (Bytes_Count > FLASH_PAGE_SIZE) Bytes_To_Write = FLASH_PAGE_SIZE;
			else Bytes_To_Write = Bytes_Count;

			// Prepare write operation
			FlashEnableWriting();

			SPISetSlaveSelectState(1);

			// Send the command
			SPITransferByte(0x02);

			// Send the address
			SPITransferByte(Address >> 16);
			SPITransferByte(Address >> 8);
			SPITransferByte(Address);

			// Send up to a page to the flash
			while (Bytes_To_Write > 0)
			{
				// Send the byte to write
				SPITransferByte(*Pointer_Buffer);

				Address++;
				Bytes_To_Write--;
				Bytes_Count--;
				Pointer_Buffer++;
			}

			// Initiate the write cycle
			SPISetSlaveSelectState(0);

			// Wait for the write cycle to terminate
			while (FlashReadStatusRegister() & 1);
		}
	}
	else
	{
		while (Bytes_Count > 0)
		{
			FlashEnableWriting();

			// Write a byte
			SPISetSlaveSelectState(1);

			// Send the command
			SPITransferByte(0x02);

			// Send the address
			SPITransferByte(Address >> 16);
			SPITransferByte(Address >> 8);
			SPITransferByte(Address);

			// Send the byte to write
			SPITransferByte(*Pointer_Buffer);

			// Initiate the write cycle
			SPISetSlaveSelectState(0);

			// Wait for the write cycle to terminate
			while (FlashReadStatusRegister() & 1);

			Address++;
			Bytes_Count--;
			Pointer_Buffer++;
		}
	}
}

void FlashEraseSectors(unsigned long Address, unsigned short Sectors_Count)
{
	// Use the "erase chip" command if the whole flash must be erased
	if ((Address == 0) && (Sectors_Count == FLASH_TOTAL_SIZE / FLASH_SECTOR_SIZE))
	{
		// Prepare for erase operation
		FlashEnableWriting();
		SPISetSlaveSelectState(1);

		// Send the command
		SPITransferByte(0x60);

		// Initiate the erase cycle
		SPISetSlaveSelectState(0);

		// Wait for the erase cycle to terminate
		while (FlashReadStatusRegister() & 1);
	}
	else
	{
		while (Sectors_Count > 0)
		{
			// Prepare for erase operation
			FlashEnableWriting();
			SPISetSlaveSelectState(1);

			// Send the command
			SPITransferByte(0x20);

			// Send the address
			SPITransferByte(Address >> 16);
			SPITransferByte(Address >> 8);
			SPITransferByte(Address);

			// Initiate the erase cycle
			SPISetSlaveSelectState(0);

			// Wait for the erase cycle to terminate
			while (FlashReadStatusRegister() & 1);

			Sectors_Count--;
			Address += FLASH_SECTOR_SIZE;
		}
	}
}

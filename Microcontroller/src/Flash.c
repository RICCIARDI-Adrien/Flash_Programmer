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

void FlashEraseSector(unsigned long Address)
{
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

	// Wait for the write cycle to terminate
	while (FlashReadStatusRegister() & 1);
}

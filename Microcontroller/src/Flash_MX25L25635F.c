/** @file Flash_MX25L25635F.c
 * Interface to the Macronix MX25L25635Fxxx flash.
 * @author Adrien RICCIARDI
 */
#include "Configuration.h"
#include "Flash.h"
#include "SPI.h"

#if CONFIGURATION_FLASH_SELECT_MX25L25635F

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Enable or disable 4-byte addresses mode.
 * @param Is_Enabled Set to 1 to enable 4-byte addresses, set to 0 to use default 3-byte addresses.
 */
static void FlashEnable4ByteAddressMode(unsigned char Is_Enabled)
{
	unsigned char Command;

	SPISetSlaveSelectState(1);

	// Send the right command
	if (Is_Enabled) Command = 0xB7;
	else Command = 0xE9;
	SPITransferByte(Command);

	SPISetSlaveSelectState(0);
}

/** Get the Configuration Register value.
 * @return The Configuration Register.
 */
static unsigned char FlashReadConfigurationRegister(void)
{
	unsigned char Configuration_Register;

	SPISetSlaveSelectState(1);

	// Send the command
	SPITransferByte(0x15);

	// Get the status register
	Configuration_Register = SPITransferByte(0xFF);

	SPISetSlaveSelectState(0);

	return Configuration_Register;
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void FlashInitialize(void)
{
	unsigned char Byte_Temp;
	unsigned short Device_ID;

	// Force 3-byte address mode
	FlashEnable4ByteAddressMode(0);
	Byte_Temp = FlashReadConfigurationRegister();
	if (Byte_Temp & 0x20) while (1);

	// Read known IDs
	FlashReadID(&Byte_Temp, &Device_ID);
	if (Byte_Temp != 0xC2) while (1);
	if (Device_ID != 0x2019) while (1);
}

#endif

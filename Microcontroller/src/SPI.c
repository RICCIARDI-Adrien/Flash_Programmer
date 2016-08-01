/** @file SPI.c
 * @see SPI.h for description.
 * @author Adrien RICCIARDI
 */
#include <compiler_defs.h>
#include <SI_C8051F970_Register_Enums.h>
#include "SPI.h"

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void SPIInitialize(void)
{
	// Configure the SPI module for master mode operations
	SPI0CFG = SPI0CFG_MSTEN__MASTER_ENABLED | SPI0CFG_CKPHA__DATA_CENTERED_FIRST | SPI0CFG_CKPOL__IDLE_LOW | SPI0CFG_SRMT__SET | SPI0CFG_RXBMT__SET; // Enable master mode, configure SPI mode 0, set all reception flags to empty value
	SPI0CKR = 0; // Set the SPI clock to the fastest speed
	SPI0CN = SPI0CN_NSSMD__3_WIRE | SPI0CN_TXBMT__SET | SPI0CN_SPIEN__ENABLED; // Select 3-wire master mode, set all transmit flags to empty value, enable the SPI module
}

unsigned char SPITransferByte(unsigned char Byte_To_Send)
{
	// Send data
	SPI0DAT = Byte_To_Send;

	// Wait for the transfer to finish
	while (!SPI0CN_SPIF);
	SPI0CN_SPIF = 0;

	return SPI0DAT;
}

void SPISetSlaveSelectState(unsigned char Is_Enabled)
{
	// The Slave Select pin is active low
	if (Is_Enabled) P1 &= ~(1 << 2);
	else P1 |= 1 << 2;
}

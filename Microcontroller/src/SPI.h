/** @file SPI.h
 * Generic SPI master driver.
 * @author Adrien RICCIARDI
 */
#ifndef H_SPI_H
#define H_SPI_H

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Initialize the SPI module in master mode at a frequency of 1 MHz. The Slave Select pin is handled manually to fit the NOR custom transfers. */
void SPIInitialize(void);

/** Send a byte through the SPI bus and receive another byte in the same time.
 * @param Byte_To_Send The data to send.
 * @return The received data.
 * @warning The Slave Select pin must be driven manually.
 */
unsigned char SPITransferByte(unsigned char Byte_To_Send);

/** Select or not the NOR chip.
 * @param Is_Enabled Set to 1 to select the chip (i.e. Slave Select pin is set to low), set to 0 to deselect the chip (i.e. Slave Select pin is set to high).
 */
void SPISetSlaveSelectState(unsigned char Is_Enabled);

#endif

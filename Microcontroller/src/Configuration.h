/** @file Configuration.h
 * Gather all program configurable parameters.
 * @author Adrien RICCIARDI
 */
#ifndef H_CONFIGURATION_H
#define H_CONFIGURATION_H

#include "UART.h"

//-------------------------------------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------------------------------------
/** The UART connected to the PC baud rate. */
#define CONFIGURATION_UART_BAUD_RATE UART_BAUD_RATE_230400

/** Select the MX25L6435E flash. */
#define CONFIGURATION_FLASH_SELECT_MX25L6435E 0
/** Select the MX25L25635F flash. */
#define CONFIGURATION_FLASH_SELECT_MX25L25635F 0
/** Select the W25Q64CV flash. */
#define CONFIGURATION_FLASH_SELECT_W25Q64CV 1

#endif

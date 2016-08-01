/** @file UART.h
 * Read from and write to the UART0 serial port.
 * @author Adrien RICCIARDI
 */
#ifndef H_UART_H
#define H_UART_H

//-------------------------------------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------------------------------------
/** Compute the timer auto-reload value to achieve the requested frequency.
 * @param Baud_Rate The desired baud rate (not all values are possible with the current timer configuration).
 */
#define UART_COMPUTE_BAUD_RATE(Baud_Rate) (-(24500000UL / (Baud_Rate * 2)) + 256)

/** The timer 1 reload value to achieve 115200 bauds with the current main clock. */
#define UART_BAUD_RATE_115200 UART_COMPUTE_BAUD_RATE(115200)
/** The timer 1 reload value to achieve 230400 bauds with the current main clock. */
#define UART_BAUD_RATE_230400 UART_COMPUTE_BAUD_RATE(230400)
/** The timer 1 reload value to achieve 921600 bauds with the current main clock. */
#define UART_BAUD_RATE_921600 UART_COMPUTE_BAUD_RATE(921600)

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Initialize the UART0 to the specified baud rate.
 * @param Baud_Rate The speed the UART will run to, use one of the available UART_BAUD_RATE_xxx constants.
 */
void UARTInitialize(unsigned char Baud_Rate);

/** Read a byte from the UART.
 * @return The read byte.
 * @note This is a blocking function.
 */
unsigned char UARTReadByte(void);

/** Read a 32-bit number from the UART. The number must be sent in big endian.
 * @return The 32-bit number.
 * @note This is a blocking function.
 */
unsigned long UARTReadDoubleWord(void);

/** Write a byte to the UART and wait for the transmission to finish.
 * @param Byte The byte to write.
 */
void UARTWriteByte(unsigned char Byte);

/** Display an ASCIIZ string through the serial port. The '\r\n' sequence is NOT automatically added at the end of the string.
 * @param String The string to display.
 */
void UARTWriteString(unsigned char *String);

/** Convert and display the hexadecimal representation of a number.
 * @param Number The number to display.
 */
void UARTWriteHexadecimalNumber(unsigned short Number);

#endif

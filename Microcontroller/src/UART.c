/** @file UART.c
 * @see UART.h for description.
 * @author Adrien RICCIARDI
 */
#include <compiler_defs.h>
#include <SI_C8051F970_Register_Enums.h>
#include "UART.h"

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** Fake mutex telling if the transmission is finished or not. */
static volatile bit Is_Transmission_Finished = 1;
/** Fake mutex telling if a byte was received from the UART or not. */
static volatile bit Is_Byte_Received = 0;

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Handle the UART0 interrupts. */
INTERRUPT(UARTInterruptsHandler, UART0_IRQn)
{
	unsigned char Previous_SFR_Page;

	// Save the current page
	Previous_SFR_Page = SFRPAGE;
	SFRPAGE = LEGACY_PAGE;

	// Transmission interrupt
	if (SCON0_TI == 1)
	{
		Is_Transmission_Finished = 1;
		SCON0_TI = 0; // Clear the interrupt flag
	}

	// Reception interrupt
	if (SCON0_RI == 1)
	{
		Is_Byte_Received = 1;
		SCON0_RI = 0; // Clear the interrupt flag
	}

	// Restore the initial page
	SFRPAGE = Previous_SFR_Page;
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void UARTInitialize(unsigned char Baud_Rate)
{
	// Configure the timer 1 module to generate the UART clock
	CKCON |= CKCON_T1M__SYSCLK; // Use the system clock as timer clock source
	TMOD &= 0x0F; // Reset all the timer 1 related fields
	TMOD |= TMOD_T1M__MODE2; // Timer 1 is always enabled, timer 1 is incremented by the internal clock, select the 8-bit auto-reload mode
	TH1 = Baud_Rate; // Set the auto-reload value
	TL1 = 0; // Initialize the timer
	TCON |= TCON_TR1__RUN; // Enable the timer 1

	// Enable the UART0 interrupt (it is mandatory to use the transmission interrupt as polling does unpredictable behavior)
	IE |= IE_ES0__ENABLED;

	// Configure the UART
	SCON0 = (1 << 6) | SCON0_REN__RECEIVE_ENABLED; // Set a bit that must always be set and enable the reception
}

unsigned char UARTReadByte(void)
{
	// Wait for a byte to be received
	while (!Is_Byte_Received);
	Is_Byte_Received = 0;

	return SBUF0;
}

unsigned long UARTReadDoubleWord(void)
{
	unsigned long Result = 0;
	unsigned char i;

	// Keil is not able to shift by 24, so here is a dirty trick
	for (i = 0; i < 4; i++)
	{
		Result <<= 8;
		Result |= UARTReadByte();
	}
	return Result;
}

void UARTWriteByte(unsigned char Byte)
{
	// Send the byte
	SBUF0 = Byte;
	Is_Transmission_Finished = 0;

	// Wait for the transmission to finish
	while (!Is_Transmission_Finished);
}

void UARTWriteString(unsigned char *String)
{
	while (*String != 0)
	{
		UARTWriteByte(*String);
		String++;
	}
}

void UARTWriteHexadecimalNumber(unsigned short Number)
{
	unsigned char Nibble;
	signed char i;

	for (i = (sizeof(Number) * 8) - 4; i >= 0; i -= 4)
	{
		// Get the leftmost 4 bits
		Nibble = (Number >> i) & 0x0F;

		// Convert it to displayable value
		if (Nibble < 10) Nibble += '0';
		else Nibble += 'A' - 10; // The number is greater than or equal to ten, so 10 must be subtracted to get the right letter

		// Display the value
		UARTWriteByte(Nibble);
	}
}

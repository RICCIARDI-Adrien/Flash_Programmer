/** @file Main.c
 * The entry point and main loop.
 * @author Adrien RICCIARDI
 */
#include <compiler_defs.h>
#include <SI_C8051F970_Register_Enums.h>
#include "Configuration.h"
#include "Flash.h"
#include "SPI.h"
#include "UART.h"

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** The led port register. */
#define LED_PORT P0
/** The led pin. */
#define LED_PIN 5

/** Read data from the flash. */
#define COMMAND_READ_FLASH 0x10
/** Write data to the flash. */
#define COMMAND_WRITE_FLASH 0x20
/** Tell that the microcontroller is ready for another task. */
#define COMMAND_MICROCONTROLLER_READY 0x42

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** A flash-sector sized buffer. */
static unsigned char xdata Buffer[FLASH_SECTOR_SIZE];

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Configure the system clock. */
static void MainClockInitialize(void)
{
	// Enable the internal precision oscillator
	OSCICN |= OSCICN_IOSCEN__ENABLED;
	while (!(OSCICN & OSCICN_IFRDY__SET)); // Wait for the precision oscillator to become stable

	// Set the system oscillator clock source as the internal precision oscillator / 2
	CLKSEL &= 0x88; // Reset the Clock Source Divider and the Clock Source Select fields
	CLKSEL |= CLKSEL_CLKDIV__SYSCLK_DIV_1; // Do not divide the system clock frequency to achieve a 24.5 MHz clock
	while (!(CLKSEL & CLKSEL_CLKRDY__SET)); // Wait for the clock frequency division to be applied
}

/** Assign the port pins to the desired peripherals. */
static void MainPinsInitialize(void) // This function is not inlined because the C51 Keil compiler does not know "inline"...
{
	SFRPAGE = CONFIG_PAGE;

	// Pinout :
	// P0.1 : UART TX
	// P0.2 : UART RX
	// P0.5 : Led
	// P1.2 : SPI /SS (manually driven)
	// P2.0 : SPI SCK
	// P2.1 : SPI MISO
	// P2.2 : SPI MOSI

	// Bypass the needed pins
	P0SKIP = 0xF9;
	P1SKIP = 0xFF;

	// Configure the following pins as push-pull outputs
	P0MDOUT |= 1 << 5; // Led
	P1MDOUT |= 1 << 2; // Manual Slave Select
	P2MDOUT |= (1 << 2) | (1 << 0); // SPI SCK and SPI MOSI

	// Enable the desired modules
	XBR0 = XBR0_URT0E__ENABLED | XBR0_SPI0E__ENABLED; // Enable UART0 and SPI0

	// Enable the crossbar to make the pins assignment work
	XBR1 = XBR1_XBARE__ENABLED | XBR1_WEAKPUD__PULL_UPS_ENABLED;

	SFRPAGE = LEGACY_PAGE;
}

#if 0
/** @warning The buffer size must be a multiple of 4. */
static void DumpBuffer(unsigned char xdata *Pointer_Buffer, unsigned short Buffer_Size)
{
	unsigned short Address = 0;

	while (Buffer_Size > 0)
	{
		// Display address
		UARTWriteString("0x");
		UARTWriteHexadecimalNumber(Address);
		UARTWriteString("   0x");

		// Display 4 bytes at a time
		UARTWriteHexadecimalNumber((Pointer_Buffer[3] << 8) | Pointer_Buffer[2]);
		UARTWriteHexadecimalNumber((Pointer_Buffer[1] << 8) | Pointer_Buffer[0]);
		UARTWriteString("\r\n");

		Address += 4;
		Pointer_Buffer += 4;
		Buffer_Size -= 4;
	}
}
#endif

#if 0
static void CheckForData(void)
{
	unsigned long Address, Data, Displayed_Address, Mask;
	unsigned short i;
	unsigned long xdata *Pointer_Buffer_Long = Buffer;
	unsigned char Bits_Count, j;

	for (Address = 0; Address < FLASH_TOTAL_SIZE; Address += 4096)
	{
		// Read a full sector
		FlashReadBytes(Address, 4096, Buffer);

		// Check for something different from 0xFFFFFFFF
		for (i = 0; i < 1024; i++)
		{
			Data = Pointer_Buffer_Long[i];

			// A lot of 4-byte data on the flash differ from 0xFFFFFFFF, so count the number of bits to discard these values
			Bits_Count = 0;
			Mask = 1;
			for (j = 0; j < 32; j++)
			{
				if (Data & Mask) Bits_Count++;
				Mask <<= 1;
			}

			if (Bits_Count < 31)
			{
				UARTWriteString("Found 0x");
				UARTWriteHexadecimalNumber(Data >> 16);
				UARTWriteHexadecimalNumber(Data);

				UARTWriteString(" at address 0x");
				Displayed_Address = Address + (i * 4);
				UARTWriteHexadecimalNumber(Displayed_Address >> 16);
				UARTWriteHexadecimalNumber(Displayed_Address);
				UARTWriteString("\r\n");
			}
		}
	}
}
#endif

/** Read data from the flash memory. */
static void CommandReadFlash(void)
{
	unsigned long Address, Bytes_Count;
	unsigned short Bytes_To_Read, i;

	// Receive the address to start reading from
	Address = UARTReadDoubleWord();

	// Receive the amount of bytes to read
	Bytes_Count = UARTReadDoubleWord();

	// Read data
	while (Bytes_Count > 0)
	{
		// Read at most one sector at a time
		if (Bytes_Count > FLASH_SECTOR_SIZE) Bytes_To_Read = FLASH_SECTOR_SIZE;
		else Bytes_To_Read = (unsigned short) Bytes_Count;
		FlashReadBytes(Address, Bytes_To_Read, Buffer);

		// Send the data
		for (i = 0; i < Bytes_To_Read; i++) UARTWriteByte(Buffer[i]);

		Address += Bytes_To_Read;
		Bytes_Count -= Bytes_To_Read;
	}
}

/** Write data to the flash memory. */
static void CommandWriteFlash(void)
{
	unsigned long Address, Bytes_Count;
	unsigned short i, Bytes_To_Write;

	// Receive the starting address
	Address = UARTReadDoubleWord();

	// Receive the data to flash size
	Bytes_Count = UARTReadDoubleWord();

	// Receive data from the UART and write it to the flash
	while (Bytes_Count > 0)
	{
		// Determine the amount of data to receive
		if (Bytes_Count > FLASH_SECTOR_SIZE) Bytes_To_Write = FLASH_SECTOR_SIZE;
		else Bytes_To_Write = (unsigned short) Bytes_Count;

		// Receive the data
		for (i = 0; i < Bytes_To_Write; i++)
		{
			UARTWriteByte(COMMAND_MICROCONTROLLER_READY); // Used as flow control
			Buffer[i] = UARTReadByte();
		}

		// Erase the current sector
		FlashEraseSector(Address);

		// Write the data
		FlashWriteBytes(Address, Bytes_To_Write, Buffer);

		Bytes_Count -= Bytes_To_Write;
		Address += Bytes_To_Write;
	}
}

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
void main(void)
{
	unsigned char Byte_Temp;

	// Disable the watchdog timer
	PCA0MD &= ~PCA0MD_WDTE__ENABLED;

	// Set the oscillator frequency to 24.5 MHz
	MainClockInitialize();

	// Initialize all peripherals
	UARTInitialize(CONFIGURATION_UART_BAUD_RATE);
	SPIInitialize();

	// Give the desired pins to the desired peripherals (must be done after the peripherals initialization because the SPI pins count is configurable)
	MainPinsInitialize();

	// Enable interrupts
	IE |= IE_EA__ENABLED;

	// Initialize the memory now that all microcontroller modules are working
	FlashInitialize();

	// Light the led to tell that the programmer is ready
	LED_PORT &= ~(1 << LED_PIN);

	// Execute commands
	while (1)
	{
		// Wait for a command
		Byte_Temp = UARTReadByte();

		// Turn off the led while the programmer is busy
		LED_PORT |= 1 << LED_PIN;

		// Execute the right command
		switch (Byte_Temp)
		{
			case COMMAND_READ_FLASH:
				CommandReadFlash();
				break;

			case COMMAND_WRITE_FLASH:
				CommandWriteFlash();
				break;

			default:
				break;
		}

		// Light the led to tell that the programmer is ready
		LED_PORT &= ~(1 << LED_PIN);
	}
}

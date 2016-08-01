/** @file Main.c
 * Really simple flash chip programmer.
 * @author Adrien RICCIARDI
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "UART.h"

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** Read the whole flash starting from address 0. */
#define COMMAND_READ_FLASH 0x10
/** Write the whole flash starting from address 0. */
#define COMMAND_WRITE_FLASH 0x20
/** Tell that the microcontroller is ready for another task. */
#define COMMAND_MICROCONTROLLER_READY 0x42

/** The flash memory total size in bytes. */
#define FLASH_TOTAL_SIZE (32 * 1024 * 1024)

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Close the previously opened UART on program exit. */
static void ExitCloseUART(void)
{
	UARTClose();
}

/** Find the size in bytes of the specified file.
 * @param File The file to retrieve the size.
 * @return The file size in bytes.
 */
static unsigned int GetFileSize(FILE *File)
{
	unsigned int Size;
	
	// Go to the file end
	fseek(File, 0, SEEK_END);
	
	// Get the size
	Size = ftell(File);
	
	// Return to the file beginning
	rewind(File);
	
	return Size;
}

/** Dump the flash content.
 * @param Address The address to start reading from.
 * @param Instructions_Count How many instructions to read.
 */
static void CommandDumpFlash(unsigned int Address, unsigned int Instructions_Count)
{
	unsigned int Instruction, i, Instructions_To_Read, Bytes_Count;
	
	// Send the read command
	UARTWriteByte(COMMAND_READ_FLASH);
	
	// Send the address
	UARTWriteByte(Address >> 24);
	UARTWriteByte(Address >> 16);
	UARTWriteByte(Address >> 8);
	UARTWriteByte(Address);
	
	// Send the number of bytes to read
	Bytes_Count = Instructions_Count * 4; // 4 bytes per instruction
	UARTWriteByte(Bytes_Count >> 24);
	UARTWriteByte(Bytes_Count >> 16);
	UARTWriteByte(Bytes_Count >> 8);
	UARTWriteByte(Bytes_Count);
	
	// Dump the flash four ARM instructions at a time
	while (Instructions_Count > 0)
	{
		// Display the address
		printf("0x%08X - ", Address);
		
		// Display 4 instructions on the same line
		if (Instructions_Count < 4) Instructions_To_Read = Instructions_Count; // Allow to exit the loop even if a non-multiple of 4 number of instructions are requested
		else Instructions_To_Read = 4;
		for (i = 0; i < Instructions_To_Read; i++)
		{
			// Receive the instruction (the byte order is reversed as the ARM is little endian)
			Instruction = UARTReadByte();
			Instruction |= UARTReadByte() << 8;
			Instruction |= UARTReadByte() << 16;
			Instruction |= UARTReadByte() << 24;
		
			// Display it
			printf("%08X ", Instruction);
		}
		printf("\n");
		
		Address += 16;
		Instructions_Count -= Instructions_To_Read;
	}
}

/** Read the flash content.
 * @param Address The address to start reading from.
 * @param Bytes_Count How many bytes to read.
 * @param String_File_Name The read data will be stored in this file.
 */
static void CommandReadFlash(unsigned int Address, unsigned int Bytes_Count, char *String_File_Name)
{
	FILE *File;
	unsigned int i;
	unsigned char Byte;
	
	// Try to open the file
	File = fopen(String_File_Name, "wb");
	if (File == NULL)
	{
		printf("Error : could not create the file '%s'.\n", String_File_Name);
		exit(EXIT_FAILURE);
	}
	
	// Send the read command
	UARTWriteByte(COMMAND_READ_FLASH);
	
	// Send the address
	UARTWriteByte(Address >> 24);
	UARTWriteByte(Address >> 16);
	UARTWriteByte(Address >> 8);
	UARTWriteByte(Address);
	
	// Send the instructions count
	UARTWriteByte(Bytes_Count >> 24);
	UARTWriteByte(Bytes_Count >> 16);
	UARTWriteByte(Bytes_Count >> 8);
	UARTWriteByte(Bytes_Count);
	
	printf("Reading data...\n");
	
	// Receive the data
	for (i = 0; i < Bytes_Count; i++)
	{
		Byte = UARTReadByte();
		if (fwrite(&Byte, 1, 1, File) != 1)
		{
			printf("Error : could not write the byte %d.\n", i);
			break;
		}
	}
	
	printf("Read bytes : %u/%u.\n", i, Bytes_Count);
	fclose(File);
}

/** Write data to the flash memory.
 * @param Address The address to start writing to.
 * @param String_File_Name The path of the file containing the data to write.
 */
static void CommandWriteFlash(unsigned int Address, char *String_File_Name)
{
	FILE *File;
	unsigned int Bytes_Count, i;
	unsigned char Byte;
	
	// Try to open the file
	File = fopen(String_File_Name, "rb");
	if (File == NULL)
	{
		printf("Error : could not open the file '%s'.\n", String_File_Name);
		exit(EXIT_FAILURE);
	}
	
	// Get the file size
	Bytes_Count = GetFileSize(File);
	
	// Send the write command
	UARTWriteByte(COMMAND_WRITE_FLASH);
	
	// Send the address
	UARTWriteByte(Address >> 24);
	UARTWriteByte(Address >> 16);
	UARTWriteByte(Address >> 8);
	UARTWriteByte(Address);
	
	// Send the bytes count
	UARTWriteByte(Bytes_Count >> 24);
	UARTWriteByte(Bytes_Count >> 16);
	UARTWriteByte(Bytes_Count >> 8);
	UARTWriteByte(Bytes_Count);
	
	printf("Writing data...\n");
	
	// Send the data
	for (i = 0; i < Bytes_Count; i++)
	{
		// Wait for the microcontroller to become ready
		if (UARTReadByte() != COMMAND_MICROCONTROLLER_READY) printf("Warning : the microcontroller did not send the expected \"ready\" code.\n");
		
		// Read a byte
		if (fread(&Byte, 1, 1, File) == 1) UARTWriteByte(Byte); // Send it
		else
		{
			printf("Error : could not read the %u byte of the file.\n", i);
			break;
		}
	}
	
	printf("Written bytes : %u/%u.\n", i, Bytes_Count);
	fclose(File);
}

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	char *String_Serial_Port_Name, *String_Command;
	unsigned int Address, Count;
	
	// Check parameters
	if (argc < 3)
	{
		printf("Error : bad parameters.\n"
			"Usage : %s Serial_Port Command [Command parameters]\n"
			"Available commands :\n"
			"  d <Address(hex)> <Instructions_Count>        Dump Instructions_Count 4-byte instructions from the specified address.\n"
			"  r <Address(hex)> <Bytes_Count> <File_Name>   Read Bytes_Count bytes from the specified address and store them in the specified File_Name.\n"
			"  w <Address(hex)> <File_Name>                 Write the File_Name content at the specified address.\n", argv[0]);
		return EXIT_FAILURE;
	}
	String_Serial_Port_Name = argv[1];
	String_Command = argv[2];
	
	// Open the serial port
	printf("Initializing serial port... ");
	fflush(stdout);
	if (UARTOpen(String_Serial_Port_Name) == 0)
	{
		printf("Error : could not open the serial port '%s'.\n", String_Serial_Port_Name);
		return EXIT_FAILURE;
	}
	atexit(ExitCloseUART);
	printf("done.\n");
	
	// Execute the right command
	switch (*String_Command)
	{
		case 'd':
			if (argc != 5) printf("Error : missing parameters.\n");
			else
			{
				// Convert parameters
				sscanf(argv[3], "%X", &Address);
				Count = atoi(argv[4]);
				CommandDumpFlash(Address, Count);
			}
			break;
			
		case 'r':
			if (argc != 6) printf("Error : missing parameters.\n");
			else
			{
				// Convert parameters
				sscanf(argv[3], "%X", &Address);
				Count = atoi(argv[4]);
				CommandReadFlash(Address, Count, argv[5]);
			}
			break;
			
		case 'w':
			if (argc != 5) printf("Error : missing parameters.\n");
			else
			{
				// Convert parameters
				sscanf(argv[3], "%X", &Address);
				CommandWriteFlash(Address, argv[4]);
			}
			break;
			
		default:
			printf("Error : unknown command.\n");
			return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}
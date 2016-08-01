/** @file UART.c
 * @see UART.h for description.
 * @author Adrien RICCIARDI
 */
#include "UART.h" 
 
#ifdef WIN32 // Windows
#include <windows.h>
 
// This variable is global in this module in order to avoid passing it as a parameter.
static HANDLE COM_Handle;
 
int UARTOpen(char *Device_File_Name)
{
	DCB COM_Parameters;
	COMMTIMEOUTS Timing_Parameters;
	
	// Open the serial port and set all access rights
	COM_Handle = CreateFile(Device_File_Name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (COM_Handle == INVALID_HANDLE_VALUE) return 0; // Error : can't access to serial port
	
	// Configure port
	COM_Parameters.DCBlength = sizeof(DCB);
    COM_Parameters.fBinary = 1; // Must be set to 1 or Windows becomes angry
	COM_Parameters.fParity = 0; // No parity
	// Ignore modem signals
	COM_Parameters.fOutxCtsFlow = 0; 
	COM_Parameters.fOutxDsrFlow = 0;
	COM_Parameters.fDtrControl = DTR_CONTROL_DISABLE;
	COM_Parameters.fDsrSensitivity = 0;
	COM_Parameters.fTXContinueOnXoff = 0;
	COM_Parameters.fOutX = 0;
	COM_Parameters.fInX = 0;
	COM_Parameters.fErrorChar = 0;
	COM_Parameters.fNull = 0;
	COM_Parameters.fRtsControl = RTS_CONTROL_DISABLE;
	COM_Parameters.fAbortOnError = 0;
	COM_Parameters.fDummy2 = 0;
	COM_Parameters.wReserved = 0;
	COM_Parameters.XonLim = 0;
	COM_Parameters.XoffLim = 0;
	COM_Parameters.ByteSize = 8; // 8 bits of data
	COM_Parameters.Parity = NOPARITY; // Parity check disabled
	COM_Parameters.StopBits = ONESTOPBIT;
	COM_Parameters.XonChar = 0;
	COM_Parameters.XoffChar = 0;
	COM_Parameters.ErrorChar = 0;
	COM_Parameters.EofChar = 0;
	COM_Parameters.EvtChar = 0;
	COM_Parameters.wReserved1 = 0;
	
	// Set transmit and receive speed
	COM_Parameters.BaudRate = CBR_230400;
	
	// Set new parameters
	SetCommState(COM_Handle, &COM_Parameters);
	
	// Make reads non blocking
	Timing_Parameters.ReadIntervalTimeout = MAXDWORD; // According to MSDN, make the ReadFile() function returns immediately
	Timing_Parameters.ReadTotalTimeoutMultiplier = 0;
	Timing_Parameters.ReadTotalTimeoutConstant = 0;
	Timing_Parameters.WriteTotalTimeoutMultiplier = 0;
	Timing_Parameters.WriteTotalTimeoutConstant = 0;
	SetCommTimeouts(COM_Handle, &Timing_Parameters);
	
	// No error
	return 1;
}

unsigned char UARTReadByte(void)
{
	unsigned char Byte;
	DWORD Number_Bytes_Read;
	
	do
	{
		ReadFile(COM_Handle, &Byte, 1, &Number_Bytes_Read, NULL);
	} while (Number_Bytes_Read == 0);
	return Byte;
}

void UARTWriteByte(unsigned char Byte)
{
	DWORD Number_Bytes_Written;
	
	WriteFile(COM_Handle, &Byte, 1, &Number_Bytes_Written, NULL);
}

int UARTIsByteAvailable(unsigned char *Available_Byte)
{
	DWORD Number_Bytes_Read;
	
	ReadFile(COM_Handle, Available_Byte, 1, &Number_Bytes_Read, NULL);
	if (Number_Bytes_Read == 0) return 0;
	return 1;
}

void UARTClose(void)
{
	CloseHandle(COM_Handle);
}	

#else // Linux / UNIX
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// File representing the UART
static int File_Descriptor_UART;

// Old UART parameters
static struct termios Parameters_Old;

int UARTOpen(char *Device_File_Name)
{
	struct termios Parameters_New;
	
	// Open device file
	File_Descriptor_UART = open(Device_File_Name, O_RDWR | O_NONBLOCK);
	if (File_Descriptor_UART == -1) return 0;
	
	// Backup old UART parameters
	if (tcgetattr(File_Descriptor_UART, &Parameters_Old) == -1) return 0;
	
	// Configure new parameters
	Parameters_New.c_iflag = IGNBRK | IGNPAR; // Ignore break, no parity
	Parameters_New.c_oflag = 0;
	Parameters_New.c_cflag = CS8 | CREAD | CLOCAL; // 8 data bits, receiver enabled, ignore modem control lines
	Parameters_New.c_lflag = 0; // Use raw mode
	
	// Set speeds
	if (cfsetispeed(&Parameters_New, B230400) == -1) return 0;
	if (cfsetospeed(&Parameters_New, B230400) == -1) return 0;
	
	// Set parameters
	if (tcsetattr(File_Descriptor_UART, TCSANOW, &Parameters_New) == -1) return 0;
	return 1;
}

unsigned char UARTReadByte(void)
{
	unsigned char Byte;
	
	while (read(File_Descriptor_UART, &Byte, 1) <= 0);
	return Byte;
}

void UARTWriteByte(unsigned char Byte)
{
	write(File_Descriptor_UART, &Byte, 1);
}

int UARTIsByteAvailable(unsigned char *Available_Byte)
{
	if (read(File_Descriptor_UART, Available_Byte, 1) == 1) return 1;
	return 0;
}

void UARTClose(void)
{
	tcsetattr(File_Descriptor_UART, TCSANOW, &Parameters_Old);
	close(File_Descriptor_UART);
}

#endif
	
/** @file Flash.h
 * Contain some common commands (implemented in Flash.c) and all commands a specific flash must support (implemented in the flash specific source file).
 * @author Adrien RICCIARDI
 */
#ifndef H_FLASH_H
#define H_FLASH_H

#include "Configuration.h"

//-------------------------------------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------------------------------------
#if CONFIGURATION_FLASH_SELECT_MX25L6435E
	/** The total flash size in bytes. */
	#define FLASH_TOTAL_SIZE 8388608UL

	/** A sector size in bytes. */
	#define FLASH_SECTOR_SIZE 4096

	/** A page size in bytes. */
	#define FLASH_PAGE_SIZE 256
	/** The corresponding bit mask. */
	#define FLASH_PAGE_SIZE_BIT_MASK 0x000000FFUL
#endif

#if CONFIGURATION_FLASH_SELECT_MX25L25635F
	/** The total flash size in bytes. */
	#define FLASH_TOTAL_SIZE 33554432UL // Keil can't successfully compute 32 * 1024 * 1024...

	/** A sector size in bytes. */
	#define FLASH_SECTOR_SIZE 4096

	/** A page size in bytes. */
	#define FLASH_PAGE_SIZE 256
	/** The corresponding bit mask. */
	#define FLASH_PAGE_SIZE_BIT_MASK 0x000000FFUL
#endif

#if CONFIGURATION_FLASH_SELECT_W25Q64CV
	/** The total flash size in bytes. */
	#define FLASH_TOTAL_SIZE 8388608UL

	/** A sector size in bytes. */
	#define FLASH_SECTOR_SIZE 4096

	/** A page size in bytes. */
	#define FLASH_PAGE_SIZE 256
	/** The corresponding bit mask. */
	#define FLASH_PAGE_SIZE_BIT_MASK 0x000000FFUL
#endif

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Read the flash IDs.
 * @param Pointer_Manufacturer_ID On output, contain the Manufacturer ID.
 * @param Pointer_Device_ID On output, contain the Device ID.
 */
void FlashReadID(unsigned char *Pointer_Manufacturer_ID, unsigned short *Pointer_Device_ID);

/** Read a specified number of bytes from the specified address.
 * @param Address The address to start reading from (only 3-byte addresses are supported).
 * @param Bytes_Count How many bytes to read.
 * @param Pointer_Buffer On output, contain the read data.
 * @note This function can read up to the XRAM size bytes of data.
 */
void FlashReadBytes(unsigned long Address, unsigned short Bytes_Count, unsigned char xdata *Pointer_Buffer);

/** Write a specified number of bytes to the specified address.
 * @param Address The address to start writing to.
 * @param Bytes_Count How many bytes to write.
 * @param Pointer_Buffer The data to write.
 * @warning This function does not automatically erase the sectors it writes into, you have to call FlashEraseSector() to erase the sectors prior to write to them.
 */
void FlashWriteBytes(unsigned long Address, unsigned short Bytes_Count, unsigned char xdata *Pointer_Buffer);

/** Erase the specified amount of sectors.
 * @param Address The beginning address of the first sector to erase.
 * @param Sectors_Count How many sectors to erase.
 */
void FlashEraseSectors(unsigned long Address, unsigned short Sectors_Count);

/** Do all needed memory initialization. The function will hang if the flash can't be initialized properly.
 * @note This function must be implemented in the specific flash file.
 */
void FlashInitialize(void);

#endif

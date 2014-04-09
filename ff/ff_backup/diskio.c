/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/
// Pavel Negrobov - SDIO_SD version

#include <string.h>

#include "stm32f4_discovery.h"
#include "diskio.h"
#include "sdio_sd.h"

#define LED_ORANGE      LED3

// Информация о карте (заполняется в SD_Init)
extern SD_CardInfo SDCardInfo;
// Флаг наличия карты
static bool isSdPresent = false;

/*-----------------------------------------------------------------------*/
/* Inicializes a Drive                                                    */
DSTATUS disk_initialize (BYTE drv)    /* Physical drive nmuber (0..) */
{
//  DSTATUS stat = RES_OK;
  SD_Error sdErr;

//  if ( isSdPresent )
//    return RES_OK;
  
  // Init SD card
  sdErr = SD_Init ( );
  if ( sdErr != SD_OK )
  {
    isSdPresent = false;
    return RES_ERROR;
  } // if
  
  isSdPresent = true;
  return RES_OK;
} // 

/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */
DSTATUS disk_status (BYTE drv		/* Physical drive nmuber (0..) */ )
{
  DSTATUS stat = 0;
  
  if (SD_Detect() != SD_PRESENT || !isSdPresent)
    stat |= STA_NODISK;

  // STA_NOTINIT - Subsystem not initailized
  // STA_PROTECTED - Write protected, MMC/SD switch if available
  
  
  return stat;
} // disk_status

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{
  SDTransferState state;
//  DRESULT res = RES_OK;
  
  if ( SD_Detect( ) != SD_PRESENT || !isSdPresent )
    return RES_NOTRDY;

  if ( count == 0)
    return RES_PARERR;

  STM_EVAL_LEDOn ( LED_ORANGE );

  SD_ReadMultiBlocksFIXED ( buff, sector, 512, 1 );
  SD_WaitReadOperation ( );
  
  while ( 1 )
  {
    state = SD_GetStatus ( );
    if ( state == SD_TRANSFER_OK )
      break;
  } // while

  STM_EVAL_LEDOff ( LED_ORANGE );

  return RES_OK;
} // disk_read

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,		/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count		/* Number of sectors to write (1..255) */
)
{
  SDTransferState state;
//  DRESULT res = RES_OK;
  
  if ( SD_Detect( ) != SD_PRESENT || !isSdPresent )
    return RES_NOTRDY;

  if ( count == 0)
    return RES_PARERR;

  STM_EVAL_LEDOn ( LED_ORANGE );

//  SD_WriteMultiBlocks ( (uint8_t *)buff, sector, 512, 1 );
  SD_WriteMultiBlocksFIXED ( (uint8_t *)buff, sector, 512, 1 );
  SD_WaitWriteOperation ( );
  
  while ( 1 )
  {
    state = SD_GetStatus ( );
    if ( state == SD_TRANSFER_OK )
      break;
  } // while

  STM_EVAL_LEDOff ( LED_ORANGE );
  
  return RES_OK;
} // disk_write
#endif // _READONLY

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
#if _USE_IOCTL == 1
DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
  if ( !isSdPresent )
    return RES_NOTRDY;
  
  switch (ctrl)
  {
    case CTRL_SYNC:
      break;
      
    case GET_SECTOR_SIZE:       // Get sector size - 4 bytes
      memcpy ( buff, &SDCardInfo.CardBlockSize, sizeof ( SDCardInfo.CardBlockSize ) );
      break;
     
    case GET_SECTOR_COUNT:      // Get sector count - 8 bytes
      memcpy ( buff, &SDCardInfo.CardCapacity, sizeof ( SDCardInfo.CardCapacity ) );
      break;
  } // switch
  
  return RES_OK;
} // disk_ioctl
#endif // _USE_IOCTL

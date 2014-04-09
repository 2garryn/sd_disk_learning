/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* by grqd_xp                                                            */
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/
/* by nemui trinomius                                                    */
/* adopted to STM32F2xx_StdPeriph_Driver V1.0.0.						 */
/*-----------------------------------------------------------------------*/
#include <string.h>
#include "diskio.h"
#include "sdio_stm32f4.h"
// #include "hardware/rtc/rtc.h"

/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
/* Note that Tiny-FatFs supports only single drive and always            */
/* accesses drive number 0.                                              */

#define SECTOR_SIZE 512

__IO SD_Error Status = SD_OK;

/* dummy values */
#define SOCKPORT	1			/* Socket contact port */
#define SOCKWP		0			/* Write protect switch */
#define SOCKINS		0			/* Card detect switch */
 
 /*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

static volatile
DSTATUS Stat = STA_NOINIT;	/* Disk status */

static volatile
DWORD Timer1, Timer2;	/* 100Hz decrement timers */
 
/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
#define SDIO_DRIVE		0


/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */

DSTATUS disk_initialize (
                         BYTE drv				/* Physical drive nmuber (0..) */
                           )
{ 
  switch (drv) 
  {
    case SDIO_DRIVE:
    {     
      /* Initialize SD Card */
      Status = SD_Init(); 
      
      if (Status != SD_OK)
        return STA_NOINIT;
      else
        return 0x00;
    }
  }
  
  return STA_NOINIT;
  
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
                     BYTE drv		/* Physical drive nmuber (0..) */
                       )
{
  switch (drv) 
  {
    case SDIO_DRIVE:
    {
      Status = SD_GetCardInfo(&SDCardInfo);

      if (Status != SD_OK)
        return STA_NOINIT;
      else
        return 0x00;
    }
  }
  
  return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
DRESULT disk_read (
                   BYTE drv,		/* Physical drive nmuber (0..) */
                   BYTE *buff,		/* Data buffer to store read data */
                   DWORD sector,	/* Sector address (LBA) */
                   BYTE count		/* Number of sectors to read (1..255) */
                     )
{
  switch (drv) 
  {
    case SDIO_DRIVE:
    {     
      Status = SD_OK;
	  
#if defined(SD_DMA_MODE) 	/* DMA Transfer */
	/* Read Multiple Blocks */
	Status = SD_ReadMultiBlocks((uint8_t*)(buff),(sector)*SECTOR_SIZE,SECTOR_SIZE,count);

	/* Check if the Transfer is finished */
	Status = SD_WaitReadOperation();

	/* Wait until end of DMA transfer */
	while(SD_GetStatus() != SD_TRANSFER_OK);

#else  /* Polling Transfer */
    for (int secNum = 0; secNum < count && status == SD_OK; secNum++)
      {
        Status = SD_ReadBlock((uint32_t*)(buff+SECTOR_SIZE*secNum),
							  (sector+secNum)*SECTOR_SIZE, 
                              SECTOR_SIZE);
    }
#endif

	if (Status == SD_OK)	return RES_OK;
	else					return RES_ERROR;
    }

  }
  return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
                    BYTE drv,			/* Physical drive nmuber (0..) */
                    const BYTE *buff,	/* Data to be written */
                    DWORD sector,		/* Sector address (LBA) */
                    BYTE count			/* Number of sectors to write (1..255) */
                      )
{
  switch (drv) 
  {
    case SDIO_DRIVE:
    {     
		Status = SD_OK;
	
#if defined(SD_DMA_MODE)  /* DMA Transfer */
	/* Write Multiple Blocks */
	Status = SD_WriteMultiBlocks((uint8_t*)(buff),(sector)*SECTOR_SIZE,SECTOR_SIZE,count);

	/* Check if the Transfer is finished */
	Status = SD_WaitWriteOperation();

	/* Wait until end of DMA transfer */
	while(SD_GetStatus() != SD_TRANSFER_OK);
	
#else  /* Polling Transfer */
     for (int secNum = 0; secNum < count && status == SD_OK; secNum++)
      {
        Status = SD_WriteBlock((uint32_t*)(buff+SECTOR_SIZE*secNum),
							  (sector+secNum)*SECTOR_SIZE, 
                              SECTOR_SIZE);
    }

#endif

	if (Status == SD_OK)	return RES_OK;
	else					return RES_ERROR;
    }

  }
  return RES_PARERR;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
                    BYTE drv,		/* Physical drive nmuber (0..) */
                    BYTE ctrl,		/* Control code */
                    void *buff		/* Buffer to send/receive control data */
                      )
{
  switch (drv) 
  {
    case SDIO_DRIVE:
    {      
      switch (ctrl)
      {
        case CTRL_SYNC:
          /* no synchronization to do since not buffering in this module */
          return RES_OK;
        case GET_SECTOR_SIZE:
          *(WORD*)buff = SECTOR_SIZE;
          return RES_OK;
        case GET_SECTOR_COUNT:
          *(DWORD*)buff = SDCardInfo.CardCapacity / SECTOR_SIZE;
          return RES_OK;
        case GET_BLOCK_SIZE:
          *(DWORD*)buff = SECTOR_SIZE;
          return RES_OK;
      }
    }
  }
  return RES_PARERR;
}


/*-----------------------------------------------------------------------*/
/* Device Timer Interrupt Procedure  (Platform dependent)                */
/*-----------------------------------------------------------------------*/
/* This function must be called in period of 10ms                        */
void disk_timerproc (void)
{
	static BYTE pv;
	BYTE n, s;

	n = Timer1;						/* 100Hz decrement timer */
	if (n) Timer1 = --n;
	n = Timer2;
	if (n) Timer2 = --n;

	n = pv;
	pv = SOCKPORT & (SOCKWP | SOCKINS);	/* Sample socket switch */

	if (n == pv) {					/* Have contacts stabled? */
		s = Stat;

		if (pv & SOCKWP)			/* WP is H (write protected) */
			s |= STA_PROTECT;
		else						/* WP is L (write enabled) */
			s &= ~STA_PROTECT;

		if (pv & SOCKINS)			/* INS = H (Socket empty) */
			s |= (STA_NODISK | STA_NOINIT);
		else						/* INS = L (Card inserted) */
			s &= ~STA_NODISK;

		Stat = s;
	}
}
/**************************************************************************/
/*!
	RealTimeClock function
*/
/**************************************************************************/
uint32_t get_fattime (void)
{
//	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
//	RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
//
//	return	  ((uint32_t)(RTC_DateStructure.RTC_Year - 80) << 25)
//			| ((uint32_t)(RTC_DateStructure.RTC_Month) << 21)
//			| ((uint32_t)(RTC_DateStructure.RTC_Date) << 16)
//			| ((uint32_t)(RTC_TimeStructure.RTC_Hours) << 11)
//			| ((uint32_t)(RTC_TimeStructure.RTC_Minutes) << 5)
//			| ((uint32_t)(RTC_TimeStructure.RTC_Seconds) >> 1);


	return	((2006UL-1980) << 25)	      // Year = 2006
			| (2UL << 21)	      // Month = Feb
			| (9UL << 16)	      // Day = 9
			| (22U << 11)	      // Hour = 22
			| (30U << 5)	      // Min = 30
			| (0U >> 1)	      // Sec = 0
			;
	/*
	clock_datetime_t dt;
	clock_current_localtime(&dt);
	Pack date and time into a DWORD variable 
	return	  ((uint32_t)(dt.year - 80) << 25)
			| ((uint32_t)(dt.month) << 21)
			| ((uint32_t)(dt.day) << 16)
			| ((uint32_t)(dt.hour) << 11)
			| ((uint32_t)(dt.min) << 5)
			| ((uint32_t)(dt.sec) >> 1); */
}



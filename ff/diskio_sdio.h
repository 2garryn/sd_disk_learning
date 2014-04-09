#ifndef __DISKIO_SDIO_H
#define __DISKIO_SDIO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Generic Inclusion */
#include <stdbool.h>
#include "stm32f4xx.h"

extern void disk_timerproc (void);
uint32_t get_fattime (void);

#ifdef __cplusplus
}
#endif

#endif /* __SDIO_STM32F2_H */

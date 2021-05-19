/*
 * freertos-openocd.c
 *
 *  Created on: Dec 25, 2020
 *      Author: hector
 *
 * Since at least FreeRTOS V7.5.3 uxTopUsedPriority is no longer
 * present in the kernel, so it has to be supplied by other means for
 * OpenOCD's threads awareness.
 *
 * Add this file to your project, and, if you're using --gc-sections,
 * ``--undefined=uxTopUsedPriority'' (or
 * ``-Wl,--undefined=uxTopUsedPriority'' when using gcc for final
 * linking) to your LDFLAGS; same with all the other symbols you need.
 */

//#include "FreeRTOS.h"
#ifdef FREERTOS_TOTAL_RUNTIME_TIMER
#include "FreeRTOSConfig.h"

#ifdef __GNUC__
#define USED __attribute__((used))
#else
#define USED
#endif

const unsigned int USED uxTopUsedPriority = configMAX_PRIORITIES - 1;
#endif

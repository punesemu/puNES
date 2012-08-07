/*
 * externalcalls.c
 *
 *  Created on: 06/set/2011
 *      Author: fhorse
 */

#include <stdio.h>
#include "externalcalls.h"

void extclInit(void) {
	/* Mappers */
	extclCpuWrMem = NULL;
	extclCpuRdMem = NULL;
	extclSaveMapper = NULL;
	/* CPU */
	extclCPUEveryCycle = NULL;
	extclCPUWr4016 = NULL;
	/* PPU */
	extclPPU000to34x = NULL;
	extclPPU000to255 = NULL;
	extclPPU256to319 = NULL;
	extclPPU320to34x = NULL;
	extcl2006Update = NULL;
	extclRdChrAfter = NULL;
	extclRdNmt = NULL;
	extclRdChr = NULL;
	extclWrChr = NULL;
	/* APU */
	extclLengthClock = NULL;
	extclEnvelopeClock = NULL;
	extclApuTick = NULL;
	/* irqA12 */
	extclIrqA12Clock = NULL;
	/* battery */
	extclBatteryIO = NULL;
	/* snd */
	extclSndStart = NULL;
}

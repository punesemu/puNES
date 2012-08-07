/*
 * externalcalls.h
 *
 *  Created on: 03/set/2011
 *      Author: fhorse
 */

#ifndef EXTERNALCALLS_H_
#define EXTERNALCALLS_H_

#include <stdio.h>
#include "common.h"

/* mappers */
#define EXTCLCPUWRMEM(n) extclCpuWrMem = extclCpuWrMem_##n
#define EXTCLCPURDMEM(n) extclCpuRdMem = extclCpuRdMem_##n
#define EXTCLSAVEMAPPER(n) extclSaveMapper = extclSaveMapper_##n

/* CPU */
#define EXTCLCPUEVERYCYCLE(n) extclCPUEveryCycle = extclCPUEveryCycle_##n
#define EXTCLCPUWR4016(n) extclCPUWr4016 = extclCPUWr4016_##n

/* PPU */
#define EXTCLPPU000TO34X(n) extclPPU000to34x = extclPPU000to34x_##n
#define EXTCLPPU000TO255(n) extclPPU000to255 = extclPPU000to255_##n
#define EXTCLPPU256TO319(n) extclPPU256to319 = extclPPU256to319_##n
#define EXTCLPPU320TO34X(n) extclPPU320to34x = extclPPU320to34x_##n
#define EXTCL2006UPDATE(n) extcl2006Update = extcl2006Update_##n
#define EXTCLRDCHRAFTER(n) extclRdChrAfter = extclRdChrAfter_##n
#define EXTCLRDNMT(n) extclRdNmt = extclRdNmt_##n
#define EXTCLRDCHR(n) extclRdChr = extclRdChr_##n
#define EXTCLWRCHR(n) extclWrChr = extclWrChr_##n

/* APU */
#define EXTCLLENGTHCLOCK(n) extclLengthClock = extclLengthClock_##n
#define EXTCLENVELOPECLOCK(n) extclEnvelopeClock = extclEnvelopeClock_##n
#define EXTCLAPUTICK(n) extclApuTick = extclApuTick_##n

/* irqA12 */
#define EXTCLIRQA12CLOCK(n) extclIrqA12Clock = extclIrqA12Clock_##n

/* battery */
#define EXTCLBATTERYIO(n) extclBatteryIO = extclBatteryIO_##n

/* snd */
#define EXTCLSNDSTART(n) extclSndStart = extclSndStart_##n

void extclInit(void);

/* mappers */
void (*extclCpuWrMem)(WORD address, BYTE value);
BYTE (*extclCpuRdMem)(WORD address, BYTE openbus, BYTE before);
BYTE (*extclSaveMapper)(BYTE mode, BYTE slot, FILE *fp);

/* CPU */
void (*extclCPUEveryCycle)(void);
/* viene chiamata ogni volta si scrive qualcosa nel registro $4016 */
void (*extclCPUWr4016)(BYTE value);

/* PPU */
/* viene chiamata sempre, ad ogni ciclo della PPU */
void (*extclPPU000to34x)(void);
/*
 * viene chiamata se (!r2002.vblank && (ppu.screenY < SCRLINES))
 * quindi per essere sicuri di essere durante il rendering della PPU
 * nella funzione devo controllare anche se r2001.visible non e' a zero.
 */
void (*extclPPU000to255)(void);
/*
 * vengono chiamate solo se la PPU e' in fase di rendering
 * (!r2002.vblank && r2001.visible && (ppu.screenY < SCRLINES))
 */
void (*extclPPU256to319)(void);
void (*extclPPU320to34x)(void);
/* viene chiamata dopo ogni cambiamento del $2006 in cpuinline.h */
void (*extcl2006Update)(WORD r2006Old);
/* vengono chiamate in ppuinline.h */
BYTE (*extclRdNmt)(WORD address);
BYTE (*extclRdChr)(WORD address);
/* viene chiamata dopo il FETCHB e dopo il fetch dello sprite */
void (*extclRdChrAfter)(WORD address);
/* viene chiamato quando si tenta di scrivere nella CHR Ram */
void (*extclWrChr)(WORD address, BYTE value);

/* APU */
void (*extclLengthClock)(void);
void (*extclEnvelopeClock)(void);
void (*extclApuTick)(void);

/* irqA12 */
void (*extclIrqA12Clock)(void);

/* battery */
void (*extclBatteryIO)(BYTE mode, FILE *fp);

/* snd */
void (*extclSndStart)(WORD samplarate);

#endif /* EXTERNALCALLS_H_ */

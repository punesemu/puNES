/*
 * memmap.h
 *
 *  Created on: 10/mag/2010
 *      Author: fhorse
 */

#ifndef MEMMAP_H_
#define MEMMAP_H_

#include "common.h"

#define mirroring_H()\
	mapper.mirroring = HORIZONTAL;\
	ntbl.bank1k[0] = ntbl.bank1k[1] = &ntbl.data[0];\
	ntbl.bank1k[2] = ntbl.bank1k[3] = &ntbl.data[0x0400]
#define mirroring_V()\
	mapper.mirroring = VERTICAL;\
	ntbl.bank1k[0] = ntbl.bank1k[2] = &ntbl.data[0];\
	ntbl.bank1k[1] = ntbl.bank1k[3] = &ntbl.data[0x0400]
#define mirroring_SCR0()\
	mapper.mirroring = SINGLESCR0;\
	ntbl.bank1k[0] = ntbl.bank1k[1] = &ntbl.data[0];\
	ntbl.bank1k[2] = ntbl.bank1k[3] = &ntbl.data[0]
#define mirroring_SCR1()\
	mapper.mirroring = SINGLESCR1;\
	ntbl.bank1k[0] = ntbl.bank1k[1] = &ntbl.data[0x0400];\
	ntbl.bank1k[2] = ntbl.bank1k[3] = &ntbl.data[0x0400]
#define mirroring_FSCR()\
	mapper.mirroring = FOURSCR;\
	ntbl.bank1k[0] = &ntbl.data[0];\
	ntbl.bank1k[1] = &ntbl.data[0x0400];\
	ntbl.bank1k[2] = &ntbl.data[0x0800];\
	ntbl.bank1k[3] = &ntbl.data[0x0C00]
#define mirroring_SCR0x1_SCR1x3()\
	mapper.mirroring = SCR0x1_SCR1x3;\
	ntbl.bank1k[0] = &ntbl.data[0];\
	ntbl.bank1k[1] = \
	ntbl.bank1k[2] = \
	ntbl.bank1k[3] = &ntbl.data[0x0400]
#define mirroring_SCR0x3_SCR1x1()\
	mapper.mirroring = SCR0x3_SCR1x1;\
	ntbl.bank1k[0] = \
	ntbl.bank1k[1] = \
	ntbl.bank1k[2] = &ntbl.data[0];\
	ntbl.bank1k[3] = &ntbl.data[0x0400]

#define prgRamPlusSize() info.prgRamPlus8kCount << 13
#define chrRamSize() info.chrRom8kCount << 13

struct _mmcpu {
	BYTE ram[0x800];  // Mirrored four times
} mmcpu;
struct _prg {
	BYTE *rom;
	BYTE *rom8k[4];   // 8k pages (0x2000)

	BYTE *ram;        // Non Battery RAM

	BYTE *ramPlus;    // PRG Ram extra
	BYTE *ramPlus8k;
	BYTE *ramBattery; // Battery RAM
} prg;
struct _chr {
	BYTE *data;
	BYTE *bank1k[8];
} chr;
struct _nametables {
	BYTE data[0x1000];
	BYTE *bank1k[4];
} ntbl;
struct _palette {
	BYTE color[0x20];
} palette;
struct _oam {
	BYTE data[256];
	BYTE *element[64];
	BYTE plus[32];
	BYTE *elementPlus[8];
} oam;

#endif /* MEMMAP_H_ */

/*
 * mapper222.c
 *
 *  Created on: 24/mar/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "cpu6502.h"
#include "irqA12.h"
#include "savestate.h"

WORD prgRom8kMax, chrRom1kMax;

void mapInit_222(void) {
	prgRom8kMax = info.prgRom8kCount - 1;
	chrRom1kMax = info.chrRom1kCount - 1;

	EXTCLCPUWRMEM(222);
	EXTCLSAVEMAPPER(222);
	EXTCLCPUEVERYCYCLE(222);
	EXTCLPPU000TO34X(MMC3);
	EXTCLPPU000TO255(MMC3);
	EXTCLPPU256TO319(MMC3);
	EXTCLPPU320TO34X(MMC3);
	EXTCL2006UPDATE(MMC3);
	EXTCLIRQA12CLOCK(222);

	mapper.intStruct[0] = (BYTE *) &m222;
	mapper.intStructSize[0] = sizeof(m222);

	memset(&m222, 0x00, sizeof(m222));
	memset(&irqA12, 0x00, sizeof(irqA12));

	irqA12.present = TRUE;
}
void extclCpuWrMem_222(WORD address, BYTE value) {
	switch (address & 0xF003) {
		case 0x8000:
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, 0, value);
			mapPrgRom8kUpdate();
			return;
		case 0x9000:
			if (value & 0x01) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			return;
		case 0xA000:
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, 1, value);
			mapPrgRom8kUpdate();
			return;
		case 0xB000:
			controlBank(chrRom1kMax)
			chr.bank1k[0] = &chr.data[value << 10];
			return;
		case 0xB002:
			controlBank(chrRom1kMax)
			chr.bank1k[1] = &chr.data[value << 10];
			return;
		case 0xC000:
			controlBank(chrRom1kMax)
			chr.bank1k[2] = &chr.data[value << 10];
			return;
		case 0xC002:
			controlBank(chrRom1kMax)
			chr.bank1k[3] = &chr.data[value << 10];
			return;
		case 0xD000:
			controlBank(chrRom1kMax)
			chr.bank1k[4] = &chr.data[value << 10];
			return;
		case 0xD002:
			controlBank(chrRom1kMax)
			chr.bank1k[5] = &chr.data[value << 10];
			return;
		case 0xE000:
			controlBank(chrRom1kMax)
			chr.bank1k[6] = &chr.data[value << 10];
			return;
		case 0xE002:
			controlBank(chrRom1kMax)
			chr.bank1k[7] = &chr.data[value << 10];
			return;
		case 0xF000:
			//irqA12.latch = value;
			m222.count = value;
			irq.high &= ~EXTIRQ;
			return;
	}
}
BYTE extclSaveMapper_222(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m222.count);
	savestateEle(mode, slot, m222.delay);

	return (EXIT_OK);
}
void extclIrqA12Clock_222(void) {
	if (!m222.count || (++m222.count < 240)) {
		return;
	}

	m222.count = 0;
	m222.delay = 16;
}
void extclCPUEveryCycle_222(void) {
	if (m222.delay && !(--m222.delay)) {
		irq.high |= EXTIRQ;
	}
}

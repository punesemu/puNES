/*
 * mapper50.c
 *
 *  Created on: 21/apr/2012
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "cpu6502.h"
#include "savestate.h"

WORD prgRom8kMax, chrRom1kMax;
BYTE *prg6000;

void mapInit_50(void) {
	prgRom8kMax = info.prgRom8kCount - 1;
	chrRom1kMax = info.chrRom1kCount - 1;

	EXTCL_CPU_WR_MEM(50);
	EXTCL_CPU_RD_MEM(50);
	EXTCL_SAVE_MAPPER(50);
	EXTCL_CPU_EVERY_CYCLE(50);
	mapper.intStruct[0] = (BYTE *) &m50;
	mapper.intStructSize[0] = sizeof(m50);

	if (info.reset >= HARD) {
		memset(&m50, 0x00, sizeof(m50));

		mapper.romMapTo[2] = 0;
	}

	prg6000 = &prg.rom[prgRom8kMax << 13];

	mapper.romMapTo[0] = 8;
	mapper.romMapTo[1] = 9;
	mapper.romMapTo[3] = 11;

	info.mapperExtendWrite = TRUE;
}
void extcl_cpu_wr_mem_50(WORD address, BYTE value) {
	if ((address <= 0x5FFF) && ((address & 0x0060) == 0x0020)) {
		if (address & 0x0100) {
			if (!(m50.enabled = value & 0x01)) {
				m50.count = 0;
				irq.high &= ~EXTIRQ;
			}
			return;
		}

		value = (value & 0x08) | ((value << 2) & 0x04) | ((value >> 1) & 0x03);
		controlBank(prgRom8kMax)
		mapPrgRom8k(1, 2, value);
		mapPrgRom8kUpdate();
		return;
	}
}
BYTE extcl_cpu_rd_mem_50(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (prg6000[address & 0x1FFF]);
}
BYTE extcl_save_mapper_50(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m50.enabled);
	savestateEle(mode, slot, m50.count);
	savestateEle(mode, slot, m50.delay);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_50(void) {
	if (m50.delay && !(--m50.delay)) {
		irq.high |= EXTIRQ;
	}

	if (m50.enabled && (++m50.count == 0x1000)) {
		m50.delay = 1;
	}
}

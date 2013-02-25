/*
 * mapperVRC3.c
 *
 *  Created on: 11/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "cpu.h"
#include "savestate.h"

WORD prgRom16kMax;

void mapInit_VRC3(void) {
	prgRom16kMax = info.prgRom16kCount - 1;

	EXTCL_CPU_WR_MEM(VRC3);
	EXTCL_SAVE_MAPPER(VRC3);
	EXTCL_CPU_EVERY_CYCLE(VRC3);
	mapper.intStruct[0] = (BYTE *) &vrc3;
	mapper.intStructSize[0] = sizeof(vrc3);

	info.prgRamPlus8kCount = 1;

	if (info.reset) {
		memset(&vrc3, 0x00, sizeof(vrc3));
		vrc3.mask = 0xFFFF;
	}
}
void extcl_cpu_wr_mem_VRC3(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
			vrc3.reload = (vrc3.reload & 0xFFF0) | (value & 0x0F);
			return;
		case 0x9000:
			vrc3.reload = (vrc3.reload & 0xFF0F) | ((value & 0x0F) << 4);
			return;
		case 0xA000:
			vrc3.reload = (vrc3.reload & 0xF0FF) | ((value & 0x0F) << 8);
			return;
		case 0xB000:
			vrc3.reload = (vrc3.reload & 0x0FFF) | ((value & 0x0F) << 12);
			return;
		case 0xC000:
			vrc3.acknowledge = value & 0x01;
			vrc3.enabled = value & 0x02;
			vrc3.mode = value & 0x04;
			vrc3.mask = 0xFFFF;
			if (vrc3.mode) {
				vrc3.mask = 0x00FF;
			}
			if (vrc3.enabled) {
				vrc3.count = vrc3.reload;
			}
			irq.high &= ~EXTIRQ;
			return;
		case 0xD000:
			vrc3.enabled = vrc3.acknowledge;
			irq.high &= ~EXTIRQ;
			return;
		case 0xF000:
			controlBankWithAND(0x0F, prgRom16kMax)
			mapPrgRom8k(2, 0, value);
			mapPrgRom8kUpdate();
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_VRC3(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, vrc3.enabled);
	savestateEle(mode, slot, vrc3.reload);
	savestateEle(mode, slot, vrc3.mode);
	savestateEle(mode, slot, vrc3.acknowledge);
	savestateEle(mode, slot, vrc3.mask);
	savestateEle(mode, slot, vrc3.count);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_VRC3(void) {
	if (!vrc3.enabled) {
		return;
	}

	if (!(++vrc3.count & vrc3.mask)) {
		vrc3.count = vrc3.reload;
		irq.delay = TRUE;
		irq.high |= EXTIRQ;
	}
}

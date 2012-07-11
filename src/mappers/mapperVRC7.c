/*
 * mapperVRC7.c
 *
 *  Created on: 24/gen/2012
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "mappers.h"
#include "memmap.h"
#include "cpu6502.h"
#include "savestate.h"
#include "mapperVRC7snd.h"

const WORD tableVRC7[2][4] = {
	{0x0000, 0x0002, 0x0001, 0x0003},
	{0x0000, 0x0001, 0x0002, 0x0003},
};

WORD prgRom8kMax, chrRom1kMax, mask;
BYTE type, delay;

void mapInit_VRC7(BYTE revision) {
	prgRom8kMax = info.prgRom8kCount - 1;
	chrRom1kMax = info.chrRom1kCount - 1;

	EXTCLCPUWRMEM(VRC7);
	EXTCLSAVEMAPPER(VRC7);
	EXTCLCPUEVERYCYCLE(VRC7);
	EXTCLSNDSTART(VRC7);
	mapper.intStruct[0] = (BYTE *) &vrc7;
	mapper.intStructSize[0] = sizeof(vrc7);

	if (info.reset >= HARD) {
		memset(&vrc7, 0x00, sizeof(vrc7));
	} else {
		vrc7.enabled = 0;
		vrc7.reload = 0;
		vrc7.mode = 0;
		vrc7.acknowledge = 0;
		vrc7.count = 0;
		vrc7.prescaler = 0;
	}

	mask = 0xF000;
	if (revision == VRC7A) {
		mask = 0xF020;
	}

	delay = 1;

	type = revision;
}
void extclCpuWrMem_VRC7(WORD address, BYTE value) {
	address = (address & mask) | tableVRC7[type][(address & 0x0018) >> 3];

	switch (address) {
		case 0x8000:
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, 0, value);
			mapPrgRom8kUpdate();
			return;
		case 0x8001:
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, 1, value);
			mapPrgRom8kUpdate();
			return;
		case 0x9000:
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, 2, value);
			mapPrgRom8kUpdate();
			return;
		case 0x9001:   // 0x9010
			vrc7.reg = value;
			return;
		case 0x9021:   // 0x9030
			opll_write_reg(vrc7.reg, value);
			return;
		case 0xA000:
            controlBank(chrRom1kMax)
            chr.bank1k[0] = &chr.data[value << 10];
			return;
		case 0xA001:
            controlBank(chrRom1kMax)
            chr.bank1k[1] = &chr.data[value << 10];
			return;
		case 0xB000:
            controlBank(chrRom1kMax)
            chr.bank1k[2] = &chr.data[value << 10];
			return;
		case 0xB001:
            controlBank(chrRom1kMax)
            chr.bank1k[3] = &chr.data[value << 10];
			return;
		case 0xC000:
            controlBank(chrRom1kMax)
            chr.bank1k[4] = &chr.data[value << 10];
			return;
		case 0xC001:
            controlBank(chrRom1kMax)
            chr.bank1k[5] = &chr.data[value << 10];
			return;
		case 0xD000:
            controlBank(chrRom1kMax)
            chr.bank1k[6] = &chr.data[value << 10];
			return;
		case 0xD001:
            controlBank(chrRom1kMax)
            chr.bank1k[7] = &chr.data[value << 10];
			return;
		case 0xE000: {
			switch (value & 0x03) {
				case 0:
					mirroring_V();
					break;
				case 1:
					mirroring_H();
					break;
				case 2:
					mirroring_SCR0();
					break;
				case 3:
					mirroring_SCR1();
					break;
			}
			return;
		}
		case 0xE001:
			vrc7.reload = value;
			return;
		case 0xF000:
			vrc7.acknowledge = value & 0x01;
			vrc7.enabled = value & 0x02;
			vrc7.mode = value & 0x04;
			if (vrc7.enabled) {
				vrc7.prescaler = 0;
				vrc7.count = vrc7.reload;
			}
			irq.high &= ~EXTIRQ;
			return;
		case 0xF001:
			vrc7.enabled = vrc7.acknowledge;
			irq.high &= ~EXTIRQ;
			return;
		default:
			return;
	}
}
BYTE extclSaveMapper_VRC7(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, vrc7.reg);
	savestateEle(mode, slot, vrc7.enabled);
	savestateEle(mode, slot, vrc7.reload);
	savestateEle(mode, slot, vrc7.mode);
	savestateEle(mode, slot, vrc7.acknowledge);
	savestateEle(mode, slot, vrc7.count);
	savestateEle(mode, slot, vrc7.prescaler);
	savestateEle(mode, slot, vrc7.delay);

	if (opll_save(mode, slot, fp) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
void extclCPUEveryCycle_VRC7(void) {
	if (vrc7.delay && !(--vrc7.delay)) {
		irq.high |= EXTIRQ;
	}

	if (!vrc7.enabled) {
		return;
	}

	if (!vrc7.mode) {
		if (vrc7.prescaler < 338) {
			vrc7.prescaler += 3;
			return;
		}
		vrc7.prescaler -= 338;
	}

	if (vrc7.count != 0xFF) {
		vrc7.count++;
		return;
	}

	vrc7.count = vrc7.reload;
	vrc7.delay = delay;
}
void extclSndStart_VRC7(WORD samplarate) {
	opll_reset(3579545, samplarate);
}

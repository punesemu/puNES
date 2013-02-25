/*
 * mapperVRC6.c
 *
 *  Created on: 22/gen/2012
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "cpu6502.h"
#include "savestate.h"

/* vecchia versione
#define vcr6SquareTick(square)\
	vrc6.square.output = 0;\
	if (--vrc6.square.timer == 0) {\
		vrc6.square.step = (vrc6.square.step + 1) & 0x0F;\
		vrc6.square.timer = vrc6.square.frequency + 1;\
	}\
	if (vrc6.square.enabled) {\
		vrc6.square.output = 0;\
		if (vrc6.square.mode || (vrc6.square.step <= vrc6.square.duty)) {\
			vrc6.square.output = vrc6.square.volume;\
		}\
	}
*/
#define vcr6SquareTick(square)\
	if (--vrc6.square.timer == 0) {\
		vrc6.square.step = (vrc6.square.step + 1) & 0x0F;\
		vrc6.square.timer = vrc6.square.frequency + 1;\
		if (vrc6.square.enabled) {\
			vrc6.square.output = 0;\
			if (vrc6.square.mode || (vrc6.square.step <= vrc6.square.duty)) {\
				vrc6.square.output = vrc6.square.volume;\
			}\
		}\
		vrc6.square.clocked = TRUE;\
	}\
	if (!vrc6.square.enabled) {\
		vrc6.square.output = 0;\
	}

#define savestateSquareVrc6(square)\
	savestateEle(mode, slot, square.enabled);\
	savestateEle(mode, slot, square.duty);\
	savestateEle(mode, slot, square.step);\
	savestateEle(mode, slot, square.volume);\
	savestateEle(mode, slot, square.mode);\
	savestateEle(mode, slot, square.timer);\
	savestateEle(mode, slot, square.frequency);\
	savestateEle(mode, slot, square.output)

WORD prgRom16kMax, prgRom8kMax, chrRom1kMax;
BYTE type, delay;

const WORD tableVRC6[2][4] = {
	{0x0000, 0x0001, 0x0002, 0x0003},
	{0x0000, 0x0002, 0x0001, 0x0003},
};

void mapInit_VRC6(BYTE revision) {
	prgRom16kMax = info.prgRom16kCount - 1;
	prgRom8kMax = info.prgRom8kCount - 1;
	chrRom1kMax = info.chrRom1kCount - 1;

	EXTCL_CPU_WR_MEM(VRC6);
	EXTCL_SAVE_MAPPER(VRC6);
	EXTCL_CPU_EVERY_CYCLE(VRC6);
	EXTCL_APU_TICK(VRC6);
	mapper.intStruct[0] = (BYTE *) &vrc6;
	mapper.intStructSize[0] = sizeof(vrc6);

	if (info.reset >= HARD) {
		memset(&vrc6, 0x00, sizeof(vrc6));
	} else {
		vrc6.enabled = 0;
		vrc6.reload = 0;
		vrc6.mode = 0;
		vrc6.acknowledge = 0;
		vrc6.count = 0;
		vrc6.prescaler = 0;
	}

	vrc6.S3.timer = 1;
	vrc6.S3.duty = 1;
	vrc6.S4.timer = 1;
	vrc6.S4.duty = 1;
	vrc6.saw.timer = 1;
	delay = 1;

	type = revision;
}
void extcl_cpu_wr_mem_VRC6(WORD address, BYTE value) {
	address = (address & 0xF000) | tableVRC6[type][(address & 0x0003)];

	switch (address) {
		case 0x8000:
		case 0x8001:
		case 0x8002:
		case 0x8003:
			controlBank(prgRom16kMax)
			mapPrgRom8k(2, 0, value);
			mapPrgRom8kUpdate();
			return;
		case 0x9000:
			vrc6.S3.volume = value & 0x0F;
			vrc6.S3.duty = (value & 0x70) >> 4;
			vrc6.S3.mode = value & 0x80;
			return;
		case 0x9001:
			vrc6.S3.frequency = (vrc6.S3.frequency & 0x0F00) | value;
			return;
		case 0x9002:
			vrc6.S3.frequency = (vrc6.S3.frequency & 0x00FF) | ((value & 0x0F) << 8);
			vrc6.S3.enabled = value & 0x80;
			return;
		case 0xA000:
			vrc6.S4.volume = value & 0x0F;
			vrc6.S4.duty = (value & 0x70) >> 4;
			vrc6.S4.mode = value & 0x80;
			return;
		case 0xA001:
			vrc6.S4.frequency = (vrc6.S4.frequency & 0x0F00) | value;
			return;
		case 0xA002:
			vrc6.S4.frequency = (vrc6.S4.frequency & 0x00FF) | ((value & 0x0F) << 8);
			vrc6.S4.enabled = value & 0x80;
			return;
		case 0xB000:
			vrc6.saw.accumulator = value & 0x3F;
			return;
		case 0xB001:
			vrc6.saw.frequency = (vrc6.saw.frequency & 0x0F00) | value;
			return;
		case 0xB002:
			vrc6.saw.frequency = (vrc6.saw.frequency & 0x00FF) | ((value & 0x0F) << 8);
			vrc6.saw.enabled = value & 0x80;
			return;
		case 0xB003: {
			switch ((value >> 2) & 0x03) {
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
		case 0xC000:
		case 0xC001:
		case 0xC002:
		case 0xC003:
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, 2, value);
			mapPrgRom8kUpdate();
			return;
		case 0xD000:
            controlBank(chrRom1kMax)
            chr.bank1k[0] = &chr.data[value << 10];
			return;
		case 0xD001:
            controlBank(chrRom1kMax)
            chr.bank1k[1] = &chr.data[value << 10];
			return;
		case 0xD002:
            controlBank(chrRom1kMax)
            chr.bank1k[2] = &chr.data[value << 10];
			return;
		case 0xD003:
            controlBank(chrRom1kMax)
            chr.bank1k[3] = &chr.data[value << 10];
			return;
		case 0xE000:
            controlBank(chrRom1kMax)
            chr.bank1k[4] = &chr.data[value << 10];
			return;
		case 0xE001:
            controlBank(chrRom1kMax)
            chr.bank1k[5] = &chr.data[value << 10];
			return;
		case 0xE002:
            controlBank(chrRom1kMax)
            chr.bank1k[6] = &chr.data[value << 10];
			return;
		case 0xE003:
            controlBank(chrRom1kMax)
            chr.bank1k[7] = &chr.data[value << 10];
			return;
		case 0xF000:
			vrc6.reload = value;
			return;
		case 0xF001:
			vrc6.acknowledge = value & 0x01;
			vrc6.enabled = value & 0x02;
			vrc6.mode = value & 0x04;
			if (vrc6.enabled) {
				vrc6.prescaler = 0;
				vrc6.count = vrc6.reload;
			}
			irq.high &= ~EXTIRQ;
			return;
		case 0xF002:
			vrc6.enabled = vrc6.acknowledge;
			irq.high &= ~EXTIRQ;
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_VRC6(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, vrc6.enabled);
	savestateEle(mode, slot, vrc6.reload);
	savestateEle(mode, slot, vrc6.mode);
	savestateEle(mode, slot, vrc6.acknowledge);
	savestateEle(mode, slot, vrc6.count);
	savestateEle(mode, slot, vrc6.prescaler);
	savestateEle(mode, slot, vrc6.delay);

	savestateSquareVrc6(vrc6.S3)
	savestateSquareVrc6(vrc6.S4)

	savestateEle(mode, slot, vrc6.saw.enabled);
	savestateEle(mode, slot, vrc6.saw.accumulator);
	savestateEle(mode, slot, vrc6.saw.step);
	savestateEle(mode, slot, vrc6.saw.internal);
	savestateEle(mode, slot, vrc6.saw.timer);
	savestateEle(mode, slot, vrc6.saw.frequency);
	savestateEle(mode, slot, vrc6.saw.output);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_VRC6(void) {
	if (vrc6.delay && !(--vrc6.delay)) {
		irq.high |= EXTIRQ;
	}

	if (!vrc6.enabled) {
		return;
	}

	if (!vrc6.mode) {
		if (vrc6.prescaler < 338) {
			vrc6.prescaler += 3;
			return;
		}
		vrc6.prescaler -= 338;
	}

	if (vrc6.count != 0xFF) {
		vrc6.count++;
		return;
	}

	vrc6.count = vrc6.reload;
	vrc6.delay = delay;
}
void extcl_apu_tick_VRC6(void) {
	vcr6SquareTick(S3)
	vcr6SquareTick(S4)

	if (--vrc6.saw.timer == 0) {
		vrc6.saw.timer = vrc6.saw.frequency + 1;
		vrc6.saw.clocked = TRUE;

		if (vrc6.saw.step && !(vrc6.saw.step & 0x01)) {
			vrc6.saw.internal += vrc6.saw.accumulator;
		}
		if (++vrc6.saw.step == 14) {
			vrc6.saw.internal = vrc6.saw.step = 0;
		}
		if (vrc6.saw.enabled) {
			vrc6.saw.output = vrc6.saw.internal;
		}
	}
}

/*
 * mapper_VRC6.c
 *
 *  Created on: 22/gen/2012
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

/* vecchia versione
#define vcr6_square_tick(square)\
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
#define vcr6_square_tick(square)\
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

#define vrc6_square_saveslot(square)\
	save_slot_ele(mode, slot, square.enabled);\
	save_slot_ele(mode, slot, square.duty);\
	save_slot_ele(mode, slot, square.step);\
	save_slot_ele(mode, slot, square.volume);\
	save_slot_ele(mode, slot, square.mode);\
	save_slot_ele(mode, slot, square.timer);\
	save_slot_ele(mode, slot, square.frequency);\
	save_slot_ele(mode, slot, square.output)

WORD prg_rom_16k_max, prg_rom_8k_max, chr_rom_1k_max;
BYTE type, delay;

const WORD table_VRC6[2][4] = {
	{0x0000, 0x0001, 0x0002, 0x0003},
	{0x0000, 0x0002, 0x0001, 0x0003},
};

void map_init_VRC6(BYTE revision) {
	prg_rom_16k_max = info.prg.rom.banks_16k - 1;
	prg_rom_8k_max = info.prg.rom.banks_8k - 1;
	chr_rom_1k_max = info.chr.rom.banks_1k - 1;

	EXTCL_CPU_WR_MEM(VRC6);
	EXTCL_SAVE_MAPPER(VRC6);
	EXTCL_CPU_EVERY_CYCLE(VRC6);
	EXTCL_APU_TICK(VRC6);
	mapper.internal_struct[0] = (BYTE *) &vrc6;
	mapper.internal_struct_size[0] = sizeof(vrc6);

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
	address = (address & 0xF000) | table_VRC6[type][(address & 0x0003)];

	switch (address) {
		case 0x8000:
		case 0x8001:
		case 0x8002:
		case 0x8003:
			control_bank(prg_rom_16k_max)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k_update();
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
			control_bank(prg_rom_8k_max)
			map_prg_rom_8k(1, 2, value);
			map_prg_rom_8k_update();
			return;
		case 0xD000:
			control_bank(chr_rom_1k_max)
			chr.bank_1k[0] = &chr.data[value << 10];
			return;
		case 0xD001:
			control_bank(chr_rom_1k_max)
			chr.bank_1k[1] = &chr.data[value << 10];
			return;
		case 0xD002:
			control_bank(chr_rom_1k_max)
			chr.bank_1k[2] = &chr.data[value << 10];
			return;
		case 0xD003:
			control_bank(chr_rom_1k_max)
			chr.bank_1k[3] = &chr.data[value << 10];
			return;
		case 0xE000:
			control_bank(chr_rom_1k_max)
			chr.bank_1k[4] = &chr.data[value << 10];
			return;
		case 0xE001:
			control_bank(chr_rom_1k_max)
			chr.bank_1k[5] = &chr.data[value << 10];
			return;
		case 0xE002:
			control_bank(chr_rom_1k_max)
			chr.bank_1k[6] = &chr.data[value << 10];
			return;
		case 0xE003:
			control_bank(chr_rom_1k_max)
			chr.bank_1k[7] = &chr.data[value << 10];
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
			irq.high &= ~EXT_IRQ;
			return;
		case 0xF002:
			vrc6.enabled = vrc6.acknowledge;
			irq.high &= ~EXT_IRQ;
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_VRC6(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, vrc6.enabled);
	save_slot_ele(mode, slot, vrc6.reload);
	save_slot_ele(mode, slot, vrc6.mode);
	save_slot_ele(mode, slot, vrc6.acknowledge);
	save_slot_ele(mode, slot, vrc6.count);
	save_slot_ele(mode, slot, vrc6.prescaler);
	save_slot_ele(mode, slot, vrc6.delay);

	vrc6_square_saveslot(vrc6.S3)
	vrc6_square_saveslot(vrc6.S4)

	save_slot_ele(mode, slot, vrc6.saw.enabled);
	save_slot_ele(mode, slot, vrc6.saw.accumulator);
	save_slot_ele(mode, slot, vrc6.saw.step);
	save_slot_ele(mode, slot, vrc6.saw.internal);
	save_slot_ele(mode, slot, vrc6.saw.timer);
	save_slot_ele(mode, slot, vrc6.saw.frequency);
	save_slot_ele(mode, slot, vrc6.saw.output);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_VRC6(void) {
	if (vrc6.delay && !(--vrc6.delay)) {
		irq.high |= EXT_IRQ;
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
	vcr6_square_tick(S3)
	vcr6_square_tick(S4)

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

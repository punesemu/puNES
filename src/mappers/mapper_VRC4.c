/*
 * mapper_VRC4.c
 *
 *  Created on: 10/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

#define chr_rom_1k_update(slot, mask, shift)\
	value = (vrc4.chr_rom_bank[slot] & mask) | ((value & 0x0F) << shift);\
	control_bank(chr_rom_1k_max)\
	chr.bank_1k[slot] = &chr.data[value << 10];\
	vrc4.chr_rom_bank[slot] = value

WORD prg_rom_8k_max, prg_rom_8k_before_last, chr_rom_1k_max;
BYTE type;

const BYTE shift_VRC4[5] = { 0x01, 0x00, 0x06, 0x02, 0x02 };
const WORD mask_VRC4[5]  = { 0x0006, 0x0003, 0x00C0, 0x000C, 0x000C };
const WORD table_VRC4[5][4] = {
	{0x0000, 0x0001, 0x0002, 0x0003},
	{0x0000, 0x0002, 0x0001, 0x0003},
	{0x0000, 0x0001, 0x0002, 0x0003},
	{0x0000, 0x0002, 0x0001, 0x0003},
	{0x0000, 0x0001, 0x0002, 0x0003},
};

void map_init_VRC4(BYTE revision) {
	prg_rom_8k_max = info.prg_rom_8k_count - 1;
	prg_rom_8k_before_last = prg_rom_8k_max - 1;
	chr_rom_1k_max = info.chr_rom_1k_count - 1;

	EXTCL_CPU_WR_MEM(VRC4);
	EXTCL_SAVE_MAPPER(VRC4);
	EXTCL_CPU_EVERY_CYCLE(VRC4);
	mapper.internal_struct[0] = (BYTE *) &vrc4;
	mapper.internal_struct_size[0] = sizeof(vrc4);

	if (info.reset >= HARD) {
		BYTE i;

		memset(&vrc4, 0x00, sizeof(vrc4));
		for (i = 0; i < 8; i++) {
			vrc4.chr_rom_bank[i] = i;
		}
	} else {
		vrc4.irq_enabled = 0;
		vrc4.irq_reload = 0;
		vrc4.irq_mode = 0;
		vrc4.irq_acknowledge = 0;
		vrc4.irq_count = 0;
		vrc4.irq_prescaler = 0;
	}

	type = revision;
}
void extcl_cpu_wr_mem_VRC4(WORD address, BYTE value) {
	WORD tmp = address & 0xF000;

	if ((tmp == 0x8000) || (tmp == 0xA000)) {
		address &= 0xF000;
	} else {
		address = (address & 0xF000)
		        | table_VRC4[type][(address & mask_VRC4[type]) >> shift_VRC4[type]];
	}

	switch (address) {
		case 0x8000:
			control_bank_with_AND(0x1F, prg_rom_8k_max)
			map_prg_rom_8k(1, vrc4.swap_mode, value);
			map_prg_rom_8k(1, 0x02 >> vrc4.swap_mode, prg_rom_8k_before_last);
			map_prg_rom_8k_update();
			return;
		case 0xA000:
			control_bank_with_AND(0x1F, prg_rom_8k_max)
			map_prg_rom_8k(1, 1, value);
			map_prg_rom_8k_update();
			return;
		case 0x9000:
		case 0x9001: {
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
		case 0x9002:
		case 0x9003:
			value &= 0x02;
			if (vrc4.swap_mode != value) {
				WORD swap = mapper.rom_map_to[0];

				mapper.rom_map_to[0] = mapper.rom_map_to[2];
				mapper.rom_map_to[2] = swap;
				map_prg_rom_8k_update();
				vrc4.swap_mode = value;
			}
			return;
		case 0xB000:
			chr_rom_1k_update(0, 0xF0, 0);
			return;
		case 0xB001:
			chr_rom_1k_update(0, 0x0F, 4);
			return;
		case 0xB002:
			chr_rom_1k_update(1, 0xF0, 0);
			return;
		case 0xB003:
			chr_rom_1k_update(1, 0x0F, 4);
			return;
		case 0xC000:
			chr_rom_1k_update(2, 0xF0, 0);
			return;
		case 0xC001:
			chr_rom_1k_update(2, 0x0F, 4);
			return;
		case 0xC002:
			chr_rom_1k_update(3, 0xF0, 0);
			return;
		case 0xC003:
			chr_rom_1k_update(3, 0x0F, 4);
			return;
		case 0xD000:
			chr_rom_1k_update(4, 0xF0, 0);
			return;
		case 0xD001:
			chr_rom_1k_update(4, 0x0F, 4);
			return;
		case 0xD002:
			chr_rom_1k_update(5, 0xF0, 0);
			return;
		case 0xD003:
			chr_rom_1k_update(5, 0x0F, 4);
			return;
		case 0xE000:
			chr_rom_1k_update(6, 0xF0, 0);
			return;
		case 0xE001:
			chr_rom_1k_update(6, 0x0F, 4);
			return;
		case 0xE002:
			chr_rom_1k_update(7, 0xF0, 0);
			return;
		case 0xE003:
			chr_rom_1k_update(7, 0x0F, 4);
			return;
		case 0xF000:
			vrc4.irq_reload = (vrc4.irq_reload & 0xF0) | (value & 0x0F);
			return;
		case 0xF001:
			vrc4.irq_reload = (vrc4.irq_reload & 0x0F) | ((value & 0x0F) << 4);
			return;
		case 0xF002:
			vrc4.irq_acknowledge = value & 0x01;
			vrc4.irq_enabled = value & 0x02;
			vrc4.irq_mode = value & 0x04;
			if (vrc4.irq_enabled) {
				vrc4.irq_prescaler = 0;
				vrc4.irq_count = vrc4.irq_reload;
			}
			irq.high &= ~EXT_IRQ;
			return;
		case 0xF003:
			vrc4.irq_enabled = vrc4.irq_acknowledge;
			irq.high &= ~EXT_IRQ;
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_VRC4(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, vrc4.chr_rom_bank);
	save_slot_ele(mode, slot, vrc4.swap_mode);
	save_slot_ele(mode, slot, vrc4.irq_enabled);
	save_slot_ele(mode, slot, vrc4.irq_reload);
	save_slot_ele(mode, slot, vrc4.irq_mode);
	save_slot_ele(mode, slot, vrc4.irq_acknowledge);
	save_slot_ele(mode, slot, vrc4.irq_count);
	save_slot_ele(mode, slot, vrc4.irq_prescaler);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_VRC4(void) {
	if (!vrc4.irq_enabled) {
		return;
	}

	if (!vrc4.irq_mode) {
		if (vrc4.irq_prescaler < 338) {
			vrc4.irq_prescaler += 3;
			return;
		}
		vrc4.irq_prescaler -= 338;
	}

	if (vrc4.irq_count != 0xFF) {
		vrc4.irq_count++;
		return;
	}

	vrc4.irq_count = vrc4.irq_reload;
	irq.delay = TRUE;
	irq.high |= EXT_IRQ;
}

void map_init_VRC4BMC(void) {
	prg_rom_8k_max = info.prg_rom_8k_count - 1;

	map_init_VRC4(VRC4E);

	EXTCL_CPU_WR_MEM(VRC4BMC);
}
void extcl_cpu_wr_mem_VRC4BMC(WORD address, BYTE value) {
	if (address < 0x6000) {
		return;
	}

	if ((address >= 0x8000) && (address <= 0x8FFF)) {
		value = (mapper.rom_map_to[0] & 0x20) | (value & 0x1F);
		control_bank(prg_rom_8k_max)
		map_prg_rom_8k(1, vrc4.swap_mode, value);
		map_prg_rom_8k_update();
		return;
	}
	if ((address >= 0xA000) && (address <= 0xAFFF)) {
		value = (mapper.rom_map_to[0] & 0x20) | (value & 0x1F);
		control_bank(prg_rom_8k_max)
		map_prg_rom_8k(1, 1, value);
		map_prg_rom_8k_update();
		return;
	}
	if ((address >= 0xB000) && (address <= 0xEFFF)) {
		BYTE save = value << 2 & 0x20;

		value = (mapper.rom_map_to[0] & 0x1F) | save ;
		control_bank(prg_rom_8k_max)
		map_prg_rom_8k(1, 0, value);

		value = (mapper.rom_map_to[1] & 0x1F) | save ;
		control_bank(prg_rom_8k_max)
		map_prg_rom_8k(1, 1, value);

		value = (mapper.rom_map_to[2] & 0x1F) | save ;
		control_bank(prg_rom_8k_max)
		map_prg_rom_8k(1, 2, value);

		value = (mapper.rom_map_to[3] & 0x1F) | save ;
		control_bank(prg_rom_8k_max)
		map_prg_rom_8k(1, 3, value);

		map_prg_rom_8k_update();
		return;
	}

	extcl_cpu_wr_mem_VRC4(address, value);
}

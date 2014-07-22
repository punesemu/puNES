/*
 * mapper_VRC2.c
 *
 *  Created on: 10/set/2011
 *      Author: fhorse
 */

#include <string.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

#define chr_rom_1k_update(slot, mask, shift)\
	value = (vrc2.chr_rom_bank[slot] & mask) | (((value >> type) & 0x0F) << shift);\
	control_bank(info.chr.rom.max.banks_1k)\
	chr.bank_1k[slot] = chr_chip_byte_pnt(0, value << 10);\
	vrc2.chr_rom_bank[slot] = value

BYTE type;

const WORD shift_VRC2[2][4] = {
	{0x0000, 0x0001, 0x0002, 0x0003},
	{0x0000, 0x0002, 0x0001, 0x0003}
};

void map_init_VRC2(BYTE revision) {
	EXTCL_CPU_WR_MEM(VRC2);
	EXTCL_SAVE_MAPPER(VRC2);
	mapper.internal_struct[0] = (BYTE *) &vrc2;
	mapper.internal_struct_size[0] = sizeof(vrc2);

	if (info.reset >= HARD) {
		BYTE i;

		memset(&vrc2, 0x00, sizeof(vrc2));
		for (i = 0; i < 8; i++) {
			vrc2.chr_rom_bank[i] = i;
		}
	}

	type = revision;
}
void extcl_cpu_wr_mem_VRC2(WORD address, BYTE value) {
	if (address < 0xB000) {
		address &= 0xF000;
	} else {
		address = (address & 0xF000) | shift_VRC2[type][address & 0x0003];
	}

	switch (address) {
		case 0x8000:
			control_bank_with_AND(0x0F, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0x9000: {
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
		case 0xA000:
			control_bank_with_AND(0x0F, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, value);
			map_prg_rom_8k_update();
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
		default:
			return;
	}
}
BYTE extcl_save_mapper_VRC2(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, vrc2.chr_rom_bank);

	return (EXIT_OK);
}

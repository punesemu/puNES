/*
 * mapperAve.c
 *
 *  Created on: 19/set/2011
 *      Author: fhorse
 */

#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "save_slot.h"

#define ave_nina06_swap()\
{\
	const BYTE save = value;\
	DBWORD bank;\
	if (prg_rom_32k_max != 0xFFFF) {\
		value >>= 3;\
		control_bank(prg_rom_32k_max)\
		map_prg_rom_8k(4, 0, value);\
		map_prg_rom_8k_update();\
		value = save;\
	}\
	control_bank_with_AND(0x07, chr_rom_8k_max)\
	bank = value << 13;\
	chr.bank_1k[0] = &chr.data[bank];\
	chr.bank_1k[1] = &chr.data[bank | 0x0400];\
	chr.bank_1k[2] = &chr.data[bank | 0x0800];\
	chr.bank_1k[3] = &chr.data[bank | 0x0C00];\
	chr.bank_1k[4] = &chr.data[bank | 0x1000];\
	chr.bank_1k[5] = &chr.data[bank | 0x1400];\
	chr.bank_1k[6] = &chr.data[bank | 0x1800];\
	chr.bank_1k[7] = &chr.data[bank | 0x1C00];\
}

WORD prg_rom_32k_max, chr_rom_8k_max;

void map_init_Ave(BYTE model) {
	prg_rom_32k_max = (info.prg_rom_16k_count >> 1) - 1;
	chr_rom_8k_max = info.chr_rom_8k_count - 1;

	switch (model) {
		case NINA06:
			EXTCL_CPU_WR_MEM(Ave_NINA06);

			info.mapper_extend_wr = TRUE;

			if (info.reset >= HARD) {
				if (prg_rom_32k_max != 0xFFFF) {
					map_prg_rom_8k(4, 0, 0);
				}
			}
			break;
		case D1012:
			EXTCL_CPU_WR_MEM(Ave_D1012);
			EXTCL_CPU_RD_MEM(Ave_D1012);
			EXTCL_SAVE_MAPPER(Ave_D1012);
			mapper.internal_struct[0] = (BYTE *) &ave_d1012;
			mapper.internal_struct_size[0] = sizeof(ave_d1012);

			info.mapper_extend_rd = TRUE;

			if (info.reset >= HARD) {
				map_prg_rom_8k(4, 0, 0);
				memset(&ave_d1012, 0x00, sizeof(ave_d1012));
			}

			mirroring_V();

			break;
	}
}

void extcl_cpu_wr_mem_Ave_NINA06(WORD address, BYTE value) {
	if ((address < 0x4100) || (address >= 0x6000)) {
		if (info.id != PUZZLEUNL) {
			return;
		}
	}

	switch (address & 0x0100) {
		case 0x0000:
			if (info.id != PUZZLEUNL) {
				return;
			}
			ave_nina06_swap()
			return;
		case 0x0100:
			ave_nina06_swap()
			return;
	}
}

void extcl_cpu_wr_mem_Ave_D1012(WORD address, BYTE value) {
	DBWORD bank;

	if (address < 0xFF80) {
		return;
	}

	switch (address & 0x00F8) {
		case 0x0080:
		case 0x0088:
		case 0x0090:
		case 0x0098:
			if (!(ave_d1012.reg[0] & 0x3F)) {
				ave_d1012.reg[0] = value;
				if (value & 0x80) {
					mirroring_H();
				} else {
					mirroring_V();
				}
				break;
			}
			return;
		case 0x00C0:
		case 0x00C8:
		case 0x00D0:
		case 0x00D8:
			if (!ave_d1012.reg[2]) {
				ave_d1012.reg[2] = value;
			}
			return;
		case 0x00E8:
		case 0x00F0:
			ave_d1012.reg[1] = value;
			break;
	}

	value = (ave_d1012.reg[0] & 0xE) | (ave_d1012.reg[(ave_d1012.reg[0] >> 6) & 0x1] & 0x1);
	control_bank(prg_rom_32k_max)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();

	value = ((ave_d1012.reg[0] << 2) & (((ave_d1012.reg[0] >> 4) & 0x4) ^ 0x3C))
        		| ((ave_d1012.reg[1] >> 4) & (((ave_d1012.reg[0] >> 4) & 0x4) | 0x3));
	control_bank(chr_rom_8k_max)
	bank = value << 13;
	chr.bank_1k[0] = &chr.data[bank];
	chr.bank_1k[1] = &chr.data[bank | 0x0400];
	chr.bank_1k[2] = &chr.data[bank | 0x0800];
	chr.bank_1k[3] = &chr.data[bank | 0x0C00];
	chr.bank_1k[4] = &chr.data[bank | 0x1000];
	chr.bank_1k[5] = &chr.data[bank | 0x1400];
	chr.bank_1k[6] = &chr.data[bank | 0x1800];
	chr.bank_1k[7] = &chr.data[bank | 0x1C00];
}
BYTE extcl_cpu_rd_mem_Ave_D1012(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0xFF80) || (address > 0xFFF7)) {
		return (openbus);
	}

	extcl_cpu_wr_mem_Ave_D1012(address, openbus);
	return (openbus);
}
BYTE extcl_save_mapper_Ave_D1012(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, ave_d1012.reg);

	return (EXIT_OK);
}

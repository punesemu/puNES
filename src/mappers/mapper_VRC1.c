/*
 * mapper_VRC1.c
 *
 *  Created on: 16/lug/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

void map_init_VRC1(void) {
	EXTCL_CPU_WR_MEM(VRC1);
}
void extcl_cpu_wr_mem_VRC1(WORD address, BYTE value) {
	DBWORD bank;

	address &= 0xF000;

	switch (address) {
		case 0x8000:
			control_bank_with_AND(0x0F, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0x9000:
			bank = (((value << 3) & 0x10) | (((chr.bank_1k[0] - chr_chip(0)) >> 12) & 0x0F)) << 12;
			chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
			chr.bank_1k[2] = chr_chip_byte_pnt(0, bank | 0x0800);
			chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0C00);
			bank = (((value << 2) & 0x10) | (((chr.bank_1k[4] - chr_chip(0)) >> 12) & 0x0F)) << 12;
			chr.bank_1k[4] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x0400);
			chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x0800);
			chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x0C00);
			if (value & 0x01) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			return;
		case 0xA000:
			control_bank_with_AND(0x0F, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, value);
			map_prg_rom_8k_update();
			return;
		case 0xC000:
			control_bank_with_AND(0x0F, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2, value);
			map_prg_rom_8k_update();
			return;
		case 0xE000:
			value = (((chr.bank_1k[0] - chr_chip(0)) >> 12) & 0x10) | (value & 0x0F);
			control_bank(info.chr.rom.max.banks_4k)
			bank = value << 12;
			chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
			chr.bank_1k[2] = chr_chip_byte_pnt(0, bank | 0x0800);
			chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0C00);
			return;
		case 0xF000:
			value = (((chr.bank_1k[4] - chr_chip(0)) >> 12) & 0x10) | (value & 0x0F);
			control_bank(info.chr.rom.max.banks_4k)
			bank = value << 12;
			chr.bank_1k[4] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x0400);
			chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x0800);
			chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x0C00);
			return;
		default:
			return;
	}
}

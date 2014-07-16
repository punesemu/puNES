/*
 * mapper_176.c
 *
 *  Created on: 6/ott/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

void map_init_176(void) {
	EXTCL_CPU_WR_MEM(176);

	if (info.reset >= RESET) {
		map_prg_rom_8k(4, 0, info.prg.rom.max.banks_32k);
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_176(WORD address, BYTE value) {
	switch (address) {
		case 0x5FF1:
			value >>= 1;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0x5FF2: {
			DBWORD bank;

			control_bank(info.chr.rom.max.banks_8k)
			bank = value << 13;
			chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
			chr.bank_1k[2] = chr_chip_byte_pnt(0, bank | 0x0800);
			chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0C00);
			chr.bank_1k[4] = chr_chip_byte_pnt(0, bank | 0x1000);
			chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x1400);
			chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x1800);
			chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x1C00);
			return;
		}
	}
}

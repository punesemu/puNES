/*
 * mapper_229.c
 *
 *  Created on: 05/feb/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

void map_init_229(void) {
	EXTCL_CPU_WR_MEM(229);

	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
	}
}
void extcl_cpu_wr_mem_229(WORD address, BYTE value) {
	DBWORD bank;

	value = address & 0x1F;

	if (address & 0x001E) {
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k(2, 2, value);
	} else {
		map_prg_rom_8k(4, 0, 0);
	}
	map_prg_rom_8k_update();

	if (address & 0x0020) {
		mirroring_H();
	} else {
		mirroring_V();
	}

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
}

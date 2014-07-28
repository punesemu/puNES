/*
 * mapper_200.c
 *
 *  Created on: 21/mar/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "info.h"
#include "mem_map.h"

void map_init_200(void) {
	EXTCL_CPU_WR_MEM(200);

	if (info.reset >= HARD) {
		extcl_cpu_wr_mem_200(0x8000, 0);
	}
}
void extcl_cpu_wr_mem_200(WORD address, BYTE value) {
	BYTE save = (address & 0xFF);
	DBWORD bank;

	value = save;
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);
	map_prg_rom_8k(2, 2, value);
	map_prg_rom_8k_update();

	value = save;
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

	if (address & 0x0008) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}

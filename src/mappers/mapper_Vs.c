/*
 * mapper_Vs.c
 *
 *  Created on: 22/set/2010
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

void map_init_Vs(void) {
	EXTCL_CPU_WR_MEM(Vs);
	EXTCL_CPU_WR_R4016(Vs);

	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
	}
}
void extcl_cpu_wr_mem_Vs(WORD address, BYTE value) {
	return;
}
void extcl_cpu_wr_r4016_Vs(BYTE value) {
	const BYTE save = value;
	DBWORD bank;

	value &= 0x04;
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(2, 0, value);
	map_prg_rom_8k_update();

	value = save >> 2;
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

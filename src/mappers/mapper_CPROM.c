/*
 * mapper_CPROM.c
 *
 *  Created on: 14/lug/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

void map_init_CPROM(void) {
	/* forzo i numeri di banchi della chr rom */
	info.chr.rom.banks_8k = 2;
	info.chr.rom.banks_4k = 4;
	info.chr.rom.banks_1k = 16;
	/* quindi setto nuovamente i valori massimi dei banchi */
	map_set_banks_max_prg_and_chr();

	if (info.reset >= HARD) {
		chr.bank_1k[4] = chr_chip_byte_pnt(0, 0x0000);
		chr.bank_1k[5] = chr_chip_byte_pnt(0, 0x0400);
		chr.bank_1k[6] = chr_chip_byte_pnt(0, 0x0800);
		chr.bank_1k[7] = chr_chip_byte_pnt(0, 0x0C00);
	}

	EXTCL_CPU_WR_MEM(CPROM);
}
void extcl_cpu_wr_mem_CPROM(WORD address, BYTE value) {
	DBWORD bank;

	/* bus conflict */
	value &= prg_rom_rd(address);

	control_bank_with_AND(0x03, info.chr.rom.max.banks_4k)
	bank = value << 12;
	chr.bank_1k[4] = chr_chip_byte_pnt(0, bank);
	chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x0400);
	chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x0800);
	chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x0C00);
}

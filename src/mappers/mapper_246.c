/*
 * mapper_246.c
 *
 *  Created on: 24/apr/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

void map_init_246(void) {
	EXTCL_CPU_WR_MEM(246);
	EXTCL_CPU_RD_MEM(246);

	if (info.reset >= HARD) {
		map_prg_rom_8k_reset();
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_246(WORD address, BYTE value) {
	BYTE reg, slot;
	DBWORD bank;

	if ((address < 0x6000) || (address > 0x67FF)) {
		return;
	}

	reg = address & 0x07;

	if (reg < 4) {
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, reg, value);
		map_prg_rom_8k_update();
		return;
	}

	slot = (reg - 4) << 1;
	control_bank(info.chr.rom.max.banks_2k)
	bank = value << 11;
	chr.bank_1k[slot] = &chr.data[bank];
	chr.bank_1k[slot + 1] = &chr.data[bank | 0x0400];
}
BYTE extcl_cpu_rd_mem_246(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x67FF)) {
		return (openbus);
	}

	return(before);
}

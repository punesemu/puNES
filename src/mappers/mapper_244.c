/*
 * mapper_244.c
 *
 *  Created on: 24/mar/2012
 *      Author: fhorse
 */

#include <stdio.h>

#include "mappers.h"
#include "mem_map.h"

void map_init_244(void) {
	EXTCL_CPU_WR_MEM(244);

	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
	}
}
void extcl_cpu_wr_mem_244(WORD address, BYTE value) {
	if ((address >= 0x8065) && (address <= 0x80A4)) {
		value = (address - 0x8065) & 0x03;
		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
		map_prg_rom_8k_update();
	};
	if ((address >= 0x80A5) && (address <= 0x80E4)) {
		DBWORD bank;

		value = (address - 0x80A5) & 0x07;
		control_bank(info.chr.rom.max.banks_8k)
		bank = value << 13;
		chr.bank_1k[0] = &chr.data[bank];
		chr.bank_1k[1] = &chr.data[bank | 0x0400];
		chr.bank_1k[2] = &chr.data[bank | 0x0800];
		chr.bank_1k[3] = &chr.data[bank | 0x0C00];
		chr.bank_1k[4] = &chr.data[bank | 0x1000];
		chr.bank_1k[5] = &chr.data[bank | 0x1400];
		chr.bank_1k[6] = &chr.data[bank | 0x1800];
		chr.bank_1k[7] = &chr.data[bank | 0x1C00];
	};
}

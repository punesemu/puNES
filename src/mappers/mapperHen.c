/*
 * mapperHen.c
 *
 *  Created on: 6/ott/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom32kMax;
BYTE type;

void map_init_Hen(BYTE model) {
	prgRom32kMax = (info.prg_rom_16k_count >> 1) - 1;

	switch (model) {
		case HEN177:
		case HENFANKONG:
			EXTCL_CPU_WR_MEM(Hen_177);
			break;
		case HENXJZB:
			EXTCL_CPU_WR_MEM(Hen_xjzb);
			info.mapper_extend_wr = TRUE;
			break;
	}

	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
	}

	type = model;
}

void extcl_cpu_wr_mem_Hen_177(WORD address, BYTE value) {
	if (type != HENFANKONG) {
		if (value & 0x20) {
			mirroring_H();
		} else {
			mirroring_V();
		}
	}

	control_bank(prgRom32kMax)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}

void extcl_cpu_wr_mem_Hen_xjzb(WORD address, BYTE value) {
	if ((address < 0x5000) || (address > 0x5FFF)) {
		return;
	}

	value >>= 1;
	control_bank(prgRom32kMax)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}

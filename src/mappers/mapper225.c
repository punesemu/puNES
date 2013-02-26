/*
 * mapper225.c
 *
 *  Created on: 06/feb/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

WORD prgRom32kMax, prgRom16kMax, chrRom8kMax;

void map_init_225(void) {
	prgRom32kMax = (info.prg_rom_16k_count >> 1) - 1;
	prgRom16kMax = info.prg_rom_16k_count - 1;
	chrRom8kMax = info.chr_rom_8k_count - 1;

	EXTCL_CPU_WR_MEM(225);

	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
	}
}
void extcl_cpu_wr_mem_225(WORD address, BYTE value) {
	DBWORD bank;

    value = (address >> 7) & 0x1F;

	if (prgRom32kMax > 0x1F) {
		value |= ((address >> 9) & 0x20);
	}

    if (address & 0x1000) {
    	value = (value << 1) | ((address >> 6) & 0x01);
    	control_bank(prgRom16kMax)
    	map_prg_rom_8k(2, 0, value);
    	map_prg_rom_8k(2, 2, value);
    } else {
		control_bank(prgRom32kMax)
		map_prg_rom_8k(4, 0, value);
    }
	map_prg_rom_8k_update();

	if (address & 0x2000) {
		mirroring_H();
		/*
		 * workaround per far funzionare correttamente il 52esimo gioco
		 * della rom "52 Games [p1].nes" che non utilizza ne il mirroring
		 * verticale ne quello orizzontale.
		 */
		if ((info.id == BMC52IN1) && (address == 0xA394)) {
			mirroring_FSCR();
		}
	} else {
		mirroring_V();
	}

	value = address & 0x3F;

	if (chrRom8kMax > 0x3F) {
		value |= ((address >> 8) & 0x40);
	}

	control_bank(chrRom8kMax)
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

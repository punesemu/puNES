/*
 * mapper186.c
 *
 *  Created on: 10/ott/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "cpu.h"
#include "savestate.h"

WORD prgRom16kMax, prgRom8kMax, chrRom1kMax;

void mapInit_186(void) {
	prgRom16kMax = info.prgRom16kCount - 1;
	prgRom8kMax = info.prgRom8kCount - 1;
	chrRom1kMax = info.chrRom1kCount - 1;

	EXTCL_CPU_WR_MEM(186);
	EXTCL_CPU_RD_MEM(186);
	EXTCL_SAVE_MAPPER(186);
	mapper.intStruct[0] = (BYTE *) &m186;
	mapper.intStructSize[0] = sizeof(m186);

	info.mapperExtendWrite = TRUE;
	info.prgRamPlus8kCount = 0;
	cpu.prg_ram_wr_active = TRUE;
	cpu.prg_ram_rd_active = TRUE;

	if (info.reset >= HARD) {
		memset(&m186, 0x00, sizeof(m186));
		m186.prgRamBank2 = &prg.rom[0];
		mapPrgRom8k(2, 0, 0);
		mapPrgRom8k(2, 2, 0);
	}
}
void extcl_cpu_wr_mem_186(WORD address, BYTE value) {
	if ((address < 0x4200) || (address > 0x4EFF)) {
		return;
	}

	if (address > 0x43FF) {
		prg.ram[address & 0x0BFF] = value;
		return;
	}

	switch (address & 0x0001) {
		case 0x0000:
			value >>= 6;
			controlBank(prgRom8kMax)
			m186.prgRamBank2 = &prg.rom[value << 13];
			return;
		case 0x0001:
			controlBank(prgRom16kMax)
			mapPrgRom8k(2, 0, value);
			mapPrgRom8kUpdate();
			return;
	}
}
BYTE extcl_cpu_rd_mem_186(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x4200) || (address > 0x7FFF)) {
		return (openbus);
	}

	switch (address) {
		case 0x4200:
		case 0x4201:
		case 0x4203:
			return (0x00);
		case 0x4202:
			return (0x40);
	}

	if (address < 0x4400) {
		return (0xFF);
	}

	if (address < 0x4F00) {
		return (prg.ram[address & 0x1FFF]);
	}

	/* mi mancano informazioni per far funzionare questa mapper */
	if (address > 0x5FFF) {
		return (m186.prgRamBank2[address & 0x1FFF]);
	}

	return (openbus);
}
BYTE extcl_save_mapper_186(BYTE mode, BYTE slot, FILE *fp) {
	savestatePos(mode, slot, prg.rom, m186.prgRamBank2);

	return (EXIT_OK);
}

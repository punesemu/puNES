/*
 * mapper120.c
 *
 *  Created on: 29/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "cpu.h"
#include "savestate.h"

WORD prgRom8kMax;

void mapInit_120(void) {
	prgRom8kMax = info.prgRom8kCount - 1;

	EXTCL_CPU_WR_MEM(120);
	EXTCL_CPU_RD_MEM(120);
	EXTCL_SAVE_MAPPER(120);
	mapper.intStruct[0] = (BYTE *) &m120;
	mapper.intStructSize[0] = sizeof(m120);

	info.mapperExtendWrite = TRUE;
	cpu.prg_ram_wr_active = FALSE;
	cpu.prg_ram_rd_active = FALSE;

	if (info.reset >= HARD) {
		memset(&m120, 0x00, sizeof(m120));
		m120.prgRamRd = &prg.rom[0];
		mapPrgRom8k(4, 0, 2);
	}
}
void extcl_cpu_wr_mem_120(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return;
	}

	if ((address & 0xE3C0) == 0x41C0) {
		controlBankWithAND(0x07, prgRom8kMax)
		m120.prgRamRd = &prg.rom[value << 13];
	}
}
BYTE extcl_cpu_rd_mem_120(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (m120.prgRamRd[address & 0x1FFF]);
}
BYTE extcl_save_mapper_120(BYTE mode, BYTE slot, FILE *fp) {
	savestatePos(mode, slot, prg.rom, m120.prgRamRd);

	return (EXIT_OK);
}

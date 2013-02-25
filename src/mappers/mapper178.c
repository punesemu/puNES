/*
 * mapper178.c
 *
 *  Created on: 6/ott/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

WORD prgRom32kMax;
BYTE type;

void mapInit_178(BYTE model) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;

	EXTCL_CPU_WR_MEM(178);
	EXTCL_SAVE_MAPPER(178);
	mapper.intStruct[0] = (BYTE *) &m178;
	mapper.intStructSize[0] = sizeof(m178);

	info.mapperExtendWrite = TRUE;

	if (info.reset >= HARD) {
		memset(&m178, 0x00, sizeof(m178));
		mapPrgRom8k(4, 0, 0);
	}

	type = model;
}
void extcl_cpu_wr_mem_178(WORD address, BYTE value) {
	switch (address) {
		case 0x4800:
			if (value & 0x01) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			return;
		case 0x4801:
			value = m178.reg = (m178.reg & 0x0C) | ((value >> 1) & 0x03);
			controlBank(prgRom32kMax)
			mapPrgRom8k(4, 0, value);
			mapPrgRom8kUpdate();
			return;
		case 0x4802:
			m178.reg = ((value << 2) & 0x0C) | (m178.reg & 0x03);
			return;
		case 0x8000:
			if (type == XINGJI) {
				controlBank(prgRom32kMax)
				mapPrgRom8k(4, 0, value);
				mapPrgRom8kUpdate();
			}
			return;
	}
}
BYTE extcl_save_mapper_178(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m178.reg);

	return (EXIT_OK);
}

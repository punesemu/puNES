/*
 * mapperCamerica.c
 *
 *  Created on: 12/lug/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom16kMax;

void mapInit_Camerica(void) {
	prgRom16kMax = info.prgRom16kCount - 1;

	switch (info.mapperType) {
		case BF9096:
			EXTCL_CPU_WR_MEM(Camerica_BF9096);
			break;
		case BF9097:
			EXTCL_CPU_WR_MEM(Camerica_BF9097);
			break;
		case GOLDENFIVE:
			EXTCL_CPU_WR_MEM(Camerica_GoldenFive);
			if (info.reset >= HARD) {
				mapPrgRom8k(2, 2, 0x0F);
			}
			break;
		default:
			EXTCL_CPU_WR_MEM(Camerica_BF9093);
			break;
	}
}
void extcl_cpu_wr_mem_Camerica_BF9093(WORD address, BYTE value) {
	controlBankWithAND(0x0F, prgRom16kMax)
	mapPrgRom8k(2, 0, value);
	mapPrgRom8kUpdate();
}
void extcl_cpu_wr_mem_Camerica_BF9096(WORD address, BYTE value) {
	BYTE base;

	switch ((address >> 12) & 0x0C) {
		case 0x08:
			base = (value >> 1) & 0x0C;
			mapPrgRom8k(2, 0, base | ((mapper.romMapTo[0] & 0x07) >> 1));
			mapPrgRom8k(2, 2, base | 0x03);
			break;
		default:
			mapPrgRom8k(2, 0, ((mapper.romMapTo[0] & 0x18) >> 1) | (value & 0x03));
			break;
	}
	mapPrgRom8kUpdate();
}
void extcl_cpu_wr_mem_Camerica_BF9097(WORD address, BYTE value) {
	switch ((address >> 12) & 0x0C) {
		case 0x08:
			if (value & 0x10) {
				mirroring_SCR0();
			} else {
				mirroring_SCR1();
			}
			return;
		default:
			controlBankWithAND(0x07, prgRom16kMax)
			mapPrgRom8k(2, 0, value);
			mapPrgRom8kUpdate();
			return;
	}
}
void extcl_cpu_wr_mem_Camerica_GoldenFive(WORD address, BYTE value) {
	BYTE base;

	switch ((address >> 12) & 0x0C) {
		case 0x08:
			if (value & 0x08) {
				base = (value << 4) & 0x70;
				mapPrgRom8k(2, 0, base | (mapper.romMapTo[0] & 0x1E) >> 1);
				mapPrgRom8k(2, 2, base | 0x0F);
			}
			break;
		default:
			mapPrgRom8k(2, 0, ((mapper.romMapTo[0] & 0xE0) >> 1) | (value & 0x0F));
			break;
	}
	mapPrgRom8kUpdate();
}

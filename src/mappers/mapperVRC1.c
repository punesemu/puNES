/*
 * mapperVRC1.c
 *
 *  Created on: 16/lug/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom8kMax, chrRom4kMax;

void mapInit_VRC1(void) {
	prgRom8kMax = info.prgRom8kCount - 1;
	chrRom4kMax = info.chrRom4kCount - 1;

	EXTCL_CPU_WR_MEM(VRC1);
}
void extcl_cpu_wr_mem_VRC1(WORD address, BYTE value) {
	DBWORD bank;

	address &= 0xF000;

	switch (address) {
		case 0x8000:
			controlBankWithAND(0x0F, prgRom8kMax)
			mapPrgRom8k(1, 0, value);
			mapPrgRom8kUpdate();
			return;
		case 0x9000:
			bank = (((value << 3) & 0x10) | (((chr.bank1k[0] - chr.data) >> 12) & 0x0F))
			        << 12;
			chr.bank1k[0] = &chr.data[bank];
			chr.bank1k[1] = &chr.data[bank | 0x0400];
			chr.bank1k[2] = &chr.data[bank | 0x0800];
			chr.bank1k[3] = &chr.data[bank | 0x0C00];
			bank = (((value << 2) & 0x10) | (((chr.bank1k[4] - chr.data) >> 12) & 0x0F))
			        << 12;
			chr.bank1k[4] = &chr.data[bank];
			chr.bank1k[5] = &chr.data[bank | 0x0400];
			chr.bank1k[6] = &chr.data[bank | 0x0800];
			chr.bank1k[7] = &chr.data[bank | 0x0C00];
			if (value & 0x01) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			return;
		case 0xA000:
			controlBankWithAND(0x0F, prgRom8kMax)
			mapPrgRom8k(1, 1, value);
			mapPrgRom8kUpdate();
			return;
		case 0xC000:
			controlBankWithAND(0x0F, prgRom8kMax)
			mapPrgRom8k(1, 2, value);
			mapPrgRom8kUpdate();
			return;
		case 0xE000:
			value = (((chr.bank1k[0] - chr.data) >> 12) & 0x10) | (value & 0x0F);
			controlBank(chrRom4kMax)
			bank = value << 12;
			chr.bank1k[0] = &chr.data[bank];
			chr.bank1k[1] = &chr.data[bank | 0x0400];
			chr.bank1k[2] = &chr.data[bank | 0x0800];
			chr.bank1k[3] = &chr.data[bank | 0x0C00];
			return;
		case 0xF000:
			value = (((chr.bank1k[4] - chr.data) >> 12) & 0x10) | (value & 0x0F);
			controlBank(chrRom4kMax)
			bank = value << 12;
			chr.bank1k[4] = &chr.data[bank];
			chr.bank1k[5] = &chr.data[bank | 0x0400];
			chr.bank1k[6] = &chr.data[bank | 0x0800];
			chr.bank1k[7] = &chr.data[bank | 0x0C00];
			return;
		default:
			return;
	}
}

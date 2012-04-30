/*
 * mapperBxROM.c
 *
 *  Created on: 16/lug/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom32kMax, chrRom4kMax;

void mapInit_BxROM(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	chrRom4kMax = info.chrRom4kCount - 1;

	if (info.reset >= HARD) {
		mapPrgRom8k(4, 0, 0);
	}

	switch (info.mapperType) {
		case BXROMUNL:
			EXTCLCPUWRMEM(BxROM_UNL);
			break;
		case AVENINA001:
			info.mapperExtendWrite = TRUE;
			EXTCLCPUWRMEM(AveNina001);
			break;
		default:
			EXTCLCPUWRMEM(BxROM);
			break;
	}
}

void extclCpuWrMem_BxROM(WORD address, BYTE value) {
	/* bus conflict */
	value &= prgRomRd(address);

	controlBankWithAND(0x0F, prgRom32kMax)
	mapPrgRom8k(4, 0, value);
	mapPrgRom8kUpdate();
}

void extclCpuWrMem_BxROM_UNL(WORD address, BYTE value) {
	controlBankWithAND(0x3F, prgRom32kMax)
	mapPrgRom8k(4, 0, value);
	mapPrgRom8kUpdate();
}

void extclCpuWrMem_AveNina001(WORD address, BYTE value) {
	DBWORD bank;

	switch (address) {
		case 0x7FFD:
			controlBankWithAND(0x03, prgRom32kMax)
			mapPrgRom8k(4, 0, value);
			mapPrgRom8kUpdate();
			break;
		case 0x7FFE:
			controlBankWithAND(0x0F, chrRom4kMax)
			bank = value << 12;
			chr.bank1k[0] = &chr.data[bank];
			chr.bank1k[1] = &chr.data[bank | 0x0400];
			chr.bank1k[2] = &chr.data[bank | 0x0800];
			chr.bank1k[3] = &chr.data[bank | 0x0C00];
			break;
		case 0x7FFF:
			controlBankWithAND(0x0F, chrRom4kMax)
			bank = value << 12;
			chr.bank1k[4] = &chr.data[bank];
			chr.bank1k[5] = &chr.data[bank | 0x0400];
			chr.bank1k[6] = &chr.data[bank | 0x0800];
			chr.bank1k[7] = &chr.data[bank | 0x0C00];
			break;
	}
}

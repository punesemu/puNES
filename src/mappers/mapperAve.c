/*
 * mapperAve.c
 *
 *  Created on: 19/set/2011
 *      Author: fhorse
 */

#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

#define avenina06swap()\
{\
	const BYTE save = value;\
	DBWORD bank;\
	if (prgRom32kMax != 0xFFFF) {\
		value >>= 3;\
		controlBank(prgRom32kMax)\
		mapPrgRom8k(4, 0, value);\
		mapPrgRom8kUpdate();\
		value = save;\
	}\
	controlBankWithAND(0x07, chrRom8kMax)\
	bank = value << 13;\
	chr.bank1k[0] = &chr.data[bank];\
	chr.bank1k[1] = &chr.data[bank | 0x0400];\
	chr.bank1k[2] = &chr.data[bank | 0x0800];\
	chr.bank1k[3] = &chr.data[bank | 0x0C00];\
	chr.bank1k[4] = &chr.data[bank | 0x1000];\
	chr.bank1k[5] = &chr.data[bank | 0x1400];\
	chr.bank1k[6] = &chr.data[bank | 0x1800];\
	chr.bank1k[7] = &chr.data[bank | 0x1C00];\
}

WORD prgRom32kMax, chrRom8kMax;

void mapInit_Ave(BYTE model) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	chrRom8kMax = info.chrRom8kCount - 1;

	switch (model) {
		case NINA06:
			EXTCLCPUWRMEM(Ave_NINA06);

			info.mapperExtendWrite = TRUE;

			if (info.reset >= HARD) {
				if (prgRom32kMax != 0xFFFF) {
					mapPrgRom8k(4, 0, 0);
				}
			}
			break;
		case D1012:
			EXTCLCPUWRMEM(Ave_D1012);
			EXTCLCPURDMEM(Ave_D1012);
			EXTCLSAVEMAPPER(Ave_D1012);
			mapper.intStruct[0] = (BYTE *) &ave_d1012;
			mapper.intStructSize[0] = sizeof(ave_d1012);

			info.mapperExtendRead = TRUE;

			if (info.reset >= HARD) {
				mapPrgRom8k(4, 0, 0);
				memset(&ave_d1012, 0x00, sizeof(ave_d1012));
			}

			mirroring_V();

			break;
	}
}

void extclCpuWrMem_Ave_NINA06(WORD address, BYTE value) {
	if ((address < 0x4100) || (address >= 0x6000)) {
		if (info.id != PUZZLEUNL) {
			return;
		}
	}

	switch (address & 0x0100) {
		case 0x0000:
			if (info.id != PUZZLEUNL) {
				return;
			}
			avenina06swap()
			return;
		case 0x0100:
			avenina06swap()
			return;
	}
}

void extclCpuWrMem_Ave_D1012(WORD address, BYTE value) {
	DBWORD bank;

	if (address < 0xFF80) {
		return;
	}

	switch (address & 0x00F8) {
		case 0x0080:
		case 0x0088:
		case 0x0090:
		case 0x0098:
			if (!(ave_d1012.reg[0] & 0x3F)) {
				ave_d1012.reg[0] = value;
				if (value & 0x80) {
					mirroring_H();
				} else {
					mirroring_V();
				}
				break;
			}
			return;
		case 0x00C0:
		case 0x00C8:
		case 0x00D0:
		case 0x00D8:
			if (!ave_d1012.reg[2]) {
				ave_d1012.reg[2] = value;
			}
			return;
		case 0x00E8:
		case 0x00F0:
			ave_d1012.reg[1] = value;
			break;
	}

	value = (ave_d1012.reg[0] & 0xE) | (ave_d1012.reg[(ave_d1012.reg[0] >> 6) & 0x1] & 0x1);
	controlBank(prgRom32kMax)
	mapPrgRom8k(4, 0, value);
	mapPrgRom8kUpdate();

	value = ((ave_d1012.reg[0] << 2) & (((ave_d1012.reg[0] >> 4) & 0x4) ^ 0x3C))
        		| ((ave_d1012.reg[1] >> 4) & (((ave_d1012.reg[0] >> 4) & 0x4) | 0x3));
	controlBank(chrRom8kMax)
	bank = value << 13;
	chr.bank1k[0] = &chr.data[bank];
	chr.bank1k[1] = &chr.data[bank | 0x0400];
	chr.bank1k[2] = &chr.data[bank | 0x0800];
	chr.bank1k[3] = &chr.data[bank | 0x0C00];
	chr.bank1k[4] = &chr.data[bank | 0x1000];
	chr.bank1k[5] = &chr.data[bank | 0x1400];
	chr.bank1k[6] = &chr.data[bank | 0x1800];
	chr.bank1k[7] = &chr.data[bank | 0x1C00];
}
BYTE extclCpuRdMem_Ave_D1012(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0xFF80) || (address > 0xFFF7)) {
		return (openbus);
	}

	extclCpuWrMem_Ave_D1012(address, openbus);
	return (openbus);
}
BYTE extclSaveMapper_Ave_D1012(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, ave_d1012.reg);

	return (EXIT_OK);
}

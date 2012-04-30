/*
 * mapperIrem.c
 *
 *  Created on: 13/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "cpu6502.h"
#include "savestate.h"

#define iremG101prgRomUpdate()\
	controlBank(prgRom8kMax)\
	if (!iremG101.prgMode) {\
		mapPrgRom8k(1, 0, value);\
		mapPrgRom8k(1, 2, prgRom8kBeforeLast);\
	} else {\
		mapPrgRom8k(1, 0, prgRom8kBeforeLast);\
		mapPrgRom8k(1, 2, value);\
	}\
	mapPrgRom8kUpdate()
#define iremLROG017ChrRam()\
	chr.bank1k[2] = &iremLROG017.chrRam[0x0000];\
	chr.bank1k[3] = &iremLROG017.chrRam[0x0400];\
	chr.bank1k[4] = &iremLROG017.chrRam[0x0800];\
	chr.bank1k[5] = &iremLROG017.chrRam[0x0C00];\
	chr.bank1k[6] = &iremLROG017.chrRam[0x1000];\
	chr.bank1k[7] = &iremLROG017.chrRam[0x1400]

WORD prgRom32kMax, prgRom16kMax, prgRom8kMax, prgRom8kBeforeLast;
WORD chrRom2kMax, chrRom1kMax;

void mapInit_Irem(BYTE model) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	prgRom16kMax = info.prgRom16kCount - 1;
	prgRom8kMax = info.prgRom8kCount - 1;
	prgRom8kBeforeLast = prgRom8kMax - 1;
	chrRom2kMax = (info.chrRom1kCount >> 1) - 1;
	chrRom1kMax = info.chrRom1kCount - 1;

	switch (model) {
		case G101:
			EXTCLCPUWRMEM(Irem_G101);
			EXTCLSAVEMAPPER(Irem_G101);
			mapper.intStruct[0] = (BYTE *) &iremG101;
			mapper.intStructSize[0] = sizeof(iremG101);

			if (info.reset >= HARD) {
				memset(&iremG101, 0x00, sizeof(iremG101));

				mapper.romMapTo[0] = 0;
				mapper.romMapTo[1] = prgRom8kMax;
				mapper.romMapTo[2] = prgRom8kBeforeLast;
				mapper.romMapTo[3] = prgRom8kMax;

				if (info.id == MAJORLEAGUE) {
					mirroring_SCR0();
				}
			}
			break;
		case H3000:
			EXTCLCPUWRMEM(Irem_H3000);
			EXTCLSAVEMAPPER(Irem_H3000);
			EXTCLCPUEVERYCYCLE(Irem_H3000);
			mapper.intStruct[0] = (BYTE *) &iremH3000;
			mapper.intStructSize[0] = sizeof(iremH3000);

			if (info.reset >= HARD) {
				memset(&iremH3000, 0x00, sizeof(iremH3000));
			}
			break;
		case LROG017:
			EXTCLCPUWRMEM(Irem_LROG017);
			EXTCLSAVEMAPPER(Irem_LROG017);
			EXTCLWRCHR(Irem_LROG017);
			mapper.intStruct[0] = (BYTE *) &iremLROG017;
			mapper.intStructSize[0] = sizeof(iremLROG017);

			if (info.reset >= HARD) {
				memset(&iremLROG017, 0x00, sizeof(iremLROG017));
				mapPrgRom8k(4, 0, 0);
			}

			iremLROG017ChrRam();

			break;
		case TAMS1:
			EXTCLCPUWRMEM(Irem_TAMS1);

			if (info.reset >= HARD) {
				mapPrgRom8k(2, 0, prgRom16kMax);
				mapPrgRom8k(2, 2, 0);
			}
			break;
	}
}

void extclCpuWrMem_Irem_G101(WORD address, BYTE value) {
	if (address >= 0xC000) {
		return;
	}

	switch (address & 0xF000) {
		case 0x8000:
			iremG101.prgReg = value;
			iremG101prgRomUpdate();
			break;
		case 0x9000:
			if (info.mapperType != G101B) {
				if (value & 0x01) {
					mirroring_H();
				} else {
					mirroring_V();
				}
			}
			iremG101.prgMode = value & 0x02;
			value = iremG101.prgReg;
			iremG101prgRomUpdate();
			break;
		case 0xA000:
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, 1, value);
			mapPrgRom8kUpdate();
			break;
		case 0xB000:
			controlBank(chrRom1kMax)
			chr.bank1k[address & 0x0007] = &chr.data[value << 10];
			break;
	}
}
BYTE extclSaveMapper_Irem_G101(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, iremG101.prgMode);
	savestateEle(mode, slot, iremG101.prgReg);

	return (EXIT_OK);
}

void extclCpuWrMem_Irem_H3000(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, 0, value);
			mapPrgRom8kUpdate();
			break;
		case 0x9000: {
			switch (address & 0x0007) {
				case 1:
					if (value & 0x80) {
						mirroring_H();
					} else {
						mirroring_V();
					}
					break;
				case 3:
					iremH3000.enable = value & 0x80;
					irq.high &= ~EXTIRQ;
					break;
				case 4:
					iremH3000.count = iremH3000.reload;
					irq.high &= ~EXTIRQ;
					break;
				case 5:
					iremH3000.reload = (iremH3000.reload & 0x00FF) | (value << 8);
					break;
				case 6:
					iremH3000.reload = (iremH3000.reload & 0xFF00) | value;
					break;
			}
			break;
		}
		case 0xB000:
			controlBank(chrRom1kMax)
			chr.bank1k[address & 0x0007] = &chr.data[value << 10];
			break;
		case 0xA000:
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, 1, value);
			mapPrgRom8kUpdate();
			break;
		case 0xC000:
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, 2, value);
			mapPrgRom8kUpdate();
			break;
	}
}
BYTE extclSaveMapper_Irem_H3000(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, iremH3000.enable);
	savestateEle(mode, slot, iremH3000.count);
	savestateEle(mode, slot, iremH3000.reload);
	savestateEle(mode, slot, iremH3000.delay);

	return (EXIT_OK);
}
void extclCPUEveryCycle_Irem_H3000(void) {
	if (iremH3000.delay && !(--iremH3000.delay)) {
		irq.high |= EXTIRQ;
	}

	if (iremH3000.enable && iremH3000.count && !(--iremH3000.count)) {
		iremH3000.enable = FALSE;
		iremH3000.delay = 1;
		return;
	}
}

void extclCpuWrMem_Irem_LROG017(WORD address, BYTE value) {
	/* bus conflict */
	const BYTE save = value &= prgRomRd(address);
	DBWORD bank;

	controlBankWithAND(0x0F, prgRom32kMax)
	mapPrgRom8k(4, 0, value);
	mapPrgRom8kUpdate();

	value = save >> 4;
	controlBank(chrRom2kMax)
	bank = value << 11;
	chr.bank1k[0] = &chr.data[bank];
	chr.bank1k[1] = &chr.data[bank | 0x0400];
}
BYTE extclSaveMapper_Irem_LROG017(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, iremLROG017.chrRam);
	if (mode == SSREAD) {
		iremLROG017ChrRam();
	}

	return (EXIT_OK);
}
void extclWrChr_Irem_LROG017(WORD address, BYTE value) {
	const BYTE slot = address >> 10;

	/*
	 * gli slot 0 e 1 sono collegati alla CHR Rom e quindi
	 * non posso scriverci.
	 */
	if (slot > 1) {
		chr.bank1k[slot][address & 0x3FF] = value;
	}
}

void extclCpuWrMem_Irem_TAMS1(WORD address, BYTE value) {
	/* bus conflict */
	value &= prgRomRd(address);

	if (value & 0x80) {
		mirroring_V();
	} else {
		mirroring_H();
	}

	controlBank(prgRom16kMax)
	mapPrgRom8k(2, 2, value);
	mapPrgRom8kUpdate();
}

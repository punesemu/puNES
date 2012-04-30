/*
 * mapperTengen.c
 *
 *  Created on: 17/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "cpu6502.h"
#include "irqA12.h"
#include "savestate.h"

enum {
	A12MODE,
	CPUMODE,
	tRamboDelayA12 = 3,
	tRamboDelayCPU = 3,
};

#define tRamboPrg24kUpdate()\
{\
	BYTE bank[3];\
	if (tRambo.prgMode) {\
		bank[0] = tRambo.prg[2];\
		bank[1] = tRambo.prg[0];\
		bank[2] = tRambo.prg[1];\
	} else {\
		bank[0] = tRambo.prg[0];\
		bank[1] = tRambo.prg[1];\
		bank[2] = tRambo.prg[2];\
	}\
	mapPrgRom8k(1, 0, bank[0]);\
	mapPrgRom8k(1, 1, bank[1]);\
	mapPrgRom8k(1, 2, bank[2]);\
	mapPrgRom8kUpdate();\
}
#define tRamboChr8kUpdate()\
{\
	BYTE bank[8] = { 0 };\
	if (tRambo.chrMode & 0x80) {\
		bank[0] = tRambo.chr[2];\
		bank[1] = tRambo.chr[3];\
		bank[2] = tRambo.chr[4];\
		bank[3] = tRambo.chr[5];\
		if (tRambo.chrMode & 0x20) {\
			tRamboChr1kControl(0, 4);\
			bank[5] = tRambo.chr[6];\
			tRamboChr1kControl(1, 6);\
			bank[7] = tRambo.chr[7];\
		} else {\
			tRamboChr2kControl(0, 4);\
			tRamboChr2kControl(1, 6);\
		}\
	} else {\
		bank[4] = tRambo.chr[2];\
		bank[5] = tRambo.chr[3];\
		bank[6] = tRambo.chr[4];\
		bank[7] = tRambo.chr[5];\
		if (tRambo.chrMode & 0x20) {\
			tRamboChr1kControl(0, 0);\
			bank[1] = tRambo.chr[6];\
			tRamboChr1kControl(1, 2);\
			bank[3] = tRambo.chr[7];\
		} else {\
			tRamboChr2kControl(0, 0);\
			tRamboChr2kControl(1, 2);\
		}\
	}\
	chr.bank1k[0] = &chr.data[bank[0] << 10];\
	chr.bank1k[1] = &chr.data[bank[1] << 10];\
	chr.bank1k[2] = &chr.data[bank[2] << 10];\
	chr.bank1k[3] = &chr.data[bank[3] << 10];\
	chr.bank1k[4] = &chr.data[bank[4] << 10];\
	chr.bank1k[5] = &chr.data[bank[5] << 10];\
	chr.bank1k[6] = &chr.data[bank[6] << 10];\
	chr.bank1k[7] = &chr.data[bank[7] << 10];\
}
#define tRamboChr1kControl(chr1k, bank1k)\
	value = tRambo.chr[chr1k];\
	controlBank(chrRom1kMax)\
	bank[bank1k] = value
#define tRamboChr2kControl(chr2k, bank2k)\
	value = tRambo.chr[chr2k] >> 1;\
	controlBank(chrRom2kMax)\
	bank[bank2k] = value << 1;\
	bank[bank2k + 1] = bank[bank2k] | 0x01

WORD prgRom8kMax, chrRom1kMax, chrRom2kMax;
BYTE type;

void mapInit_Tengen(BYTE model) {
	prgRom8kMax = info.prgRom8kCount  - 1;
	chrRom1kMax = info.chrRom1kCount  - 1;
	chrRom2kMax = (info.chrRom1kCount >> 1) - 1;

	switch (model) {
		case T800037:
		case TRAMBO:
			EXTCLCPUWRMEM(Tengen_Rambo);
			EXTCLSAVEMAPPER(Tengen_Rambo);
			EXTCLPPU000TO34X(MMC3);
			EXTCLPPU000TO255(Tengen_Rambo);
			EXTCLPPU256TO319(Tengen_Rambo);
			EXTCLPPU320TO34X(Tengen_Rambo);
			EXTCL2006UPDATE(MMC3);
			EXTCLIRQA12CLOCK(Tengen_Rambo);
			EXTCLCPUEVERYCYCLE(Tengen_Rambo);
			mapper.intStruct[0] = (BYTE *) &tRambo;
			mapper.intStructSize[0] = sizeof(tRambo);
			mapper.intStruct[1] = (BYTE *) &irqA12;
			mapper.intStructSize[1] = sizeof(irqA12);

			if (info.reset >= HARD) {
				BYTE i;

				memset(&tRambo, 0x00, sizeof(tRambo));
				memset(&irqA12, 0x00, sizeof(irqA12));
				tRambo.irqMode = CPUMODE;

				for (i = 0; i < LENGTH(tRambo.chr); i++) {
					tRambo.chr[i] = i;
				}

				for (i = 0; i < LENGTH(tRambo.prg); i++) {
					tRambo.prg[i] = i;
				}
			} else {
				tRambo.irqMode = CPUMODE;
			}

			irqA12.present = TRUE;

			{
				BYTE value = 0;
				tRamboPrg24kUpdate()
				tRamboChr8kUpdate()
			}
			break;
	}

	type = model;
}

void extclCpuWrMem_Tengen_Rambo(WORD address, BYTE value) {
	switch (address & 0xF001) {
		case 0x8000:
			tRambo.regIndex = value & 0x0F;
			if ((value & 0x40) != tRambo.prgMode) {
				tRambo.prgMode = value & 0x40;
				tRamboPrg24kUpdate()
			}
			if ((value & 0xA0) != tRambo.chrMode) {
				tRambo.chrMode = value & 0xA0;
				tRamboChr8kUpdate()
			}
			return;
		case 0x8001: {
			switch (tRambo.regIndex) {
				case 0x00:
				case 0x01:
					if ((type == T800037) && !(tRambo.chrMode & 0x80)) {
						const BYTE slot = tRambo.regIndex << 1;
						ntbl.bank1k[slot] = &ntbl.data[((value >> 7) ^ 0x01) << 10];
						ntbl.bank1k[slot | 0x01] = ntbl.bank1k[slot];
					}
					if (tRambo.chr[tRambo.regIndex] != value) {
						tRambo.chr[tRambo.regIndex] = value;
						tRamboChr8kUpdate()
					}
					break;
				case 0x02:
				case 0x03:
				case 0x04:
				case 0x05:
					if ((type == T800037) && (tRambo.chrMode & 0x80)) {
						ntbl.bank1k[tRambo.regIndex - 2] = &ntbl.data[((value >> 7) ^ 0x01)
						        << 10];
					}
					controlBank(chrRom1kMax)
					if (tRambo.chr[tRambo.regIndex] != value) {
						tRambo.chr[tRambo.regIndex] = value;
						tRamboChr8kUpdate()
					}
					break;
				case 0x06:
				case 0x07: {
					const BYTE index = tRambo.regIndex & 0x01;
					controlBank(prgRom8kMax)
					if (tRambo.prg[index] != value) {
						tRambo.prg[index] = value;
						tRamboPrg24kUpdate()
					}
					break;
				}
				case 0x08:
				case 0x09: {
					const BYTE index = tRambo.regIndex - 0x02;
					controlBank(chrRom1kMax)
					if (tRambo.chr[index] != value) {
						tRambo.chr[index] = value;
						tRamboChr8kUpdate()
					}
					break;
				}
				case 0x0F:
					controlBank(prgRom8kMax)
					if (tRambo.prg[2] != value) {
						tRambo.prg[2] = value;
						tRamboPrg24kUpdate()
					}
					break;
			}
			return;
		}
		case 0xA000:
			if (type == T800037) {
				return;
			}
			if (value & 0x01) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			return;
		case 0xC000:
			irqA12.latch = value;
			return;
		case 0xC001:
			irqA12.reload = TRUE;
			if (value & 0x01) {
				/* abilito' la modalita' CPU cycles */
				tRambo.irqMode = CPUMODE;
				tRambo.irqPrescaler = 0;
			} else {
				/* abilito' la modalita' A12 */
				tRambo.irqMode = A12MODE;
				irqA12.counter = 0;
			}
			return;
		case 0xE000:
			irqA12.enable = FALSE;
			irq.high &= ~EXTIRQ;
			return;
		case 0xE001:
			irqA12.enable = TRUE;
			return;
		default:
			return;
	}
}
BYTE extclSaveMapper_Tengen_Rambo(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, tRambo.prgMode);
	savestateEle(mode, slot, tRambo.chrMode);
	savestateEle(mode, slot, tRambo.regIndex);
	savestateEle(mode, slot, tRambo.chr);
	savestateEle(mode, slot, tRambo.prg);
	savestateEle(mode, slot, tRambo.irqMode);
	savestateEle(mode, slot, tRambo.irqDelay);
	savestateEle(mode, slot, tRambo.irqPrescaler);

	return (EXIT_OK);
}
void extclPPU000to255_Tengen_Rambo(void) {
	if ((tRambo.irqMode == A12MODE) && (r2001.visible)) {
		irqA12_SB();
	}
}
void extclPPU256to319_Tengen_Rambo(void) {
	if (tRambo.irqMode == A12MODE) {
		irqA12_BS();
	}
}
void extclPPU320to34x_Tengen_Rambo(void) {
	if (tRambo.irqMode == A12MODE) {
		irqA12_SB();
	}
}
void extclIrqA12Clock_Tengen_Rambo(void) {
	if (tRambo.irqMode != A12MODE) {
		return;
	}

	/* usa una versione un po' modificata dell'irqA12 */
	if (!irqA12.counter) {
		irqA12.counter = irqA12.latch;
		if (irqA12.reload) {
			irqA12.counter++;
			irqA12.saveCounter = 0;
		}
		if (!irqA12.counter && irqA12.reload) {
			irqA12.saveCounter = 1;
		}
		irqA12.reload = FALSE;
	} else {
		irqA12.counter--;
	}
	if (!irqA12.counter && irqA12.saveCounter && irqA12.enable) {
		tRambo.irqDelay = tRamboDelayA12;
	}
	irqA12.saveCounter = irqA12.counter;
}
void extclCPUEveryCycle_Tengen_Rambo(void) {
	if (tRambo.irqDelay && !(--tRambo.irqDelay)) {
		irq.high |= EXTIRQ;
	}

	if (tRambo.irqMode != CPUMODE) {
		return;
	}

	if (tRambo.irqPrescaler < 3) {
		tRambo.irqPrescaler++;
		return;
	}
	tRambo.irqPrescaler = 0;

	if (irqA12.reload) {
		irqA12.counter = irqA12.latch + 1;
		irqA12.reload = FALSE;
		return;
	}

	if (irqA12.counter) {
		if (!(--irqA12.counter) && irqA12.enable) {
			tRambo.irqDelay = tRamboDelayCPU;
		}
		return;
	}

	irqA12.counter = irqA12.latch;
	return;
}

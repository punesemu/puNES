/*
 * mapperKasing.c
 *
 *  Created on: 28/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "irqA12.h"
#include "savestate.h"

#define kasingSwapPrgRom32k()\
	value = kasing.prgHigh;\
	controlBank(prgRom32kMax)\
	mapPrgRom8k(4, 0, value)
#define kasingIntercept8001Prg(slot)\
	if (kasing.prgMode) {\
		kasingSwapPrgRom32k();\
	} else {\
		controlBank(prgRom8kMax)\
		mapPrgRom8k(1, slot, value);\
	}\
	mapPrgRom8kUpdate()
#define kasingSwapChrRomBank1k(slot1, slot2)\
{\
	WORD tmp = kasing.chrRomBank[slot1];\
	kasing.chrRomBank[slot1] = kasing.chrRomBank[slot2];\
	kasing.chrRomBank[slot2] = tmp;\
}
#define kasingChr1kUpdate(slot)\
{\
	WORD tmp = (kasing.chrHigh << 8) & 0x0100;\
	chr.bank1k[slot] = &chr.data[(tmp | kasing.chrRomBank[slot]) << 10];\
}
#define kasingIntercept8001Chr(slot, val)\
{\
	BYTE bank = slot;\
	kasing.chrRomBank[bank] = val;\
	kasingChr1kUpdate(bank)\
}
#define kasingChrUpdate()\
{\
	BYTE i;\
	for (i = 0; i < 8 ; i++) {\
		kasingChr1kUpdate(i)\
	}\
}

WORD prgRom32kMax, prgRom8kMax, prgRom8kBeforeLast, chrRom1kMax;

void mapInit_Kasing(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	prgRom8kMax = info.prgRom8kCount - 1;
	prgRom8kBeforeLast = info.prgRom8kCount - 2;
	chrRom1kMax = info.chrRom1kCount - 1;

	EXTCL_CPU_WR_MEM(Kasing);
	EXTCL_SAVE_MAPPER(Kasing);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.intStruct[0] = (BYTE *) &kasing;
	mapper.intStructSize[0] = sizeof(kasing);
	mapper.intStruct[1] = (BYTE *) &mmc3;
	mapper.intStructSize[1] = sizeof(mmc3);

	if (info.reset >= HARD) {
		BYTE i;

		memset(&kasing, 0x00, sizeof(kasing));
		memset(&mmc3, 0x00, sizeof(mmc3));
		memset(&irqA12, 0x00, sizeof(irqA12));

		for (i = 0; i < 8; i++) {
			kasing.chrRomBank[i] = i;
		}

	} else {
		memset(&irqA12, 0x00, sizeof(irqA12));
	}

	info.mapperExtendWrite = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_Kasing(WORD address, BYTE value) {
	if (address < 0x6000) {
		return;
	}

	/* intercetto cio' che mi interessa */
	if (address == 0x6000) {
		const BYTE prgModeOld = kasing.prgMode;
		const BYTE prgHighOld = kasing.prgHigh;

		kasing.prgHigh = (value & 0x7F) >> 1;
		kasing.prgMode = value & 0x80;

		if ((kasing.prgHigh != prgHighOld) || (kasing.prgMode != prgModeOld)) {
			if (kasing.prgMode) {
				kasing.prgRomBank[0] = mapper.romMapTo[0];
				kasing.prgRomBank[1] = mapper.romMapTo[1];
				kasing.prgRomBank[2] = mapper.romMapTo[2];
				kasing.prgRomBank[3] = mapper.romMapTo[3];
				kasingSwapPrgRom32k();
			} else {
				mapper.romMapTo[0] = kasing.prgRomBank[0];
				mapper.romMapTo[1] = kasing.prgRomBank[1];
				mapper.romMapTo[2] = kasing.prgRomBank[2];
				mapper.romMapTo[3] = kasing.prgRomBank[3];
			}
			mapPrgRom8kUpdate();
		}
		return;
	} else if (address == 0x6001) {
		if (kasing.chrHigh != value) {
			kasing.chrHigh = value;
			kasingChrUpdate()
		}
		return;
	}

	if (address < 0x8000) {
		return;
	}

	address &= 0xE001;

	if (address == 0x8000) {
		BYTE prgRomCfg = (value & 0x40) >> 5;
		BYTE chrRomCfg = (value & 0x80) >> 5;

		if (mmc3.chrRomCfg != chrRomCfg) {
			mmc3.chrRomCfg = chrRomCfg;
			kasingSwapChrRomBank1k(0, 4)
			kasingSwapChrRomBank1k(1, 5)
			kasingSwapChrRomBank1k(2, 6)
			kasingSwapChrRomBank1k(3, 7)
		}
		if (mmc3.prgRomCfg != prgRomCfg) {
			/* intercetto solo se il prgMode non e' a zero */
			if (kasing.prgMode) {
				mmc3.prgRomCfg = prgRomCfg;
				kasingSwapPrgRom32k();
				mapPrgRom8kUpdate();
			}
		}
	} else if (address == 0x8001) {
		switch (mmc3.bankToUpdate) {
			case 0:
				kasingIntercept8001Chr(mmc3.chrRomCfg, value)
				kasingIntercept8001Chr(mmc3.chrRomCfg | 0x01, value + 1)
				break;
			case 1:
				kasingIntercept8001Chr(mmc3.chrRomCfg | 0x02, value)
				kasingIntercept8001Chr(mmc3.chrRomCfg | 0x03, value + 1)
				break;
			case 2:
				kasingIntercept8001Chr(mmc3.chrRomCfg ^ 0x04, value)
				break;
			case 3:
				kasingIntercept8001Chr((mmc3.chrRomCfg ^ 0x04) | 0x01, value)
				break;
			case 4:
				kasingIntercept8001Chr((mmc3.chrRomCfg ^ 0x04) | 0x02, value)
				break;
			case 5:
				kasingIntercept8001Chr((mmc3.chrRomCfg ^ 0x04) | 0x03, value)
				break;
			case 6:
				kasingIntercept8001Prg(mmc3.prgRomCfg);
				break;
			case 7:
				kasingIntercept8001Prg(1);
				break;
		}
		return;
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
BYTE extcl_save_mapper_Kasing(BYTE mode, BYTE slot, FILE *fp) {
	if (savestate.version >= 6) {
		savestateEle(mode, slot, kasing.prgMode);
	}
	savestateEle(mode, slot, kasing.prgHigh);
	if (savestate.version < 6) {
		if (mode == SSREAD) {
			BYTE old_prgRomBank[4], i;

			savestateEle(mode, slot, old_prgRomBank)

			for (i = 0; i < 4; i++) {
				kasing.prgRomBank[i] = old_prgRomBank[i];
			}
		} else if (mode == SSCOUNT) {
			savestate.totSize[slot] += sizeof(BYTE) * 4;
		}
	} else {
		savestateEle(mode, slot, kasing.prgRomBank);
	}
	savestateEle(mode, slot, kasing.chrHigh);
	savestateEle(mode, slot, kasing.chrRomBank);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

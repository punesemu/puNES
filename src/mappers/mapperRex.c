/*
 * mapperRex.c
 *
 *  Created on: 11/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "irqA12.h"
#include "savestate.h"

#define rexDbzSwapChrRomBank1k(slot1, slot2)\
{\
	WORD tmp = rexDbz.chrRomBank[slot1];\
	rexDbz.chrRomBank[slot1] = rexDbz.chrRomBank[slot2];\
	rexDbz.chrRomBank[slot2] = tmp;\
}
#define rexDbzChr1kUpdate(slot)\
{\
	WORD tmp = (rexDbz.chrHigh << ((slot >= 4) ? 4 : 8)) & 0x0100;\
	chr.bank1k[slot] = &chr.data[(tmp | rexDbz.chrRomBank[slot]) << 10];\
}
#define rexDbzIntercept8001(slot, val)\
{\
	BYTE bank = slot;\
	rexDbz.chrRomBank[bank] = val;\
	rexDbzChr1kUpdate(bank)\
}
#define rexDbzChrUpdate()\
{\
	BYTE i;\
	for (i = 0; i < 8 ; i++) {\
		rexDbzChr1kUpdate(i)\
	}\
}

WORD prgRom8kMax, prgRom8kBeforeLast, chrRom1kMax;

void mapInit_Rex(BYTE model) {
	prgRom8kMax = info.prgRom8kCount - 1;
	chrRom1kMax = info.chrRom1kCount - 1;
	prgRom8kBeforeLast = info.prgRom8kCount - 2;

	if (model == DBZ) {
		EXTCLCPUWRMEM(Rex_dbz);
		EXTCLCPURDMEM(Rex_dbz);
		EXTCLSAVEMAPPER(Rex_dbz);
		EXTCLCPUEVERYCYCLE(MMC3);
		EXTCLPPU000TO34X(MMC3);
		EXTCLPPU000TO255(MMC3);
		EXTCLPPU256TO319(MMC3);
		EXTCLPPU320TO34X(MMC3);
		EXTCL2006UPDATE(MMC3);
		mapper.intStruct[0] = (BYTE *) &rexDbz;
		mapper.intStructSize[0] = sizeof(rexDbz);
		mapper.intStruct[1] = (BYTE *) &mmc3;
		mapper.intStructSize[1] = sizeof(mmc3);

		if (info.reset >= HARD) {
			BYTE i;

			memset(&rexDbz, 0x00, sizeof(rexDbz));
			memset(&mmc3, 0x00, sizeof(mmc3));
			memset(&irqA12, 0x00, sizeof(irqA12));

			for (i = 0; i < 8; i++) {
				rexDbz.chrRomBank[i] = i;
			}
		} else {
			memset(&irqA12, 0x00, sizeof(irqA12));
		}

		info.mapperExtendWrite = TRUE;

		irqA12.present = TRUE;
		irqA12_delay = 1;
	}
}
void extclCpuWrMem_Rex_dbz(WORD address, BYTE value) {
	WORD adr = address & 0xE001;

	/* intercetto cio' che mi interessa */
	if ((address >= 0x4100) && (address < 0x8000)) {
		if (rexDbz.chrHigh != value) {
			rexDbz.chrHigh = value;
			rexDbzChrUpdate()
		}
		return;
	} else if (adr == 0x8000) {
		BYTE chrRomCfg = (value & 0x80) >> 5;

		if (mmc3.chrRomCfg != chrRomCfg) {
			rexDbzSwapChrRomBank1k(0, 4)
			rexDbzSwapChrRomBank1k(1, 5)
			rexDbzSwapChrRomBank1k(2, 6)
			rexDbzSwapChrRomBank1k(3, 7)

			mmc3.chrRomCfg = chrRomCfg;
		}
	} else if (adr == 0x8001) {
		if (mmc3.bankToUpdate <= 5) {
			switch (mmc3.bankToUpdate) {
				case 0:
					rexDbzIntercept8001(mmc3.chrRomCfg, value)
					rexDbzIntercept8001(mmc3.chrRomCfg | 0x01, value + 1)
					break;
				case 1:
					rexDbzIntercept8001(mmc3.chrRomCfg | 0x02, value)
					rexDbzIntercept8001(mmc3.chrRomCfg | 0x03, value + 1)
					break;
				case 2:
					rexDbzIntercept8001(mmc3.chrRomCfg ^ 0x04, value)
					break;
				case 3:
					rexDbzIntercept8001((mmc3.chrRomCfg ^ 0x04) | 0x01, value)
					break;
				case 4:
					rexDbzIntercept8001((mmc3.chrRomCfg ^ 0x04) | 0x02, value)
					break;
				case 5:
					rexDbzIntercept8001((mmc3.chrRomCfg ^ 0x04) | 0x03, value)
					break;
			}
			return;
		}
	}
	extclCpuWrMem_MMC3(address, value);
}
BYTE extclCpuRdMem_Rex_dbz(WORD address, BYTE openbus, BYTE before) {
	if ((address >= 0x4100) || (address < 0x6000)) {
		/* TODO:
		 * se disabilito questo return ed avvio la rom,
		 * il testo non sara' piu' in cinese ma in inglese.
		 */
		return (0x01);
	}
	return (openbus);
}
BYTE extclSaveMapper_Rex_dbz(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, rexDbz.chrRomBank);
	savestateEle(mode, slot, rexDbz.chrHigh);
	extclSaveMapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

/*
 * mapperMMC2andMMC4.c
 *
 *  Created on: 10/lug/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

WORD prgRom16kMax, prgRom8kMax, chrRom4kMax;

void mapInit_MMC2and4(void) {
	prgRom16kMax = info.prgRom16kCount - 1;
	prgRom8kMax = info.prgRom8kCount - 1;
	chrRom4kMax = info.chrRom4kCount - 1;

	EXTCLCPUWRMEM(MMC2and4);
	EXTCLSAVEMAPPER(MMC2and4);
	EXTCLRDCHRAFTER(MMC2and4);
	mapper.intStruct[0] = (BYTE *) &mmc2and4;
	mapper.intStructSize[0] = sizeof(mmc2and4);

	if (info.reset >= HARD) {
		memset(&mmc2and4, 0x00, sizeof(mmc2and4));
		mmc2and4.latch1 = 2;

		/* MMC2 */
		if (info.mapper == 9) {
			mapper.romMapTo[1] = info.prgRom8kCount - 3;
		}
	}
}
void extclCpuWrMem_MMC2and4(WORD address, BYTE value) {
	DBWORD tmp;

	address &= 0xF000;

	switch (address) {
		case 0xA000:
			if (info.mapper == 9) {
				/* MMC2 */
				controlBankWithAND(0x0F, prgRom8kMax)
				mapPrgRom8k(1, 0, value);
			} else {
				/* MMC4 */
				controlBankWithAND(0x0F, prgRom16kMax)
				mapPrgRom8k(2, 0, value);
			}
			mapPrgRom8kUpdate();
			return;
		case 0xB000:
			controlBankWithAND(0x1F, chrRom4kMax)
			mmc2and4.regs[0] = value;
			break;
		case 0xC000:
			controlBankWithAND(0x1F, chrRom4kMax)
			mmc2and4.regs[1] = value;
			break;
		case 0xD000:
			controlBankWithAND(0x1F, chrRom4kMax)
			mmc2and4.regs[2] = value;
			break;
		case 0xE000:
			controlBankWithAND(0x1F, chrRom4kMax)
			mmc2and4.regs[3] = value;
			break;
		case 0xF000:
			if (value & 0x01) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			return;
	}
	tmp = mmc2and4.regs[mmc2and4.latch0] << 12;
	chr.bank1k[0] = &chr.data[tmp];
	chr.bank1k[1] = &chr.data[tmp | 0x0400];
	chr.bank1k[2] = &chr.data[tmp | 0x0800];
	chr.bank1k[3] = &chr.data[tmp | 0x0C00];
	tmp = mmc2and4.regs[mmc2and4.latch1] << 12;
	chr.bank1k[4] = &chr.data[tmp];
	chr.bank1k[5] = &chr.data[tmp | 0x0400];
	chr.bank1k[6] = &chr.data[tmp | 0x0800];
	chr.bank1k[7] = &chr.data[tmp | 0x0C00];
}
BYTE extclSaveMapper_MMC2and4(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, mmc2and4.regs);
	savestateEle(mode, slot, mmc2and4.latch0);
	savestateEle(mode, slot, mmc2and4.latch1);
	return (EXIT_OK);
}
void extclRdChrAfter_MMC2and4(WORD address) {
	WORD bank, latch = address & 0xFFF0;
	DBWORD value;

	switch (latch) {
		case 0x0FD0:
			mmc2and4.latch0 = 0;
			value = mmc2and4.regs[mmc2and4.latch0];
			bank = 0;
			break;
		case 0x0FE0:
			mmc2and4.latch0 = 1;
			value = mmc2and4.regs[mmc2and4.latch0];
			bank = 0;
			break;
		case 0x1FD0:
			mmc2and4.latch1 = 2;
			value = mmc2and4.regs[mmc2and4.latch1];
			bank = 4;
			break;
		case 0x1FE0:
			mmc2and4.latch1 = 3;
			value = mmc2and4.regs[mmc2and4.latch1];
			bank = 4;
			break;
		default:
			return;
	}
	value <<= 12;
	chr.bank1k[0 | bank] = &chr.data[value];
	chr.bank1k[1 | bank] = &chr.data[value | 0x0400];
	chr.bank1k[2 | bank] = &chr.data[value | 0x0800];
	chr.bank1k[3 | bank] = &chr.data[value | 0x0C00];
}

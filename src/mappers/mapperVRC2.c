/*
 * mapperVRC2.c
 *
 *  Created on: 10/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

#define chrRom1kUpdate(slot, mask, shift)\
	value = (vrc2.chrRomBank[slot] & mask) | (((value >> type) & 0x0F) << shift);\
	controlBank(chrRom1kMax)\
	chr.bank1k[slot] = &chr.data[value << 10];\
	vrc2.chrRomBank[slot] = value

WORD prgRom8kMax, chrRom1kMax;
BYTE type;

const WORD shiftVRC2[2][4] = {
	{0x0000, 0x0001, 0x0002, 0x0003},
	{0x0000, 0x0002, 0x0001, 0x0003}
};

void mapInit_VRC2(BYTE revision) {
	prgRom8kMax = info.prgRom8kCount - 1;
	chrRom1kMax = info.chrRom1kCount - 1;

	EXTCL_CPU_WR_MEM(VRC2);
	EXTCL_SAVE_MAPPER(VRC2);
	mapper.intStruct[0] = (BYTE *) &vrc2;
	mapper.intStructSize[0] = sizeof(vrc2);

	if (info.reset >= HARD) {
		BYTE i;

		memset(&vrc2, 0x00, sizeof(vrc2));
		for (i = 0; i < 8; i++) {
			vrc2.chrRomBank[i] = i;
		}
	}

	type = revision;
}
void extcl_cpu_wr_mem_VRC2(WORD address, BYTE value) {
	if (address < 0xB000) {
		address &= 0xF000;
	} else {
		address = (address & 0xF000) | shiftVRC2[type][address & 0x0003];
	}

	switch (address) {
		case 0x8000:
			controlBankWithAND(0x0F, prgRom8kMax)
			mapPrgRom8k(1, 0, value);
			mapPrgRom8kUpdate();
			return;
		case 0x9000: {
			switch (value & 0x03) {
				case 0:
					mirroring_V();
					break;
				case 1:
					mirroring_H();
					break;
				case 2:
					mirroring_SCR0();
					break;
				case 3:
					mirroring_SCR1();
					break;
			}
			return;
		}
		case 0xA000:
			controlBankWithAND(0x0F, prgRom8kMax)
			mapPrgRom8k(1, 1, value);
			mapPrgRom8kUpdate();
			return;
		case 0xB000:
			chrRom1kUpdate(0, 0xF0, 0);
			return;
		case 0xB001:
			chrRom1kUpdate(0, 0x0F, 4);
			return;
		case 0xB002:
			chrRom1kUpdate(1, 0xF0, 0);
			return;
		case 0xB003:
			chrRom1kUpdate(1, 0x0F, 4);
			return;
		case 0xC000:
			chrRom1kUpdate(2, 0xF0, 0);
			return;
		case 0xC001:
			chrRom1kUpdate(2, 0x0F, 4);
			return;
		case 0xC002:
			chrRom1kUpdate(3, 0xF0, 0);
			return;
		case 0xC003:
			chrRom1kUpdate(3, 0x0F, 4);
			return;
		case 0xD000:
			chrRom1kUpdate(4, 0xF0, 0);
			return;
		case 0xD001:
			chrRom1kUpdate(4, 0x0F, 4);
			return;
		case 0xD002:
			chrRom1kUpdate(5, 0xF0, 0);
			return;
		case 0xD003:
			chrRom1kUpdate(5, 0x0F, 4);
			return;
		case 0xE000:
			chrRom1kUpdate(6, 0xF0, 0);
			return;
		case 0xE001:
			chrRom1kUpdate(6, 0x0F, 4);
			return;
		case 0xE002:
			chrRom1kUpdate(7, 0xF0, 0);
			return;
		case 0xE003:
			chrRom1kUpdate(7, 0x0F, 4);
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_VRC2(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, vrc2.chrRomBank);

	return (EXIT_OK);
}

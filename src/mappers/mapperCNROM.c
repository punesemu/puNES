/*
 * mapperCNROM.c
 *
 *  Created on: 19/mag/2010
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

WORD chrRom8kMax;
BYTE type, mask, state;

void mapInit_CNROM(BYTE model) {
	chrRom8kMax = info.chrRom8kCount - 1;

	EXTCLCPUWRMEM(CNROM);

	mask = state = 0x00;

	if ((info.mapperType >= CNROM26CE27CE) && (info.mapperType <= CNROM26NCE27NCE)) {

		EXTCLRDCHR(CNROM);
		EXTCLSAVEMAPPER(CNROM);
		mapper.intStruct[0] = (BYTE *) &cnrom2627;
		mapper.intStructSize[0] = sizeof(cnrom2627);

		memset(&cnrom2627, 0x00, sizeof(cnrom2627));
		mask = 0x03;

		switch (info.mapperType) {
			case CNROM26CE27CE:
				state = 0x03;
				break;
			case CNROM26CE27NCE:
				state = 0x01;
				break;
			case CNROM26NCE27CE:
				state = 0x02;
				break;
			case CNROM26NCE27NCE:
				state = 0x00;
				break;
		}
	}

	type = model;
}
void extclCpuWrMem_CNROM(WORD address, BYTE value) {
	DBWORD bank;

	if (type == CNROMCNFL) {
		/* bus conflict */
		value &= prgRomRd(address);
	}

	if (mask) {
		if ((value & mask) == state) {
			cnrom2627.chrReadEnable = FALSE;
		} else {
			cnrom2627.chrReadEnable = TRUE;
		}
		value &= ~mask;
	}

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
BYTE extclSaveMapper_CNROM(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, cnrom2627.chrReadEnable);

	return (EXIT_OK);
}
BYTE extclRdChr_CNROM(WORD address) {
	if (cnrom2627.chrReadEnable == TRUE) {
		return (0xFF);
	}
	return (chr.bank1k[address >> 10][address & 0x3FF]);
}

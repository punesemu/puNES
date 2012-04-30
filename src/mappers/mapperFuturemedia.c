/*
 * mapperFuturemedia.c
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

WORD prgRom8kMax, chrRom1kMax;

void mapInit_Futuremedia(void) {
	prgRom8kMax = info.prgRom8kCount - 1;
	chrRom1kMax = info.chrRom1kCount - 1;

	EXTCLCPUWRMEM(Futuremedia);
	EXTCLSAVEMAPPER(Futuremedia);
	EXTCLPPU000TO34X(MMC3);
	EXTCLPPU000TO255(MMC3);
	EXTCLPPU256TO319(MMC3);
	EXTCLPPU320TO34X(MMC3);
	EXTCL2006UPDATE(MMC3);
	EXTCLIRQA12CLOCK(Futuremedia);
	EXTCLCPUEVERYCYCLE(Futuremedia);
	mapper.intStruct[0] = (BYTE *) &futuremedia;
	mapper.intStructSize[0] = sizeof(futuremedia);

	memset(&futuremedia, 0x00, sizeof(futuremedia));
	memset(&irqA12, 0x00, sizeof(irqA12));

	irqA12.present = TRUE;
}
void extclCpuWrMem_Futuremedia(WORD address, BYTE value) {
	switch (address) {
		case 0x8000:
		case 0x8001:
		case 0x8002:
		case 0x8003:
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, address & 0x0003, value);
			mapPrgRom8kUpdate();
			return;
		case 0xA000:
		case 0xA001:
		case 0xA002:
		case 0xA003:
		case 0xA004:
		case 0xA005:
		case 0xA006:
		case 0xA007:
			controlBank(chrRom1kMax)
			chr.bank1k[address & 0x0007] = &chr.data[value << 10];
			return;
		case 0xC001:
			irqA12.reload = value;
			return;
		case 0xC002:
			irq.high &= ~EXTIRQ;
			return;
		case 0xC003:
			irqA12.counter = irqA12.reload;
			return;
		case 0xD000:
			if (value & 0x01) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			return;
		case 0xE000:
			irqA12.enable = value & 0x01;
			return;
	}
}
BYTE extclSaveMapper_Futuremedia(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, futuremedia.delay);

	return (EXIT_OK);
}
void extclIrqA12Clock_Futuremedia(void) {
	if (irqA12.enable && irqA12.counter && !(--irqA12.counter)) {
		futuremedia.delay = 2;
	}
}
void extclCPUEveryCycle_Futuremedia(void) {
	if (futuremedia.delay && !(--futuremedia.delay)) {
		irq.high |= EXTIRQ;
	}
}

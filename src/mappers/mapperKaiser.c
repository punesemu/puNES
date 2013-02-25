/*
 * mapperKaiser.c
 *
 *  Created on: 4/ott/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "cpu.h"
#include "savestate.h"

WORD prgRom16kMax, prgRom8kMax, chrRom8kMax, chrRom4kMax, chrRom1kMax;

void mapInit_Kaiser(BYTE model) {
	prgRom16kMax = info.prgRom16kCount - 1;
	prgRom8kMax = info.prgRom8kCount - 1;
	chrRom8kMax = info.chrRom8kCount - 1;
	chrRom4kMax = info.chrRom4kCount - 1;
	chrRom1kMax = info.chrRom1kCount - 1;

	switch (model) {
		case KS202:
		case KS7032:
			EXTCL_CPU_WR_MEM(Kaiser_ks202);
			if (model == KS7032) {
				EXTCL_CPU_RD_MEM(Kaiser_ks202);
			}
			EXTCL_SAVE_MAPPER(Kaiser_ks202);
			EXTCL_CPU_EVERY_CYCLE(Kaiser_ks202);
			mapper.intStruct[0] = (BYTE *) &ks202;
			mapper.intStructSize[0] = sizeof(ks202);

			if (model == KS7032) {
				cpu.prg_ram_wr_active = FALSE;
				cpu.prg_ram_rd_active = FALSE;
			} else {
				info.prgRamPlus8kCount = 1;
				cpu.prg_ram_wr_active = TRUE;
				cpu.prg_ram_rd_active = TRUE;
			}

			if (info.reset >= HARD) {
				memset(&ks202, 0x00, sizeof(ks202));
				ks202.prgRamRd = &prg.rom[0];
			}
			break;
		case KS7058:
			EXTCL_CPU_WR_MEM(Kaiser_ks7058);
			break;
		case KS7022:
			EXTCL_CPU_WR_MEM(Kaiser_ks7022);
			EXTCL_CPU_RD_MEM(Kaiser_ks7022);
			EXTCL_SAVE_MAPPER(Kaiser_ks7022);
			mapper.intStruct[0] = (BYTE *) &ks7022;
			mapper.intStructSize[0] = sizeof(ks7022);

			info.mapperExtendRead = TRUE;

			memset(&ks7022, 0x00, sizeof(ks7022));

			if (info.reset >= HARD) {
				mapPrgRom8k(2, 0, 0);
				mapPrgRom8k(2, 2, 0);
			}
			break;
	}
}

void extcl_cpu_wr_mem_Kaiser_ks202(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
			ks202.reload = (ks202.reload & 0xFFF0) | (value & 0x0F);
			return;
		case 0x9000:
			ks202.reload = (ks202.reload & 0xFF0F) | ((value & 0x0F) << 4);
			return;
		case 0xA000:
			ks202.reload = (ks202.reload & 0xF0FF) | ((value & 0x0F) << 8);
			return;
		case 0xB000:
			ks202.reload = (ks202.reload & 0x0FFF) | ((value & 0x0F) << 12);
			return;
		case 0xC000:
			ks202.enabled = value & 0x0F;
			if (ks202.enabled) {
				ks202.count = ks202.reload;
			}
			irq.high &= ~EXTIRQ;
			return;
		case 0xD000:
			irq.high &= ~EXTIRQ;
			return;
		case 0xE000:
			ks202.reg = value;
			return;
		case 0xF000: {
			const BYTE save = value;
			const BYTE slot = (ks202.reg & 0x0F) - 1;

			switch (slot) {
				case 0:
				case 1:
				case 2: {
					value = (mapper.romMapTo[slot] & 0x10) | (value & 0x0F);
					controlBank(prgRom8kMax)
					mapPrgRom8k(1, slot, value);
					mapPrgRom8kUpdate();
					break;
				}
				case 3:
					controlBank(prgRom8kMax)
					ks202.prgRamRd = &prg.rom[value << 13];
					break;
			}

			switch (address & 0x0C00) {
				case 0x0000:
					address &= 0x0003;
					if (address < 3) {
						value = (save & 0x10) | (mapper.romMapTo[address] & 0x0F);
						controlBank(prgRom8kMax)
						mapPrgRom8k(1, address, value);
						mapPrgRom8kUpdate();
					}
					break;
				case 0x0800:
					if (save & 0x01) {
						mirroring_V();
					} else {
						mirroring_H();
					}
					break;
				case 0x0C00:
					value = save;
					controlBank(chrRom1kMax)
					chr.bank1k[address & 0x0007] = &chr.data[value << 10];
					break;
			}
			return;
		}
	}
}
BYTE extcl_cpu_rd_mem_Kaiser_ks202(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (ks202.prgRamRd[address & 0x1FFF]);
}
BYTE extcl_save_mapper_Kaiser_ks202(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, ks202.enabled);
	savestateEle(mode, slot, ks202.count);
	savestateEle(mode, slot, ks202.reload);
	savestateEle(mode, slot, ks202.delay);
	savestateEle(mode, slot, ks202.reg);
	savestatePos(mode, slot, prg.rom, ks202.prgRamRd);

	return (EXIT_OK);

}
void extcl_cpu_every_cycle_Kaiser_ks202(void) {
	if (ks202.delay && !(--ks202.delay)) {
		irq.high |= EXTIRQ;
	}

	if (!ks202.enabled) {
		return;
	}

	if (++ks202.count == 0xFFFF) {
		ks202.count = ks202.reload;
		ks202.delay = 1;
	}
}

void extcl_cpu_wr_mem_Kaiser_ks7058(WORD address, BYTE value) {
	DBWORD bank;

	switch (address & 0xF080) {
		case 0xF000:
			controlBank(chrRom4kMax)
			bank = value << 12;
			chr.bank1k[0] = &chr.data[bank];
			chr.bank1k[1] = &chr.data[bank | 0x0400];
			chr.bank1k[2] = &chr.data[bank | 0x0800];
			chr.bank1k[3] = &chr.data[bank | 0x0C00];
			return;
		case 0xF080:
			controlBank(chrRom4kMax)
			bank = value << 12;
			chr.bank1k[4] = &chr.data[bank];
			chr.bank1k[5] = &chr.data[bank | 0x0400];
			chr.bank1k[6] = &chr.data[bank | 0x0800];
			chr.bank1k[7] = &chr.data[bank | 0x0C00];
			return;
	}
}

void extcl_cpu_wr_mem_Kaiser_ks7022(WORD address, BYTE value) {
	switch (address) {
		case 0x8000:
			if (value & 0x04) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			return;
		case 0xA000:
			ks7022.reg = value & 0x0F;
			return;
	}
}
BYTE extcl_cpu_rd_mem_Kaiser_ks7022(WORD address, BYTE openbus, BYTE before) {
	if (address == 0xFFFC) {
		BYTE value = ks7022.reg;
		DBWORD bank;

		controlBank(prgRom16kMax)
		mapPrgRom8k(2, 0, value);
		mapPrgRom8k(2, 2, value);
		mapPrgRom8kUpdate();

		value = ks7022.reg;
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

		return (prgRomRd(address));
	}

	return (openbus);
}
BYTE extcl_save_mapper_Kaiser_ks7022(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, ks7022.reg);

	return (EXIT_OK);

}

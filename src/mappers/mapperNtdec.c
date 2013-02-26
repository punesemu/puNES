/*
 * mapperNtdec.c
 *
 *  Created on: 26/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

#define asderChrCtrlBank(src, ctrl)\
	if (src > ctrl) {\
		src &= ctrl;\
	}
#define asderChr2kUpdate(shift, slot, slot1, slot2)\
	newValue = ((chrHigh << shift) & 0x0080) | asder.reg[slot];\
	asderChrCtrlBank(newValue, chrRom2kMax)\
	bank = newValue << 11;\
	chr.bank_1k[slot1] = &chr.data[bank];\
	chr.bank_1k[slot2] = &chr.data[bank | 0x0400]
#define asderChr1kUpdate(shift, slot)\
	newValue = ((chrHigh << shift) & 0x0100) | asder.reg[slot];\
	asderChrCtrlBank(newValue, chrRom1kMax)\
	bank = newValue << 10;\
	chr.bank_1k[slot] = &chr.data[bank]

WORD prgRom32kMax, prgRom8kMax, chrRom4kMax, chrRom2kMax, chrRom1kMax;

void map_init_Ntdec(BYTE model) {
	prgRom32kMax = (info.prg_rom_16k_count >> 1) - 1;
	prgRom8kMax = info.prg_rom_8k_count - 1;
	chrRom4kMax = info.chr_rom_4k_count - 1;
	chrRom2kMax = (info.chr_rom_1k_count >> 1) - 1;
	chrRom1kMax = info.chr_rom_1k_count - 1;

	switch (model) {
		case ASDER:
			EXTCL_CPU_WR_MEM(Ntdec_asder);
			EXTCL_SAVE_MAPPER(Ntdec_asder);
			mapper.internal_struct[0] = (BYTE *) &asder;
			mapper.internal_struct_size[0] = sizeof(asder);

			if (info.reset >= HARD) {
				memset(&asder, 0x00, sizeof(asder));
			}

			break;
		case FHERO:
			EXTCL_CPU_WR_MEM(Ntdec_fhero);

			info.mapper_extend_wr = TRUE;

			if (info.reset >= HARD) {
				map_prg_rom_8k(4, 0, prgRom32kMax);
			}
			break;
	}

	mirroring_V();
}

void extcl_cpu_wr_mem_Ntdec_asder(WORD address, BYTE value) {
	switch (address & 0xE001) {
		case 0x8000:
			asder.address = value & 0x07;
			return;
		case 0xA000: {
			switch (asder.address) {
				case 0:
				case 1:
					control_bank(prgRom8kMax)
					map_prg_rom_8k(1, asder.address, value);
					map_prg_rom_8k_update();
					return;
				case 2:
				case 3:
					asder.reg[asder.address] = value >> 1;
					break;
				case 4:
				case 5:
				case 6:
				case 7:
					asder.reg[asder.address] = value;
					break;
			}
			break;
		}
		case 0xC000:
			asder.reg[0] = value;
			break;
		case 0xE000:
			asder.reg[1] = value;
			if (value & 0x01) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			break;
	}
	{
		const WORD chrHigh = (asder.reg[1] & 0x02) ? asder.reg[0] : 0;
		WORD newValue;
		DBWORD bank;

		asderChr2kUpdate(5, 2, 0, 1);
		asderChr2kUpdate(4, 3, 2, 3);
		asderChr1kUpdate(4, 4);
		asderChr1kUpdate(3, 5);
		asderChr1kUpdate(2, 6);
		asderChr1kUpdate(1, 7);
	}
}
BYTE extcl_save_mapper_Ntdec_asder(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, asder.address);
	save_slot_ele(mode, slot, asder.reg);

	return (EXIT_OK);
}

void extcl_cpu_wr_mem_Ntdec_fhero(WORD address, BYTE value) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return;
	}

	switch (address & 0x0003) {
		case 0: {
			DBWORD bank;

			value >>= 2;
			control_bank(chrRom4kMax)
			bank = value << 12;
			chr.bank_1k[0] = &chr.data[bank];
			chr.bank_1k[1] = &chr.data[bank | 0x0400];
			chr.bank_1k[2] = &chr.data[bank | 0x0800];
			chr.bank_1k[3] = &chr.data[bank | 0x0C00];
			return;
		}
		case 1: {
			DBWORD bank;

			value >>= 1;
			control_bank(chrRom2kMax)
			bank = value << 11;
			chr.bank_1k[4] = &chr.data[bank];
			chr.bank_1k[5] = &chr.data[bank | 0x0400];
			return;
		}
		case 2: {
			DBWORD bank;

			value >>= 1;
			control_bank(chrRom2kMax)
			bank = value << 11;
			chr.bank_1k[6] = &chr.data[bank];
			chr.bank_1k[7] = &chr.data[bank | 0x0400];
			return;
		}
		case 3:
			control_bank(prgRom8kMax)
			map_prg_rom_8k(1, 0, value);
			map_prg_rom_8k_update();
			return;
	}
}

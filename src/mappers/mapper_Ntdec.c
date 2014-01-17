/*
 * mapper_Ntdec.c
 *
 *  Created on: 26/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "save_slot.h"

#define asder_chr_ctrl_bank(src, ctrl)\
	if (src > ctrl) {\
		src &= ctrl;\
	}
#define asder_chr_2k_update(shift, slot, slot1, slot2)\
	new_value = ((chr_high << shift) & 0x0080) | asder.reg[slot];\
	asder_chr_ctrl_bank(new_value, chr_rom_2k_max)\
	bank = new_value << 11;\
	chr.bank_1k[slot1] = &chr.data[bank];\
	chr.bank_1k[slot2] = &chr.data[bank | 0x0400]
#define asder_chr_1k_update(shift, slot)\
	new_value = ((chr_high << shift) & 0x0100) | asder.reg[slot];\
	asder_chr_ctrl_bank(new_value, chr_rom_1k_max)\
	bank = new_value << 10;\
	chr.bank_1k[slot] = &chr.data[bank]

WORD prg_rom_32k_max, prg_rom_8k_max, chr_rom_4k_max, chr_rom_2k_max, chr_rom_1k_max;

void map_init_Ntdec(BYTE model) {
	prg_rom_32k_max = (info.prg.rom.banks_16k >> 1) - 1;
	prg_rom_8k_max = info.prg.rom.banks_8k - 1;
	chr_rom_4k_max = info.chr.rom.banks_4k - 1;
	chr_rom_2k_max = (info.chr.rom.banks_1k >> 1) - 1;
	chr_rom_1k_max = info.chr.rom.banks_1k - 1;

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

			info.mapper.extend_wr = TRUE;

			if (info.reset >= HARD) {
				map_prg_rom_8k(4, 0, prg_rom_32k_max);
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
					control_bank(prg_rom_8k_max)
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
		DBWORD bank;
		const WORD chr_high = (asder.reg[1] & 0x02) ? asder.reg[0] : 0;
		WORD new_value;

		asder_chr_2k_update(5, 2, 0, 1);
		asder_chr_2k_update(4, 3, 2, 3);
		asder_chr_1k_update(4, 4);
		asder_chr_1k_update(3, 5);
		asder_chr_1k_update(2, 6);
		asder_chr_1k_update(1, 7);
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
			control_bank(chr_rom_4k_max)
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
			control_bank(chr_rom_2k_max)
			bank = value << 11;
			chr.bank_1k[4] = &chr.data[bank];
			chr.bank_1k[5] = &chr.data[bank | 0x0400];
			return;
		}
		case 2: {
			DBWORD bank;

			value >>= 1;
			control_bank(chr_rom_2k_max)
			bank = value << 11;
			chr.bank_1k[6] = &chr.data[bank];
			chr.bank_1k[7] = &chr.data[bank | 0x0400];
			return;
		}
		case 3:
			control_bank(prg_rom_8k_max)
			map_prg_rom_8k(1, 0, value);
			map_prg_rom_8k_update();
			return;
	}
}

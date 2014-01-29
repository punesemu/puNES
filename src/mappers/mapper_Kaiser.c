/*
 * mapper_Kaiser.c
 *
 *  Created on: 4/ott/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

WORD chr_rom_8k_max, chr_rom_4k_max;

void map_init_Kaiser(BYTE model) {
	chr_rom_8k_max = info.chr.rom.banks_8k - 1;
	chr_rom_4k_max = info.chr.rom.banks_4k - 1;

	switch (model) {
		case KS202:
		case KS7032:
			EXTCL_CPU_WR_MEM(Kaiser_ks202);
			if (model == KS7032) {
				EXTCL_CPU_RD_MEM(Kaiser_ks202);
			}
			EXTCL_SAVE_MAPPER(Kaiser_ks202);
			EXTCL_CPU_EVERY_CYCLE(Kaiser_ks202);
			mapper.internal_struct[0] = (BYTE *) &ks202;
			mapper.internal_struct_size[0] = sizeof(ks202);

			if (model == KS7032) {
				cpu.prg_ram_wr_active = FALSE;
				cpu.prg_ram_rd_active = FALSE;
			} else {
				info.prg.ram.banks_8k_plus = 1;
				cpu.prg_ram_wr_active = TRUE;
				cpu.prg_ram_rd_active = TRUE;
			}

			if (info.reset >= HARD) {
				memset(&ks202, 0x00, sizeof(ks202));
				ks202.prg_ram_rd = &prg.rom[0];
			}
			break;
		case KS7058:
			EXTCL_CPU_WR_MEM(Kaiser_ks7058);
			break;
		case KS7022:
			EXTCL_CPU_WR_MEM(Kaiser_ks7022);
			EXTCL_CPU_RD_MEM(Kaiser_ks7022);
			EXTCL_SAVE_MAPPER(Kaiser_ks7022);
			mapper.internal_struct[0] = (BYTE *) &ks7022;
			mapper.internal_struct_size[0] = sizeof(ks7022);

			info.mapper.extend_rd = TRUE;

			memset(&ks7022, 0x00, sizeof(ks7022));

			if (info.reset >= HARD) {
				map_prg_rom_8k(2, 0, 0);
				map_prg_rom_8k(2, 2, 0);
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
			irq.high &= ~EXT_IRQ;
			return;
		case 0xD000:
			irq.high &= ~EXT_IRQ;
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
					value = (mapper.rom_map_to[slot] & 0x10) | (value & 0x0F);
					control_bank(info.prg.rom.max.banks_8k)
					map_prg_rom_8k(1, slot, value);
					map_prg_rom_8k_update();
					break;
				}
				case 3:
					control_bank(info.prg.rom.max.banks_8k)
					ks202.prg_ram_rd = &prg.rom[value << 13];
					break;
			}

			switch (address & 0x0C00) {
				case 0x0000:
					address &= 0x0003;
					if (address < 3) {
						value = (save & 0x10) | (mapper.rom_map_to[address] & 0x0F);
						control_bank(info.prg.rom.max.banks_8k)
						map_prg_rom_8k(1, address, value);
						map_prg_rom_8k_update();
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
					control_bank(info.chr.rom.max.banks_1k)
					chr.bank_1k[address & 0x0007] = &chr.data[value << 10];
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

	return (ks202.prg_ram_rd[address & 0x1FFF]);
}
BYTE extcl_save_mapper_Kaiser_ks202(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, ks202.enabled);
	save_slot_ele(mode, slot, ks202.count);
	save_slot_ele(mode, slot, ks202.reload);
	save_slot_ele(mode, slot, ks202.delay);
	save_slot_ele(mode, slot, ks202.reg);
	save_slot_pos(mode, slot, prg.rom, ks202.prg_ram_rd);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_Kaiser_ks202(void) {
	if (ks202.delay && !(--ks202.delay)) {
		irq.high |= EXT_IRQ;
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
			control_bank(chr_rom_4k_max)
			bank = value << 12;
			chr.bank_1k[0] = &chr.data[bank];
			chr.bank_1k[1] = &chr.data[bank | 0x0400];
			chr.bank_1k[2] = &chr.data[bank | 0x0800];
			chr.bank_1k[3] = &chr.data[bank | 0x0C00];
			return;
		case 0xF080:
			control_bank(chr_rom_4k_max)
			bank = value << 12;
			chr.bank_1k[4] = &chr.data[bank];
			chr.bank_1k[5] = &chr.data[bank | 0x0400];
			chr.bank_1k[6] = &chr.data[bank | 0x0800];
			chr.bank_1k[7] = &chr.data[bank | 0x0C00];
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

		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k(2, 2, value);
		map_prg_rom_8k_update();

		value = ks7022.reg;
		control_bank(chr_rom_8k_max)
		bank = value << 13;
		chr.bank_1k[0] = &chr.data[bank];
		chr.bank_1k[1] = &chr.data[bank | 0x0400];
		chr.bank_1k[2] = &chr.data[bank | 0x0800];
		chr.bank_1k[3] = &chr.data[bank | 0x0C00];
		chr.bank_1k[4] = &chr.data[bank | 0x1000];
		chr.bank_1k[5] = &chr.data[bank | 0x1400];
		chr.bank_1k[6] = &chr.data[bank | 0x1800];
		chr.bank_1k[7] = &chr.data[bank | 0x1C00];

		return (prg_rom_rd(address));
	}

	return (openbus);
}
BYTE extcl_save_mapper_Kaiser_ks7022(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, ks7022.reg);

	return (EXIT_OK);
}

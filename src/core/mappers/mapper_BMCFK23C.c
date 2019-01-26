/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

static void prg_fix_BMCFK23CPW(BYTE value);
static void prg_swap_BMCFK23CPW(WORD address, WORD value);
static void chr_fix_BMCFK23CPW(BYTE value);
static void chr_swap_BMCFK23CCW(WORD address, WORD value);

void map_init_BMCFK23C(void) {
	EXTCL_CPU_WR_MEM(BMCFK23C);
	EXTCL_SAVE_MAPPER(BMCFK23C);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *) &bmcfk23c;
	mapper.internal_struct_size[0] = sizeof(bmcfk23c);
	mapper.internal_struct[1] = (BYTE *) &mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	if (info.id & BMCFK23CA) {
		EXTCL_WR_CHR(BMCFK23C);
		/* utilizza 0x2000 di CHR RAM extra */
		map_chr_ram_extra_init(0x2000);
	}

	if (info.reset >= HARD) {
		memset(&mmc3, 0x00, sizeof(mmc3));
		memset(&irqA12, 0x00, sizeof(irqA12));
		memset(&bmcfk23c, 0x00, sizeof(bmcfk23c));
		map_chr_ram_extra_reset();
	}

	bmcfk23c.reg[0] = bmcfk23c.reg[1] = bmcfk23c.reg[2] = bmcfk23c.reg[3] = 0;
	bmcfk23c.reg[4] = bmcfk23c.reg[5] = bmcfk23c.reg[6] = bmcfk23c.reg[7] = 0xFF;

	bmcfk23c.mmc3[0] = 0;
	bmcfk23c.mmc3[1] = 2;
	bmcfk23c.mmc3[2] = 4;
	bmcfk23c.mmc3[3] = 5;
	bmcfk23c.mmc3[4] = 6;
	bmcfk23c.mmc3[5] = 7;
	bmcfk23c.mmc3[6] = 0;
	bmcfk23c.mmc3[7] = 1;

	bmcfk23c.chr_map[0] = 0xFFFF;
	bmcfk23c.chr_map[1] = 0xFFFF;
	bmcfk23c.chr_map[2] = 0xFFFF;
	bmcfk23c.chr_map[3] = 0xFFFF;
	bmcfk23c.chr_map[4] = 0xFFFF;
	bmcfk23c.chr_map[5] = 0xFFFF;
	bmcfk23c.chr_map[6] = 0xFFFF;
	bmcfk23c.chr_map[7] = 0xFFFF;

	{
		BYTE prg_bonus = 0;

		switch (info.id & ~BMCFK23CA) {
			case BMCFK23C_1:
				prg_bonus = 1;
				break;
			case BMCFK23C_0:
			default:
				break;
		}

		bmcfk23c.prg_mask = 0x7F >> prg_bonus;
	}

	prg_fix_BMCFK23CPW(mmc3.bank_to_update);
	chr_fix_BMCFK23CPW(mmc3.bank_to_update);

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_BMCFK23C(WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		if (address & (1 << (bmcfk23c.dipswitch + 4))) {
			BYTE remap = FALSE;

			bmcfk23c.reg[address & 0x0003] = value;
			/*
			 * sometimes writing to reg0 causes remappings to occur.
			 * We think the 2 signifies this. If not, 0x24 is a value
			 * that is known to work however, the low 4 bits are known
			 * to control the mapping mode, so 0x20 is more likely to
			 * be the immediate remap flag.
			 */
			remap |= ((bmcfk23c.reg[0] & 0xF0) == 0x20);
			/*
			 * this is an actual mapping reg. I think reg0 controls what
			 * happens when reg1 is written. Anyway, we have to immediately
			 * remap these.
			 */
			remap |= (address & 0x0003) == 1;
			/* this too */
			remap |= (address & 0x0003) == 2;
			if (remap) {
				prg_fix_BMCFK23CPW(mmc3.bank_to_update);
				chr_fix_BMCFK23CPW(mmc3.bank_to_update);
			}
		}
		if (info.id & BMCFK23CA) {
			if (bmcfk23c.reg[3] & 0x02) {
				// hacky hacky! if someone wants extra banking
				// then for sure doesn't want mode 4 for it! (allow to run A version
				// boards on normal mapper)
				bmcfk23c.reg[0] &= ~7;
			}
		}
		return;
	}

	if (address >= 0x8000) {
		BYTE mask;

		if (bmcfk23c.reg[0] & 0x40) {
			switch (address & 0xF003) {
				case 0xA000:
				case 0xA001:
				case 0xE000:
				case 0xE001:
					extcl_cpu_wr_mem_MMC3(address, value);
					return;
				default:
					mask = address & 0x03;

					if (bmcfk23c.reg[0] & 0x10) {
						mask = (address >> 2) & 0x03;
					}
					break;
			}
			if (bmcfk23c.unromchr != (value & mask)) {
				bmcfk23c.unromchr = value & mask;
				chr_fix_BMCFK23CPW(mmc3.bank_to_update);
			}
		} else {
			if ((address == 0x8001) && ((bmcfk23c.reg[3] & 0x02) && (mmc3.bank_to_update & 0x08))) {
				bmcfk23c.reg[4 | (mmc3.bank_to_update & 0x03)] = value;
				prg_fix_BMCFK23CPW(mmc3.bank_to_update);
				chr_fix_BMCFK23CPW(mmc3.bank_to_update);
			} else if (address < 0xC000) {
				/*
				if (info.format == UNIF_FORMAT) {
				//if (info.format != UNIF_FORMAT) {
					// hacky... strange behaviour, must be bit scramble due to pcb layot restrictions
					// check if it not interfer with other dumps
					if (address == 0x8000) {
						if (value == 0x46) {


							fprintf(stderr, "0x%04X : 0x%02X\n", address, value);


							value = 0x47;
						} else if (value == 0x47) {


							fprintf(stderr, "0x%04X : 0x%02X\n", address, value);


							value = 0x46;
						}
					}
				}
				*/
				switch (address & 0xE001) {
					case 0x8000:
						if ((value & 0x40) != (mmc3.bank_to_update & 0x40)) {
							prg_fix_BMCFK23CPW(value);
						}
						if ((value & 0x80) != (mmc3.bank_to_update & 0x80)) {
							chr_fix_BMCFK23CPW(value);
						}
						mmc3.bank_to_update = value;
						return;
					case 0x8001: {
						WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

						bmcfk23c.mmc3[mmc3.bank_to_update & 0x07] = value;

						switch (mmc3.bank_to_update & 0x07) {
							case 0:
								chr_swap_BMCFK23CCW(cbase ^ 0x0000, value & (~1));
								chr_swap_BMCFK23CCW(cbase ^ 0x0400, value | 1);
								return;
							case 1:
								chr_swap_BMCFK23CCW(cbase ^ 0x0800, value & (~1));
								chr_swap_BMCFK23CCW(cbase ^ 0x0C00, value | 1);
								return;
							case 2:
								chr_swap_BMCFK23CCW(cbase ^ 0x1000, value);
								return;
							case 3:
								chr_swap_BMCFK23CCW(cbase ^ 0x1400, value);
								return;
							case 4:
								chr_swap_BMCFK23CCW(cbase ^ 0x1800, value);
								return;
							case 5:
								chr_swap_BMCFK23CCW(cbase ^ 0x1C00, value);
								return;
							case 6:
								if (mmc3.bank_to_update & 0x40) {
									prg_swap_BMCFK23CPW(0xC000, value);
								} else {
									prg_swap_BMCFK23CPW(0x8000, value);
								}
								return;
							case 7:
								prg_swap_BMCFK23CPW(0xA000, value);
								return;
						}
						return;
					}
					case 0xA000:
						/*
						 * secondo me il bit 7 ha una qualche funzione,
						 * per il momento lo salvo.
						 */
						bmcfk23c.A000 = value;
						break;
					case 0xA001:
						bmcfk23c.A001 = value;
						return;
				}
				extcl_cpu_wr_mem_MMC3(address, value);
				//prg_fix_BMCFK23CPW(mmc3.bank_to_update);
			} else {
				extcl_cpu_wr_mem_MMC3(address, value);
			}
		}
	}
}
void extcl_wr_chr_BMCFK23C(WORD address, BYTE value) {
	BYTE slot = address >> 10;

	if ((bmcfk23c.chr_map[slot] != 0xFFFF) || (mapper.write_vram == TRUE)) {
		chr.bank_1k[slot][address & 0x3FF] = value;
	}
}
BYTE extcl_save_mapper_BMCFK23C(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, bmcfk23c.dipswitch);
	save_slot_ele(mode, slot, bmcfk23c.unromchr);
	save_slot_ele(mode, slot, bmcfk23c.A000);
	save_slot_ele(mode, slot, bmcfk23c.A001);
	save_slot_ele(mode, slot, bmcfk23c.reg);
	save_slot_ele(mode, slot, bmcfk23c.mmc3);
	save_slot_ele(mode, slot, bmcfk23c.chr_map);
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (info.id & BMCFK23CA) {
		save_slot_mem(mode, slot, chr.extra.data, chr.extra.size, FALSE)
		if (mode == SAVE_SLOT_READ) {
			chr_fix_BMCFK23CPW(mmc3.bank_to_update);
		}
	}

	return (EXIT_OK);
}

static void prg_fix_BMCFK23CPW(BYTE value) {
	if (value & 0x40) {
		prg_swap_BMCFK23CPW(0x8000, ~1);
		prg_swap_BMCFK23CPW(0xC000, bmcfk23c.mmc3[6]);
	} else {
		prg_swap_BMCFK23CPW(0x8000, bmcfk23c.mmc3[6]);
		prg_swap_BMCFK23CPW(0xC000, ~1);
	}
	prg_swap_BMCFK23CPW(0xA000, bmcfk23c.mmc3[7]);
	prg_swap_BMCFK23CPW(0xE000, ~0);
}
static void prg_swap_BMCFK23CPW(WORD address, WORD value) {
	if ((bmcfk23c.reg[0] & 0x07) == 0x04) {
		value = bmcfk23c.reg[1] >> 1;
		control_bank(info.prg.rom[0].max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	} else if ((bmcfk23c.reg[0] & 0x07) == 0x03) {
		value = bmcfk23c.reg[1];
		control_bank(info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k(2, 2, value);
	} else {
		if (bmcfk23c.reg[0] & 0x03) {
			WORD blocksize = 6 - (bmcfk23c.reg[0] & 0x03);
			WORD mask = (1 << blocksize) - 1;

			value &= mask;
			value |= (bmcfk23c.reg[1] << 1);
		} else {
			value &= bmcfk23c.prg_mask;
		}

		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, (address >> 13) & 0x03, value);

		if (bmcfk23c.reg[3] & 0x02) {
			value = bmcfk23c.reg[4];
			control_bank(info.prg.rom[0].max.banks_8k)
			map_prg_rom_8k(1, 2, value);

			value = bmcfk23c.reg[5];
			control_bank(info.prg.rom[0].max.banks_8k)
			map_prg_rom_8k(1, 3, value);
		}
	}

	map_prg_rom_8k_update();

	//setprg8r(0x10, 0x6000, A001B & 3);
}

static void chr_fix_BMCFK23CPW(BYTE value) {
	WORD cbase = (value & 0x80) << 5;

	chr_swap_BMCFK23CCW(cbase ^ 0x0000, bmcfk23c.mmc3[0] & (~1));
	chr_swap_BMCFK23CCW(cbase ^ 0x0400, bmcfk23c.mmc3[0] |   1);
	chr_swap_BMCFK23CCW(cbase ^ 0x0800, bmcfk23c.mmc3[1] & (~1));
	chr_swap_BMCFK23CCW(cbase ^ 0x0C00, bmcfk23c.mmc3[1] |   1);

	chr_swap_BMCFK23CCW(cbase ^ 0x1000, bmcfk23c.mmc3[2]);
	chr_swap_BMCFK23CCW(cbase ^ 0x1400, bmcfk23c.mmc3[3]);
	chr_swap_BMCFK23CCW(cbase ^ 0x1800, bmcfk23c.mmc3[4]);
	chr_swap_BMCFK23CCW(cbase ^ 0x1c00, bmcfk23c.mmc3[5]);
}
static void chr_swap_BMCFK23CCW(WORD address, WORD value) {
	DBWORD bank;

	if (bmcfk23c.reg[0] & 0x40) {
		value = bmcfk23c.reg[2] | bmcfk23c.unromchr;
		control_bank(info.chr.rom[0].max.banks_8k)
		bank = value << 13;
		chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
		chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
		chr.bank_1k[2] = chr_chip_byte_pnt(0, bank | 0x0800);
		chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0C00);
		chr.bank_1k[4] = chr_chip_byte_pnt(0, bank | 0x1000);
		chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x1400);
		chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x1800);
		chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x1C00);

		bmcfk23c.chr_map[0] = 0xFFFF;
		bmcfk23c.chr_map[1] = 0xFFFF;
		bmcfk23c.chr_map[2] = 0xFFFF;
		bmcfk23c.chr_map[3] = 0xFFFF;
		bmcfk23c.chr_map[4] = 0xFFFF;
		bmcfk23c.chr_map[5] = 0xFFFF;
		bmcfk23c.chr_map[6] = 0xFFFF;
		bmcfk23c.chr_map[7] = 0xFFFF;
	} else if (bmcfk23c.reg[0] & 0x20) {
		if (chr.extra.data) {
			value &= 0x07;
			chr.bank_1k[address >> 10] = &chr.extra.data[value << 10];
			bmcfk23c.chr_map[address >> 10] = value;
		}
	} else {
		WORD base = (bmcfk23c.reg[2] & 0x7F) << 3;

		value |= base;
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[address >> 10] = chr_chip_byte_pnt(0, value << 10);
		bmcfk23c.chr_map[address >> 10] = 0xFFFF;

		if (bmcfk23c.reg[3] & 0x02) {
			WORD cbase = (mmc3.bank_to_update & 0x80) >> 5;

			value = base | bmcfk23c.mmc3[0];
			control_bank(info.chr.rom[0].max.banks_1k)
			chr.bank_1k[cbase ^ 0x00] = chr_chip_byte_pnt(0, value << 10);
			bmcfk23c.chr_map[cbase ^ 0x00] = 0xFFFF;

			value = base | bmcfk23c.reg[6];
			control_bank(info.chr.rom[0].max.banks_1k)
			chr.bank_1k[cbase ^ 0x01] = chr_chip_byte_pnt(0, value << 10);
			bmcfk23c.chr_map[cbase ^ 0x01] = 0xFFFF;

			value = base | bmcfk23c.mmc3[1];
			control_bank(info.chr.rom[0].max.banks_1k)
			chr.bank_1k[cbase ^ 0x02] = chr_chip_byte_pnt(0, value << 10);
			bmcfk23c.chr_map[cbase ^ 0x02] = 0xFFFF;

			value = base | bmcfk23c.reg[7];
			control_bank(info.chr.rom[0].max.banks_1k)
			chr.bank_1k[cbase ^ 0x03] = chr_chip_byte_pnt(0, value << 10);
			bmcfk23c.chr_map[cbase ^ 0x03] = 0xFFFF;
		}
	}
}


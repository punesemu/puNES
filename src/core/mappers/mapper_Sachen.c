/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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
#include "save_slot.h"

INLINE static void prg_fix_Sachen_sa74374x(void);
INLINE static void chr_fix_Sachen_sa74374x(void);
INLINE static void mirroring_fix_Sachen_sa74374x(void);


struct _sa8259 {
	BYTE ctrl;
	BYTE reg[8];
} sa8259;
struct _tcu0x {
	BYTE reg[4];
	BYTE RRR;
} tcu0x;
struct _sa74374x {
	BYTE index;
	BYTE reg[8];
} sa74374x;
struct _sachentmp {
	BYTE type;
	BYTE shift;
	BYTE ored[3];
	BYTE dipswitch;
} sachentmp;

void map_init_Sachen(BYTE model) {
	switch (model) {
		case SA0036:
			EXTCL_CPU_WR_MEM(Sachen_sa0036);
			break;
		case SA0037:
			EXTCL_CPU_WR_MEM(Sachen_sa0037);

			if (info.reset >= HARD) {
				if (info.prg.rom.max.banks_32k != 0xFFFF) {
					map_prg_rom_8k(4, 0, 0);
				}
			}
			break;
		case SA8259A:
		case SA8259B:
		case SA8259C:
		case SA8259D: {
			EXTCL_CPU_WR_MEM(Sachen_sa8259x);
			EXTCL_SAVE_MAPPER(Sachen_sa8259x);
			mapper.internal_struct[0] = (BYTE *)&sa8259;
			mapper.internal_struct_size[0] = sizeof(sa8259);

			info.mapper.extend_wr = TRUE;

			if (info.reset >= HARD) {
				memset(&sa8259, 0x00, sizeof(sa8259));

				if (info.prg.rom.max.banks_32k != 0xFFFF) {
					map_prg_rom_8k(4, 0, 0);
				}
			}

			switch (model) {
				case SA8259A:
					sachentmp.shift = 1;
					sachentmp.ored[0] = 1;
					sachentmp.ored[1] = 0;
					sachentmp.ored[2] = 1;
					break;
				case SA8259B:
					sachentmp.shift = 0;
					sachentmp.ored[0] = 0;
					sachentmp.ored[1] = 0;
					sachentmp.ored[2] = 0;
					break;
				case SA8259C:
					sachentmp.shift = 2;
					sachentmp.ored[0] = 1;
					sachentmp.ored[1] = 2;
					sachentmp.ored[2] = 3;
					break;
				case SA8259D:
					if (!mapper.write_vram) {
						const DBWORD bank = info.chr.rom.max.banks_4k << 12;

						chr.bank_1k[4] = chr_pnt(bank);
						chr.bank_1k[5] = chr_pnt(bank | 0x0400);
						chr.bank_1k[6] = chr_pnt(bank | 0x0800);
						chr.bank_1k[7] = chr_pnt(bank | 0x0C00);
					}
					break;
			}
			break;
		}
		case TCA01:
			EXTCL_CPU_WR_MEM(Sachen_tca01);
			EXTCL_CPU_RD_MEM(Sachen_tca01);
			break;
		case TCU01:
			EXTCL_CPU_WR_MEM(Sachen_tcu01);
			EXTCL_CPU_RD_MEM(Sachen_tcu01);
			EXTCL_SAVE_MAPPER(Sachen_tcu01);
			mapper.internal_struct[0] = (BYTE *)&tcu0x;
			mapper.internal_struct_size[0] = sizeof(tcu0x);

			memset(&tcu0x, 0x00, sizeof(tcu0x));

			tcu0x.reg[1] = 0x01;

			info.mapper.extend_wr = TRUE;

			extcl_cpu_wr_mem_Sachen_tcu02(0x8000, 0x00);
			break;
		case TCU02:
			EXTCL_CPU_WR_MEM(Sachen_tcu02);
			EXTCL_CPU_RD_MEM(Sachen_tcu02);
			EXTCL_SAVE_MAPPER(Sachen_tcu02);
			mapper.internal_struct[0] = (BYTE *)&tcu0x;
			mapper.internal_struct_size[0] = sizeof(tcu0x);

			memset(&tcu0x, 0x00, sizeof(tcu0x));
			
			tcu0x.reg[1] = 0x01;

			info.mapper.extend_wr = TRUE;

			extcl_cpu_wr_mem_Sachen_tcu02(0x8000, 0x00);
			break;
		case SA72007:
			EXTCL_CPU_WR_MEM(Sachen_sa72007);

			info.mapper.extend_wr = TRUE;
			break;
		case SA72008:
			EXTCL_CPU_WR_MEM(Sachen_sa72008);

			info.mapper.extend_wr = TRUE;
			break;
		case SA74374A:
		case SA74374B:
			EXTCL_AFTER_MAPPER_INIT(Sachen_sa74374x);
			EXTCL_CPU_WR_MEM(Sachen_sa74374x);
			EXTCL_CPU_RD_MEM(Sachen_sa74374x);
			EXTCL_SAVE_MAPPER(Sachen_sa74374x);
			mapper.internal_struct[0] = (BYTE *)&sa74374x;
			mapper.internal_struct_size[0] = sizeof(sa74374x);

			if (info.crc32.chr == 0xD74E9FF2) { // Poker III 5-in-1 (Sachen) [!].nes
				info.mapper.id = 150;
				model = SA74374B;
			} else if (
				(info.crc32.chr == 0xA5DD3E05) || // Honey Peach (Asia) (Ja) (Unl).nes
				(info.crc32.chr == 0x709890D8)) { // Poker III [!].nes
				info.mapper.id = 243;
				model = SA74374A;
			}

			if (
				(info.crc32.prg == 0xE93400B2) || // Poker III (Sachen) [!].nes
				(info.crc32.prg == 0x24A2B2BC)) { // Poker III (Sachen) [a1].nes
				info.mapper.submapper = 1;
			}

			if (info.mapper.id == 150) {
				if (info.reset == RESET) {
					sachentmp.dipswitch ^= 0x04;
				} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
					sachentmp.dipswitch = 0;
				}
			} else {
				sachentmp.dipswitch = 0;
			}

			if (info.reset >= HARD) {
				memset(&sa74374x, 0x00, sizeof(sa74374x));
				sa74374x.index = sachentmp.dipswitch;
			}

			info.mapper.extend_wr = TRUE;

			break;
	}

	sachentmp.type = model;
}

void extcl_cpu_wr_mem_Sachen_sa0036(UNUSED(WORD address), BYTE value) {
	DBWORD bank;

	value >>= 7;
	control_bank(info.chr.rom.max.banks_8k)
	bank = value << 13;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
}

void extcl_cpu_wr_mem_Sachen_sa0037(WORD address, BYTE value) {
	/* bus conflict */
	const BYTE save = value &= prg_rom_rd(address);
	DBWORD bank;

	if (info.prg.rom.max.banks_32k != 0xFFFF) {
		value >>= 3;
		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
		map_prg_rom_8k_update();
		value = save;
	}

	control_bank(info.chr.rom.max.banks_8k)
	bank = value << 13;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
}

void extcl_cpu_wr_mem_Sachen_sa8259x(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x7FFF)) {
		return;
	}

	switch (address & 0x0101) {
		case 0x0100:
			sa8259.ctrl = value;
			return;
		case 0x0101: {
			const BYTE slot = sa8259.ctrl & 0x07;

			sa8259.reg[slot] = value;
			switch (slot) {
				case 5:
					if (info.prg.rom.max.banks_32k != 0xFFFF) {
						control_bank(info.prg.rom.max.banks_32k)
						map_prg_rom_8k(4, 0, value);
						map_prg_rom_8k_update();
					}
					break;
				case 7:
					if (value & 0x01) {
						mirroring_V();
					} else {
						switch ((value >> 1) & 0x03) {
							case 0:
								mirroring_V();
								break;
							case 1:
								mirroring_H();
								break;
							case 2:
								mirroring_SCR0x1_SCR1x3();
								break;
							case 3:
								mirroring_SCR0();
								break;
						}
					}
					break;
				default:
					if (!mapper.write_vram) {
						DBWORD bank;

						if (sachentmp.type == SA8259D) {

							value = sa8259.reg[0] & 0x07;
							control_bank(info.chr.rom.max.banks_1k)
							bank = value << 10;
							chr.bank_1k[0] = chr_pnt(bank);

							value = (sa8259.reg[1] & 0x07) | ((sa8259.reg[4] << 4) & 0x10);
							control_bank(info.chr.rom.max.banks_1k)
							bank = value << 10;
							chr.bank_1k[1] = chr_pnt(bank);

							value = (sa8259.reg[2] & 0x07) | ((sa8259.reg[4] << 3) & 0x10);
							control_bank(info.chr.rom.max.banks_1k)
							bank = value << 10;
							chr.bank_1k[2] = chr_pnt(bank);

							value = (sa8259.reg[3] & 0x07) | ((sa8259.reg[4] << 2) & 0x10) | ((sa8259.reg[6] << 3) & 0x08);
							control_bank(info.chr.rom.max.banks_1k)
							bank = value << 10;
							chr.bank_1k[3] = chr_pnt(bank);
						} else {
							const BYTE high = (sa8259.reg[4] << 3) & 0x38;

							value = (high | (sa8259.reg[0] & 0x07)) << sachentmp.shift;
							control_bank(info.chr.rom.max.banks_2k)
							bank = value << 11;
							chr.bank_1k[0] = chr_pnt(bank);
							chr.bank_1k[1] = chr_pnt(bank | 0x0400);

							value = ((high | (sa8259.reg[(sa8259.reg[7] & 0x01) ? 0 : 1] & 0x07))
								<< sachentmp.shift) | sachentmp.ored[0];
							control_bank(info.chr.rom.max.banks_2k)
							bank = value << 11;
							chr.bank_1k[2] = chr_pnt(bank);
							chr.bank_1k[3] = chr_pnt(bank | 0x0400);

							value = ((high | (sa8259.reg[(sa8259.reg[7] & 0x01) ? 0 : 2] & 0x07))
								<< sachentmp.shift) | sachentmp.ored[1];
							control_bank(info.chr.rom.max.banks_2k)
							bank = value << 11;
							chr.bank_1k[4] = chr_pnt(bank);
							chr.bank_1k[5] = chr_pnt(bank | 0x0400);

							value = ((high | (sa8259.reg[(sa8259.reg[7] & 0x01) ? 0 : 3] & 0x07))
								<< sachentmp.shift) | sachentmp.ored[2];
							control_bank(info.chr.rom.max.banks_2k)
							bank = value << 11;
							chr.bank_1k[6] = chr_pnt(bank);
							chr.bank_1k[7] = chr_pnt(bank | 0x0400);
						}
					}
					break;
			}
			return;
		}
	}
}
BYTE extcl_save_mapper_Sachen_sa8259x(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, sa8259.ctrl);
	save_slot_ele(mode, slot, sa8259.reg);

	return (EXIT_OK);
}

void extcl_cpu_wr_mem_Sachen_tca01(UNUSED(WORD address), UNUSED(BYTE value)) {}
BYTE extcl_cpu_rd_mem_Sachen_tca01(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return (openbus);
	}

	if (address & 0x0100) {
		return ((~address & 0x003F) | (openbus & 0xC0));
	}

	return (openbus);
}

void extcl_cpu_wr_mem_Sachen_tcu01(WORD address, BYTE value) {
	if ((address >= 0x4000) && (address <= 0x5FFF)) {
		switch (address & 0x0103) {
			case 0x0100:
				// Write $4100: When Mode==0: Bits 0-5 of Register := Input, bits 0-3 being inverted if Invert==1.
				//              When Mode==1: Bits 0-3 of Register incremented by one, bits 4-5 unaffected.
				tcu0x.reg[0] = value;

				if (tcu0x.reg[3] & 0x01) {
					tcu0x.RRR = (tcu0x.RRR & 0x30) | ((tcu0x.RRR + 1) & 0x0F);
				} else {
					tcu0x.RRR = (tcu0x.reg[2] & 0x30) | (((tcu0x.reg[1] & 0x01) ? ~tcu0x.reg[2] : tcu0x.reg[2]) & 0x0F);
				}
				break;
			case 0x0101:
				// Write $4101: Invert := Written value bit 2.
				tcu0x.reg[1] = value >> 2;
				break;
			case 0x0102:
				// Write $4102: Input := Written value bits 2-7.
				tcu0x.reg[2] = value >> 2;
				break;
			case 0x0103:
				// Write $4103: Mode := Written value bit 2.
				tcu0x.reg[3] = value >> 2;
				break;
		}
	}

	value = ((tcu0x.RRR & 0x20) >> 4) | (tcu0x.RRR & 0x01);
	control_bank(info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();

	{
		DBWORD bank;

		value = (tcu0x.RRR >> 1) & 0x0F;
		control_bank(info.chr.rom.max.banks_8k)
		bank = value << 13;
		chr.bank_1k[0] = chr_pnt(bank);
		chr.bank_1k[1] = chr_pnt(bank | 0x0400);
		chr.bank_1k[2] = chr_pnt(bank | 0x0800);
		chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
		chr.bank_1k[4] = chr_pnt(bank | 0x1000);
		chr.bank_1k[5] = chr_pnt(bank | 0x1400);
		chr.bank_1k[6] = chr_pnt(bank | 0x1800);
		chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
	}
}
BYTE extcl_cpu_rd_mem_Sachen_tcu01(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x4000) && (address <= 0x5FFF)) {
		if ((address & 0x103) == 0x0100) {
			// Read $4100-$4103: [RRRR RR..]: Read Register (with its bits 0-5), connected to the CPU data bus bits 2-7.
			// Register bits 4-5 (CPU bits 6-7) are inverted if Invert==1. CPU bits 0-1 are open bus.
			return ((openbus & 0x03) |
				((tcu0x.reg[1] & 0x01 ? ((tcu0x.RRR & 0x10) << 1) | ((tcu0x.RRR & 0x20) >> 1) : (tcu0x.RRR & 0x30)) |
				(tcu0x.RRR & 0x0F) << 2));
		}
	}
	return (openbus);
}
BYTE extcl_save_mapper_Sachen_tcu01(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, tcu0x.reg);
	save_slot_ele(mode, slot, tcu0x.RRR);

	return (EXIT_OK);
}

void extcl_cpu_wr_mem_Sachen_tcu02(WORD address, BYTE value) {
	if ((address >= 0x4000) && (address <= 0x5FFF)) {
		switch (address & 0x0103) {
			case 0x0100:
				// Write $4100: When Mode==0: Bits 0-5 of Register := Input, bits 0-3 being inverted if Invert==1.
				//             When Mode==1: Bits 0-3 of Register incremented by one, bits 4-5 unaffected.
				tcu0x.reg[0] = value;

				if (tcu0x.reg[3] & 0x01) {
					tcu0x.RRR = (tcu0x.RRR & 0x30) | ((tcu0x.RRR + 1) & 0x0F);
				} else {
					tcu0x.RRR = (tcu0x.reg[2] & 0x30) | (((tcu0x.reg[1] & 0x01) ? ~tcu0x.reg[2] : tcu0x.reg[2]) & 0x0F);
				}
				break;
			case 0x0101:
				// Write $4101: Invert := Written value bit 0.
				tcu0x.reg[1] = value;
				break;
			case 0x0102:
				// Write $4102: Input := Written value bits 0-5.
				tcu0x.reg[2] = value;
				break;
			case 0x0103:
				// Write $4103: Mode := Written value bit 0.
				tcu0x.reg[3] = value;
				break;
		}
	}

	value = (tcu0x.RRR >> 4) & 0x01;
	control_bank(info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();

	{
		DBWORD bank;

		value = tcu0x.RRR & 0x07;
		control_bank(info.chr.rom.max.banks_8k)
		bank = value << 13;
		chr.bank_1k[0] = chr_pnt(bank);
		chr.bank_1k[1] = chr_pnt(bank | 0x0400);
		chr.bank_1k[2] = chr_pnt(bank | 0x0800);
		chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
		chr.bank_1k[4] = chr_pnt(bank | 0x1000);
		chr.bank_1k[5] = chr_pnt(bank | 0x1400);
		chr.bank_1k[6] = chr_pnt(bank | 0x1800);
		chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
	}
}
BYTE extcl_cpu_rd_mem_Sachen_tcu02(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x4000) && (address <= 0x5FFF)) {
		if ((address & 0x103) == 0x0100) {
			// Read $4100-$4103: [..RR RRRR]: Read Register. Bits 4-5 are inverted if Invert==1. Bits 6-7 are open bus.
			return ((openbus & 0xC0) |
				(tcu0x.reg[1] & 0x01 ? ((tcu0x.RRR & 0x10) << 1) | ((tcu0x.RRR & 0x20) >> 1) : (tcu0x.RRR & 0x30)) |
				(tcu0x.RRR & 0x0F));
		}
	}
	return (openbus);
}
BYTE extcl_save_mapper_Sachen_tcu02(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, tcu0x.reg);
	save_slot_ele(mode, slot, tcu0x.RRR);

	return (EXIT_OK);
}

void extcl_cpu_wr_mem_Sachen_sa72007(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return;
	}

	if (address & 0x0100) {
		DBWORD bank;

		value >>= 7;
		control_bank(info.chr.rom.max.banks_8k)
		bank = value << 13;
		chr.bank_1k[0] = chr_pnt(bank);
		chr.bank_1k[1] = chr_pnt(bank | 0x0400);
		chr.bank_1k[2] = chr_pnt(bank | 0x0800);
		chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
		chr.bank_1k[4] = chr_pnt(bank | 0x1000);
		chr.bank_1k[5] = chr_pnt(bank | 0x1400);
		chr.bank_1k[6] = chr_pnt(bank | 0x1800);
		chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
	}
}

void extcl_cpu_wr_mem_Sachen_sa72008(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return;
	}

	{
		const BYTE save = value;
		DBWORD bank;

		if (info.prg.rom.max.banks_32k != 0xFFFF) {
			value >>= 2;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			map_prg_rom_8k_update();
			value = save;
		}

		control_bank(info.chr.rom.max.banks_8k)
		bank = value << 13;
		chr.bank_1k[0] = chr_pnt(bank);
		chr.bank_1k[1] = chr_pnt(bank | 0x0400);
		chr.bank_1k[2] = chr_pnt(bank | 0x0800);
		chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
		chr.bank_1k[4] = chr_pnt(bank | 0x1000);
		chr.bank_1k[5] = chr_pnt(bank | 0x1400);
		chr.bank_1k[6] = chr_pnt(bank | 0x1800);
		chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
	}
}

void extcl_after_mapper_init_Sachen_sa74374x(void) {
	prg_fix_Sachen_sa74374x();
	chr_fix_Sachen_sa74374x();
	mirroring_fix_Sachen_sa74374x();
}
void extcl_cpu_wr_mem_Sachen_sa74374x(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return;
	}

	value = (value & 0x07) | sachentmp.dipswitch;

	switch (address & 0x0101) {
		case 0x0100:
			sa74374x.index = value;
			break;
		case 0x0101:
			sa74374x.reg[sa74374x.index & 0x07] = value;
			prg_fix_Sachen_sa74374x();
			chr_fix_Sachen_sa74374x();
			mirroring_fix_Sachen_sa74374x();
			break;
	}
}
BYTE extcl_cpu_rd_mem_Sachen_sa74374x(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return (openbus);
	}
	if ((address & 0x0101) == 0x0101) {
		return ((sa74374x.reg[sa74374x.index] & (0x07 & ~sachentmp.dipswitch)) | (openbus & ~(0x07 & ~sachentmp.dipswitch)));
	}
	return (openbus);
}
BYTE extcl_save_mapper_Sachen_sa74374x(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, sa74374x.index);
	save_slot_ele(mode, slot, sa74374x.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_Sachen_sa74374x(void) {
	WORD bank;

	bank = sa74374x.reg[5] | (sa74374x.reg[2] & 0x01);
	_control_bank(bank, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, bank);
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_Sachen_sa74374x(void) {
	DBWORD bank;

	if (sachentmp.type == SA74374A) {
		if (info.mapper.submapper == 1) {
			bank = ((sa74374x.reg[2] & 0x01) << 3) | ((sa74374x.reg[6] & 0x03) << 1) | (sa74374x.reg[4] & 0x01);
		} else {
			bank = ((sa74374x.reg[6] & 0x03) << 2) | ((sa74374x.reg[4] & 0x01) << 1) | (sa74374x.reg[2] & 0x01);
		}
	} else {
		bank = (sa74374x.reg[3] << 3) | ((sa74374x.reg[4] & 0x01) << 2) | (sa74374x.reg[6] & 0x03);
	}
	_control_bank(bank, info.chr.rom.max.banks_8k)
	bank <<= 13;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
}
INLINE static void mirroring_fix_Sachen_sa74374x(void) {
	switch ((sa74374x.reg[7] & 0x06) >> 1) {
		default:
		case 0:
			mirroring_SCR0x3_SCR1x1();
			break;
		case 1:
			mirroring_H();
			break;
		case 2:
			mirroring_V();
			break;
		case 3:
			mirroring_SCR1();
			break;
	}
}

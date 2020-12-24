/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#define sa74347x_chr_8k_swap(bank8k)\
{\
	DBWORD bank;\
	value = bank8k;\
	control_bank(info.chr.rom[0].max.banks_8k)\
	bank = value << 13;\
	chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);\
	chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);\
	chr.bank_1k[2] = chr_chip_byte_pnt(0, bank | 0x0800);\
	chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0C00);\
	chr.bank_1k[4] = chr_chip_byte_pnt(0, bank | 0x1000);\
	chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x1400);\
	chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x1800);\
	chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x1C00);\
	sa74374x.chr_rom_8k_bank = value;\
}

struct _sa8259 {
	BYTE ctrl;
	BYTE reg[8];
} sa8259;
struct _tcu02 {
	BYTE reg;
} tcu02;
struct _sa74374x {
	BYTE reg;
	BYTE chr_rom_8k_bank;
} sa74374x;
struct _sachentmp {
	BYTE type;
	BYTE shift;
	BYTE ored[3];
} sachentmp;

void map_init_Sachen(BYTE model) {
	switch (model) {
		case SA0036:
			EXTCL_CPU_WR_MEM(Sachen_sa0036);
			break;
		case SA0037:
			EXTCL_CPU_WR_MEM(Sachen_sa0037);

			if (info.reset >= HARD) {
				if (info.prg.rom[0].max.banks_32k != 0xFFFF) {
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
			mapper.internal_struct[0] = (BYTE *) &sa8259;
			mapper.internal_struct_size[0] = sizeof(sa8259);

			info.mapper.extend_wr = TRUE;

			if (info.reset >= HARD) {
				memset(&sa8259, 0x00, sizeof(sa8259));

				if (info.prg.rom[0].max.banks_32k != 0xFFFF) {
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
						const DBWORD bank = info.chr.rom[0].max.banks_4k << 12;

						chr.bank_1k[4] = chr_chip_byte_pnt(0, bank);
						chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x0400);
						chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x0800);
						chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x0C00);
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

			info.mapper.extend_wr = TRUE;

			if (info.reset >= HARD) {
				if (info.prg.rom[0].max.banks_32k != 0xFFFF) {
					map_prg_rom_8k(4, 0, 0);
				}
			}
			break;
		case TCU02:
			EXTCL_CPU_WR_MEM(Sachen_tcu02);
			EXTCL_CPU_RD_MEM(Sachen_tcu02);
			EXTCL_SAVE_MAPPER(Sachen_tcu02);
			mapper.internal_struct[0] = (BYTE *) &tcu02;
			mapper.internal_struct_size[0] = sizeof(tcu02);

			info.mapper.extend_wr = TRUE;

			if (info.reset >= HARD) {
				memset(&tcu02, 0x00, sizeof(tcu02));
			}
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
		case SA74374B: {
			BYTE i;

			for (i = 0; i < LENGTH(pokeriiichr); i++) {
				if (!(memcmp(pokeriiichr[i], info.sha1sum.chr.string, 40))) {
					if (i == 0) {
						/* Poker III 5-in-1 (Sachen) [!].nes */
						info.mapper.id = 150;
						model = SA74374B;
					} else {
						/* Poker III [!].nes */
						info.mapper.id = 243;
						model = SA74374A;
					}
				}
			}

			if (model == SA74374A) {
				EXTCL_CPU_WR_MEM(Sachen_sa74374a);
			} else {
				EXTCL_CPU_WR_MEM(Sachen_sa74374b);
			}
			EXTCL_SAVE_MAPPER(Sachen_sa74374x);
			mapper.internal_struct[0] = (BYTE *) &sa74374x;
			mapper.internal_struct_size[0] = sizeof(sa74374x);

			info.mapper.extend_wr = TRUE;

			if (info.reset >= HARD) {
				memset(&sa74374x, 0x00, sizeof(sa74374x));
				map_prg_rom_8k(4, 0, 0);
			}
			break;
		}
	}

	sachentmp.type = model;
}

void extcl_cpu_wr_mem_Sachen_sa0036(UNUSED(WORD address), BYTE value) {
	DBWORD bank;

	value >>= 7;
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
}

void extcl_cpu_wr_mem_Sachen_sa0037(WORD address, BYTE value) {
	/* bus conflict */
	const BYTE save = value &= prg_rom_rd(address);
	DBWORD bank;

	if (info.prg.rom[0].max.banks_32k != 0xFFFF) {
		value >>= 3;
		control_bank(info.prg.rom[0].max.banks_32k)
		map_prg_rom_8k(4, 0, value);
		map_prg_rom_8k_update();
		value = save;
	}

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
					if (info.prg.rom[0].max.banks_32k != 0xFFFF) {
						control_bank(info.prg.rom[0].max.banks_32k)
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
							control_bank(info.chr.rom[0].max.banks_1k)
							bank = value << 10;
							chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);

							value = (sa8259.reg[1] & 0x07) | ((sa8259.reg[4] << 4) & 0x10);
							control_bank(info.chr.rom[0].max.banks_1k)
							bank = value << 10;
							chr.bank_1k[1] = chr_chip_byte_pnt(0, bank);

							value = (sa8259.reg[2] & 0x07) | ((sa8259.reg[4] << 3) & 0x10);
							control_bank(info.chr.rom[0].max.banks_1k)
							bank = value << 10;
							chr.bank_1k[2] = chr_chip_byte_pnt(0, bank);

							value = (sa8259.reg[3] & 0x07) | ((sa8259.reg[4] << 2) & 0x10) | ((sa8259.reg[6] << 3) & 0x08);
							control_bank(info.chr.rom[0].max.banks_1k)
							bank = value << 10;
							chr.bank_1k[3] = chr_chip_byte_pnt(0, bank);
						} else {
							const BYTE high = (sa8259.reg[4] << 3) & 0x38;

							value = (high | (sa8259.reg[0] & 0x07)) << sachentmp.shift;
							control_bank(info.chr.rom[0].max.banks_2k)
							bank = value << 11;
							chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
							chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);

							value = ((high | (sa8259.reg[(sa8259.reg[7] & 0x01) ? 0 : 1] & 0x07))
								<< sachentmp.shift) | sachentmp.ored[0];
							control_bank(info.chr.rom[0].max.banks_2k)
							bank = value << 11;
							chr.bank_1k[2] = chr_chip_byte_pnt(0, bank);
							chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0400);

							value = ((high | (sa8259.reg[(sa8259.reg[7] & 0x01) ? 0 : 2] & 0x07))
								<< sachentmp.shift) | sachentmp.ored[1];
							control_bank(info.chr.rom[0].max.banks_2k)
							bank = value << 11;
							chr.bank_1k[4] = chr_chip_byte_pnt(0, bank);
							chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x0400);

							value = ((high | (sa8259.reg[(sa8259.reg[7] & 0x01) ? 0 : 3] & 0x07))
								<< sachentmp.shift) | sachentmp.ored[2];
							control_bank(info.chr.rom[0].max.banks_2k)
							bank = value << 11;
							chr.bank_1k[6] = chr_chip_byte_pnt(0, bank);
							chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x0400);
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
	if (address < 0x4100) {
		return;
	}

	if ((address & 0x0003) == 0x0002) {
		const BYTE save = value;
		DBWORD bank;

		if (info.prg.rom[0].max.banks_32k != 0xFFFF) {
			value = ((value >> 6) & 0x02) | ((value >> 2) & 0x01);
			control_bank(info.prg.rom[0].max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			map_prg_rom_8k_update();
			value = save;
		}

		value = (value >> 3);
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
	}
}

void extcl_cpu_wr_mem_Sachen_tcu02(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x7FFF)) {
		return;
	}

	if ((address & 0x0003) == 0x0002) {
		DBWORD bank;

		tcu02.reg = (value & 0x30) | ((value + 3) & 0x0F);
		value = tcu02.reg;
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
	}
}
BYTE extcl_cpu_rd_mem_Sachen_tcu02(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return (openbus);
	}

	if ((address & 0x0003) == 0x0000) {
		return (tcu02.reg | 0x40);
	}

	return (openbus);
}
BYTE extcl_save_mapper_Sachen_tcu02(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, tcu02.reg);

	return (EXIT_OK);
}

void extcl_cpu_wr_mem_Sachen_sa72007(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return;
	}

	if (address & 0x0100) {
		DBWORD bank;

		value >>= 7;
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
	}
}

void extcl_cpu_wr_mem_Sachen_sa72008(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return;
	}

	{
		const BYTE save = value;
		DBWORD bank;

		if (info.prg.rom[0].max.banks_32k != 0xFFFF) {
			value >>= 2;
			control_bank(info.prg.rom[0].max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			map_prg_rom_8k_update();
			value = save;
		}

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
	}
}

void extcl_cpu_wr_mem_Sachen_sa74374a(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return;
	}

	switch (address & 0x4101) {
		case 0x4100:
			sa74374x.reg = value & 0x07;
			return;
		case 0x4101: {
			switch (sa74374x.reg) {
				case 0:
					map_prg_rom_8k(4, 0, 0);
					map_prg_rom_8k_update();

					sa74347x_chr_8k_swap(3)
					break;
				case 2:
					sa74347x_chr_8k_swap((sa74374x.chr_rom_8k_bank & 0x07) | ((value << 3) & 0x08))
					break;
				case 4:
					sa74347x_chr_8k_swap((sa74374x.chr_rom_8k_bank & 0x0E) | (value & 0x01))
					break;
				case 5:
					value &= 0x01;
					control_bank(info.prg.rom[0].max.banks_32k)
					map_prg_rom_8k(4, 0, value);
					map_prg_rom_8k_update();
					break;
				case 6:
					sa74347x_chr_8k_swap((sa74374x.chr_rom_8k_bank & 0x09) | ((value << 1) & 0x06))
					break;
				case 7: {
					switch (value & 0x03) {
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
					break;
				}
			}
			return;
		}
	}
}
void extcl_cpu_wr_mem_Sachen_sa74374b(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return;
	}

	switch (address & 0x4101) {
		case 0x4100:
			sa74374x.reg = value & 0x07;
			return;
		case 0x4101: {
			switch (sa74374x.reg) {
				case 2: {
					const BYTE save = value;
					sa74347x_chr_8k_swap((sa74374x.chr_rom_8k_bank & 0x07) | ((value << 3) & 0x08))

					value = save & 0x01;
					control_bank(info.prg.rom[0].max.banks_32k)
					map_prg_rom_8k(4, 0, value);
					map_prg_rom_8k_update();
					break;
				}
				case 4:
					sa74347x_chr_8k_swap((sa74374x.chr_rom_8k_bank & 0x0B) | ((value << 2) & 0x04))
					break;
				case 5:
					value &= 0x07;
					control_bank(info.prg.rom[0].max.banks_32k)
					map_prg_rom_8k(4, 0, value);
					map_prg_rom_8k_update();
					break;
				case 6:
					sa74347x_chr_8k_swap((sa74374x.chr_rom_8k_bank & 0x0C) | (value & 0x03))
					break;
				case 7: {
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
					break;
				}
			}
			return;
		}
	}
}
BYTE extcl_save_mapper_Sachen_sa74374x(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, sa74374x.reg);
	save_slot_ele(mode, slot, sa74374x.chr_rom_8k_bank);

	return (EXIT_OK);
}

/*
 * mapper_Sachen.c
 *
 *  Created on: 27/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "save_slot.h"

#define sa74347x_chr_8k_swap(bank8k)\
{\
	DBWORD bank;\
	value = bank8k;\
	control_bank(chr_rom_8k_max)\
	bank = value << 13;\
	chr.bank_1k[0] = &chr.data[bank];\
	chr.bank_1k[1] = &chr.data[bank | 0x0400];\
	chr.bank_1k[2] = &chr.data[bank | 0x0800];\
	chr.bank_1k[3] = &chr.data[bank | 0x0C00];\
	chr.bank_1k[4] = &chr.data[bank | 0x1000];\
	chr.bank_1k[5] = &chr.data[bank | 0x1400];\
	chr.bank_1k[6] = &chr.data[bank | 0x1800];\
	chr.bank_1k[7] = &chr.data[bank | 0x1C00];\
	sa74374x.chr_rom_8k_bank = value;\
}

WORD chr_rom_8k_max, chr_rom_4k_max, chr_rom_2k_max;
BYTE type, shift, ored[3];

void map_init_Sachen(BYTE model) {
	chr_rom_8k_max = info.chr.rom.banks_8k - 1;
	chr_rom_4k_max = info.chr.rom.banks_4k - 1;
	chr_rom_2k_max = (info.chr.rom.banks_1k >> 1) - 1;

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
			mapper.internal_struct[0] = (BYTE *) &sa8259;
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
					shift = 1;
					ored[0] = 1;
					ored[1] = 0;
					ored[2] = 1;
					break;
				case SA8259B:
					shift = 0;
					ored[0] = 0;
					ored[1] = 0;
					ored[2] = 0;
					break;
				case SA8259C:
					shift = 2;
					ored[0] = 1;
					ored[1] = 2;
					ored[2] = 3;
					break;
				case SA8259D:
					if (!mapper.write_vram) {
						const DBWORD bank = chr_rom_4k_max << 12;

						chr.bank_1k[4] = &chr.data[bank];
						chr.bank_1k[5] = &chr.data[bank | 0x0400];
						chr.bank_1k[6] = &chr.data[bank | 0x0800];
						chr.bank_1k[7] = &chr.data[bank | 0x0C00];
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
				if (info.prg.rom.max.banks_32k != 0xFFFF) {
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

	type = model;
}

void extcl_cpu_wr_mem_Sachen_sa0036(WORD address, BYTE value) {
	DBWORD bank;

	value >>= 7;
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

						if (type == SA8259D) {

							value = sa8259.reg[0] & 0x07;
							control_bank(info.chr.rom.max.banks_1k)
							bank = value << 10;
							chr.bank_1k[0] = &chr.data[bank];

							value = (sa8259.reg[1] & 0x07) | ((sa8259.reg[4] << 4) & 0x10);
							control_bank(info.chr.rom.max.banks_1k)
							bank = value << 10;
							chr.bank_1k[1] = &chr.data[bank];

							value = (sa8259.reg[2] & 0x07) | ((sa8259.reg[4] << 3) & 0x10);
							control_bank(info.chr.rom.max.banks_1k)
							bank = value << 10;
							chr.bank_1k[2] = &chr.data[bank];

							value = (sa8259.reg[3] & 0x07) | ((sa8259.reg[4] << 2) & 0x10)
							        		| ((sa8259.reg[6] << 3) & 0x08);
							control_bank(info.chr.rom.max.banks_1k)
							bank = value << 10;
							chr.bank_1k[3] = &chr.data[bank];
						} else {
							const BYTE high = (sa8259.reg[4] << 3) & 0x38;

							value = (high | (sa8259.reg[0] & 0x07)) << shift;
							control_bank(chr_rom_2k_max)
							bank = value << 11;
							chr.bank_1k[0] = &chr.data[bank];
							chr.bank_1k[1] = &chr.data[bank | 0x0400];

							value = ((high | (sa8259.reg[(sa8259.reg[7] & 0x01) ? 0 : 1] & 0x07))
									<< shift) | ored[0];
							control_bank(chr_rom_2k_max)
							bank = value << 11;
							chr.bank_1k[2] = &chr.data[bank];
							chr.bank_1k[3] = &chr.data[bank | 0x0400];

							value = ((high | (sa8259.reg[(sa8259.reg[7] & 0x01) ? 0 : 2] & 0x07))
									<< shift) | ored[1];
							control_bank(chr_rom_2k_max)
							bank = value << 11;
							chr.bank_1k[4] = &chr.data[bank];
							chr.bank_1k[5] = &chr.data[bank | 0x0400];

							value = ((high | (sa8259.reg[(sa8259.reg[7] & 0x01) ? 0 : 3] & 0x07))
									<< shift) | ored[2];
							control_bank(chr_rom_2k_max)
							bank = value << 11;
							chr.bank_1k[6] = &chr.data[bank];
							chr.bank_1k[7] = &chr.data[bank | 0x0400];
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

void extcl_cpu_wr_mem_Sachen_tca01(WORD address, BYTE value) {
	return;
}
BYTE extcl_cpu_rd_mem_Sachen_tca01(WORD address, BYTE openbus, BYTE before) {
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

		if (info.prg.rom.max.banks_32k != 0xFFFF) {
			value = ((value >> 6) & 0x02) | ((value >> 2) & 0x01);
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			map_prg_rom_8k_update();
			value = save;
		}

		value = (value >> 3);
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
	}
}
BYTE extcl_cpu_rd_mem_Sachen_tcu02(WORD address, BYTE openbus, BYTE before) {
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
					control_bank(info.prg.rom.max.banks_32k)
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
					control_bank(info.prg.rom.max.banks_32k)
					map_prg_rom_8k(4, 0, value);
					map_prg_rom_8k_update();
					break;
				}
				case 4:
					sa74347x_chr_8k_swap((sa74374x.chr_rom_8k_bank & 0x0B) | ((value << 2) & 0x04))
					break;
				case 5:
					value &= 0x07;
					control_bank(info.prg.rom.max.banks_32k)
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

/*
 * mapper_Taito.c
 *
 *  Created on: 17/lug/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mem_map.h"
#include "mappers.h"
#include "irqA12.h"
#include "save_slot.h"

BYTE type;

void map_init_Taito(BYTE model) {
	switch (model) {
		case TC0190FMC: {
			switch (info.mapper.submapper) {
				case TC0190FMCPAL16R4:
					EXTCL_CPU_WR_MEM(Taito_TC0190FMCPAL16R4);
					EXTCL_CPU_EVERY_CYCLE(MMC3);
					EXTCL_PPU_000_TO_34X(Taito_TC0190FMCPAL16R4);
					EXTCL_PPU_000_TO_255(Taito_TC0190FMCPAL16R4);
					EXTCL_PPU_256_TO_319(Taito_TC0190FMCPAL16R4);
					EXTCL_PPU_320_TO_34X(Taito_TC0190FMCPAL16R4);
					EXTCL_UPDATE_R2006(MMC3);

					if (info.reset >= HARD) {
						memset(&irqA12, 0x00, sizeof(irqA12));
					}
					irqA12.present = TRUE;
					irqA12_delay = 7;

					mirroring_V();
					break;
				default:
					EXTCL_CPU_WR_MEM(Taito_TC0190FMC);
					break;
			}
			break;
		}
		case X1005A:
		case X1005B:
			EXTCL_CPU_WR_MEM(Taito_X1005);
			EXTCL_CPU_RD_MEM(Taito_X1005);
			EXTCL_SAVE_MAPPER(Taito_X1005);
			EXTCL_BATTERY_IO(Taito_X1005);
			mapper.internal_struct[0] = (BYTE *) &taito_X1005;
			mapper.internal_struct_size[0] = sizeof(taito_X1005);

			info.mapper.extend_wr = TRUE;

			if (info.reset > HARD) {
				memset(&taito_X1005, 0x00, sizeof(taito_X1005));
			} else if (info.reset == HARD) {
				taito_X1005.enable = 0;
			}

			if (model == X1005A) {
				mirroring_H();
			} else {
				mirroring_SCR0();
			}

			if (info.id == X1005_NO_BAT) {
				info.prg.ram.bat.banks = FALSE;
			} else {
				info.prg.ram.bat.banks = TRUE;
			}

			break;
		case X1017:
			EXTCL_CPU_WR_MEM(Taito_X1017);
			EXTCL_SAVE_MAPPER(Taito_X1017);
			mapper.internal_struct[0] = (BYTE *) &taito_X1017;
			mapper.internal_struct_size[0] = sizeof(taito_X1017);

			info.mapper.extend_wr = TRUE;

			info.prg.ram.banks_8k_plus = 1;
			info.prg.ram.bat.banks = 1;

			break;
	}

	type = model;
}

void extcl_cpu_wr_mem_Taito_TC0190FMC(WORD address, BYTE value) {
	DBWORD bank;

	switch (address & 0xF003) {
		case 0x8000:
			if (value & 0x40) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			control_bank_with_AND(0x3F, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0x8001:
			control_bank_with_AND(0x3F, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, value);
			map_prg_rom_8k_update();
			return;
		case 0x8002:
			control_bank(info.chr.rom.max.banks_2k)
			bank = value << 11;
			chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
			return;
		case 0x8003:
			control_bank(info.chr.rom.max.banks_2k)
			bank = value << 11;
			chr.bank_1k[2] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0400);
			return;
		case 0xA000:
			control_bank(info.chr.rom.max.banks_1k)
			bank = value << 10;
			chr.bank_1k[4] = chr_chip_byte_pnt(0, bank);
			return;
		case 0xA001:
			control_bank(info.chr.rom.max.banks_1k)
			bank = value << 10;
			chr.bank_1k[5] = chr_chip_byte_pnt(0, bank);
			return;
		case 0xA002:
			control_bank(info.chr.rom.max.banks_1k)
			bank = value << 10;
			chr.bank_1k[6] = chr_chip_byte_pnt(0, bank);
			return;
		case 0xA003:
			control_bank(info.chr.rom.max.banks_1k)
			bank = value << 10;
			chr.bank_1k[7] = chr_chip_byte_pnt(0, bank);
			return;
	}
}

void extcl_cpu_wr_mem_Taito_TC0190FMCPAL16R4(WORD address, BYTE value) {
	DBWORD bank;

	switch (address & 0xF003) {
		case 0x8000:
			control_bank_with_AND(0x3F, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0x8001:
			control_bank_with_AND(0x3F, info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, value);
			map_prg_rom_8k_update();
			return;
		case 0x8002:
			control_bank(info.chr.rom.max.banks_2k)
			bank = value << 11;
			chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
			return;
		case 0x8003:
			control_bank(info.chr.rom.max.banks_2k)
			bank = value << 11;
			chr.bank_1k[2] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0400);
			return;
		case 0xA000:
			control_bank(info.chr.rom.max.banks_1k)
			bank = value << 10;
			chr.bank_1k[4] = chr_chip_byte_pnt(0, bank);
			return;
		case 0xA001:
			control_bank(info.chr.rom.max.banks_1k)
			bank = value << 10;
			chr.bank_1k[5] = chr_chip_byte_pnt(0, bank);
			return;
		case 0xA002:
			control_bank(info.chr.rom.max.banks_1k)
			bank = value << 10;
			chr.bank_1k[6] = chr_chip_byte_pnt(0, bank);
			return;
		case 0xA003:
			control_bank(info.chr.rom.max.banks_1k)
			bank = value << 10;
			chr.bank_1k[7] = chr_chip_byte_pnt(0, bank);
			return;
		case 0xC000:
			irqA12.latch = (0x100 - value) & 0xFF;
			return;
		case 0xC001:
			irqA12.reload = TRUE;
			irqA12.counter = 0;
			return;
		case 0xC002:
			irqA12.enable = TRUE;
			return;
		case 0xC003:
			irqA12.enable = FALSE;
			irq.high &= ~EXT_IRQ;
			return;
		case 0xE000:
			if (value & 0x40) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			return;
	}
}
void extcl_ppu_000_to_34x_Taito_TC0190FMCPAL16R4(void) {
	irqA12_RS();
}
void extcl_ppu_000_to_255_Taito_TC0190FMCPAL16R4(void) {
	if (r2001.visible) {
		irqA12_SB();
	}
}
void extcl_ppu_256_to_319_Taito_TC0190FMCPAL16R4(void) {
	irqA12_BS();
}
void extcl_ppu_320_to_34x_Taito_TC0190FMCPAL16R4(void) {
	irqA12_SB();
}

void extcl_cpu_wr_mem_Taito_X1005(WORD address, BYTE value) {
	if ((address < 0x7EF0) || (address > 0x7FFF)) {
		return;
	}

	switch (address & 0xFFFE) {
		case 0x7EF0: {
			const BYTE slot = (address & 0x0001) << 1;
			DBWORD bank;

			if (type == X1005B) {
				if (value & 0x80) {
					mirroring_SCR1();
				} else {
					mirroring_SCR0();
				}
			}
			value >>= 1;
			control_bank(info.chr.rom.max.banks_2k)
			bank = value << 11;
			chr.bank_1k[slot] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[slot | 0x01] = chr_chip_byte_pnt(0, bank | 0x0400);
			return;
		}
		case 0x7EF2:
		case 0x7EF4:
			if (type == X1005B) {
				if (value & 0x80) {
					mirroring_SCR1();
				} else {
					mirroring_SCR0();
				}
			}
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[(address & 0x0007) + 2] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0x7EF6:
			if (type == X1005A) {
				if (value & 0x01) {
					mirroring_V();
				} else {
					mirroring_H();
				}
			}
			return;
		case 0x7EF8:
			taito_X1005.enable = value;
			return;
		case 0x7EFA:
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0x7EFC:
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, value);
			map_prg_rom_8k_update();
			return;
		case 0x7EFE:
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2, value);
			map_prg_rom_8k_update();
			return;
		default:
			if (taito_X1005.enable == 0xA3) {
				taito_X1005.ram[address & 0x007F] = value;
			}
			return;
	}
}
BYTE extcl_cpu_rd_mem_Taito_X1005(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x7EF8) || (address > 0x7FFF)) {
		return (openbus);
	}

	if ((address & 0xFFFE) == 0x7FE8) {
		return (taito_X1005.enable);
	}

	if ((address & 0xFF00) == 0x7F00) {
		if (taito_X1005.enable == 0xA3) {
			return (taito_X1005.ram[address & 0x007F]);
		}
		return (0x7F);
	}

	return (openbus);
}
BYTE extcl_save_mapper_Taito_X1005(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, taito_X1005.ram);
	save_slot_ele(mode, slot, taito_X1005.enable);

	return (EXIT_OK);
}
void extcl_battery_io_Taito_X1005(BYTE mode, FILE *fp) {
	if (!fp) {
		return;
	}

	if (info.prg.ram.bat.banks) {
		if (mode == WR_BAT) {
			if (fwrite(&taito_X1005.ram[0], LENGTH(taito_X1005.ram), 1, fp) < 1) {
				fprintf(stderr, "error on write battery memory\n");
			}
		} else {
			if (fread(&taito_X1005.ram[0], LENGTH(taito_X1005.ram), 1, fp) < 1) {
				fprintf(stderr, "error on read battery memory\n");
			}
		}
	}
}

void extcl_cpu_wr_mem_Taito_X1017(WORD address, BYTE value) {
	if ((address < 0x7EF0) || (address > 0x7EFF)) {
		return;
	}

	switch (address & 0x7FFF) {
		case 0x7EF0:
		case 0x7EF1: {
			BYTE slot = address & 0x0001;

			value >>= 1;
			control_bank(info.chr.rom.max.banks_2k)
			if (taito_X1017.chr[slot] != value) {
				const BYTE tmp = (taito_X1017.control & 0x02) << 1;
				const BYTE chr_1k = slot << 1;
				const DBWORD bank = value << 11;

				chr.bank_1k[chr_1k | tmp] = chr_chip_byte_pnt(0, bank);
				chr.bank_1k[(chr_1k + 1) | tmp] = chr_chip_byte_pnt(0, bank | 0x0400);
				taito_X1017.chr[slot] = value;
			}
			return;
		}
		case 0x7EF2:
		case 0x7EF3:
		case 0x7EF4:
		case 0x7EF5: {
			BYTE slot = address & 0x0007;

			control_bank(info.chr.rom.max.banks_1k)
			if (taito_X1017.chr[slot] != value) {
				const DBWORD bank = value << 10;

				if (taito_X1017.control & 0x02) {
					chr.bank_1k[slot - 2] = chr_chip_byte_pnt(0, bank);
				} else {
					chr.bank_1k[slot + 2] = chr_chip_byte_pnt(0, bank);
				}
				taito_X1017.chr[slot] = value;
			}
			return;
		}
		case 0x7EF6:
			if (taito_X1017.control != value) {
				if (value & 0x01) {
					mirroring_V();
				} else {
					mirroring_H();
				}
				if (value & 0x02) {
					DBWORD bank;

					chr.bank_1k[0] = chr_chip_byte_pnt(0, taito_X1017.chr[2] << 10);
					chr.bank_1k[1] = chr_chip_byte_pnt(0, taito_X1017.chr[3] << 10);
					chr.bank_1k[2] = chr_chip_byte_pnt(0, taito_X1017.chr[4] << 10);
					chr.bank_1k[3] = chr_chip_byte_pnt(0, taito_X1017.chr[5] << 10);
					bank = taito_X1017.chr[0] << 11;
					chr.bank_1k[4] = chr_chip_byte_pnt(0, bank);
					chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x0400);
					bank = taito_X1017.chr[1] << 11;
					chr.bank_1k[6] = chr_chip_byte_pnt(0, bank);
					chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x0400);
				} else {
					DBWORD bank;

					bank = taito_X1017.chr[0] << 11;
					chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
					chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
					bank = taito_X1017.chr[1] << 11;
					chr.bank_1k[2] = chr_chip_byte_pnt(0, bank);
					chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0400);
					chr.bank_1k[4] = chr_chip_byte_pnt(0, taito_X1017.chr[2] << 10);
					chr.bank_1k[5] = chr_chip_byte_pnt(0, taito_X1017.chr[3] << 10);
					chr.bank_1k[6] = chr_chip_byte_pnt(0, taito_X1017.chr[4] << 10);
					chr.bank_1k[7] = chr_chip_byte_pnt(0, taito_X1017.chr[5] << 10);
				}
				taito_X1017.control = value;
			}
			return;
		case 0x7EFA:
			value >>= 2;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0x7EFB:
			value >>= 2;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, value);
			map_prg_rom_8k_update();
			return;
		case 0x7EFC:
			value >>= 2;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2, value);
			map_prg_rom_8k_update();
			return;
	}
}
BYTE extcl_save_mapper_Taito_X1017(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, taito_X1017.chr);
	save_slot_ele(mode, slot, taito_X1017.control);

	return (EXIT_OK);
}

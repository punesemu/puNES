/*
 * mapper_Jaleco.c
 *
 *  Created on: 16/lug/2011
 *      Author: fhorse
 */

#include <string.h>
#include "mem_map.h"
#include "info.h"
#include "mappers.h"
#include "cpu.h"
#include "ppu.h"
#include "save_slot.h"

#define prg_rom_8k_update(slot, mask, shift)\
	value = (mapper.rom_map_to[slot] & mask) | ((value & 0x0F) << shift);\
	control_bank(info.prg.rom.max.banks_8k)\
	map_prg_rom_8k(1, slot, value);\
	map_prg_rom_8k_update()
#define chr_rom_1k_update(slot, mask, shift)\
	value = (ss8806.chr_rom_bank[slot] & mask) | ((value & 0x0F) << shift);\
	control_bank(info.chr.rom.max.banks_1k)\
	chr.bank_1k[slot] = chr_chip_byte_pnt(0, value << 10);\
	ss8806.chr_rom_bank[slot] = value

void map_init_Jaleco(BYTE model) {
	switch (model) {
		case JF05:
			EXTCL_CPU_WR_MEM(Jaleco_JF05);
			info.mapper.extend_wr = TRUE;
			break;
		case JF11:
			EXTCL_CPU_WR_MEM(Jaleco_JF11);
			info.mapper.extend_wr = TRUE;
			if (info.reset >= HARD) {
				map_prg_rom_8k(4, 0, 0);
			}
			break;
		case JF13:
			EXTCL_CPU_WR_MEM(Jaleco_JF13);
			info.mapper.extend_wr = TRUE;
			if (info.reset >= HARD) {
				map_prg_rom_8k(4, 0, 0);
			}
			break;
		case JF16:
			EXTCL_CPU_WR_MEM(Jaleco_JF16);
			break;
		case JF17:
			EXTCL_CPU_WR_MEM(Jaleco_JF17);
			break;
		case JF19:
			EXTCL_CPU_WR_MEM(Jaleco_JF19);
			break;
		case SS8806:
			EXTCL_CPU_WR_MEM(Jaleco_SS8806);
			EXTCL_SAVE_MAPPER(Jaleco_SS8806);
			EXTCL_CPU_EVERY_CYCLE(Jaleco_SS8806);
			mapper.internal_struct[0] = (BYTE *) &ss8806;
			mapper.internal_struct_size[0] = sizeof(ss8806);

			if (info.reset >= HARD) {
				BYTE i;

				memset(&ss8806, 0x00, sizeof(ss8806));
				ss8806.mask = 0xFFFF;
				ss8806.delay = 255;

				for (i = 0; i < 8; i++) {
					ss8806.chr_rom_bank[i] = i;
				}
			} else {
				ss8806.mask = 0xFFFF;
				ss8806.reload = 0;
				ss8806.count = 0;
				ss8806.delay = 255;
			}

			switch (info.id) {
				case JAJAMARU:
				case MEZASETOPPRO:
					info.prg.ram.banks_8k_plus = 1;
					info.prg.ram.bat.banks = 1;
					break;
			}
			break;
		default:
			break;
	}
}

void extcl_cpu_wr_mem_Jaleco_JF05(WORD address, BYTE value) {
	DBWORD bank;

	if ((address < 0x6000) || (address >= 0x8000)) {
		return;
	}

	value = (((value >> 1) & 0x1) | ((value << 1) & 0x2));
	control_bank_with_AND(0x03, info.chr.rom.max.banks_8k)
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

void extcl_cpu_wr_mem_Jaleco_JF11(WORD address, BYTE value) {
	BYTE save = value;
	DBWORD bank;

	if ((address < 0x6000) || (address >= 0x8000)) {
		return;
	}

	value >>= 4;
	control_bank_with_AND(0x03, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();

	value = save;
	control_bank_with_AND(0x0F, info.chr.rom.max.banks_8k)
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

void extcl_cpu_wr_mem_Jaleco_JF13(WORD address, BYTE value) {
	BYTE save = value;
	DBWORD bank;

	/* FIXME: da 0x7000 a 0x7FFF c'e' la gestione dell'audio */
	if ((address < 0x6000) || (address >= 0x7000)) {
		return;
	}

	value >>= 4;
	control_bank_with_AND(0x03, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();

	value = ((save & 0x40) >> 4) | (save & 0x03);
	control_bank(info.chr.rom.max.banks_8k)
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

void extcl_cpu_wr_mem_Jaleco_JF16(WORD address, BYTE value) {
	BYTE save = value &= prg_rom_rd(address);
	DBWORD bank;

	control_bank_with_AND(0x07, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);
	map_prg_rom_8k_update();

	value = save >> 4;
	control_bank_with_AND(0x0F, info.chr.rom.max.banks_8k)
	bank = value << 13;
	chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
	chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
	chr.bank_1k[2] = chr_chip_byte_pnt(0, bank | 0x0800);
	chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0C00);
	chr.bank_1k[4] = chr_chip_byte_pnt(0, bank | 0x1000);
	chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x1400);
	chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x1800);
	chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x1C00);

	if (save & 0x08) {
		mirroring_SCR1();
		if (info.mapper.submapper == HOLYDIVER) {
			mirroring_V();
		}
	} else {
		mirroring_SCR0();
		if (info.mapper.submapper == HOLYDIVER) {
			mirroring_H();
		}
	}
}

void extcl_cpu_wr_mem_Jaleco_JF17(WORD address, BYTE value) {
	/* bus conflict */
	BYTE save = value &= prg_rom_rd(address);
	DBWORD bank;

	if (save & 0x80) {
		control_bank_with_AND(0x0F, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k_update();
	}

	if (save & 0x40) {
		value = save;
		control_bank_with_AND(0x0F, info.chr.rom.max.banks_8k)
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

	/* FIXME : aggiungere l'emulazione del D7756C */
}

void extcl_cpu_wr_mem_Jaleco_JF19(WORD address, BYTE value) {
	/* bus conflict */
	BYTE save = value &= prg_rom_rd(address);
	DBWORD bank;

	if (save & 0x80) {
		control_bank_with_AND(0x0F, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 2, value);
		map_prg_rom_8k_update();
	}

	if (save & 0x40) {
		value = save;
		control_bank_with_AND(0x0F, info.chr.rom.max.banks_8k)
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

	/* FIXME : aggiungere l'emulazione del D7756C */
}

void extcl_cpu_wr_mem_Jaleco_SS8806(WORD address, BYTE value) {
	switch (address) {
		case 0x8000:
			prg_rom_8k_update(0, 0xF0, 0);
			break;
		case 0x8001:
			prg_rom_8k_update(0, 0x0F, 4);
			break;
		case 0x8002:
			prg_rom_8k_update(1, 0xF0, 0);
			break;
		case 0x8003:
			prg_rom_8k_update(1, 0x0F, 4);
			break;
		case 0x9000:
			prg_rom_8k_update(2, 0xF0, 0);
			break;
		case 0x9001:
			prg_rom_8k_update(2, 0x0F, 4);
			break;
		case 0x9002:
			if ((value != 0x03) || !value) {
				break;
			}

			if (value & 0x03) {
				cpu.prg_ram_rd_active = cpu.prg_ram_wr_active = TRUE;
			} else {
				cpu.prg_ram_rd_active = cpu.prg_ram_wr_active = FALSE;
			}
			break;
		case 0xA000:
			chr_rom_1k_update(0, 0xF0, 0);
			break;
		case 0xA001:
			chr_rom_1k_update(0, 0x0F, 4);
			break;
		case 0xA002:
			chr_rom_1k_update(1, 0xF0, 0);
			break;
		case 0xA003:
			chr_rom_1k_update(1, 0x0F, 4);
			break;
		case 0xB000:
			chr_rom_1k_update(2, 0xF0, 0);
			break;
		case 0xB001:
			chr_rom_1k_update(2, 0x0F, 4);
			break;
		case 0xB002:
			chr_rom_1k_update(3, 0xF0, 0);
			break;
		case 0xB003:
			chr_rom_1k_update(3, 0x0F, 4);
			break;
		case 0xC000:
			chr_rom_1k_update(4, 0xF0, 0);
			break;
		case 0xC001:
			chr_rom_1k_update(4, 0x0F, 4);
			break;
		case 0xC002:
			chr_rom_1k_update(5, 0xF0, 0);
			break;
		case 0xC003:
			chr_rom_1k_update(5, 0x0F, 4);
			break;
		case 0xD000:
			chr_rom_1k_update(6, 0xF0, 0);
			break;
		case 0xD001:
			chr_rom_1k_update(6, 0x0F, 4);
			break;
		case 0xD002:
			chr_rom_1k_update(7, 0xF0, 0);
			break;
		case 0xD003:
			chr_rom_1k_update(7, 0x0F, 4);
			break;
		case 0xE000:
			ss8806.reload = (ss8806.reload & 0xFFF0) | (value & 0x0F);
			break;
		case 0xE001:
			ss8806.reload = (ss8806.reload & 0xFF0F) | (value & 0x0F) << 4;
			break;
		case 0xE002:
			ss8806.reload = (ss8806.reload & 0xF0FF) | (value & 0x0F) << 8;
			break;
		case 0xE003:
			ss8806.reload = (ss8806.reload & 0x0FFF) | (value & 0x0F) << 12;
			break;
		case 0xF000:
			ss8806.count = ss8806.reload;
			irq.high &= ~EXT_IRQ;
			break;
		case 0xF001:
			ss8806.enabled = value & 0x01;
			if (value & 0x8) {
				ss8806.mask = 0x000F;
			} else if (value & 0x4) {
				ss8806.mask = 0x00FF;
			} else if (value & 0x2) {
				ss8806.mask = 0x0FFF;
			} else {
				ss8806.mask = 0xFFFF;
			}
			irq.high &= ~EXT_IRQ;
			break;
		case 0xF002:
			if (value & 0x02) {
				mirroring_SCR0();
			} else if (value & 0x01) {
				mirroring_V();
			} else {
				mirroring_H();
			}
			break;
	}
}
BYTE extcl_save_mapper_Jaleco_SS8806(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, ss8806.chr_rom_bank);
	save_slot_ele(mode, slot, ss8806.enabled);
	save_slot_ele(mode, slot, ss8806.mask);
	save_slot_ele(mode, slot, ss8806.reload);
	save_slot_ele(mode, slot, ss8806.count);
	save_slot_ele(mode, slot, ss8806.delay);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_Jaleco_SS8806(void) {
	if (!ss8806.enabled) {
		return;
	}

	/* gestisco questo delay sempre per la sincronizzazzione con la CPU */
	if (ss8806.delay != 255) {
		if (!(--ss8806.delay)) {
			irq.delay = TRUE;
			irq.high |= EXT_IRQ;
			ss8806.delay = 255;
		}
	}

	if ((ss8806.count & ss8806.mask) && !(--ss8806.count & ss8806.mask)) {
		ss8806.delay = 1;
	}
}

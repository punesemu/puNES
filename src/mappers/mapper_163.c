/*
 * mapper_163.c
 *
 *  Created on: 6/ott/2011
 *      Author: fhorse
 */

#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "ppu.h"
#include "save_slot.h"

void map_init_163(void) {
	EXTCL_CPU_WR_MEM(163);
	EXTCL_CPU_RD_MEM(163);
	EXTCL_PPU_UPDATE_SCREEN_Y(163);
	EXTCL_SAVE_MAPPER(163);
	mapper.internal_struct[0] = (BYTE *) &m163;
	mapper.internal_struct_size[0] = sizeof(m163);

	memset(&m163, 0x00, sizeof(m163));
	m163.reg = 1;
	m163.prg = 0x0F;

	{
		BYTE value = m163.prg;

		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	}

	info.mapper.extend_wr = TRUE;

	info.prg.ram.banks_8k_plus = 1;
	info.prg.ram.bat.banks = 1;
}
void extcl_cpu_wr_mem_163(WORD address, BYTE value) {
	if ((address == 0x5100) && (value == 0x06)) {
		map_prg_rom_8k(4, 0, 3);
		map_prg_rom_8k_update();
		return;
	} else if (address == 0x5101) {
		if (m163.reg && !value) {
			m163.trigger ^= 0x01;
		}
		m163.reg = value;
		return;
	}

	switch (address & 0x7300) {
		case 0x5000:
			/* CHR */
			if (!(m163.chr = value & 0x80) && (ppu.screen_y < 128)) {
				m163.chr_mode = 0;
				chr.bank_1k[0] = chr_chip_byte_pnt(0, 0x0000);
				chr.bank_1k[1] = chr_chip_byte_pnt(0, 0x0400);
				chr.bank_1k[2] = chr_chip_byte_pnt(0, 0x0800);
				chr.bank_1k[3] = chr_chip_byte_pnt(0, 0x0C00);
				chr.bank_1k[4] = chr_chip_byte_pnt(0, 0x1000);
				chr.bank_1k[5] = chr_chip_byte_pnt(0, 0x1400);
				chr.bank_1k[6] = chr_chip_byte_pnt(0, 0x1800);
				chr.bank_1k[7] = chr_chip_byte_pnt(0, 0x1C00);
			}

			/* PRG */
			m163.prg = (m163.prg & 0xF0) | (value & 0x0F);
			value = m163.prg;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0x5200:
			m163.prg = (m163.prg & 0x0F) | (value << 4);
			value = m163.prg;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0x5300:
			m163.security = value;
			return;
	}
}
BYTE extcl_cpu_rd_mem_163(WORD address, BYTE openbus, BYTE before) {
	switch (address & 0x7700) {
		case 0x5000:
		case 0x5200:
		case 0x5300:
		case 0x5400:
		case 0x5600:
		case 0x5700:
			return (0x04);
		case 0x5100:
			return (m163.security);
		case 0x5500:
			if (m163.trigger) {
				return (m163.security);
			}
			return (0);
	}
	return (openbus);
}
void extcl_ppu_update_screen_y_163(void) {
	if (m163.chr && r2001.visible && !r2002.vblank) {
		if (ppu.screen_y == 240) {
			m163.chr_mode = 1;
			chr.bank_1k[0] = chr_chip_byte_pnt(0, 0x0000);
			chr.bank_1k[1] = chr_chip_byte_pnt(0, 0x0400);
			chr.bank_1k[2] = chr_chip_byte_pnt(0, 0x0800);
			chr.bank_1k[3] = chr_chip_byte_pnt(0, 0x0C00);
			chr.bank_1k[4] = chr_chip_byte_pnt(0, 0x0000);
			chr.bank_1k[5] = chr_chip_byte_pnt(0, 0x0400);
			chr.bank_1k[6] = chr_chip_byte_pnt(0, 0x0800);
			chr.bank_1k[7] = chr_chip_byte_pnt(0, 0x0C00);
		} else if (ppu.screen_y == 128) {
			m163.chr_mode = 2;
			chr.bank_1k[0] = chr_chip_byte_pnt(0, 0x1000);
			chr.bank_1k[1] = chr_chip_byte_pnt(0, 0x1400);
			chr.bank_1k[2] = chr_chip_byte_pnt(0, 0x1800);
			chr.bank_1k[3] = chr_chip_byte_pnt(0, 0x1C00);
			chr.bank_1k[4] = chr_chip_byte_pnt(0, 0x1000);
			chr.bank_1k[5] = chr_chip_byte_pnt(0, 0x1400);
			chr.bank_1k[6] = chr_chip_byte_pnt(0, 0x1800);
			chr.bank_1k[7] = chr_chip_byte_pnt(0, 0x1C00);
		}
	}
}
BYTE extcl_save_mapper_163(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m163.prg);
	save_slot_ele(mode, slot, m163.chr);
	save_slot_ele(mode, slot, m163.reg);
	save_slot_ele(mode, slot, m163.security);
	save_slot_ele(mode, slot, m163.trigger);
	save_slot_ele(mode, slot, m163.chr_mode);

	if (mode == SAVE_SLOT_READ) {
		switch(m163.chr_mode) {
			case 0:
				chr.bank_1k[0] = chr_chip_byte_pnt(0, 0x0000);
				chr.bank_1k[1] = chr_chip_byte_pnt(0, 0x0400);
				chr.bank_1k[2] = chr_chip_byte_pnt(0, 0x0800);
				chr.bank_1k[3] = chr_chip_byte_pnt(0, 0x0C00);
				chr.bank_1k[4] = chr_chip_byte_pnt(0, 0x1000);
				chr.bank_1k[5] = chr_chip_byte_pnt(0, 0x1400);
				chr.bank_1k[6] = chr_chip_byte_pnt(0, 0x1800);
				chr.bank_1k[7] = chr_chip_byte_pnt(0, 0x1C00);
				break;
			case 1:
				chr.bank_1k[0] = chr_chip_byte_pnt(0, 0x0000);
				chr.bank_1k[1] = chr_chip_byte_pnt(0, 0x0400);
				chr.bank_1k[2] = chr_chip_byte_pnt(0, 0x0800);
				chr.bank_1k[3] = chr_chip_byte_pnt(0, 0x0C00);
				chr.bank_1k[4] = chr_chip_byte_pnt(0, 0x0000);
				chr.bank_1k[5] = chr_chip_byte_pnt(0, 0x0400);
				chr.bank_1k[6] = chr_chip_byte_pnt(0, 0x0800);
				chr.bank_1k[7] = chr_chip_byte_pnt(0, 0x0C00);
				break;
			case 2:
				chr.bank_1k[0] = chr_chip_byte_pnt(0, 0x1000);
				chr.bank_1k[1] = chr_chip_byte_pnt(0, 0x1400);
				chr.bank_1k[2] = chr_chip_byte_pnt(0, 0x1800);
				chr.bank_1k[3] = chr_chip_byte_pnt(0, 0x1C00);
				chr.bank_1k[4] = chr_chip_byte_pnt(0, 0x1000);
				chr.bank_1k[5] = chr_chip_byte_pnt(0, 0x1400);
				chr.bank_1k[6] = chr_chip_byte_pnt(0, 0x1800);
				chr.bank_1k[7] = chr_chip_byte_pnt(0, 0x1C00);
				break;
		}
	}
	return (EXIT_OK);
}

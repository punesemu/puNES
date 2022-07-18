/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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
#include "mem_map.h"
#include "ppu.h"
#include "save_slot.h"

// TODO : aggiungere l'emulazione del floppy disk controller e della tastiera.

INLINE static void prg_fix_518(void);
INLINE static void prg_ram_fix_518(void);
INLINE static void mirroring_fix_518(void);

_m518 m518;
struct _m518tmp {
	BYTE *prg_8000;
	BYTE *prg_A000;
	BYTE *prg_C000;
	BYTE *prg_E000;
} m518tmp;

void map_init_518(void) {
	EXTCL_AFTER_MAPPER_INIT(518);
	EXTCL_CPU_WR_MEM(518);
	EXTCL_CPU_RD_MEM(518);
	EXTCL_SAVE_MAPPER(518);
	EXTCL_RD_NMT(518);
	EXTCL_RD_CHR(518);
	EXTCL_CPU_EVERY_CYCLE(518);
	mapper.internal_struct[0] = (BYTE *)&m518;
	mapper.internal_struct_size[0] = sizeof(m518);

	memset(&m518, 0x00, sizeof(m518));

	m518tmp.prg_8000 = m518tmp.prg_A000 = NULL;
	m518tmp.prg_C000 = m518tmp.prg_E000 = NULL;

	info.prg.ram.banks_8k_plus = 16;

	info.mapper.extend_wr = info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_518(void) {
	prg_fix_518();
	prg_ram_fix_518();
	mirroring_fix_518();
}
void extcl_cpu_wr_mem_518(WORD address, BYTE value) {
	switch (address & 0xFF00) {
		case 0x5000:
			m518.reg[0] = value;
			prg_fix_518();
			break;
		case 0x5200:
			m518.reg[1] = value;
			prg_fix_518();
			mirroring_fix_518();
			break;
		case 0x5300:
			m518.dac.out = value;
			m518.dac.status = 0x00;
			break;
	}
}
BYTE extcl_cpu_rd_mem_518(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xFF00) {
		case 0x5300:
			return (m518.dac.status);
		case 0x8000: case 0x8100: case 0x8200: case 0x8300: case 0x8400: case 0x8500: case 0x8600: case 0x8700:
		case 0x8800: case 0x8900: case 0x8A00: case 0x8B00: case 0x8C00: case 0x8D00: case 0x8E00: case 0x8F00:
		case 0x9000: case 0x9100: case 0x9200: case 0x9300: case 0x9400: case 0x9500: case 0x9600: case 0x9700:
		case 0x9800: case 0x9900: case 0x9A00: case 0x9B00: case 0x9C00: case 0x9D00: case 0x9E00: case 0x9F00:
			return (m518tmp.prg_8000 ? m518tmp.prg_8000[address & 0x1FFF] : openbus);
			break;
		case 0xA000: case 0xA100: case 0xA200: case 0xA300: case 0xA400: case 0xA500: case 0xA600: case 0xA700:
		case 0xA800: case 0xA900: case 0xAA00: case 0xAB00: case 0xAC00: case 0xAD00: case 0xAE00: case 0xAF00:
		case 0xB000: case 0xB100: case 0xB200: case 0xB300: case 0xB400: case 0xB500: case 0xB600: case 0xB700:
		case 0xB800: case 0xB900: case 0xBA00: case 0xBB00: case 0xBC00: case 0xBD00: case 0xBE00: case 0xBF00:
			return (m518tmp.prg_A000 ? m518tmp.prg_A000[address & 0x1FFF] : openbus);
			break;
		case 0xC000: case 0xC100: case 0xC200: case 0xC300: case 0xC400: case 0xC500: case 0xC600: case 0xC700:
		case 0xC800: case 0xC900: case 0xCA00: case 0xCB00: case 0xCC00: case 0xCD00: case 0xCE00: case 0xCF00:
		case 0xD000: case 0xD100: case 0xD200: case 0xD300: case 0xD400: case 0xD500: case 0xD600: case 0xD700:
		case 0xD800: case 0xD900: case 0xDA00: case 0xDB00: case 0xDC00: case 0xDD00: case 0xDE00: case 0xDF00:
			return (m518tmp.prg_C000 ? m518tmp.prg_C000[address & 0x1FFF] : openbus);
			break;
		case 0xE000: case 0xE100: case 0xE200: case 0xE300: case 0xE400: case 0xE500: case 0xE600: case 0xE700:
		case 0xE800: case 0xE900: case 0xEA00: case 0xEB00: case 0xEC00: case 0xED00: case 0xEE00: case 0xEF00:
		case 0xF000: case 0xF100: case 0xF200: case 0xF300: case 0xF400: case 0xF500: case 0xF600: case 0xF700:
		case 0xF800: case 0xF900: case 0xFA00: case 0xFB00: case 0xFC00: case 0xFD00: case 0xFE00: case 0xFF00:
			return (m518tmp.prg_E000 ? m518tmp.prg_E000[address & 0x1FFF] : openbus);
			break;
		default:
			return (openbus);
	}
}
BYTE extcl_save_mapper_518(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m518.reg);
	save_slot_ele(mode, slot, m518.chr_bank);
	save_slot_ele(mode, slot, m518.dac.out);
	save_slot_ele(mode, slot, m518.dac.status);
	save_slot_ele(mode, slot, m518.dac.count);

	if (mode == SAVE_SLOT_READ) {
		prg_fix_518();
	}

	return (EXIT_OK);
}
BYTE extcl_rd_nmt_518(WORD address) {
	BYTE slot = (address & 0x0FFF) >> 10;

	m518.chr_bank = ((slot >> (m518.reg[1] & 0x01)) & 0x01) << 2;
	return (ntbl.bank_1k[slot][address & 0x3FF]);
}
BYTE extcl_rd_chr_518(WORD address) {
	BYTE slot = (address & 0x1FFF) >> 10;

	return (chr.bank_1k[(address < 0x1000) && (m518.reg[1] & 0x02) ? m518.chr_bank | (slot & 0x03) : slot][address & 0x3FF]);
}
void extcl_cpu_every_cycle_518(void) {
	m518.dac.count++;
	if (m518.dac.count == 1789772 / 11025) {
		m518.dac.count = 0;
		m518.dac.status = 0x80 | (m518.dac.out & 0x7F);
	}
}

INLINE static void prg_fix_518(void) {
	DBWORD bank;

	m518tmp.prg_8000 = m518tmp.prg_A000 = NULL;
	m518tmp.prg_C000 = m518tmp.prg_E000 = NULL;

	if (m518.reg[0] & 0x80) {
		if (m518.reg[1] & 0x04) {
			bank = (m518.reg[0] & 0x03) << 15;
			m518tmp.prg_8000 = &prg.ram_plus[bank | 0x0000];
			m518tmp.prg_A000 = &prg.ram_plus[bank | 0x2000];
			m518tmp.prg_C000 = &prg.ram_plus[bank | 0x4000];
			m518tmp.prg_E000 = &prg.ram_plus[bank | 0x8000];
		} else {
			bank = (m518.reg[0] & 0x07) << 14;
			m518tmp.prg_8000 = &prg.ram_plus[bank | 0x0000];
			m518tmp.prg_A000 = &prg.ram_plus[bank | 0x2000];

			bank = 0;
			_control_bank(bank, info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, bank);
		}
	} else {
		if (m518.reg[1] & 0x04) {
			bank = m518.reg[0];
			_control_bank(bank, info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, bank);
		} else {
			bank = m518.reg[0];
			_control_bank(bank, info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, bank);

			bank = 0;
			_control_bank(bank, info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, bank);
		}
	}
	map_prg_rom_8k_update();
}
INLINE static void prg_ram_fix_518(void) {
	prg.ram_plus_8k = &prg.ram_plus[0x0F << 13];
}
INLINE static void mirroring_fix_518(void) {
	if (m518.reg[1] & 0x01) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}

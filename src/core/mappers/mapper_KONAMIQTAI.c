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
#include "info.h"
#include "mem_map.h"
#include "cpu.h"
#include "ppu.h"
#include "save_slot.h"

INLINE static BYTE prg_bank_calculate(BYTE reg);
INLINE static void prg_fix_KONAMIQTAI(void);
INLINE static BYTE prg_ram_bank_calculate(BYTE reg);
INLINE static void prg_ram_fix_KONAMIQTAI(void);
INLINE static void chr_fix_KONAMIQTAI(void);
INLINE static void mirroring_fix_KONAMIQTAI(void);

static const BYTE page_table[0x24] ={
	// JIS X 0208 rows $20-$4F. $20 is not a valid row number.
	0x0,0x0,0x2,0x2,0x1,0x1,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,
	// JIS X 0208 rows $50-$7F. $7F is not a valid row number.
	0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,0xD,0xD
};
struct _konamiqtai {
	BYTE reg[16];
	BYTE qt_ram[0x800];
	BYTE qt_byte;
	struct __konamiqtai_irq {
		BYTE after;
		BYTE enable;
		WORD counter;
		WORD latch;
	} irq;
} konamiqtai;
struct _konamiqtaitmp {
	BYTE *prg_6000;
	BYTE *prg_7000;
} konamiqtaitmp;

void map_init_KONAMIQTAI(void) {
	EXTCL_AFTER_MAPPER_INIT(KONAMIQTAI);
	EXTCL_CPU_WR_MEM(KONAMIQTAI);
	EXTCL_CPU_RD_MEM(KONAMIQTAI);
	EXTCL_SAVE_MAPPER(KONAMIQTAI);
	EXTCL_CPU_EVERY_CYCLE(KONAMIQTAI);
	EXTCL_WR_NMT(KONAMIQTAI);
	EXTCL_RD_NMT(KONAMIQTAI);
	EXTCL_WR_CHR(KONAMIQTAI);
	EXTCL_RD_CHR(KONAMIQTAI);
	mapper.internal_struct[0] = (BYTE *)&konamiqtai;
	mapper.internal_struct_size[0] = sizeof(konamiqtai);

	memset(&konamiqtai, 0x00, sizeof(konamiqtai));

	info.prg.ram.banks_8k_plus = 2;
	info.prg.ram.bat.banks = 1;
	info.prg.ram.bat.start = 1;

	if (info.format != NES_2_0) {
		info.chr.ram.banks_8k_plus = 1;
	}

	map_chr_ram_extra_init(info.chr.ram.banks_8k_plus * 0x2000);

	info.mapper.ram_plus_op_controlled_by_mapper = TRUE;
	info.mapper.extend_wr = info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_KONAMIQTAI(void) {
	prg_fix_KONAMIQTAI();
	prg_ram_fix_KONAMIQTAI();
	chr_fix_KONAMIQTAI();
	mirroring_fix_KONAMIQTAI();
}
void extcl_cpu_wr_mem_KONAMIQTAI(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x6000:
			konamiqtaitmp.prg_6000[address & 0x0FFF] = value;
			break;
		case 0x7000:
			konamiqtaitmp.prg_7000[address & 0x0FFF] = value;
			break;
		case 0xD000: {
			BYTE reg = (address & 0x0F00) >> 8;

			konamiqtai.reg[reg] = value;

			switch (address & 0xFF00) {
				case 0xD000:
				case 0xD100:
					prg_ram_fix_KONAMIQTAI();
					break;
				case 0xD200:
				case 0xD300:
				case 0xD400:
					prg_fix_KONAMIQTAI();
					break;
				case 0xD500:
					chr_fix_KONAMIQTAI();
					break;
				case 0xD600:
					konamiqtai.irq.latch = (konamiqtai.irq.latch & 0xFF00) | value;
					break;
				case 0xD700:
					konamiqtai.irq.latch = (konamiqtai.irq.latch & 0x00FF) | (value << 8);
					break;
				case 0xD800:
					konamiqtai.irq.enable = konamiqtai.irq.after;
					irq.high &= ~EXT_IRQ;
					break;
				case 0xD900:
					konamiqtai.irq.after = value & 0x01;
					konamiqtai.irq.enable = value & 0x02;
					if (konamiqtai.irq.enable) {
						konamiqtai.irq.counter = konamiqtai.irq.latch;
					}
					irq.high &= ~EXT_IRQ;
					break;
				case 0xDA00:
					mirroring_fix_KONAMIQTAI();
					break;
			}
			break;
		}
	}
}
BYTE extcl_cpu_rd_mem_KONAMIQTAI(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xF000) {
		case 0x6000:
			return (konamiqtaitmp.prg_6000[address & 0x0FFF]);
		case 0x7000:
			return (konamiqtaitmp.prg_7000[address & 0x0FFF]);
		case 0xD000:
			if ((address == 0xDC00) || (address == 0xDD00)) {
				BYTE row = konamiqtai.reg[13] - 0x20;
				BYTE col = konamiqtai.reg[12] - 0x20;

				// "row" and "col" are the first and second 7-bit JIS X 0208 code byte, respectively, each minus the $21 offset.
				if ((row < 0x60) && (col < 0x60)) {
					WORD code =
						(col % 32) +              // First, go through 32 columns of a column-third.
						(row % 16) * 32 +         // Then, through 16 rows of a row-third.
						(col / 32) * 32 * 16 +    // Then, through three column-thirds.
						(row / 16) * 32 * 16 * 3; // Finally, through three row-thirds.
					WORD glyph = (code & 0xFF) | (page_table[code >> 8] << 8);
					DBWORD tile = glyph * 4;      // four tiles per glyph

					if (address == 0xDC00) {
						// tile number
						return ((tile & 0xFF) | (konamiqtai.reg[11] & 0x03));
					} else {
						// bank byte
						return ((tile >> 8) | (konamiqtai.reg[11] & 0x04 ? 0x80 : 0x00) | 0x40);
					}
				} else {
					return (0);
				}
			}
			break;
	}
	return (openbus);
}
BYTE extcl_save_mapper_KONAMIQTAI(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, konamiqtai.reg);
	save_slot_ele(mode, slot, konamiqtai.qt_ram);
	save_slot_ele(mode, slot, konamiqtai.qt_byte);
	save_slot_ele(mode, slot, konamiqtai.irq.after);
	save_slot_ele(mode, slot, konamiqtai.irq.enable);
	save_slot_ele(mode, slot, konamiqtai.irq.counter);
	save_slot_ele(mode, slot, konamiqtai.irq.latch);

	if (mode == SAVE_SLOT_READ) {
		prg_ram_fix_KONAMIQTAI();
		chr_fix_KONAMIQTAI();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_KONAMIQTAI(void) {
	if (konamiqtai.irq.enable) {
		konamiqtai.irq.counter++;
		if (!konamiqtai.irq.counter) {
			konamiqtai.irq.counter = konamiqtai.irq.latch;
			irq.high |= EXT_IRQ;
		}
	}
}
void extcl_wr_nmt_KONAMIQTAI(WORD address, BYTE value) {
	if (konamiqtai.reg[10] & 0x01) {
		BYTE bank = mapper.mirroring == MIRRORING_HORIZONTAL ? (address & 0x800) ? 1 : 0 : (address & 0x400) ? 1 : 0;

		konamiqtai.qt_ram[(bank << 10) | (address & 0x3FF)] = value;
		return;
	}
	ntbl.bank_1k[(address & 0x0FFF) >> 10][address & 0x3FF] = value;
}

BYTE extcl_rd_nmt_KONAMIQTAI(WORD address) {
	address &= 0x0FFF;
	if ((address & 0x03FF) < 0x3C0) {
		BYTE bank = mapper.mirroring == MIRRORING_HORIZONTAL ? (address & 0x800) ? 1 : 0 : (address & 0x400) ? 1 : 0;

		konamiqtai.qt_byte = konamiqtai.qt_ram[(bank << 10) | (address & 0x3FF)];
	}
	return (ntbl.bank_1k[address >> 10][address & 0x3FF]);
}
void extcl_wr_chr_KONAMIQTAI(WORD address, BYTE value) {
	chr.bank_1k[address >> 10][address & 0x3FF] = value;
}
BYTE extcl_rd_chr_KONAMIQTAI(WORD address) {
	// controllo di trattare il background
	if (((address & 0xFFF7) == ppu.bck_adr)) {
		if (konamiqtai.qt_byte & 0x40) {
			if (address & 0x0008) {
				return (konamiqtai.qt_byte & 0x80 ? 0xFF : 0x00);
			} else {
				DBWORD addr256K = ((konamiqtai.qt_byte & 0x3F) << 12) | (address & 0x0FFF);

				if (chr_size() == (128 * 1024)) {
					addr256K = ((addr256K & 0x00007) << 1) | ((addr256K & 0x00010) >> 4) | ((addr256K & 0x3FFE0) >> 1);
				}
				return (chr_rom()[addr256K]);
			}
		}
		return (chr.extra.data[((konamiqtai.qt_byte & 0x01) << 12) | (address & 0x0FFF)]);
	}
	return (chr.bank_1k[address >> 10][address & 0x3FF]);
}

INLINE static void prg_fix_KONAMIQTAI(void) {
	BYTE value;

	value = prg_bank_calculate(2);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0, value);

	value = prg_bank_calculate(3);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, value);

	value = prg_bank_calculate(4);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2, value);

	value = 0x4F;
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, value);

	map_prg_rom_8k_update();
}
INLINE static BYTE prg_bank_calculate(BYTE reg) {
	return (konamiqtai.reg[reg] & 0x40 ? ((konamiqtai.reg[reg] & 0x3F) + 0x10): (konamiqtai.reg[reg] & 0x0F));
}
INLINE static void prg_ram_fix_KONAMIQTAI(void) {
	konamiqtaitmp.prg_6000 = &prg.ram_plus_8k[prg_ram_bank_calculate(0) << 12];
	konamiqtaitmp.prg_7000 = &prg.ram_plus_8k[prg_ram_bank_calculate(1) << 12];
}
INLINE static BYTE prg_ram_bank_calculate(BYTE reg) {
	return ((konamiqtai.reg[reg] & 0x01) | (!(konamiqtai.reg[reg] & 0x80) >> 2));
}
INLINE static void chr_fix_KONAMIQTAI(void) {
	DBWORD value;

	value = konamiqtai.reg[5] & 0x01;
	control_bank(info.chr.ram.max.banks_4k)
	value <<= 12;
	chr.bank_1k[0] = &chr.extra.data[value];
	chr.bank_1k[1] = &chr.extra.data[value | 0x400];
	chr.bank_1k[2] = &chr.extra.data[value | 0x800];
	chr.bank_1k[3] = &chr.extra.data[value | 0xC00];

	value = 1;
	control_bank(info.chr.ram.max.banks_4k)
	value <<= 12;
	chr.bank_1k[4] = &chr.extra.data[value];
	chr.bank_1k[5] = &chr.extra.data[value | 0x400];
	chr.bank_1k[6] = &chr.extra.data[value | 0x800];
	chr.bank_1k[7] = &chr.extra.data[value | 0xC00];
}
INLINE static void mirroring_fix_KONAMIQTAI(void) {
	if (konamiqtai.reg[10] & 0x02) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}

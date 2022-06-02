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
#include "ppu_inline.h"

INLINE static void prg_setup_90_209_211(void);
INLINE static void chr_setup_90_209_211(void);
INLINE static void nmt_setup_90_209_211(void);
INLINE static void irq_clock_prescaler_90_209_211(void);

#define chr_90_209_211(index)\
	((m90_209_211.chr.low[index] | (m90_209_211.chr.high[index] << 8)))
#define nmt_rom_90_209_211(index)\
	value = (m90_209_211.nmt.reg[index] & mask) | outer;\
	control_bank(info.chr.rom.max.banks_1k)\
	ntbl.bank_1k[index] = chr_pnt(value << 10);\
	m90_209_211.nmt.write[index] = FALSE
#define nmt_ram_90_209_211(index)\
	value = m90_209_211.nmt.reg[index] & 0x01;\
	ntbl.bank_1k[index] = &ntbl.data[value << 10];\
	m90_209_211.nmt.write[index] = TRUE

struct _m90_209_211 {
	BYTE mul[2];
	BYTE single_byte_ram;
	BYTE add;
	BYTE mode[4];
	BYTE prg[4];
	struct _m90_209_211_chr {
		BYTE latch[2];
		BYTE low[8];
		BYTE high[8];
	} chr;
	struct _m90_209_211_nmt {
		BYTE extended_mode;
		WORD reg[4];
		BYTE write[4];
	} nmt;
	struct _m90_209_211_irq {
		BYTE active;
		BYTE mode;
		BYTE prescaler;
		BYTE count;
		BYTE xor_value;
		BYTE pre_size;
		BYTE premask;
	} irq;
} m90_209_211;
struct _m90_209_211tmp {
	BYTE model;
	BYTE dipswitch;
	struct _m90_209_211_m6000 {
		WORD prg;
		BYTE *rom_8k;
	} m6000;
} m90_209_211tmp;

void map_init_90_209_211(BYTE model) {
	BYTE i;

	EXTCL_CPU_WR_MEM(90_209_211);
	EXTCL_CPU_RD_MEM(90_209_211);
	EXTCL_SAVE_MAPPER(90_209_211);
	EXTCL_CPU_EVERY_CYCLE(90_209_211);
	EXTCL_RD_PPU(90_209_211);
	EXTCL_RD_CHR(90_209_211);
	EXTCL_WR_NMT(90_209_211);
	EXTCL_PPU_000_TO_255(90_209_211);
	EXTCL_PPU_256_TO_319(90_209_211);
	EXTCL_PPU_320_TO_34X(90_209_211);
	EXTCL_UPDATE_R2006(90_209_211);

	mapper.internal_struct[0] = (BYTE *)&m90_209_211;
	mapper.internal_struct_size[0] = sizeof(m90_209_211);

	if (info.reset == RESET) {
		if (
			(info.crc32.prg == 0x642E8D63) || // Tiny Toon Adventures 6 (Unl) [!].nes
			(info.crc32.prg == 0x2572906E)) { // Final Fight 3 (Unl) [!].nes
			m90_209_211tmp.dipswitch = m90_209_211tmp.dipswitch ? 0x00 : 0x040;
		} else if (
			(info.crc32.prg == 0xCE4BA157) || // 45-in-1 (JY-120A) (Unl) [b1].nes
			(info.crc32.prg == 0x3886BCF7) || // Aladdin (Unl).nes
			(info.crc32.prg == 0xCDDB21DA) || // Tekken 2 (Unl) [!].nes
			(info.crc32.prg == 0xEF3E7897) || // Tekken 2 (Unl) [T+Kor].nes
			(info.crc32.prg == 0x859E81FC) || // Tekken 2 (Unl) [T+Kor][a1].nes
			(info.crc32.prg == 0xD12E3F63) || // Popeye 2 - Travels in Persia (Unl) [!].nes
			(info.crc32.prg == 0x73A9F3AE) || // 1996 Super Aladdin III 18-in-1 Series (JY-064).nes
			(info.crc32.prg == 0xA0859966) || // 1996 Super Mortal Kombat III 18-in-1 Series (JY-062).nes
			(info.crc32.prg == 0x2A268152)) { // Mortal Kombat 3 - Special 56 Peoples (Unl) [!].nes
			m90_209_211tmp.dipswitch = m90_209_211tmp.dipswitch ? 0x00 : 0x080;
		} else if (
			(info.crc32.prg == 0x826E8D77)) { // Donkey Kong Country 4 (Unl) [!].nes
			m90_209_211tmp.dipswitch = m90_209_211tmp.dipswitch == 0x00 ? 0xC0 : m90_209_211tmp.dipswitch == 0xC0 ? 0x080 : 0x00;
		} else {
			m90_209_211tmp.dipswitch = (m90_209_211tmp.dipswitch + 0x40) & 0xC0;
		}
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		if (
			(info.crc32.prg == 0x642E8D63)) { // Tiny Toon Adventures 6 (Unl) [!].nes
			m90_209_211tmp.dipswitch = 0x40;
		} else if (
			(info.crc32.prg == 0x73A9F3AE) || // 1996 Super Aladdin III 18-in-1 Series (JY-064).nes
			(info.crc32.prg == 0xD12E3F63)) { // Popeye 2 - Travels in Persia (Unl) [!].nes
			m90_209_211tmp.dipswitch = 0x80;
		} else {
			m90_209_211tmp.dipswitch = 0x00;
		}
	}

	if (info.reset >= HARD) {
		memset(&m90_209_211, 0x00, sizeof(m90_209_211));

		m90_209_211tmp.model = model;

		m90_209_211.mul[0] = 0xFF;
		m90_209_211.mul[1] = 0xFF;
		m90_209_211.single_byte_ram = 0xFF;

		for (i = 0; i < 8; i++) {
			if (i < 4) {
				m90_209_211.prg[i] = 0xFF;
				m90_209_211.nmt.write[i] = TRUE;
			}
			m90_209_211.chr.low[i] = 0xFF;
			m90_209_211.chr.high[i] = 0xFF;
		}

		m90_209_211.chr.latch[0] = 0;
		m90_209_211.chr.latch[1] = 4;

		m90_209_211.irq.prescaler = 0xFF;
		m90_209_211.irq.premask = 0x07;
	} else {
		for (i = 0; i < 8; i++) {
			if (i < 4) {
				m90_209_211.prg[i] = 0xFF;
				m90_209_211.mode[i] = 0;
			}
		}
	}
	prg_setup_90_209_211();
	chr_setup_90_209_211();

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_90_209_211(WORD address, BYTE value) {
	BYTE index;

	if ((m90_209_211.irq.mode & 0x03) == 3) {
		irq_clock_prescaler_90_209_211();
	}

	switch (address & 0xF000) {
		case 0x4000:
			return;
		case 0x5000:
			switch (address & 0x5803) {
				case 0x5800:
					m90_209_211.mul[0] = value;
					break;
				case 0x5801:
					m90_209_211.mul[1] = value;
					break;
				case 0x5802:
					m90_209_211.add += value;
					break;
				case 0x5803:
					m90_209_211.single_byte_ram = value;
					m90_209_211.add = 0;
					break;
			}
			return;
		case 0x6000:
		case 0x7000:
			return;
		case 0x8000:
			if (address & 0x0800) {
				return;
			}
			m90_209_211.prg[address & 0x0003] = value;
			prg_setup_90_209_211();
			return;
		case 0x9000:
			if (address & 0x0800) {
				return;
			}
			m90_209_211.chr.low[address & 0x0007] = value;
			chr_setup_90_209_211();
			return;
		case 0xA000:
			if (address & 0x0800) {
				return;
			}
			m90_209_211.chr.high[address & 0x0007] = value;
			chr_setup_90_209_211();
			return;
		case 0xB000:
			if (address & 0x0800) {
				return;
			}
			index = address & 0x0003;
			if (address & 0x0004) {
				m90_209_211.nmt.reg[index] &= 0x00FF;
				m90_209_211.nmt.reg[index] |= (value << 8);
			} else {
				m90_209_211.nmt.reg[index] &= 0xFF00;
				m90_209_211.nmt.reg[index] |= value;
			}
			nmt_setup_90_209_211();
			return;
		case 0xC000:
			switch (address & 0x0007) {
				case 0:
					m90_209_211.irq.active = value & 0x01;
					if (!m90_209_211.irq.active) {
						m90_209_211.irq.prescaler = 0;
						irq.high &= ~EXT_IRQ;
					}
					break;
				case 1:
					m90_209_211.irq.mode = value;
					if (m90_209_211.irq.mode & 0x04) {
						m90_209_211.irq.premask = 0x07;
					} else {
						m90_209_211.irq.premask = 0xFF;
					}
					break;
				case 2:
					m90_209_211.irq.active = 0;
					m90_209_211.irq.prescaler = 0;
					irq.high &= ~EXT_IRQ;
					break;
				case 3:
					m90_209_211.irq.active = 1;
					break;
				case 4:
					m90_209_211.irq.prescaler = value ^ m90_209_211.irq.xor_value;
					break;
				case 5:
					m90_209_211.irq.count = value ^ m90_209_211.irq.xor_value;
					break;
				case 6:
					m90_209_211.irq.xor_value = value;
					break;
				case 7:
					m90_209_211.irq.pre_size = value;
					break;
			}
			return;
		case 0xD000:
			if (address & 0x0800) {
				return;
			}
			index = address & 0x0003;
			switch (index) {
				case 0:
					switch (m90_209_211tmp.model) {
						case MAP209:
						case MAP211:
						case MAP281:
						case MAP282:
						case MAP295:
							m90_209_211.nmt.extended_mode = !!(value & 0x20);
							break;
						case MAP90:
						default:
							m90_209_211.nmt.extended_mode = FALSE;
							break;
					}
					m90_209_211.mode[index] = value;
					break;
				case 1:
					if (m90_209_211tmp.model == MAP90) {
						value &= ~0x08;
					}
					m90_209_211.mode[index] = value;
					break;
				case 2:
				case 3:
					m90_209_211.mode[index] = value;
					break;
			}
			prg_setup_90_209_211();
			chr_setup_90_209_211();
			nmt_setup_90_209_211();
			return;
	}
}
BYTE extcl_cpu_rd_mem_90_209_211(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (address <= 0x4FFF) {
		return (openbus);
	}
	if (address <= 0x5FFF) {
		if (!(address & 0x03FF) && (address != 0x5800)) {
			return ((m90_209_211tmp.dipswitch & 0xC0) | (openbus & 0x3F));
		}
		switch (address & 0x5803) {
			case 0x5800:
				return (m90_209_211.mul[0] * m90_209_211.mul[1]);
			case 0x5801:
				return ((m90_209_211.mul[0] * m90_209_211.mul[1]) >> 8);
			case 0x5802:
				return (m90_209_211.add);
			case 0x5803:
				return (m90_209_211.single_byte_ram);
		}
		return (openbus);
	}
	if ((address <= 0x7FFF) && (m90_209_211.mode[0] & 0x80)) {
		return (m90_209_211tmp.m6000.rom_8k[address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_90_209_211(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m90_209_211.mul);
	save_slot_ele(mode, slot, m90_209_211.single_byte_ram);
	save_slot_ele(mode, slot, m90_209_211.add);

	save_slot_ele(mode, slot, m90_209_211.mode);

	save_slot_ele(mode, slot, m90_209_211.prg);

	save_slot_ele(mode, slot, m90_209_211.chr.latch);
	save_slot_ele(mode, slot, m90_209_211.chr.low);
	save_slot_ele(mode, slot, m90_209_211.chr.high);

	save_slot_ele(mode, slot, m90_209_211.nmt.extended_mode);
	save_slot_ele(mode, slot, m90_209_211.nmt.reg);
	save_slot_ele(mode, slot, m90_209_211.nmt.write);

	save_slot_ele(mode, slot, m90_209_211.irq.active);
	save_slot_ele(mode, slot, m90_209_211.irq.mode);
	save_slot_ele(mode, slot, m90_209_211.irq.prescaler);
	save_slot_ele(mode, slot, m90_209_211.irq.count);
	save_slot_ele(mode, slot, m90_209_211.irq.xor_value);
	save_slot_ele(mode, slot, m90_209_211.irq.pre_size);
	save_slot_ele(mode, slot, m90_209_211.irq.premask);

	if (mode == SAVE_SLOT_READ) {
		prg_setup_90_209_211();
		chr_setup_90_209_211();
		nmt_setup_90_209_211();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_90_209_211(void) {
	if ((m90_209_211.irq.mode & 0x03) == 0) {
		irq_clock_prescaler_90_209_211();
	}
}
void extcl_rd_ppu_90_209_211(UNUSED(WORD address)) {
	if ((m90_209_211.irq.mode & 0x03) == 2) {
		irq_clock_prescaler_90_209_211();
	}
}
BYTE extcl_rd_chr_90_209_211(WORD address) {
	if (m90_209_211.mode[3] & 0x80) {
		switch (address & 0x0FF8) {
			case 0x0FD8:
			case 0x0FE8: {
				BYTE last = chr.bank_1k[address >> 10][address & 0x3FF];

				m90_209_211.chr.latch[address >> 12] = (address >> 4) & (((address >> 10) & 0x04) | 0x02);
				if ((m90_209_211.mode[0] & 0x18) == 0x08) {
					chr_setup_90_209_211();
				}
				return (last);
			}
		}
	}
	return (chr.bank_1k[address >> 10][address & 0x3FF]);
}
void extcl_wr_nmt_90_209_211(WORD address, BYTE value) {
	BYTE index = (address & 0x0FFF) >> 10;

	if ((m90_209_211.nmt.extended_mode == TRUE) && (m90_209_211.nmt.write[index] == FALSE)) {
		return;
	}
	ntbl.bank_1k[index][address & 0x3FF] = value;
}
void extcl_ppu_000_to_255_90_209_211(void) {
	if (r2001.visible) {
		extcl_ppu_320_to_34x_90_209_211();
	}
}
void extcl_ppu_256_to_319_90_209_211(void) {
	if ((ppu.frame_x & 0x0007) != 0x0003) {
		return;
	}

	if ((!spr_ev.count_plus || (spr_ev.tmp_spr_plus == spr_ev.count_plus)) && (r2000.size_spr == 16)) {
		ppu.spr_adr = r2000.spt_adr;
	} else {
		ppu_spr_adr((ppu.frame_x & 0x0038) >> 3);
	}
	if ((ppu.spr_adr & 0x1000) > (ppu.bck_adr & 0x1000)) {
		if ((m90_209_211.irq.mode & 0x03) == 1) {
			irq_clock_prescaler_90_209_211();
		}
	}
}
void extcl_ppu_320_to_34x_90_209_211(void) {
	if ((ppu.frame_x & 0x0007) != 0x0003) {
		return;
	}

	if (ppu.frame_x == 323) {
		ppu_spr_adr(7);
	}

	ppu_bck_adr(r2000.bpt_adr, r2006.value);

	if ((ppu.bck_adr & 0x1000) > (ppu.spr_adr & 0x1000)) {
		if ((m90_209_211.irq.mode & 0x03) == 1) {
			irq_clock_prescaler_90_209_211();
		}
	}
}
void extcl_update_r2006_90_209_211(WORD new_r2006, WORD old_r2006) {
	if ((new_r2006 & 0x1000) > (old_r2006 & 0x1000)) {
		if ((m90_209_211.irq.mode & 0x03) == 1) {
			irq_clock_prescaler_90_209_211();
		}
	}
}

INLINE static void prg_setup_90_209_211(void) {
	BYTE outer, mask;
	WORD value;

	switch(m90_209_211tmp.model) {
		case MAP281:
			outer = ((m90_209_211.mode[3] & 0x03) << 5);
			mask = 0x1F;
			break;
		case MAP282:
			outer = ((m90_209_211.mode[3] & 0x06) << 4);
			mask = 0x1F;
			break;
		case MAP295:
			outer = ((m90_209_211.mode[3] & 0x07) << 4);
			mask = 0x0F;
			break;
		default:
			outer = ((m90_209_211.mode[3] & 0x06) << 5);
			mask = 0x3F;
			break;
	}

	switch (m90_209_211.mode[0] & 0x07) {
		case 0:
			// prg rom da 8k al 0x6000
			value = (((m90_209_211.prg[3] << 2) | 0x03) & mask) | outer;
			control_bank(info.prg.rom.max.banks_8k)
			m90_209_211tmp.m6000.prg = value;

			// prg rom switch normale
			value = (mask >> 2) | (outer >> 2);
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			break;
		case 1:
			// prg rom da 8k al 0x6000
			value = (((m90_209_211.prg[3] << 1) | 0x01) & mask) | outer;
			control_bank(info.prg.rom.max.banks_8k)
			m90_209_211tmp.m6000.prg = value;

			// prg rom switch normale
			value = (m90_209_211.prg[1] & (mask >> 1)) | (outer >> 1);
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			value = (mask >> 1) | (outer >> 1);
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, value);
			break;
		case 3: // bit reversion
		case 2:
			// prg rom da 8k al 0x6000
			value = (m90_209_211.prg[3] & mask) | outer;
			control_bank(info.prg.rom.max.banks_8k)
			m90_209_211tmp.m6000.prg = value;

			// prg rom switch normale
			value = (m90_209_211.prg[0] & mask) | outer;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, value);
			value = (m90_209_211.prg[1] & mask) | outer;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, value);
			value = (m90_209_211.prg[2] & mask) | outer;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2, value);
			value = mask | outer;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 3, value);
			break;
		case 4:
			// prg rom da 8k al 0x6000
			value = (((m90_209_211.prg[3] << 2) | 0x03) & mask) | outer;
			control_bank(info.prg.rom.max.banks_8k)
			m90_209_211tmp.m6000.prg = value;

			// prg rom switch normale
			value = (m90_209_211.prg[3] & (mask >> 2)) | (outer >> 2);
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			break;
		case 5:
			// prg rom da 8k al 0x6000
			value = (((m90_209_211.prg[3] << 1) | 0x01) & mask) | outer;
			control_bank(info.prg.rom.max.banks_8k)
			m90_209_211tmp.m6000.prg = value;

			// prg rom switch normale
			value = (m90_209_211.prg[1] & (mask >> 1)) | (outer >> 1);
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			value = (m90_209_211.prg[3] & (mask >> 1)) | (outer >> 1);
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, value);
			break;
		case 7: // bit reversion
		case 6:
			// prg rom da 8k al 0x6000
			value = (m90_209_211.prg[3] & mask) | outer;
			control_bank(info.prg.rom.max.banks_8k)
			m90_209_211tmp.m6000.prg = value;

			// prg rom switch normale
			value = (m90_209_211.prg[0] & mask) | outer;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, value);
			value = (m90_209_211.prg[1] & mask) | outer;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, value);
			value = (m90_209_211.prg[2] & mask) | outer;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2, value);
			value = (m90_209_211.prg[3] & mask) | outer;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 3, value);
			break;
	}
	m90_209_211tmp.m6000.rom_8k = prg_pnt(m90_209_211tmp.m6000.prg << 13);
	map_prg_rom_8k_update();
}
INLINE static void chr_setup_90_209_211(void) {
	WORD outer = 0, mask = 0xFFFF;
	DBWORD bank;

	switch(m90_209_211tmp.model) {
		case MAP281:
			outer = (m90_209_211.mode[3] & 0x03) << 8;
			mask = 0xFF;
			break;
		case MAP295:
			outer = (m90_209_211.mode[3] & 0x03) << 7;
			mask = 0x7F;
			break;
		default:
			if (!(m90_209_211.mode[3] & 0x20)) {
				outer = ((m90_209_211.mode[3] & 0x01) | ((m90_209_211.mode[3] & 0x18) >> 2)) << 8;
				mask = 0xFF;
			} else {
				outer = (m90_209_211.mode[3] & 0x18) << 6;
				mask = 0x1FF;
			}
			break;
	}

	switch (m90_209_211.mode[0] & 0x18) {
		case 0x00:
			bank = (chr_90_209_211(0) & (mask >> 3)) | (outer >> 3);
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
			break;
		case 0x08:
			bank = (chr_90_209_211(m90_209_211.chr.latch[0]) & (mask >> 2)) | (outer >> 2);
			_control_bank(bank, info.chr.rom.max.banks_4k)
			bank <<= 12;
			chr.bank_1k[0] = chr_pnt(bank);
			chr.bank_1k[1] = chr_pnt(bank | 0x0400);
			chr.bank_1k[2] = chr_pnt(bank | 0x0800);
			chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
			bank = (chr_90_209_211(m90_209_211.chr.latch[1]) & (mask >> 2)) | (outer >> 2);
			_control_bank(bank, info.chr.rom.max.banks_4k)
			bank <<= 12;
			chr.bank_1k[4] = chr_pnt(bank);
			chr.bank_1k[5] = chr_pnt(bank | 0x0400);
			chr.bank_1k[6] = chr_pnt(bank | 0x0800);
			chr.bank_1k[7] = chr_pnt(bank | 0x0C00);
			break;
		case 0x10:
			bank = (chr_90_209_211(0) & (mask >> 1)) | (outer >> 1);
			_control_bank(bank, info.chr.rom.max.banks_2k)
			bank <<= 11;
			chr.bank_1k[0] = chr_pnt(bank);
			chr.bank_1k[1] = chr_pnt(bank | 0x0400);
			bank = (chr_90_209_211(2) & (mask >> 1)) | (outer >> 1);
			_control_bank(bank, info.chr.rom.max.banks_2k)
			bank <<= 11;
			chr.bank_1k[2] = chr_pnt(bank);
			chr.bank_1k[3] = chr_pnt(bank | 0x0400);
			bank = (chr_90_209_211(4) & (mask >> 1)) | (outer >> 1);
			_control_bank(bank, info.chr.rom.max.banks_2k)
			bank <<= 11;
			chr.bank_1k[4] = chr_pnt(bank);
			chr.bank_1k[5] = chr_pnt(bank | 0x0400);
			bank = (chr_90_209_211(6) & (mask >> 1)) | (outer >> 1);
			_control_bank(bank, info.chr.rom.max.banks_2k)
			bank <<= 11;
			chr.bank_1k[6] = chr_pnt(bank);
			chr.bank_1k[7] = chr_pnt(bank | 0x0400);
			break;
		case 0x18:
			bank = (chr_90_209_211(0) & mask) | outer;
			_control_bank(bank, info.chr.rom.max.banks_1k)
			bank <<= 10;
			chr.bank_1k[0] = chr_pnt(bank);
			bank = (chr_90_209_211(1) & mask) | outer;
			_control_bank(bank, info.chr.rom.max.banks_1k)
			bank <<= 10;
			chr.bank_1k[1] = chr_pnt(bank);
			bank = (chr_90_209_211(2) & mask) | outer;
			_control_bank(bank, info.chr.rom.max.banks_1k)
			bank <<= 10;
			chr.bank_1k[2] = chr_pnt(bank);
			bank = (chr_90_209_211(3) & mask) | outer;
			_control_bank(bank, info.chr.rom.max.banks_1k)
			bank <<= 10;
			chr.bank_1k[3] = chr_pnt(bank);
			bank = (chr_90_209_211(4) & mask) | outer;
			_control_bank(bank, info.chr.rom.max.banks_1k)
			bank <<= 10;
			chr.bank_1k[4] = chr_pnt(bank);
			bank = (chr_90_209_211(5) & mask) | outer;
			_control_bank(bank, info.chr.rom.max.banks_1k)
			bank <<= 10;
			chr.bank_1k[5] = chr_pnt(bank);
			bank = (chr_90_209_211(6) & mask) | outer;
			_control_bank(bank, info.chr.rom.max.banks_1k)
			bank <<= 10;
			chr.bank_1k[6] = chr_pnt(bank);
			bank = (chr_90_209_211(7) & mask) | outer;
			_control_bank(bank, info.chr.rom.max.banks_1k)
			bank <<= 10;
			chr.bank_1k[7] = chr_pnt(bank);
			break;
	}
}
INLINE static void nmt_setup_90_209_211(void) {
	WORD value;
	BYTE i;

	if (m90_209_211.nmt.extended_mode == TRUE) {
		WORD outer, mask;

		switch(m90_209_211tmp.model) {
			case MAP281:
				outer = (m90_209_211.mode[3] & 0x03) << 8;
				mask = 0xFF;
				break;
			case MAP295:
				outer = (m90_209_211.mode[3] & 0x03) << 7;
				mask = 0x7F;
				break;
			default:
				if (!(m90_209_211.mode[3] & 0x20)) {
					outer = ((m90_209_211.mode[3] & 0x01) | ((m90_209_211.mode[3] & 0x18) >> 2)) << 8;
					mask = 0xFF;
				} else {
					outer = (m90_209_211.mode[3] & 0x18) << 6;
					mask = 0x1FF;
				}
				break;
		}

		for (i = 0; i < 4; i++) {
			if (((m90_209_211.mode[2] ^ m90_209_211.nmt.reg[i]) & 0x80) | (m90_209_211.mode[0] & 0x40)) {
				nmt_rom_90_209_211(i);
			} else {
				nmt_ram_90_209_211(i);
			}
		}
	} else if (m90_209_211.mode[1] & 0x08) {
		for (i = 0; i < 4; i++) {
			nmt_ram_90_209_211(i);
		}
	} else {
		switch (m90_209_211.mode[1] & 0x03) {
			case 0:
				mirroring_V();
				break;
			case 1:
				mirroring_H();
				break;
			case 2:
				mirroring_SCR0();
				break;
			case 3:
				mirroring_SCR1();
				break;
		}
	}
}
INLINE static void irq_clock_prescaler_90_209_211(void) {
	BYTE type;

	if (!m90_209_211.irq.active) {
		return;
	}

	type = m90_209_211.irq.mode >> 6;

	if (type == 1) {
		if ((++m90_209_211.irq.prescaler & m90_209_211.irq.premask) == 0) {
			if (!(m90_209_211.irq.mode & 0x08)) {
				m90_209_211.irq.count++;
			}
			if (m90_209_211.irq.count == 0x00) {
				irq.high |= EXT_IRQ;
			}
		}
	} else if (type == 2) {
		if ((--m90_209_211.irq.prescaler & m90_209_211.irq.premask) == m90_209_211.irq.premask) {
			if (!(m90_209_211.irq.mode & 0x08)) {
				m90_209_211.irq.count--;
			}
			if (m90_209_211.irq.count == 0xFF) {
				irq.high |= EXT_IRQ;
			}
		}
	}
}

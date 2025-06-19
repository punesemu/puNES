/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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
#include "save_slot.h"

enum _m005_modes { MODE0, MODE1, MODE2, MODE3 };
enum _m005_split_sides { SPLIT_LEFT, SPLIT_RIGHT = 0x40 };

INLINE static void prg_fix_005(void);
INLINE static void prg_swap_005(WORD address, WORD value);
INLINE static void chr_fix_005(void);
INLINE static void wram_fix_005(void);
INLINE static void mirroring_fix_005(void);
INLINE static void mirroring_swap_005(WORD address);

INLINE static void chr_s(void);
INLINE static void chr_b(void);

const BYTE filler_attrib[4] = { 0x00, 0x55, 0xAA, 0xFF };

_m005 m005;

void map_init_005(void) {
	EXTCL_AFTER_MAPPER_INIT(005);
	EXTCL_CPU_WR_MEM(005);
	EXTCL_CPU_RD_MEM(005);
	EXTCL_SAVE_MAPPER(005);
	EXTCL_PPU_256_TO_319(005);
	EXTCL_PPU_320_TO_34X(005);
	EXTCL_RD_R2007(005);
	EXTCL_AFTER_RD_CHR(005);
	EXTCL_RD_NMT(005);
	EXTCL_RD_CHR(005);
	EXTCL_CPU_EVERY_CYCLE(005);
	EXTCL_LENGTH_CLOCK(005);
	EXTCL_ENVELOPE_CLOCK(005);
	EXTCL_APU_TICK(005);
	map_internal_struct_init((BYTE *)&m005, sizeof(m005));

	if (info.reset >= HARD) {
		memset(&m005, 0x00, sizeof(m005));
		memset(&nes[0].irql2f, 0x00, sizeof(_irql2f));

		m005.prg_mode = MODE3;
		m005.chr_mode = MODE0;
		m005.ext_mode = MODE0;

		m005.prg[0] = 0xFB;
		m005.prg[1] = 0xFC;
		m005.prg[2] = 0xFD;
		m005.prg[3] = 0xFE;
		m005.prg[4] = 0xFF;

		m005.chr[1] = 1;
		m005.chr[2] = 2;
		m005.chr[3] = 3;
		m005.chr[4] = 4;
		m005.chr[5] = 5;
		m005.chr[6] = 6;
		m005.chr[7] = 7;
		m005.chr[8] = 8;
		m005.chr[9] = 9;
		m005.chr[10] = 10;
		m005.chr[11] = 11;
		m005.chr_last = 0;

		m005.factor[0] = 0xFF;
		m005.factor[1] = 0xFF;

		m005.snd.S3.frequency = 1;
		m005.snd.S4.frequency = 1;

		nes[0].irql2f.scanline = 255;
		nes[0].irql2f.frame_x = 339;
	} else {
		m005.snd.S3.length.enabled = 0;
		m005.snd.S3.length.value = 0;
		m005.snd.S4.length.enabled = 0;
		m005.snd.S4.length.value = 0;
	}

	// Because no ExROM game is known to write PRG-RAM with one bank value and then attempt
	// to read back the same data with a different bank value, emulating the PRG-RAM as 64K at all times
	// can be used as a compatible superset for all games.
	if (info.format == iNES_1_0) {
		if (info.mapper.battery) {
			wram_set_nvram_size(S16K);
			wram_set_ram_size(S48K);
		} else {
			wram_set_ram_size(S64K);
		}
	}

	info.mapper.extend_wr = TRUE;
	nes[0].irql2f.present = TRUE;
}
void map_init_NSF_005(void) {
	memset(&m005, 0x00, sizeof(m005));

	m005.snd.S3.frequency = 1;
	m005.snd.S4.frequency = 1;
	m005.snd.S3.length.enabled = 0;
	m005.snd.S3.length.value = 0;
	m005.snd.S4.length.enabled = 0;
	m005.snd.S4.length.value = 0;
}
void extcl_after_mapper_init_005(void) {
	prg_fix_005();
	chr_fix_005();
	wram_fix_005();
	mirroring_fix_005();
}
void extcl_cpu_wr_mem_005(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		switch (address) {
			case 0x5000:
				square_reg0(m005.snd.S3);
				return;
			case 0x5001:
				// lo sweep non e' utilizzato
				return;
			case 0x5002:
				square_reg2(m005.snd.S3);
				return;
			case 0x5003:
				square_reg3(m005.snd.S3);
				return;
			case 0x5004:
				square_reg0(m005.snd.S4);
				return;
			case 0x5005:
				// lo sweep non e' utilizzato
				return;
			case 0x5006:
				square_reg2(m005.snd.S4);
				return;
			case 0x5007:
				square_reg3(m005.snd.S4);
				return;
			case 0x5010:
				m005.snd.pcm.enabled = ~value & 0x01;
				m005.snd.pcm.output = 0;
				if (m005.snd.pcm.enabled) {
					m005.snd.pcm.output = m005.snd.pcm.amp;
				}
				m005.snd.clocked = TRUE;
				return;
			case 0x5011:
				m005.snd.pcm.amp = value;
				m005.snd.pcm.output = 0;
				if (m005.snd.pcm.enabled) {
					m005.snd.pcm.output = m005.snd.pcm.amp;
				}
				m005.snd.clocked = TRUE;
				return;
			case 0x5015:
				m005.snd.S3.length.enabled = value & 0x01;
				if (!m005.snd.S3.length.enabled) {
					m005.snd.S3.length.value = 0;
				}
				m005.snd.S4.length.enabled = value & 0x02;
				if (!m005.snd.S4.length.enabled) {
					m005.snd.S4.length.value = 0;
				}
				return;
			case 0x5100:
				m005.prg_mode = value & 0x03;
				prg_fix_005();
				return;
			case 0x5101:
				m005.chr_mode = value & 0x03;
				return;
			case 0x5102:
			case 0x5103:
				m005.wram_protect[address & 0x0001] = value & 0x03;
				prg_fix_005();
				wram_fix_005();
				return;
			case 0x5104:
				m005.ext_mode = value & 0x03;
				return;
			case 0x5105:
				m005.nmt = value;
				mirroring_fix_005();
				return;
// --------------------------------- PRG bankswitching ---------------------------------
			case 0x5106:
				m005.fill_tile = value;
				memset(&m005.fill_table[0], m005.fill_tile, 0x3C0);
				return;
			case 0x5107:
				m005.fill_attr = value & 0x03;
				memset(&m005.fill_table[0x3C0], filler_attrib[m005.fill_attr], 0x40);
				return;
			case 0x5113:
			case 0x5114:
			case 0x5115:
			case 0x5116:
			case 0x5117:
				if (!(value & 0x80) && (wram_size() == S16K)) {
					value >>= 2;
				}
				m005.prg[address - 0x5113] = value;
				prg_fix_005();
				wram_fix_005();
				return;
// --------------------------------- CHR bankswitching ---------------------------------
			case 0x5120:
			case 0x5121:
			case 0x5122:
			case 0x5123:
			case 0x5124:
			case 0x5125:
			case 0x5126:
			case 0x5127:
			case 0x5128:
			case 0x5129:
			case 0x512A:
			case 0x512B:
				m005.chr_last = address - 0x5120;
				m005.chr[m005.chr_last] = (m005.chr_high << 2) | value;
				return;
			case 0x5130:
				m005.chr_high = (value & 0x03) << 6;
				return;
			case 0x5200:
				m005.split = value & 0x80;
				m005.split_side = value & 0x40;
				m005.split_st_tile = value & 0x1F;
				return;
			case 0x5201:
				if (value >= 240) {
					value -= 16;
				}
				m005.split_scrl = value;
				return;
			case 0x5202:
				value = chrrom_control_bank(S4K, value);
				m005.split_bank = value << 12;
				return;
			case 0x5203:
				nes[nidx].irql2f.scanline = value;
				return;
			case 0x5204:
				if (value & 0x80) {
					nes[nidx].irql2f.enable = TRUE;
					return;
				}
				nes[nidx].irql2f.enable = FALSE;
				nes[nidx].c.irq.high &= ~EXT_IRQ;
				return;
			case 0x5205:
				m005.factor[0] = value;
				return;
			case 0x5206:
				m005.factor[1] = value;
				return;
			case 0x5209:
				m005.timer_count = (m005.timer_count & 0xFF00) | value;
				m005.timer_running = TRUE;
				return;
			case 0x520A:
				m005.timer_count = (m005.timer_count & 0x00FF) | (value << 8);
				return;
			default:
				if ((address >= 0x5C00) && (address <= 0x5FFF)) {
					m005.ext_ram[address - 0x5C00] = value;
				}
				return;
		}
	}
}
BYTE extcl_cpu_rd_mem_005(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if ((address >= 0x5C00) && (address <= 0x5FFF)) {
		return (m005.ext_ram[address & 0x03FF]);
	} else {
		BYTE value = 0;

		switch (address) {
			case 0x5015:
				// azzero la varibile d'uscita
				value = 0;
				// per ogni canale controllo se il length counter
				// non e' a 0 e se si setto a 1 il bit corrispondente
				// nel byte di ritorno.
				if (m005.snd.S3.length.value) {
					value |= 0x01;
				}
				if (m005.snd.S4.length.value) {
					value |= 0x02;
				}
				return (value);
			case 0x5204:
				value = nes[nidx].irql2f.pending | nes[nidx].irql2f.in_frame;
				nes[nidx].irql2f.pending = FALSE;
				// disabilito l'IRQ del MMC5
				nes[nidx].c.irq.high &= ~EXT_IRQ;
				return (value);
			case 0x5205:
				return ((WORD)(m005.factor[0] * m005.factor[1]) & 0x00FF);
			case 0x5206:
				return ((WORD)(m005.factor[0] * m005.factor[1]) >> 8);
			case 0x5209:
				value = (m005.timer_irq ? 0x80 : 0x00);
				m005.timer_irq = FALSE;
				nes[nidx].c.irq.high &= ~EXT_IRQ;
				return (value);
			default:
				return (wram_rd(nidx, address));
		}
	}
}
BYTE extcl_save_mapper_005(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m005.prg_mode);
	save_slot_ele(mode, slot, m005.chr_mode);
	save_slot_ele(mode, slot, m005.ext_mode);

	save_slot_ele(mode, slot, m005.prg);
	save_slot_ele(mode, slot, m005.chr);
	save_slot_ele(mode, slot, m005.chr_high);
	save_slot_ele(mode, slot, m005.chr_last);
	save_slot_ele(mode, slot, m005.wram_protect);
	save_slot_ele(mode, slot, m005.nmt);

	save_slot_ele(mode, slot, m005.fill_tile);
	save_slot_ele(mode, slot, m005.fill_attr);
	save_slot_ele(mode, slot, m005.split);
	save_slot_ele(mode, slot, m005.split_st_tile);
	save_slot_ele(mode, slot, m005.split_side);
	save_slot_ele(mode, slot, m005.split_scrl);
	save_slot_ele(mode, slot, m005.split_in_reg);
	save_slot_ele(mode, slot, m005.split_x);
	save_slot_ele(mode, slot, m005.split_y);
	save_slot_ele(mode, slot, m005.split_tile);
	save_slot_ele(mode, slot, m005.split_bank);
	save_slot_ele(mode, slot, m005.factor);

	save_slot_ele(mode, slot, m005.timer_running);
	save_slot_ele(mode, slot, m005.timer_irq);
	save_slot_ele(mode, slot, m005.timer_count);

	save_slot_square(m005.snd.S3, slot);
	save_slot_square(m005.snd.S4, slot);

	save_slot_ele(mode, slot, m005.snd.pcm.enabled);
	save_slot_ele(mode, slot, m005.snd.pcm.output);
	save_slot_ele(mode, slot, m005.snd.pcm.amp);

	if (mode == SAVE_SLOT_READ) {
		memset(&m005.fill_table[0], m005.fill_tile, 0x3C0);
		memset(&m005.fill_table[0x3C0], filler_attrib[m005.fill_attr], 0x40);
	}

	return (EXIT_OK);
}
void extcl_ppu_256_to_319_005(BYTE nidx) {
	if (nes[nidx].p.ppu.frame_x != 256) {
		return;
	};

	if ((nes[nidx].p.r2000.size_spr == 8) || nes[nidx].p.r2001.visible) {
		chr_s();
	} else {
		if (m005.chr_last < 8) {
			chr_s();
		} else {
			chr_b();
		}
	}
}
void extcl_ppu_320_to_34x_005(BYTE nidx) {
	irql2f_tick(nidx);

	if (nes[nidx].p.ppu.frame_x != 320) {
		return;
	};

	if (m005.split) {
		m005.split_x = 0x1F;
		if (nes[nidx].p.ppu.screen_y == SCR_ROWS - 1) {
			m005.split_y = m005.split_scrl - 1;
		} else {
			if (m005.split_y < 239) {
				m005.split_y++;
			} else {
				m005.split_y = 0;
			}
		}
	}

	if (nes[nidx].p.r2000.size_spr == 8) {
		chr_s();
	} else if (nes[nidx].p.r2001.visible) {
		chr_b();
	} else  {
		if (m005.chr_last < 8) {
			chr_s();
		} else {
			chr_b();
		}
	}
}
void extcl_rd_r2007_005(BYTE nidx) {
	// When 8x8 sprites are used, the registers from $5120-5127 ("set A") are used for everything
	// (sprites, BG tiles, and reads from $2007 during vblank or forced blank).
	// Registers $5128-512B ("set B") are unused.

	// When 8x16 sprites are used, sprites use the "set A" banks; BG tiles use the "set B" banks;
	// reads from $2007 during vblank or forced blank are treated as sprite accesses
	// (using bank set A) if the most recently written register was $5120-5127; and treated as
	// BG accesses (using bank set B) if the most recently written register was $5128-512B
	if (nes[nidx].p.r2000.size_spr == 8) {
		chr_s();
	} else {
		if (m005.chr_last < 8) {
			chr_s();
		} else {
			chr_b();
		}
	}
}
void extcl_after_rd_chr_005(UNUSED(BYTE nidx), UNUSED(WORD address)) {
	// dopo ogni fetch del high byte del background azzero il flag con cui indico se il tile era
	// nella regione dello split (questo indipendentemente che lo fosse realmente o meno). In questo modo sono
	// sicuro che non rimanga settato quando in realta' non serve.
	m005.split_in_reg = FALSE;
}
BYTE extcl_rd_chr_005(BYTE nidx, WORD address) {
	// se non sto trattando il background esco
	if ((address & 0xFFF7) != nes[nidx].p.ppu.bck_adr) {
		return (chr_rd(nidx, address));
	}
	// sono nella regione di split?
	if (m005.split && m005.split_in_reg) {
		return (chrrom_byte(m005.split_bank | (address & 0x0FFF)));
	}
	// se non sono nella modalita' 1 esco normalmente
	if (m005.ext_mode != MODE1) {
		return (chr_rd(nidx, address));
	}
	{
		WORD value = chrrom_control_bank(S4K, (m005.chr_high | (m005.ext_ram[nes[nidx].p.r2006.value & 0x03FF] & 0x3F)));

		return (chrrom_byte(((value << 12) | (address & 0x0FFF))));
	}
}
BYTE extcl_rd_nmt_005(BYTE nidx, WORD address) {
	if ((memmap_chunk_pnt(nidx, MMPPU(address)) == &m005.ext_ram[0]) && (m005.ext_mode > MODE1)) {
		return (0);
	}
	if (m005.split) {
		WORD adr = (address & 0x03FF);

		// attributi
		if (adr >= 0x03C0) {
			m005.split_x = (m005.split_x + 1) & 0x1F;
			m005.split_in_reg = FALSE;
			if (((m005.split_side == SPLIT_LEFT ) && (m005.split_x <= m005.split_st_tile)) ||
				((m005.split_side == SPLIT_RIGHT) && (m005.split_x >= m005.split_st_tile))) {
				m005.split_tile = ((m005.split_y & 0xF8) << 2) | m005.split_x;
				m005.split_in_reg = TRUE;
				return (filler_attrib[(m005.ext_ram[0x3C0 | ((m005.split_tile >> 4) & 0x38) | ((m005.split_tile >> 2) & 0x07)] >>
					(((m005.split_tile >> 4) & 0x04) | (m005.split_tile & 0x2))) & 0x03]);
			}
		}
		// tile
		if (m005.split_in_reg) {
			return (m005.ext_ram[m005.split_tile]);
		}
	}
	if (m005.ext_mode != MODE1) {
		return (nmt_rd(nidx, address));
	}
	if ((address & 0x03FF) >= 0x03C0) {
		BYTE shift = (((nes[nidx].p.r2006.value & 0x40) >> 4) | (nes[nidx].p.r2006.value & 0x02));

		return (((m005.ext_ram[nes[nidx].p.r2006.value & 0x03FF] & 0xC0) >> 6) << shift);
	}
	return (nmt_rd(nidx, address));
}
void extcl_cpu_every_cycle_005(BYTE nidx) {
	if (nes[nidx].irql2f.delay && !(--nes[nidx].irql2f.delay)) {
		nes[nidx].c.irq.high |= EXT_IRQ;
	}
	if (m005.timer_running && !--m005.timer_count) {
		m005.timer_irq = TRUE;
		nes[nidx].c.irq.high |= EXT_IRQ;
	}
}
void extcl_length_clock_005(void) {
	length_run(m005.snd.S3)
	length_run(m005.snd.S4)
}
void extcl_envelope_clock_005(void) {
	envelope_run(m005.snd.S3)
	envelope_run(m005.snd.S4)
}
void extcl_apu_tick_005(void) {
	square_tick(m005.snd.S3, 0, m005.snd)
	square_tick(m005.snd.S4, 0, m005.snd)
}

INLINE static void prg_fix_005(void) {
	switch (m005.prg_mode) {
		case MODE0:
			memmap_auto_32k(0, MMCPU(0x8000), (m005.prg[4] >> 2));
			break;
		case MODE1:
			prg_swap_005(0x8000, m005.prg[2] & ~1);
			prg_swap_005(0xA000, m005.prg[2] |  1);
			memmap_auto_16k(0, MMCPU(0xC000), (m005.prg[4] >> 1));
			break;
		case MODE2:
			prg_swap_005(0x8000, m005.prg[2] & ~1);
			prg_swap_005(0xA000, m005.prg[2] |  1);
			prg_swap_005(0xC000, m005.prg[3]);
			memmap_auto_8k(0, MMCPU(0xE000), m005.prg[4]);
			break;
		case MODE3:
			prg_swap_005(0x8000, m005.prg[1]);
			prg_swap_005(0xA000, m005.prg[2]);
			prg_swap_005(0xC000, m005.prg[3]);
			memmap_auto_8k(0, MMCPU(0xE000), m005.prg[4]);
			break;
	}
}
INLINE static void prg_swap_005(WORD address, WORD value) {
	if (value & 0x80) {
		memmap_auto_8k(0, MMCPU(address), value);
	} else {
		BYTE enable = (m005.wram_protect[0] == 0x02) && (m005.wram_protect[1] == 0x01);

		memmap_wram_wp_8k(0, MMCPU(address), value, TRUE, enable);
	}
}
INLINE static void chr_fix_005(void) {
	chr_s();
}
INLINE static void wram_fix_005(void) {
	BYTE enable = (m005.wram_protect[0] == 0x02) && (m005.wram_protect[1] == 0x01);

	memmap_auto_wp_8k(0, MMCPU(0x6000), m005.prg[0], TRUE, enable);
}
INLINE static void mirroring_fix_005(void) {
	mirroring_swap_005(0x2000);
	mirroring_swap_005(0x2400);
	mirroring_swap_005(0x2800);
	mirroring_swap_005(0x2C00);
}
INLINE static void mirroring_swap_005(WORD address) {
	BYTE bits = (address & 0x0E00) >> 9;
	BYTE mode = (m005.nmt >> bits) & 0x03;

	switch (mode) {
		case MODE0:
			memmap_auto_1k(0, MMPPU(address), 0);
			memmap_auto_1k(0, MMPPU(address | 0x1000), 0);
			return;
		case MODE1:
			memmap_auto_1k(0, MMPPU(address), 1);
			memmap_auto_1k(0, MMPPU(address | 0x1000), 1);
			return;
		case MODE2:
			memmap_other_1k(0, MMPPU(address), 0, &m005.ext_ram[0], S1K, TRUE, TRUE);
			memmap_other_1k(0, MMPPU(address | 0x1000), 0, &m005.ext_ram[0], S1K, TRUE, TRUE);
			return;
		case MODE3:
			memmap_other_1k(0, MMPPU(address), 0, &m005.fill_table[0], S1K, TRUE, TRUE);
			memmap_other_1k(0, MMPPU(address | 0x1000), 0, &m005.fill_table[0], S1K, TRUE, TRUE);
			return;
	}
}

INLINE static void chr_s(void) {
	switch (m005.chr_mode) {
		case MODE0:
			memmap_auto_8k(0, MMPPU(0x0000), m005.chr[7]);
			return;
		case MODE1:
			memmap_auto_4k(0, MMPPU(0x0000), m005.chr[3]);
			memmap_auto_4k(0, MMPPU(0x1000), m005.chr[7]);
			return;
		case MODE2:
			memmap_auto_2k(0, MMPPU(0x0000), m005.chr[1]);
			memmap_auto_2k(0, MMPPU(0x0800), m005.chr[3]);
			memmap_auto_2k(0, MMPPU(0x1000), m005.chr[5]);
			memmap_auto_2k(0, MMPPU(0x1800), m005.chr[7]);
			return;
		case MODE3:
			memmap_auto_1k(0, MMPPU(0x0000), m005.chr[0]);
			memmap_auto_1k(0, MMPPU(0x0400), m005.chr[1]);
			memmap_auto_1k(0, MMPPU(0x0800), m005.chr[2]);
			memmap_auto_1k(0, MMPPU(0x0C00), m005.chr[3]);
			memmap_auto_1k(0, MMPPU(0x1000), m005.chr[4]);
			memmap_auto_1k(0, MMPPU(0x1400), m005.chr[5]);
			memmap_auto_1k(0, MMPPU(0x1800), m005.chr[6]);
			memmap_auto_1k(0, MMPPU(0x1C00), m005.chr[7]);
			return;
	}
}
INLINE static void chr_b(void) {
	switch (m005.chr_mode) {
		case MODE0:
			memmap_auto_8k(0, MMPPU(0x0000), m005.chr[11]);
			return;
		case MODE1:
			memmap_auto_4k(0, MMPPU(0x0000), m005.chr[11]);
			memmap_auto_4k(0, MMPPU(0x1000), m005.chr[11]);
			return;
		case MODE2:
			memmap_auto_2k(0, MMPPU(0x0000), m005.chr[9]);
			memmap_auto_2k(0, MMPPU(0x0800), m005.chr[11]);
			memmap_auto_2k(0, MMPPU(0x1000), m005.chr[9]);
			memmap_auto_2k(0, MMPPU(0x1800), m005.chr[11]);
			return;
		case MODE3:
			memmap_auto_1k(0, MMPPU(0x0000), m005.chr[8]);
			memmap_auto_1k(0, MMPPU(0x0400), m005.chr[9]);
			memmap_auto_1k(0, MMPPU(0x0800), m005.chr[10]);
			memmap_auto_1k(0, MMPPU(0x0C00), m005.chr[11]);
			memmap_auto_1k(0, MMPPU(0x1000), m005.chr[8]);
			memmap_auto_1k(0, MMPPU(0x1400), m005.chr[9]);
			memmap_auto_1k(0, MMPPU(0x1800), m005.chr[10]);
			memmap_auto_1k(0, MMPPU(0x1C00), m005.chr[11]);
			return;
	}
}

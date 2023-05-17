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
#include "cpu.h"
#include "save_slot.h"
#include "emu.h"
#include "tas.h"
#include "gui.h"

INLINE static void prg_fix_019(void);
INLINE static void chr_fix_019(void);
INLINE static void chr_swap_019(WORD address, WORD value, BYTE force_chrom);
INLINE static void wram_fix_019(void);
INLINE static void mirroring_fix_019(void);
INLINE static void nmt_swap_019(BYTE slot, WORD value);
INLINE static void snd_set_volume_019(void);
INLINE static SWORD snd_wave_019(int cycles, int channel_offset);

_m019 m019;
struct _m019tmp {
	BYTE ram[0x80];
	BYTE chr_bank_writable[8];
	SWORD volume;
} m019tmp;

void map_init_019(void) {
	EXTCL_AFTER_MAPPER_INIT(019);
	EXTCL_CPU_WR_MEM(019);
	EXTCL_CPU_RD_MEM(019);
	EXTCL_SAVE_MAPPER(019);
	EXTCL_WR_CHR(019);
	EXTCL_CPU_EVERY_CYCLE(019);
	EXTCL_BATTERY_IO(019);
	EXTCL_APU_TICK(019);
	mapper.internal_struct[0] = (BYTE *)&m019;
	mapper.internal_struct_size[0] = sizeof(m019);

	snd_set_volume_019();

	if (info.reset >= HARD) {
		m019.prg[0] = 0xFC;
		m019.prg[1] = 0xFD;
		m019.prg[2] = 0xFE;
		m019.prg[3] = 0xFF;

		m019.chr[0] = 0;
		m019.chr[1] = 1;
		m019.chr[2] = 2;
		m019.chr[3] = 3;
		m019.chr[4] = 4;
		m019.chr[5] = 5;
		m019.chr[6] = 6;
		m019.chr[7] = 7;

		m019.nmt[0] = 0xE0;
		m019.nmt[1] = 0xE1;
		m019.nmt[2] = 0xE0;
		m019.nmt[3] = 0xE1;

		m019.wram_protect = 0xFF;
		m019.irq.delay = 0;
		m019.irq.count = 0;

		m019.snd.enabled = 0;
		m019.snd.adr = 0;
		m019.snd.auto_inc = 1;
		m019.snd.tick = 0;
		m019.snd.channel = 0;
		m019.snd.channel_start = 0;
		memset(&m019.snd.output, 0x00, sizeof(m019.snd.output));

		emu_initial_ram(m019tmp.ram, sizeof(m019tmp.ram));
	} else {
		m019.irq.delay = 0;
	}

	info.mapper.extend_wr = TRUE;
}
void map_init_NSF_N163(void) {
	memset(&m019, 0x00, sizeof(m019));
	memset(&m019tmp.ram, 0x00, sizeof(m019tmp.ram));

	snd_set_volume_019();

	m019.snd.enabled = 1;
	m019.snd.auto_inc = 1;
}
void extcl_after_mapper_init_019(void) {
	prg_fix_019();
	chr_fix_019();
	wram_fix_019();
	mirroring_fix_019();
}
void extcl_cpu_wr_mem_019(WORD address, BYTE value) {
	switch (address & 0xF800) {
		case 0x4800:
			m019tmp.ram[m019.snd.adr] = value;
			m019.snd.adr = (m019.snd.adr + m019.snd.auto_inc) & 0x07F;
			return;
		case 0x5000:
			m019.irq.count = (m019.irq.count & 0xFF00) | value;
			irq.high &= ~EXT_IRQ;
			return;
		case 0x5800:
			m019.irq.count = (value << 8) | (m019.irq.count & 0x00FF);
			irq.high &= ~EXT_IRQ;
			return;
		case 0x8000:
		case 0x8800:
		case 0x9000:
		case 0x9800:
		case 0xA000:
		case 0xA800:
		case 0xB000:
		case 0xB800:
			m019.chr[(address >> 11) & 0x07] = value;
			chr_fix_019();
			return;
		case 0xC000:
		case 0xC800:
		case 0xD000:
		case 0xD800:
			m019.nmt[(address >> 11) & 0x03] = value;
			mirroring_fix_019();
			return;
		case 0xE000:
			m019.snd.enabled = !(value & 0x40);
			m019.prg[0] = value;
			prg_fix_019();
			return;
		case 0xE800:
		case 0xF000:
			m019.prg[(address >> 11) & 0x03] = value;
			prg_fix_019();
			chr_fix_019();
			return;
		case 0xF800:
			m019.wram_protect = value;
			m019.snd.auto_inc = (value & 0x80) >> 7;
			m019.snd.adr = value & 0x7F;
			wram_fix_019();
			return;
		default:
			return;
	}
}
BYTE extcl_cpu_rd_mem_019(WORD address, BYTE openbus) {
	switch (address & 0xF800) {
		case 0x4800:
			openbus = m019tmp.ram[m019.snd.adr];
			m019.snd.adr = (m019.snd.adr + m019.snd.auto_inc) & 0x07F;
			return (openbus);
		case 0x5000:
			return (m019.irq.count & 0xFF);
		case 0x5800:
			return ((m019.irq.count >> 8) & 0xFF);
		default:
			return (openbus);
	}
}
BYTE extcl_save_mapper_019(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m019.prg);
	save_slot_ele(mode, slot, m019.chr);
	save_slot_ele(mode, slot, m019.nmt);
	save_slot_ele(mode, slot, m019.wram_protect);
	save_slot_ele(mode, slot, m019.snd.enabled);
	save_slot_ele(mode, slot, m019.snd.adr);
	save_slot_ele(mode, slot, m019.snd.auto_inc);
	save_slot_ele(mode, slot, m019.snd.tick);
	save_slot_ele(mode, slot, m019.snd.channel);
	save_slot_ele(mode, slot, m019.snd.channel_start);
	save_slot_ele(mode, slot, m019.snd.output);
	save_slot_ele(mode, slot, m019.irq.delay);
	save_slot_ele(mode, slot, m019.irq.count);

	if (mode == SAVE_SLOT_READ) {
		chr_fix_019();
		mirroring_fix_019();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_019(void) {
	if (m019.irq.delay) {
		m019.irq.delay = FALSE;
		irq.high |= EXT_IRQ;
	}
	if (((m019.irq.count - 0x8000) < 0x7FFF) && (++m019.irq.count == 0xFFFF)) {
		// vale sempre il solito discorso di un ciclo di delay
		m019.irq.delay = TRUE;
	}
}
void extcl_wr_chr_019(WORD address, BYTE value) {
	const BYTE slot = address >> 10;

	if (m019tmp.chr_bank_writable[slot]) {
		chr.bank_1k[slot][address & 0x3FF] = value;
	}
}
void extcl_apu_tick_019(void) {
	if (m019.snd.enabled) {
		m019.snd.channel_start = (~m019tmp.ram[0x7F] >> 4) & 0x07;

		if (m019.snd.channel > 7) {
			m019.snd.channel = m019.snd.channel_start;
		}

		m019.snd.output[m019.snd.channel] = (SWORD)((snd_wave_019(m019.snd.tick ? 0 : 1, m019.snd.channel * 8 + 0x40) * m019tmp.volume) / 40);
		if (++m019.snd.tick >= 15) {
			m019.snd.tick = 0;
			m019.snd.channel++;
		}
	}
}
void extcl_battery_io_019(BYTE mode, FILE *fp) {
	if (wram.battery.in_use) {
		if (mode == WR_BAT) {
			if (fwrite(m019tmp.ram, sizeof(m019tmp.ram), 1, fp) < 1) {
				log_error(uL("mapper_019;error on write 128 bytes ram"));
			}
		} else {
			if (fread(m019tmp.ram, sizeof(m019tmp.ram), 1, fp) < 1) {
				log_error(uL("mapper_019;error on read 128 bytes ram"));
			}
		}
	}
}

INLINE static void prg_fix_019(void) {
	WORD mask = 0x3F;
	WORD bank = 0;

	bank = m019.prg[0] & mask;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0, bank);

	bank = m019.prg[1] & mask;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, bank);

	bank = m019.prg[2] & mask;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2, bank);

	bank = m019.prg[3] & mask;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, bank);

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_019(void) {
	BYTE force_chrom = 0;

	force_chrom = (m019.prg[1] & 0x40) ? TRUE : FALSE;
	chr_swap_019(0x0000, m019.chr[0], force_chrom);
	chr_swap_019(0x0400, m019.chr[1], force_chrom);
	chr_swap_019(0x0800, m019.chr[2], force_chrom);
	chr_swap_019(0x0C00, m019.chr[3], force_chrom);

	force_chrom = (m019.prg[1] & 0x80) ? TRUE : FALSE;
	chr_swap_019(0x1000, m019.chr[4], force_chrom);
	chr_swap_019(0x1400, m019.chr[5], force_chrom);
	chr_swap_019(0x1800, m019.chr[6], force_chrom);
	chr_swap_019(0x1C00, m019.chr[7], force_chrom);
}
INLINE static void chr_swap_019(WORD address, WORD value, BYTE force_chrom) {
	const BYTE slot = (address >> 10) & 0x07;

	if ((value < 0xE0) || force_chrom) {
		map_chr_rom_1k(address, value);
		m019tmp.chr_bank_writable[slot] = mapper.write_vram;
	} else {
		value &= (mapper.mirroring == MIRRORING_FOURSCR ? 0x03 : 0x01);
		chr.bank_1k[slot] = &ntbl.data[value << 10];
		m019tmp.chr_bank_writable[slot] = TRUE;
	}
}
INLINE static void wram_fix_019(void) {
	memmap_auto_wp_2k(0x6000, 0, TRUE, ((m019.wram_protect & (0xF0 | (0x01 << 0))) == 0x40));
	memmap_auto_wp_2k(0x6800, 1, TRUE, ((m019.wram_protect & (0xF0 | (0x01 << 1))) == 0x40));
	memmap_auto_wp_2k(0x7000, 2, TRUE, ((m019.wram_protect & (0xF0 | (0x01 << 2))) == 0x40));
	memmap_auto_wp_2k(0x7800, 3, TRUE, ((m019.wram_protect & (0xF0 | (0x01 << 3))) == 0x40));
}
INLINE static void mirroring_fix_019(void) {
	nmt_swap_019(0, m019.nmt[0]);
	nmt_swap_019(1, m019.nmt[1]);
	nmt_swap_019(2, m019.nmt[2]);
	nmt_swap_019(3, m019.nmt[3]);
}
INLINE static void nmt_swap_019(BYTE slot, WORD value) {
	if (value < 0xE0) {
		map_nmt_chr_rom_1k(slot, value);
	} else {
		value &= (mapper.mirroring == MIRRORING_FOURSCR ? 0x03 : 0x01);
		map_nmt_1k(slot, value);
	}
}
INLINE static void snd_set_volume_019(void) {
	const SWORD volume[6] = { 0x22, 0x22,  0, 0x22, 0x3B, 0x44 };

	m019tmp.volume = volume[info.mapper.submapper >= 6 ? 0 : info.mapper.submapper];
}
INLINE static SWORD snd_wave_019(int cycles, int channel_offset) {
	int phase =
		(m019tmp.ram[channel_offset + 5] << 16) |
		(m019tmp.ram[channel_offset + 3] << 8) |
		m019tmp.ram[channel_offset + 1];
	int freq =
		((m019tmp.ram[channel_offset + 4] & 0x03) << 16) |
		(m019tmp.ram[channel_offset + 2] << 8) |
		(m019tmp.ram[channel_offset]);
	int length = 256 - (m019tmp.ram[channel_offset + 4] & 0xFC);
	int offset = m019tmp.ram[channel_offset + 6];
	int volume = m019tmp.ram[channel_offset + 7] & 0x0F;
	int sample = 0, output = 0;

	while (cycles--) {
		phase = (phase + freq) % (length << 16);
	}

	sample = ((phase >> 16) + offset) & 0xFF;
	output = (m019tmp.ram[sample >> 1] >> ((sample & 0x01) << 2)) & 0x0F;

	m019tmp.ram[channel_offset + 5] = (phase >> 16) & 0xFF;
	m019tmp.ram[channel_offset + 3] = (phase >> 8) & 0xFF;
	m019tmp.ram[channel_offset + 1] = (phase >> 0) & 0xFF;

	return (SWORD)(((output - 8) * volume));
}

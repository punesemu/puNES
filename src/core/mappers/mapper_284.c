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
#include "cpu.h"
#include "ppu.h"
#include "save_slot.h"

INLINE static void prg_fix_284(void);
INLINE static void chr_fix_284(void);
INLINE static void wram_fix_284(void);
INLINE static void mirroring_fix_284(void);

INLINE static void channel_output(_m284_channel *channel, SWORD output);
INLINE static void channel_reset(_m284_channel *channel);
INLINE static void channel_tick(_m284_channel *channel);
INLINE static BYTE channel_status(_m284_channel *channel);
INLINE static void channel_reg(_m284_channel *channel, BYTE address, BYTE value);

_m284 m284;

void map_init_284(void) {
	EXTCL_AFTER_MAPPER_INIT(284);
	EXTCL_CPU_WR_MEM(284);
	EXTCL_CPU_RD_MEM(284);
	EXTCL_SAVE_MAPPER(284);
	EXTCL_RD_NMT(284);
	EXTCL_CPU_EVERY_CYCLE(284);
	mapper.internal_struct[0] = (BYTE *)&m284;
	mapper.internal_struct_size[0] = sizeof(m284);

	m284.control = 0;
	m284.prg = 0;
	m284.chr[0] = 0;
	m284.chr[1] = 1;
	m284.chr[2] = 2;
	m284.chr[3] = 3;
	memset(&m284.extended_attributes[0][0], 0x00, sizeof(m284.extended_attributes));
	m284.channel[0].freq = 0;
	m284.channel[1].freq = 0;
	channel_reset(&m284.channel[0]);
	channel_reset(&m284.channel[1]);

	if (info.reset == RESET) {
		m284.jumper = !m284.jumper ? 0x80 : 0;
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		m284.jumper = 0;
	}
}
void extcl_after_mapper_init_284(void) {
	prg_fix_284();
	chr_fix_284();
	wram_fix_284();
	mirroring_fix_284();
}
void extcl_cpu_wr_mem_284(WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x8000:
		case 0xA000:
			switch (address & 0x000F) {
				case 0x0:
				case 0x1:
				case 0x2:
				case 0x3:
					channel_reg(&m284.channel[0], address & 0x0003, value);
					break;
				case 0x4:
				case 0x5:
				case 0x6:
				case 0x7:
					channel_reg(&m284.channel[1], address & 0x0003, value);
					break;
				case 0x8:
					m284.irq.latch = value;
					break;
				case 0x9:
					m284.irq.counter = ((value & 0x7F) << 8) | m284.irq.latch;
					m284.irq.enabled = value & 0x80;
					nes.c.irq.high &= ~EXT_IRQ;
					break;
				case 0xA:
					m284.control = value & 0x0F;
					wram_fix_284();
					mirroring_fix_284();
					break;
				case 0xB:
					m284.prg = value & 0x0F;
					prg_fix_284();
					break;
				case 0xC:
				case 0xD:
				case 0xE:
				case 0xF:
					m284.chr[address & 0x0003] = value & 0x0F;
					chr_fix_284();
					break;
			}
			break;
		case 0xC000:
		case 0xE000:
			m284.extended_attributes[(address & 0x0400) >> 10][address & 0x3FF] = value & 0x03;
			break;
	}
}
BYTE extcl_cpu_rd_mem_284(WORD address, UNUSED(BYTE openbus)) {
	switch (address & 0xF000) {
		case 0x4000:
			return (address & 0x0800 ? m284.jumper | 'd' : wram_rd(address));
		case 0x5000:
			return (address & 0x0800 ? channel_status(&m284.channel[1]) : channel_status(&m284.channel[0]));
		default:
			return (wram_rd(address));
	}
}
BYTE extcl_save_mapper_284(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m284.jumper);
	save_slot_ele(mode, slot, m284.control);
	save_slot_ele(mode, slot, m284.prg);
	save_slot_ele(mode, slot, m284.chr);
	save_slot_ele(mode, slot, m284.extended_attributes);
	save_slot_ele(mode, slot, m284.irq.enabled);
	save_slot_ele(mode, slot, m284.irq.latch);
	save_slot_ele(mode, slot, m284.irq.counter);
	save_slot_ele(mode, slot, m284.channel[0].out);
	save_slot_ele(mode, slot, m284.channel[0].fifo);
	save_slot_ele(mode, slot, m284.channel[0].full);
	save_slot_ele(mode, slot, m284.channel[0].empty);
	save_slot_ele(mode, slot, m284.channel[0].freq);
	save_slot_ele(mode, slot, m284.channel[0].vol);
	save_slot_ele(mode, slot, m284.channel[0].timer);
	save_slot_ele(mode, slot, m284.channel[0].pos.read);
	save_slot_ele(mode, slot, m284.channel[0].pos.write);
	save_slot_ele(mode, slot, m284.channel[1].out);
	save_slot_ele(mode, slot, m284.channel[1].fifo);
	save_slot_ele(mode, slot, m284.channel[1].full);
	save_slot_ele(mode, slot, m284.channel[1].empty);
	save_slot_ele(mode, slot, m284.channel[1].freq);
	save_slot_ele(mode, slot, m284.channel[1].vol);
	save_slot_ele(mode, slot, m284.channel[1].timer);
	save_slot_ele(mode, slot, m284.channel[1].pos.read);
	save_slot_ele(mode, slot, m284.channel[1].pos.write);

	return (EXIT_OK);
}
BYTE extcl_rd_nmt_284(WORD address) {
	if ((m284.control & 0x04) && ((address & 0x3FF) >= 0x3C0)) {
		const BYTE ext_attrib[4] = { 0x00, 0x55, 0xAA, 0xFF };
		BYTE bank = 0;

		address &= 0x0FFF;

		switch (mapper.mirroring) {
			default:
			case MIRRORING_SINGLE_SCR0:
				bank = 0;
				break;
			case MIRRORING_SINGLE_SCR1:
				bank = 1;
				break;
			case MIRRORING_HORIZONTAL:
				bank = (address & 0x800) ? 1 : 0;
				break;
			case MIRRORING_VERTICAL:
				bank = (address & 0x400) ? 1 : 0;
				break;
		}
		return (ext_attrib[m284.extended_attributes[bank][nes.p.r2006.value & 0x3FF]]);
	}
	return (nmt_rd(address));
}
void extcl_cpu_every_cycle_284(void) {
	if (m284.irq.enabled) {
		if (m284.irq.counter > 0) {
			m284.irq.counter--;
			if (!m284.irq.counter) {
				m284.irq.enabled = FALSE;
				nes.c.irq.high |= EXT_IRQ;
			}
		}
	}
	channel_tick(&m284.channel[0]);
	channel_tick(&m284.channel[1]);
}

INLINE static void prg_fix_284(void) {
	memmap_auto_16k(MMCPU(0x8000), m284.prg);
	memmap_auto_16k(MMCPU(0xC000), 0x0F);
}
INLINE static void chr_fix_284(void) {
	memmap_auto_2k(MMPPU(0x0000), m284.chr[0]);
	memmap_auto_2k(MMPPU(0x0800), m284.chr[1]);
	memmap_auto_2k(MMPPU(0x1000), m284.chr[2]);
	memmap_auto_2k(MMPPU(0x1800), m284.chr[3]);
}
INLINE static void wram_fix_284(void) {
	BYTE enabled = (m284.control & 0x08) >> 3;

	memmap_auto_wp_8k(MMCPU(0x6000), 0, enabled, enabled);
}
INLINE static void mirroring_fix_284(void) {
	switch (m284.control & 0x03) {
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

INLINE static void channel_tick(_m284_channel *channel) {
	if (channel->empty) {
		return;
	}
	if (!channel->timer--) {
		channel->timer = channel->freq;
		if (channel->pos.read == channel->pos.write) {
			channel->full = FALSE;
		}
		channel_output(channel, (SWORD)((channel->fifo[++channel->pos.read] - 0x80) * channel->vol));
		if (channel->pos.read == channel->pos.write) {
			channel->empty = TRUE;
		}
	}
}
INLINE static void channel_output(_m284_channel *channel, SWORD output) {
	channel->out = (SWORD)(output / 10);
}
INLINE static void channel_reset(_m284_channel *channel) {
	memset(channel->fifo, 0x00, 256);
	channel->pos.read = channel->pos.write = 0;
	channel->full = FALSE;
	channel->empty = TRUE;
	channel->timer = channel->freq;
	channel_output(channel, 0);
}
INLINE static BYTE channel_status(_m284_channel *channel) {
	int result = 0;

	if (channel->full) {
		result |= 0x80;
	}
	if (channel->empty) {
		result |= 0x40;
	}
	return (result);
}
INLINE static void channel_reg(_m284_channel *channel, BYTE address, BYTE value) {
	switch (address) {
		case 0:
			channel_reset(channel);
			return;
		case 1:
			if (channel->pos.read == channel->pos.write) {
				channel->empty = FALSE;
				channel_output(channel, (SWORD)((value - 0x80) * channel->vol));
				channel->timer = channel->freq;
			}
			channel->fifo[channel->pos.write++] = value;
			if (channel->pos.read == channel->pos.write) {
				channel->full = TRUE;
			}
			return;
		case 2:
			channel->freq = (channel->freq & 0x0F00) | value;
			return;
		case 3:
			channel->freq = (channel->freq & 0x00FF) | ((value & 0x0F) << 8);
			channel->vol = (value & 0xF0) >> 4;
			if (!channel->empty) {
				channel_output(channel, (SWORD)((channel->fifo[channel->pos.read] - 0x80) * channel->vol));
			}
			return;
		default:
			return;
	}
}

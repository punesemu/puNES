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
#include "mem_map.h"
#include "cpu.h"
#include "ppu.h"
#include "save_slot.h"

INLINE static void prg_fix_DRIPGAME(void);
INLINE static void chr_fix_DRIPGAME(void);
INLINE static void mirroring_fix_DRIPGAME(void);
INLINE static void channel_output(_dripgame_channel *channel, SWORD output);
INLINE static void channel_reset(_dripgame_channel *channel);
INLINE static void channel_tick(_dripgame_channel *channel);
INLINE static BYTE channel_status(_dripgame_channel *channel);
INLINE static void channel_reg(_dripgame_channel *channel, BYTE address, BYTE value);

const BYTE dripgame_ext_attrib[4] = { 0x00, 0x55, 0xAA, 0xFF };
_dripgame dripgame;

void map_init_DRIPGAME(void) {
	EXTCL_AFTER_MAPPER_INIT(DRIPGAME);
	EXTCL_CPU_WR_MEM(DRIPGAME);
	EXTCL_CPU_RD_MEM(DRIPGAME);
	EXTCL_SAVE_MAPPER(DRIPGAME);
	EXTCL_RD_NMT(DRIPGAME);
	EXTCL_CPU_EVERY_CYCLE(DRIPGAME);
	mapper.internal_struct[0] = (BYTE *)&dripgame;
	mapper.internal_struct_size[0] = sizeof(dripgame);

	dripgame.control = 0;
	dripgame.prg = 0;
	dripgame.chr[0] = 0;
	dripgame.chr[1] = 1;
	dripgame.chr[2] = 2;
	dripgame.chr[3] = 3;
	memset(&dripgame.extended_attributes[0][0], 0x00, sizeof(dripgame.extended_attributes));
	dripgame.channel[0].freq = 0;
	dripgame.channel[1].freq = 0;
	channel_reset(&dripgame.channel[0]);
	channel_reset(&dripgame.channel[1]);

	if (info.reset == RESET) {
		dripgame.dipswitch = !dripgame.dipswitch ? 0x80 : 0;
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		dripgame.dipswitch = 0;
	}
}
void extcl_after_mapper_init_DRIPGAME(void) {
	prg_fix_DRIPGAME();
	chr_fix_DRIPGAME();
	mirroring_fix_DRIPGAME();
}
void extcl_cpu_wr_mem_DRIPGAME(WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x8000:
		case 0xA000:
			switch (address & 0x000F) {
				case 0x0:
				case 0x1:
				case 0x2:
				case 0x3:
					channel_reg(&dripgame.channel[0], address & 0x0003, value);
					break;
				case 0x4:
				case 0x5:
				case 0x6:
				case 0x7:
					channel_reg(&dripgame.channel[1], address & 0x0003, value);
					break;
				case 0x8:
					dripgame.irq.latch = value;
					break;
				case 0x9:
					dripgame.irq.counter = ((value & 0x7F) << 8) | dripgame.irq.latch;
					dripgame.irq.enabled = value & 0x80;
					irq.high &= ~EXT_IRQ;
					break;
				case 0xA:
					dripgame.control = value & 0x0F;
					cpu.prg_ram_wr_active = (value & 0x08) >> 3;
					mirroring_fix_DRIPGAME();
					break;
				case 0xB:
					dripgame.prg = value & 0x0F;
					prg_fix_DRIPGAME();
					break;
				case 0xC:
				case 0xD:
				case 0xE:
				case 0xF:
					dripgame.chr[address & 0x0003] = value & 0x0F;
					chr_fix_DRIPGAME();
					break;
			}
			break;
		case 0xC000:
		case 0xE000:
			dripgame.extended_attributes[(address & 0x0400) >> 10][address & 0x3FF] = value & 0x03;
			break;
	}
}
BYTE extcl_cpu_rd_mem_DRIPGAME(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xF000) {
		case 0x4000:
			return (address & 0x0800 ? dripgame.dipswitch | 'd' : openbus);
		case 0x5000:
			return (address & 0x0800 ? channel_status(&dripgame.channel[1]) : channel_status(&dripgame.channel[0]));
	}
	return (openbus);
}
BYTE extcl_save_mapper_DRIPGAME(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, dripgame.dipswitch);
	save_slot_ele(mode, slot, dripgame.control);
	save_slot_ele(mode, slot, dripgame.prg);
	save_slot_ele(mode, slot, dripgame.chr);
	save_slot_ele(mode, slot, dripgame.extended_attributes);
	save_slot_ele(mode, slot, dripgame.irq.enabled);
	save_slot_ele(mode, slot, dripgame.irq.latch);
	save_slot_ele(mode, slot, dripgame.irq.counter);
	save_slot_ele(mode, slot, dripgame.channel[0].out);
	save_slot_ele(mode, slot, dripgame.channel[0].fifo);
	save_slot_ele(mode, slot, dripgame.channel[0].full);
	save_slot_ele(mode, slot, dripgame.channel[0].empty);
	save_slot_ele(mode, slot, dripgame.channel[0].freq);
	save_slot_ele(mode, slot, dripgame.channel[0].vol);
	save_slot_ele(mode, slot, dripgame.channel[0].timer);
	save_slot_ele(mode, slot, dripgame.channel[0].pos.read);
	save_slot_ele(mode, slot, dripgame.channel[0].pos.write);
	save_slot_ele(mode, slot, dripgame.channel[1].out);
	save_slot_ele(mode, slot, dripgame.channel[1].fifo);
	save_slot_ele(mode, slot, dripgame.channel[1].full);
	save_slot_ele(mode, slot, dripgame.channel[1].empty);
	save_slot_ele(mode, slot, dripgame.channel[1].freq);
	save_slot_ele(mode, slot, dripgame.channel[1].vol);
	save_slot_ele(mode, slot, dripgame.channel[1].timer);
	save_slot_ele(mode, slot, dripgame.channel[1].pos.read);
	save_slot_ele(mode, slot, dripgame.channel[1].pos.write);

	return (EXIT_OK);
}
BYTE extcl_rd_nmt_DRIPGAME(WORD address) {
	address &= 0x0FFF;

	if ((dripgame.control & 0x04) && ((address & 0x3FF) >= 0x3C0)) {
		BYTE bank;

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
		return (dripgame_ext_attrib[dripgame.extended_attributes[bank][r2006.value & 0x3FF]]);
	}
	return (ntbl.bank_1k[address >> 10][address & 0x3FF]);
}
void extcl_cpu_every_cycle_DRIPGAME(void) {
	if (dripgame.irq.enabled) {
		if (dripgame.irq.counter > 0) {
			dripgame.irq.counter--;
			if (!dripgame.irq.counter) {
				dripgame.irq.enabled = FALSE;
				irq.high |= EXT_IRQ;
			}
		}
	}
	channel_tick(&dripgame.channel[0]);
	channel_tick(&dripgame.channel[1]);
}

INLINE static void prg_fix_DRIPGAME(void) {
	BYTE value;

	value = dripgame.prg;
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);

	value = ~0;
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, value);

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_DRIPGAME(void) {
	DBWORD bank;

	bank = dripgame.chr[0];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);

	bank = dripgame.chr[1];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[2] = chr_pnt(bank);
	chr.bank_1k[3] = chr_pnt(bank | 0x0400);

	bank = dripgame.chr[2];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[4] = chr_pnt(bank);
	chr.bank_1k[5] = chr_pnt(bank | 0x0400);

	bank = dripgame.chr[3];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[6] = chr_pnt(bank);
	chr.bank_1k[7] = chr_pnt(bank | 0x0400);
}
INLINE static void mirroring_fix_DRIPGAME(void) {
	switch (dripgame.control & 0x03) {
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

INLINE static void channel_tick(_dripgame_channel *channel) {
	if (channel->empty) {
		return;
	}
	if (!channel->timer--) {
		channel->timer = channel->freq;
		if (channel->pos.read == channel->pos.write) {
			channel->full = FALSE;
		}
		channel_output(channel, (channel->fifo[++channel->pos.read] - 0x80) * channel->vol);
		if (channel->pos.read == channel->pos.write) {
			channel->empty = TRUE;
		}
	}
}
INLINE static void channel_output(_dripgame_channel *channel, SWORD output) {
	channel->out = output / 10;
}
INLINE static void channel_reset(_dripgame_channel *channel) {
	memset(channel->fifo, 0x00, 256);
	channel->pos.read = channel->pos.write = 0;
	channel->full = FALSE;
	channel->empty = TRUE;
	channel->timer = channel->freq;
	channel_output(channel, 0);
}
INLINE static BYTE channel_status(_dripgame_channel *channel) {
	int result = 0;

	if (channel->full) {
		result |= 0x80;
	}
	if (channel->empty) {
		result |= 0x40;
	}
	return (result);
}
INLINE static void channel_reg(_dripgame_channel *channel, BYTE address, BYTE value) {
	switch (address) {
		case 0:
			channel_reset(channel);
			break;
		case 1:
			if (channel->pos.read == channel->pos.write) {
				channel->empty = FALSE;
				channel_output(channel, (value - 0x80) * channel->vol);
				channel->timer = channel->freq;
			}
			channel->fifo[channel->pos.write++] = value;
			if (channel->pos.read == channel->pos.write) {
				channel->full = TRUE;
			}
			break;
		case 2:
			channel->freq = (channel->freq & 0x0F00) | value;
			break;
		case 3:
			channel->freq = (channel->freq & 0x00FF) | ((value & 0x0F) << 8);
			channel->vol = (value & 0xF0) >> 4;
			if (!channel->empty) {
				channel_output(channel, (channel->fifo[channel->pos.read] - 0x80) * channel->vol);
			}
			break;
	}
}

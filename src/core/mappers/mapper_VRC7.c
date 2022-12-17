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
#include <math.h>
#include <float.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"
#include "mapper_VRC7_snd.h"

const WORD table_VRC7[2][4] = {
	{0x0000, 0x0002, 0x0001, 0x0003},
	{0x0000, 0x0001, 0x0002, 0x0003},
};

_vrc7 vrc7;
struct _vrc7tmp {
	WORD mask;
	BYTE type;
	BYTE delay;
} vrc7tmp;

void map_init_VRC7(BYTE revision) {
	EXTCL_CPU_WR_MEM(VRC7);
	EXTCL_SAVE_MAPPER(VRC7);
	EXTCL_CPU_EVERY_CYCLE(VRC7);
	EXTCL_APU_TICK(VRC7);

	mapper.internal_struct[0] = (BYTE *)&vrc7;
	mapper.internal_struct_size[0] = sizeof(vrc7);

	if (info.reset >= HARD) {
		memset(&vrc7, 0x00, sizeof(vrc7));
	} else {
		vrc7.enabled = 0;
		vrc7.reload = 0;
		vrc7.mode = 0;
		vrc7.acknowledge = 0;
		vrc7.count = 0;
		vrc7.prescaler = 0;
	}

	vrc7tmp.mask = 0xF000;
	if (revision == VRC7A) {
		vrc7tmp.mask = 0xF020;
	}

	vrc7tmp.delay = 1;

	vrc7tmp.type = revision;

	opll_reset();
}
void map_init_NSF_VRC7(BYTE revision) {
	memset(&vrc7, 0x00, sizeof(vrc7));

	vrc7tmp.mask = 0xF000;
	if (revision == VRC7A) {
		vrc7tmp.mask = 0xF020;
	}

	vrc7tmp.type = revision;

	opll_reset();
}
void extcl_cpu_wr_mem_VRC7(WORD address, BYTE value) {
	address = (address & vrc7tmp.mask) | table_VRC7[vrc7tmp.type][(address & 0x0018) >> 3];

	switch (address) {
		case 0x8000:
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0x8001:
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, value);
			map_prg_rom_8k_update();
			return;
		case 0x9000:
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2, value);
			map_prg_rom_8k_update();
			return;
		case 0x9001:
			// 0x9010
			vrc7.reg = value;
			opll_write_reg(0, value);
			return;
		case 0x9021:
			// 0x9030
			opll_write_reg(1, value);
			return;
		case 0xA000:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[0] = chr_pnt(value << 10);
			return;
		case 0xA001:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[1] = chr_pnt(value << 10);
			return;
		case 0xB000:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[2] = chr_pnt(value << 10);
			return;
		case 0xB001:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[3] = chr_pnt(value << 10);
			return;
		case 0xC000:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[4] = chr_pnt(value << 10);
			return;
		case 0xC001:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[5] = chr_pnt(value << 10);
			return;
		case 0xD000:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[6] = chr_pnt(value << 10);
			return;
		case 0xD001:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[7] = chr_pnt(value << 10);
			return;
		case 0xE000: {
			switch (value & 0x03) {
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
			return;
		}
		case 0xE001:
			vrc7.reload = value;
			return;
		case 0xF000:
			vrc7.acknowledge = value & 0x01;
			vrc7.enabled = value & 0x02;
			vrc7.mode = value & 0x04;
			if (vrc7.enabled) {
				vrc7.prescaler = 0;
				vrc7.count = vrc7.reload;
			}
			irq.high &= ~EXT_IRQ;
			return;
		case 0xF001:
			vrc7.enabled = vrc7.acknowledge;
			irq.high &= ~EXT_IRQ;
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_VRC7(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, vrc7.reg);
	save_slot_ele(mode, slot, vrc7.enabled);
	save_slot_ele(mode, slot, vrc7.reload);
	save_slot_ele(mode, slot, vrc7.mode);
	save_slot_ele(mode, slot, vrc7.acknowledge);
	save_slot_ele(mode, slot, vrc7.count);
	save_slot_ele(mode, slot, vrc7.prescaler);
	save_slot_ele(mode, slot, vrc7.delay);

	return (opll_save(mode, slot, fp));
}
void extcl_cpu_every_cycle_VRC7(void) {
	if (vrc7.delay && !(--vrc7.delay)) {
		irq.high |= EXT_IRQ;
	}

	if (!vrc7.enabled) {
		return;
	}

	if (!vrc7.mode) {
		if (vrc7.prescaler < 338) {
			vrc7.prescaler += 3;
			return;
		}
		vrc7.prescaler -= 338;
	}

	if (vrc7.count != 0xFF) {
		vrc7.count++;
		return;
	}

	vrc7.count = vrc7.reload;
	vrc7.delay = vrc7tmp.delay;
}
void extcl_apu_tick_VRC7(void) {
	opll_update();
}

void map_init_VRC7UNL(void) {
	EXTCL_CPU_WR_MEM(VRC7UNL);
	EXTCL_SAVE_MAPPER(VRC7UNL);
	EXTCL_CPU_EVERY_CYCLE(VRC7UNL);
	mapper.internal_struct[0] = (BYTE *)&vrc7;
	mapper.internal_struct_size[0] = sizeof(vrc7);

	if (info.reset >= HARD) {
		memset(&vrc7, 0x00, sizeof(vrc7));
	} else {
		vrc7.enabled = 0;
		vrc7.reload = 0;
		vrc7.mode = 0;
		vrc7.acknowledge = 0;
		vrc7.count = 0;
		vrc7.prescaler = 0;
	}

	vrc7tmp.delay = 1;
}
void extcl_cpu_wr_mem_VRC7UNL(WORD address, BYTE value) {
	switch (address & 0xF008) {
		case 0x8000:
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0x8008:
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, value);
			map_prg_rom_8k_update();
			return;
		case 0x9000:
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2, value);
			map_prg_rom_8k_update();
			return;
		case 0xA000:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[0] = chr_pnt(value << 10);
			return;
		case 0xA008:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[1] = chr_pnt(value << 10);
			return;
		case 0xB000:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[2] = chr_pnt(value << 10);
			return;
		case 0xB008:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[3] = chr_pnt(value << 10);
			return;
		case 0xC000:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[4] = chr_pnt(value << 10);
			return;
		case 0xC008:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[5] = chr_pnt(value << 10);
			return;
		case 0xD000:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[6] = chr_pnt(value << 10);
			return;
		case 0xD008:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[7] = chr_pnt(value << 10);
			return;
		case 0xE000: {
			switch (value & 0x03) {
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
			return;
		}
		case 0xE008:
			vrc7.reload = value;
			return;
		case 0xF000:
			vrc7.acknowledge = value & 0x01;
			vrc7.enabled = value & 0x02;
			vrc7.mode = value & 0x04;
			if (vrc7.enabled) {
				vrc7.prescaler = 0;
				vrc7.count = vrc7.reload;
			}
			irq.high &= ~EXT_IRQ;
			return;
		case 0xF008:
			if (vrc7.acknowledge) {
				vrc7.enabled = 0x02;
			} else {
				vrc7.enabled = 0x00;
			}
			irq.high &= ~EXT_IRQ;
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_VRC7UNL(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, vrc7.reg);
	save_slot_ele(mode, slot, vrc7.enabled);
	save_slot_ele(mode, slot, vrc7.reload);
	save_slot_ele(mode, slot, vrc7.mode);
	save_slot_ele(mode, slot, vrc7.acknowledge);
	save_slot_ele(mode, slot, vrc7.count);
	save_slot_ele(mode, slot, vrc7.prescaler);
	save_slot_ele(mode, slot, vrc7.delay);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_VRC7UNL(void) {
	if (vrc7.delay && !(--vrc7.delay)) {
		irq.high |= EXT_IRQ;
	}

	if (!vrc7.enabled) {
		return;
	}

	if (!vrc7.mode) {
		if (vrc7.prescaler < 338) {
			vrc7.prescaler += 3;
			return;
		}
		vrc7.prescaler -= 338;
	}

	if (vrc7.count != 0xF7) {
		vrc7.count++;
		return;
	}

	vrc7.count = vrc7.reload;
	vrc7.delay = vrc7tmp.delay;
}

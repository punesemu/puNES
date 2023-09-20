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

#include "mappers.h"
#include "cpu.h"
#include "ppu.h"
#include "save_slot.h"
#include "ppu_inline.h"

INLINE static void prg_fix_471(void);
INLINE static void chr_fix_471(void);

INLINE static void irq_clock_471(void);

struct m471 {
	WORD reg;
} m471;

void map_init_471(void) {
	EXTCL_AFTER_MAPPER_INIT(471);
	EXTCL_CPU_WR_MEM(471);
	EXTCL_SAVE_MAPPER(471);
	EXTCL_PPU_000_TO_255(471);
	EXTCL_PPU_256_TO_319(471);
	EXTCL_PPU_320_TO_34X(471);
	EXTCL_UPDATE_R2006(471);
	mapper.internal_struct[0] = (BYTE *)&m471;
	mapper.internal_struct_size[0] = sizeof(m471);

	if (info.reset >= HARD) {
		m471.reg = 0;
	}
}
void extcl_after_mapper_init_471(void) {
	prg_fix_471();
	chr_fix_471();
}
void extcl_cpu_wr_mem_471(WORD address, UNUSED(BYTE value)) {
	m471.reg = address;
	cpudata.irq.high &= ~EXT_IRQ;
	prg_fix_471();
	chr_fix_471();
}
BYTE extcl_save_mapper_471(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m471.reg);

	return (EXIT_OK);
}
void extcl_ppu_000_to_255_471(void) {
	if (ppudata.r2001.visible) {
		extcl_ppu_320_to_34x_471();
	}
}
void extcl_ppu_256_to_319_471(void) {
	if ((ppudata.ppu.frame_x & 0x0007) != 0x0003) {
		return;
	}

	if ((!ppudata.spr_ev.count_plus || (ppudata.spr_ev.tmp_spr_plus == ppudata.spr_ev.count_plus)) && (ppudata.r2000.size_spr == 16)) {
		ppudata.ppu.spr_adr = ppudata.r2000.spt_adr;
	} else {
		ppu_spr_adr((ppudata.ppu.frame_x & 0x0038) >> 3);
	}
	if ((ppudata.ppu.spr_adr & 0x1000) > (ppudata.ppu.bck_adr & 0x1000)) {
		irq_clock_471();
	}
}
void extcl_ppu_320_to_34x_471(void) {
	if ((ppudata.ppu.frame_x & 0x0007) != 0x0003) {
		return;
	}

	if (ppudata.ppu.frame_x == 323) {
		ppu_spr_adr(7);
	}

	ppu_bck_adr(ppudata.r2000.bpt_adr, ppudata.r2006.value);

	if ((ppudata.ppu.bck_adr & 0x1000) > (ppudata.ppu.spr_adr & 0x1000)) {
		irq_clock_471();
	}
}
void extcl_update_r2006_471(WORD new_r2006, WORD old_r2006) {
	if ((new_r2006 & 0x1000) > (old_r2006 & 0x1000)) {
		irq_clock_471();
	}
}

INLINE static void prg_fix_471(void) {
	memmap_auto_32k(MMCPU(0x8000), (m471.reg & 0xFF));
}
INLINE static void chr_fix_471(void) {
	memmap_auto_8k(MMPPU(0x0000), (m471.reg & 0xFF));
}

INLINE static void irq_clock_471(void) {
	cpudata.irq.high |= EXT_IRQ;
}

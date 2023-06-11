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
#include "irqA12.h"

void map_init_436(void) {
	EXTCL_AFTER_MAPPER_INIT(436);
	EXTCL_MAPPER_QUIT(OneBus);
	EXTCL_CPU_WR_MEM(OneBus);
	EXTCL_CPU_RD_MEM(OneBus);
	EXTCL_SAVE_MAPPER(OneBus);
	EXTCL_WR_PPU_REG(OneBus);
	EXTCL_WR_APU(OneBus);
	EXTCL_RD_APU(OneBus);
	EXTCL_RD_CHR(OneBus);
	EXTCL_CPU_EVERY_CYCLE(OneBus);
	EXTCL_IRQ_A12_CLOCK(OneBus);
	EXTCL_PPU_000_TO_34X(OneBus);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&onebus;
	mapper.internal_struct_size[0] = sizeof(onebus);

	memset(&irqA12, 0x00, sizeof(irqA12));

	init_OneBus();

	irqA12.present = TRUE;
}
void extcl_after_mapper_init_436(void) {
	if (info.mapper.submapper == 1) {
		OneBus_prg_fix_8k(0xF7FF, ((onebus.reg.cpu[0x1C] & 0x01) && (onebus.reg.cpu[0x0F] & 0x04) ? 0x0000 : 0x0800));
		OneBus_chr_fix(0xBFFF, ((onebus.reg.cpu[0x1C] & 0x01) && (onebus.reg.cpu[0x0F] & 0x04) ? 0x0000 : 0x4000));
		OneBus_wram_fix(0xF7FF, ((onebus.reg.cpu[0x1C] & 0x01) && (onebus.reg.cpu[0x0F] & 0x04) ? 0x0000 : 0x0800));
	} else {
		OneBus_prg_fix_8k(0xF3FF, ((onebus.reg.cpu[0x00] & 0x40) << 5) | ((onebus.reg.cpu[0x0F] & 0x20) << 5));
		OneBus_chr_fix(0x9FFF, ((onebus.reg.cpu[0x00] & 0x04) << 12) | ((onebus.reg.cpu[0x0F] & 0x20) << 8));
		OneBus_wram_fix(0xF3FF, ((onebus.reg.cpu[0x00] & 0x40) << 5) | ((onebus.reg.cpu[0x0F] & 0x20) << 5));
	}
	OneBus_mirroring_fix();
}

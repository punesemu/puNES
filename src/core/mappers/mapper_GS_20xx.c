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

#include "mappers.h"
#include "mem_map.h"
#include "info.h"

struct _gs20xxtmp {
	BYTE gs2004;
	BYTE *prg_6000;
} gs20xxtmp;

void map_init_GS_20xx(void) {
	EXTCL_CPU_WR_MEM(GS_20xx);
	EXTCL_CPU_RD_MEM(GS_20xx);

	{
		BYTE value;

		map_prg_rom_8k(4, 0, 0);
		gs20xxtmp.gs2004 = (info.prg.rom[0].banks_8k != 0) && ((info.prg.rom[0].banks_8k & (info.prg.rom[0].banks_8k - 1)) == 0);
		value = gs20xxtmp.gs2004 ? 0xFF : 0x1F;
		control_bank(info.prg.rom[0].max.banks_8k)
		gs20xxtmp.prg_6000 = prg_chip_byte_pnt(0, value << 13);
	}
}
void extcl_cpu_wr_mem_GS_20xx(UNUSED(WORD address), BYTE value) {
	if (gs20xxtmp.gs2004) {
		control_bank(info.prg.rom[0].max.banks_32k)
	} else {
		control_bank_with_AND(0x0F, info.prg.rom[0].max.banks_32k)
	}
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}
BYTE extcl_cpu_rd_mem_GS_20xx(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return (gs20xxtmp.prg_6000[address & 0x1FFF]);
	}
	return (openbus);
}

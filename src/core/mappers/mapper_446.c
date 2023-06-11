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
#include <stdlib.h>
#include "mappers.h"
#include "save_slot.h"
#include "SST39SF040.h"
#include "gui.h"

INLINE static void switch_mode(void);
INLINE static void fix_all(void);
INLINE static WORD prg_base(void);
INLINE static WORD prg_mask(void);
INLINE static WORD chr_base(void);

INLINE static void prg_fix_no_mapper_446(void);
INLINE static void chr_fix_no_mapper_446(void);
INLINE static void wram_fix_no_mapper_446(void);
INLINE static void mirroring_fix_no_mapper_446(void);

INLINE static void prg_fix_nrom_446(void);
INLINE static void chr_fix_nrom_446(void);
INLINE static void wram_fix_nrom_446(void);
INLINE static void mirroring_fix_nrom_446(void);

INLINE static void prg_fix_cnrom_446(void);
INLINE static void chr_fix_cnrom_446(void);
INLINE static void wram_fix_cnrom_446(void);
INLINE static void mirroring_fix_cnrom_446(void);

INLINE static void prg_fix_unrom_446(void);
INLINE static void chr_fix_unrom_446(void);
INLINE static void wram_fix_unrom_446(void);
INLINE static void mirroring_fix_unrom_446(void);

INLINE static void prg_fix_bandai_446(void);
INLINE static void chr_fix_bandai_446(void);
INLINE static void wram_fix_bandai_446(void);
INLINE static void mirroring_fix_bandai_446(void);

INLINE static void prg_fix_anrom_446(void);
INLINE static void chr_fix_anrom_446(void);
INLINE static void wram_fix_anrom_446(void);
INLINE static void mirroring_fix_anrom_446(void);

INLINE static void prg_fix_gnrom_446(void);
INLINE static void chr_fix_gnrom_446(void);
INLINE static void wram_fix_gnrom_446(void);
INLINE static void mirroring_fix_gnrom_446(void);

void prg_swap_mmc1_446(WORD address, WORD value);
void chr_swap_mmc1_446(WORD address, WORD value);

void prg_swap_mmc3_446(WORD address, WORD value);
void chr_swap_mmc3_446(WORD address, WORD value);

void prg_swap_mmc3_tlsrom_446(WORD address, WORD value);
void chr_swap_mmc3_tlsrom_446(WORD address, WORD value);

void prg_swap_mmc3_m189_446(WORD address, WORD value);
void chr_swap_mmc3_m189_446(WORD address, WORD value);

void prg_swap_vrc2and4_m022_446(WORD address, WORD value);
void chr_swap_vrc2and4_m022_446(WORD address, WORD value);

void prg_swap_vrc2and4_m023m025_446(WORD address, WORD value);
void chr_swap_vrc2and4_m023m025_446(WORD address, WORD value);

void prg_swap_vrc6_446(WORD address, WORD value);
void chr_swap_vrc6_446(WORD address, WORD value);
void nmt_swap_vrc6_446(WORD address, WORD value);

enum _m116_mappers {
	M446_UNROM,
	M446_MMC3,
	M446_NROM,
	M446_CNROM,
	M446_ANROM,
	M446_SLROM,
	M446_SNROM,
	M446_SUROM,
	M446_GNROM,
	M446_PNROM,
	M446_HNROM,
	M446_BANDAI,
	M446_TLSROM,
	M446_189,
	M446_VRC6,
	M446_VRC2_22,
	M446_VRC4_25,
	M446_VRC4_23,
	M446_VRC1
};

struct _m446 {
	BYTE reg[8];
	BYTE reg189;
	BYTE latch;
	BYTE mapper;
} m446;
struct _m446tmp {
	BYTE *sst39sf040;
} m446tmp;

void map_init_446(void) {
	EXTCL_AFTER_MAPPER_INIT(446);
	EXTCL_MAPPER_QUIT(446);
	EXTCL_CPU_WR_MEM(446);
	EXTCL_CPU_RD_MEM(446);
	EXTCL_SAVE_MAPPER(446);
	EXTCL_CPU_EVERY_CYCLE(446);
	EXTCL_PPU_000_TO_34X(446);
	EXTCL_PPU_000_TO_255(446);
	EXTCL_PPU_256_TO_319(446);
	EXTCL_PPU_320_TO_34X(446);
	EXTCL_UPDATE_R2006(446);
	EXTCL_BATTERY_IO(446);
	mapper.internal_struct[0] = (BYTE *)&m446;
	mapper.internal_struct_size[0] = sizeof(m446);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);
	mapper.internal_struct[2] = (BYTE *)&mmc1;
	mapper.internal_struct_size[2] = sizeof(mmc1);
	mapper.internal_struct[3] = (BYTE *)&vrc2and4;
	mapper.internal_struct_size[3] = sizeof(vrc2and4);
	mapper.internal_struct[4] = (BYTE *)&vrc6;
	mapper.internal_struct_size[4] = sizeof(vrc6);

	if (info.reset >= HARD) {
		memset(&m446, 0x00, sizeof(m446));
	}

	if (info.mapper.submapper == DEFAULT) {
		info.mapper.submapper = 0;
	}

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		m446tmp.sst39sf040 = (BYTE *)malloc(prgrom_size());
		memcpy(m446tmp.sst39sf040, prgrom_pnt(), prgrom_size());
		sst39sf040_init(m446tmp.sst39sf040, prgrom_size(), 0x01, 0x76, 0x0AAA, 0x0555, 131072);
	}

	info.mapper.force_battery_io = TRUE;
	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_446(void) {
	switch_mode();
	fix_all();
}
void extcl_mapper_quit_446(void) {
	if (m446tmp.sst39sf040) {
		free(m446tmp.sst39sf040);
		m446tmp.sst39sf040 = NULL;
	}
}
void extcl_cpu_wr_mem_446(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x5000:
			if (!(m446.reg[0] & 0x80)) {
				address &= 0x0007;
				if (!address && (info.mapper.submapper == 0) && ((value & 0x1F) == 0x01)) {
					value = (value & ~0x1F) | M446_SNROM;
				}
				m446.reg[address] = value;
				switch_mode();
				fix_all();
			}
			return;
		case 0x6000:
			if ((m446.reg[0] & 0x80) && (m446.mapper == M446_189)) {
				m446.reg189 = address & 0xFF;
				fix_all();
			}
			return;
		case 0x7000:
			return;
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			if (m446.reg[0] & 0x80) {
				switch (m446.mapper) {
					case M446_ANROM:
					case M446_CNROM:
					case M446_UNROM:
					case M446_GNROM:
					case M446_BANDAI:
						m446.latch = value;
						fix_all();
						return;
					case M446_SLROM:
					case M446_SNROM:
						extcl_cpu_wr_mem_MMC1(address, value);
						return;
					case M446_189:
					case M446_MMC3:
					case M446_TLSROM:
						extcl_cpu_wr_mem_MMC3(address, value);
						return;
					case M446_VRC2_22:
					case M446_VRC4_23:
					case M446_VRC4_25:
						extcl_cpu_wr_mem_VRC2and4(address, value);
						return;
					case M446_VRC6:
						extcl_cpu_wr_mem_VRC6(address, value);
						return;
					default:
						return;
				}
			} else {
				sst39sf040_write(address, value);
			}
			return;
		default:
			return;
	}
}
BYTE extcl_cpu_rd_mem_446(WORD address, BYTE openbus) {
	switch (address & 0xF000) {
		case 0x6000:
			return ((m446.reg[0] & 0x80) && (m446.mapper == M446_VRC2_22)
				? extcl_cpu_rd_mem_VRC2and4(address, openbus)
				: openbus);
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			return (sst39sf040_read(address));
		default:
			return (openbus);
	}
}
BYTE extcl_save_mapper_446(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m446.reg);
	save_slot_ele(mode, slot, m446.reg189);
	save_slot_ele(mode, slot, m446.latch);
	save_slot_ele(mode, slot, m446.mapper);
	if (extcl_save_mapper_MMC1(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	if (extcl_save_mapper_MMC3(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	if (extcl_save_mapper_VRC2and4(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	if (extcl_save_mapper_VRC6(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	return (sst39sf040_save_mapper(mode, slot, fp));
}
void extcl_cpu_every_cycle_446(void) {
	sst39sf040_tick();
	if (m446.reg[0] & 0x80) {
		switch (m446.mapper) {
			case M446_189:
			case M446_MMC3:
			case M446_TLSROM:
				extcl_cpu_every_cycle_MMC3();
				return;
			case M446_VRC2_22:
			case M446_VRC4_23:
			case M446_VRC4_25:
				extcl_cpu_every_cycle_VRC2and4();
				return;
			case M446_VRC6:
				extcl_cpu_every_cycle_VRC6();
				return;
			default:
				return;
		}
	}
}
void extcl_ppu_000_to_34x_446(void) {
	if (m446.reg[0] & 0x80) {
		switch (m446.mapper) {
			case M446_189:
			case M446_MMC3:
			case M446_TLSROM:
				extcl_ppu_000_to_34x_MMC3();
				return;
			default:
				return;
		}
	}
}
void extcl_ppu_000_to_255_446(void) {
	if (m446.reg[0] & 0x80) {
		switch (m446.mapper) {
			case M446_189:
			case M446_MMC3:
			case M446_TLSROM:
				extcl_ppu_000_to_255_MMC3();
				return;
			default:
				return;
		}
	}
}
void extcl_ppu_256_to_319_446(void) {
	if (m446.reg[0] & 0x80) {
		switch (m446.mapper) {
			case M446_189:
			case M446_MMC3:
			case M446_TLSROM:
				extcl_ppu_256_to_319_MMC3();
				return;
			default:
				return;
		}
	}
}
void extcl_ppu_320_to_34x_446(void) {
	if (m446.reg[0] & 0x80) {
		switch (m446.mapper) {
			case M446_189:
			case M446_MMC3:
			case M446_TLSROM:
				extcl_ppu_320_to_34x_MMC3();
				return;
			default:
				return;
		}
	}
}
void extcl_update_r2006_446(WORD new_r2006, WORD old_r2006) {
	if (m446.reg[0] & 0x80) {
		switch (m446.mapper) {
			case M446_189:
			case M446_MMC3:
			case M446_TLSROM:
				extcl_update_r2006_MMC3(new_r2006, old_r2006);
				return;
			default:
				return;
		}
	}
}
void extcl_battery_io_446(BYTE mode, FILE *fp) {
	if (mode == WR_BAT) {
		if (fwrite(m446tmp.sst39sf040, prgrom_size(), 1, fp) < 1) {
			log_error(uL("mapper_446;error on write flash chip"));
		}
	} else {
		if (fread(m446tmp.sst39sf040, prgrom_size(), 1, fp) < 1) {
			log_error(uL("mapper_446;error on read flash chip"));
		}
	}
}

INLINE static void switch_mode(void) {
	if (m446.reg[0] & 0x80) {
		m446.mapper = m446.reg[0] & 0x1F;
		switch (m446.mapper) {
			case M446_ANROM:
			case M446_CNROM:
			case M446_UNROM:
			case M446_GNROM:
			case M446_BANDAI:
				m446.latch = 0;
				break;
			case M446_SLROM:
			case M446_SNROM:
				init_MMC1(MMC1B);
				MMC1_prg_swap = prg_swap_mmc1_446;
				MMC1_chr_swap = chr_swap_mmc1_446;
				break;
			case M446_MMC3:
				init_MMC3();
				MMC3_prg_swap = prg_swap_mmc3_446;
				MMC3_chr_swap = chr_swap_mmc3_446;
				break;
			case M446_TLSROM:
				init_MMC3();
				MMC3_prg_swap = prg_swap_mmc3_tlsrom_446;
				MMC3_chr_swap = chr_swap_mmc3_tlsrom_446;
				break;
			case M446_189:
				init_MMC3();
				MMC3_prg_swap = prg_swap_mmc3_m189_446;
				MMC3_chr_swap = chr_swap_mmc3_m189_446;
				break;
			case M446_VRC2_22:
				init_VRC2and4(VRC24_VRC2, 0x02, 0x01, TRUE);
				VRC2and4_prg_swap = prg_swap_vrc2and4_m022_446;
				VRC2and4_chr_swap = chr_swap_vrc2and4_m022_446;
				break;
			case M446_VRC4_23:
				init_VRC2and4(VRC24_VRC4, 0x05, 0x0A, TRUE);
				VRC2and4_prg_swap = prg_swap_vrc2and4_m023m025_446;
				VRC2and4_chr_swap = chr_swap_vrc2and4_m023m025_446;
				break;
			case M446_VRC4_25:
				init_VRC2and4(VRC24_VRC4, 0x0A, 0x05, TRUE);
				VRC2and4_prg_swap = prg_swap_vrc2and4_m023m025_446;
				VRC2and4_chr_swap = chr_swap_vrc2and4_m023m025_446;
				break;
			case M446_VRC6:
				init_VRC6(0x01, 0x02);
				VRC6_prg_swap = prg_swap_vrc6_446;
				VRC6_chr_swap = chr_swap_vrc6_446;
				VRC6_nmt_swap = nmt_swap_vrc6_446;
				break;
			default:
				break;
		}
	}
}
INLINE static void fix_all(void) {
	if (m446.reg[0] & 0x80) {
		switch (m446.mapper) {
			case M446_NROM:
				prg_fix_nrom_446();
				chr_fix_nrom_446();
				wram_fix_nrom_446();
				mirroring_fix_nrom_446();
				return;
			case M446_CNROM:
				prg_fix_cnrom_446();
				chr_fix_cnrom_446();
				wram_fix_cnrom_446();
				mirroring_fix_cnrom_446();
				return;
			case M446_UNROM:
				prg_fix_unrom_446();
				chr_fix_unrom_446();
				wram_fix_unrom_446();
				mirroring_fix_unrom_446();
				return;
			case M446_BANDAI:
				prg_fix_bandai_446();
				chr_fix_bandai_446();
				wram_fix_bandai_446();
				mirroring_fix_bandai_446();
				return;
			case M446_ANROM:
				prg_fix_anrom_446();
				chr_fix_anrom_446();
				wram_fix_anrom_446();
				mirroring_fix_anrom_446();
				return;
			case M446_GNROM:
				prg_fix_gnrom_446();
				chr_fix_gnrom_446();
				wram_fix_gnrom_446();
				mirroring_fix_gnrom_446();
				return;
			case M446_SLROM:
			case M446_SNROM:
				MMC1_prg_fix();
				MMC1_chr_fix();
				MMC1_wram_fix();
				MMC1_mirroring_fix();
				return;
			case M446_MMC3:
			case M446_TLSROM:
			case M446_189:
				MMC3_prg_fix();
				MMC3_chr_fix();
				MMC3_wram_fix();
				MMC3_mirroring_fix();
				return;
			case M446_VRC2_22:
			case M446_VRC4_23:
			case M446_VRC4_25:
				VRC2and4_prg_fix();
				VRC2and4_chr_fix();
				VRC2and4_wram_fix();
				VRC2and4_mirroring_fix();
				return;
			case M446_VRC6:
				VRC6_prg_fix();
				VRC6_chr_fix();
				VRC6_wram_fix();
				VRC6_mirroring_fix();
				return;
		}
	} else {
		prg_fix_no_mapper_446();
		chr_fix_no_mapper_446();
		wram_fix_no_mapper_446();
		mirroring_fix_no_mapper_446();
	}
	if (m446.reg[5] & 0x04) {
		chr_disable_write();
	}
}
INLINE static WORD prg_base(void) {
	return (m446.reg[1] | (m446.reg[2] << 8));
}
INLINE static WORD prg_mask(void) {
	return (~m446.reg[3]);
}
INLINE static WORD chr_base(void) {
	return (m446.reg[6]);
}

INLINE static void prg_fix_no_mapper_446(void) {
	memmap_auto_8k(MMCPU(0x8000), prg_base());
	memmap_auto_8k(MMCPU(0xA000), 0x3D);
	memmap_auto_8k(MMCPU(0xC000), 0x3E);
	memmap_auto_8k(MMCPU(0xE000), 0x3F);
}
INLINE static void chr_fix_no_mapper_446(void) {
	memmap_auto_8k(MMPPU(0x0000), chr_base());
}
INLINE static void wram_fix_no_mapper_446(void) {}
INLINE static void mirroring_fix_no_mapper_446(void) {
	if (m446.reg[4] & 0x01) {
		mirroring_V();
	} else {
		mirroring_H();
	}
}

INLINE static void prg_fix_nrom_446(void) {
	WORD base = prg_base();
	WORD mask = prg_mask();

	memmap_auto_8k(MMCPU(0x8000), (base | (0x00 & mask)));
	memmap_auto_8k(MMCPU(0xA000), (base | (0x01 & mask)));
	memmap_auto_8k(MMCPU(0xC000), (base | (0x02 & mask)));
	memmap_auto_8k(MMCPU(0xE000), (base | (0x03 & mask)));
}
INLINE static void chr_fix_nrom_446(void) {
	memmap_auto_8k(MMPPU(0x0000), chr_base());
}
INLINE static void wram_fix_nrom_446(void) {
	memmap_disable_8k(MMCPU(0x6000));
}
INLINE static void mirroring_fix_nrom_446(void) {
	if (m446.reg[4] & 0x01) {
		mirroring_V();
	} else {
		mirroring_H();
	}
}

INLINE static void prg_fix_cnrom_446(void) {
	WORD base = prg_base();
	WORD mask = prg_mask();

	memmap_auto_8k(MMCPU(0x8000), (base | (0x00 & mask)));
	memmap_auto_8k(MMCPU(0xA000), (base | (0x01 & mask)));
	memmap_auto_8k(MMCPU(0xC000), (base | (0x02 & mask)));
	memmap_auto_8k(MMCPU(0xE000), (base | (0x03 & mask)));
}
INLINE static void chr_fix_cnrom_446(void) {
	memmap_auto_8k(MMPPU(0x0000), (m446.latch & 0x03));
}
INLINE static void wram_fix_cnrom_446(void) {
	memmap_disable_8k(MMCPU(0x6000));
}
INLINE static void mirroring_fix_cnrom_446(void) {
	if (m446.reg[4] & 0x01) {
		mirroring_V();
	} else {
		mirroring_H();
	}
}

INLINE static void prg_fix_unrom_446(void) {
	WORD base = prg_base() >> 1;
	WORD mask = prg_mask() >> 1;
	WORD bank = m446.latch;

	memmap_auto_16k(MMCPU(0x8000), (base | (bank & mask)));
	memmap_auto_16k(MMCPU(0xC000), (base | (0x1F & mask)));
}
INLINE static void chr_fix_unrom_446(void) {
	memmap_auto_8k(MMPPU(0x0000), chr_base());
}
INLINE static void wram_fix_unrom_446(void) {
	memmap_disable_8k(MMCPU(0x6000));
}
INLINE static void mirroring_fix_unrom_446(void) {
	if (m446.reg[4] & 0x01) {
		mirroring_V();
	} else {
		mirroring_H();
	}
}

INLINE static void prg_fix_bandai_446(void) {
	WORD base = prg_base() >> 1;
	WORD mask = prg_mask() >> 1;
	WORD bank = m446.latch >> 4;

	memmap_auto_16k(MMCPU(0x8000), (base | (bank & mask)));
	memmap_auto_16k(MMCPU(0xC000), (base | (0xFF & mask)));
}
INLINE static void chr_fix_bandai_446(void) {
	memmap_auto_8k(MMPPU(0x0000), (m446.latch & 0x0F));
}
INLINE static void wram_fix_bandai_446(void) {
	memmap_disable_8k(MMCPU(0x6000));
}
INLINE static void mirroring_fix_bandai_446(void) {
	if (m446.latch & 0x10) {
		mirroring_SCR1();
	} else {
		mirroring_SCR0();
	}
}

INLINE static void prg_fix_anrom_446(void) {
	WORD base = prg_base() >> 2;
	WORD mask = prg_mask() >> 2;
	WORD bank = m446.latch;

	memmap_auto_32k(MMCPU(0x8000), (base | (bank & mask)));
}
INLINE static void chr_fix_anrom_446(void) {
	memmap_auto_8k(MMPPU(0x0000), (m446.latch & 0x03));
}
INLINE static void wram_fix_anrom_446(void) {
	memmap_disable_8k(MMCPU(0x6000));
}
INLINE static void mirroring_fix_anrom_446(void) {
	if (m446.reg[4] & 0x01) {
		mirroring_V();
	} else {
		mirroring_H();
	}
}

INLINE static void prg_fix_gnrom_446(void) {
	WORD base = prg_base() >> 2;
	WORD mask = prg_mask() >> 2;
	WORD bank = m446.latch >> 4;

	memmap_auto_32k(MMCPU(0x8000), (base | (bank & mask)));
}
INLINE static void chr_fix_gnrom_446(void) {
	memmap_auto_8k(MMPPU(0x0000), (m446.latch & 0x03));
}
INLINE static void wram_fix_gnrom_446(void) {
	memmap_disable_8k(MMCPU(0x6000));
}
INLINE static void mirroring_fix_gnrom_446(void) {
	if (m446.reg[4] & 0x01) {
		mirroring_V();
	} else {
		mirroring_H();
	}
}

void prg_swap_mmc1_446(WORD address, WORD value) {
	WORD base = prg_base() >> 1;
	WORD mask = prg_mask() >> 1;

	prg_swap_MMC1_base(address, (base | (value & mask)));
}
void chr_swap_mmc1_446(WORD address, WORD value) {
	WORD base = chr_base() >> 2;
	WORD mask = 0x1F;

	chr_swap_MMC1_base(address, (base | (value & mask)));
}

void prg_swap_mmc3_446(WORD address, WORD value) {
	WORD base = prg_base();
	WORD mask = prg_mask();

	prg_swap_MMC3_base(address, (base | (value & mask)));
}
void chr_swap_mmc3_446(WORD address, WORD value) {
	WORD base = chr_base();
	WORD mask = 0xFF;

	chr_swap_MMC3_base(address, (base | (value & mask)));
}

void prg_swap_mmc3_tlsrom_446(WORD address, WORD value) {
	WORD base = prg_base();
	WORD mask = prg_mask();

	prg_swap_MMC3_base(address, (base | (value & mask)));
}
void chr_swap_mmc3_tlsrom_446(WORD address, WORD value) {
	WORD base = chr_base();
	WORD mask = 0x7F;

	chr_swap_MMC3_base(address, (base | (value & mask)));
}

void prg_swap_mmc3_m189_446(WORD address, UNUSED(WORD value)) {
	WORD slot = (address >> 13) & 0x03;
	WORD base = prg_base() & ~3;

	prg_swap_MMC3_base(address, (base | ((m446.reg189 & 0x03) << 2) | slot));
}
void chr_swap_mmc3_m189_446(WORD address, WORD value) {
	WORD base = chr_base();
	WORD mask = 0xFF;

	chr_swap_MMC3_base(address, (base | (value & mask)));
}

void prg_swap_vrc2and4_m022_446(WORD address, WORD value) {
	WORD base = prg_base();
	WORD mask = prg_mask();

	prg_swap_VRC2and4_base(address, (base | (value & mask)));
}
void chr_swap_vrc2and4_m022_446(WORD address, WORD value) {
	chr_swap_VRC2and4_base(address, (value >> 1));
}

void prg_swap_vrc2and4_m023m025_446(WORD address, WORD value) {
	WORD base = prg_base();
	WORD mask = prg_mask();

	prg_swap_VRC2and4_base(address, (base | (value & mask)));
}
void chr_swap_vrc2and4_m023m025_446(WORD address, WORD value) {
	chr_swap_VRC2and4_base(address, (value & 0xFF));
}

void prg_swap_vrc6_446(WORD address, WORD value) {
	WORD base = prg_base();
	WORD mask = prg_mask();

	prg_swap_VRC6_base(address, (base | (value & mask)));
}
void chr_swap_vrc6_446(WORD address, WORD value) {
	chr_swap_VRC6_base(address, (value & 0xFF));
}
void nmt_swap_vrc6_446(WORD address, WORD value) {
	nmt_swap_VRC6_base(address, (value & 0xFF));
}

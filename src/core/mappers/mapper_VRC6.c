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

INLINE static void vrc6_update_chr_and_mirroring(void);

/* vecchia versione
#define vcr6_square_tick(square)\
	vrc6.square.output = 0;\
	if (--vrc6.square.timer == 0) {\
		vrc6.square.step = (vrc6.square.step + 1) & 0x0F;\
		vrc6.square.timer = vrc6.square.frequency + 1;\
	}\
	if (vrc6.square.enabled) {\
		vrc6.square.output = 0;\
		if (vrc6.square.mode || (vrc6.square.step <= vrc6.square.duty)) {\
			vrc6.square.output = vrc6.square.volume;\
		}\
	}
*/
#define vcr6_square_tick(square)\
	if (--vrc6.square.timer == 0) {\
		vrc6.square.step = (vrc6.square.step + 1) & 0x0F;\
		vrc6.square.timer = vrc6.square.frequency + 1;\
		if (vrc6.square.enabled) {\
			vrc6.square.output = 0;\
			if (vrc6.square.mode || (vrc6.square.step <= vrc6.square.duty)) {\
				vrc6.square.output = vrc6.square.volume;\
			}\
		}\
		vrc6.clocked = TRUE;\
	}\
	if (!vrc6.square.enabled) {\
		vrc6.square.output = 0;\
	}
#define vrc6_square_saveslot(square)\
	save_slot_ele(mode, slot, square.enabled);\
	save_slot_ele(mode, slot, square.duty);\
	save_slot_ele(mode, slot, square.step);\
	save_slot_ele(mode, slot, square.volume);\
	save_slot_ele(mode, slot, square.mode);\
	save_slot_ele(mode, slot, square.timer);\
	save_slot_ele(mode, slot, square.frequency);\
	save_slot_ele(mode, slot, square.output)

_vrc6 vrc6;
struct _vrc6tmp {
	BYTE type;
	BYTE delay;
} vrc6tmp;

const WORD table_VRC6[2][4] = {
	{0x0000, 0x0001, 0x0002, 0x0003},
	{0x0000, 0x0002, 0x0001, 0x0003},
};

void map_init_VRC6(BYTE revision) {
	EXTCL_CPU_WR_MEM(VRC6);
	EXTCL_SAVE_MAPPER(VRC6);
	EXTCL_CPU_EVERY_CYCLE(VRC6);
	EXTCL_APU_TICK(VRC6);
	mapper.internal_struct[0] = (BYTE *)&vrc6;
	mapper.internal_struct_size[0] = sizeof(vrc6);

	if (info.reset >= HARD) {
		memset(&vrc6, 0x00, sizeof(vrc6));
	} else {
		vrc6.enabled = 0;
		vrc6.reload = 0;
		vrc6.mode = 0;
		vrc6.acknowledge = 0;
		vrc6.count = 0;
		vrc6.prescaler = 0;
	}

	vrc6.S3.timer = 1;
	vrc6.S3.duty = 1;
	vrc6.S4.timer = 1;
	vrc6.S4.duty = 1;
	vrc6.saw.timer = 1;
	vrc6tmp.delay = 1;

	vrc6tmp.type = revision;
}

void map_init_NSF_VRC6(BYTE revision) {
	memset(&vrc6, 0x00, sizeof(vrc6));

	vrc6.S3.timer = 1;
	vrc6.S3.duty = 1;
	vrc6.S4.timer = 1;
	vrc6.S4.duty = 1;
	vrc6.saw.timer = 1;

	vrc6tmp.type = revision;
}
void extcl_cpu_wr_mem_VRC6(WORD address, BYTE value) {
	address = (address & 0xF000) | table_VRC6[vrc6tmp.type][(address & 0x0003)];

	switch (address) {
		case 0x8000:
		case 0x8001:
		case 0x8002:
		case 0x8003:
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0x9000:
			vrc6.S3.volume = value & 0x0F;
			vrc6.S3.duty = (value & 0x70) >> 4;
			vrc6.S3.mode = value & 0x80;
			return;
		case 0x9001:
			vrc6.S3.frequency = (vrc6.S3.frequency & 0x0F00) | value;
			return;
		case 0x9002:
			vrc6.S3.frequency = (vrc6.S3.frequency & 0x00FF) | ((value & 0x0F) << 8);
			vrc6.S3.enabled = value & 0x80;
			return;
		case 0xA000:
			vrc6.S4.volume = value & 0x0F;
			vrc6.S4.duty = (value & 0x70) >> 4;
			vrc6.S4.mode = value & 0x80;
			return;
		case 0xA001:
			vrc6.S4.frequency = (vrc6.S4.frequency & 0x0F00) | value;
			return;
		case 0xA002:
			vrc6.S4.frequency = (vrc6.S4.frequency & 0x00FF) | ((value & 0x0F) << 8);
			vrc6.S4.enabled = value & 0x80;
			return;
		case 0xB000:
			vrc6.saw.accumulator = value & 0x3F;
			return;
		case 0xB001:
			vrc6.saw.frequency = (vrc6.saw.frequency & 0x0F00) | value;
			return;
		case 0xB002:
			vrc6.saw.frequency = (vrc6.saw.frequency & 0x00FF) | ((value & 0x0F) << 8);
			vrc6.saw.enabled = value & 0x80;
			return;
		case 0xB003:
			vrc6.b003 = value;
			vrc6_update_chr_and_mirroring();
			return;
		case 0xC000:
		case 0xC001:
		case 0xC002:
		case 0xC003:
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2, value);
			map_prg_rom_8k_update();
			return;
		case 0xD000:
			vrc6.chr_map[0] = value;
			vrc6_update_chr_and_mirroring();
			return;
		case 0xD001:
			vrc6.chr_map[1] = value;
			vrc6_update_chr_and_mirroring();
			return;
		case 0xD002:
			vrc6.chr_map[2] = value;
			vrc6_update_chr_and_mirroring();
			return;
		case 0xD003:
			vrc6.chr_map[3] = value;
			vrc6_update_chr_and_mirroring();
			return;
		case 0xE000:
			vrc6.chr_map[4] = value;
			vrc6_update_chr_and_mirroring();
			return;
		case 0xE001:
			vrc6.chr_map[5] = value;
			vrc6_update_chr_and_mirroring();
			return;
		case 0xE002:
			vrc6.chr_map[6] = value;
			vrc6_update_chr_and_mirroring();
			return;
		case 0xE003:
			vrc6.chr_map[7] = value;
			vrc6_update_chr_and_mirroring();
			return;
		case 0xF000:
			vrc6.reload = value;
			return;
		case 0xF001:
			vrc6.acknowledge = value & 0x01;
			vrc6.enabled = value & 0x02;
			vrc6.mode = value & 0x04;
			if (vrc6.enabled) {
				vrc6.prescaler = 0;
				vrc6.count = vrc6.reload;
			}
			irq.high &= ~EXT_IRQ;
			return;
		case 0xF002:
			vrc6.enabled = vrc6.acknowledge;
			irq.high &= ~EXT_IRQ;
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_VRC6(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, vrc6.enabled);
	save_slot_ele(mode, slot, vrc6.reload);
	save_slot_ele(mode, slot, vrc6.mode);
	save_slot_ele(mode, slot, vrc6.acknowledge);
	save_slot_ele(mode, slot, vrc6.count);
	save_slot_ele(mode, slot, vrc6.prescaler);
	save_slot_ele(mode, slot, vrc6.delay);

	vrc6_square_saveslot(vrc6.S3);
	vrc6_square_saveslot(vrc6.S4);

	save_slot_ele(mode, slot, vrc6.saw.enabled);
	save_slot_ele(mode, slot, vrc6.saw.accumulator);
	save_slot_ele(mode, slot, vrc6.saw.step);
	save_slot_ele(mode, slot, vrc6.saw.internal);
	save_slot_ele(mode, slot, vrc6.saw.timer);
	save_slot_ele(mode, slot, vrc6.saw.frequency);
	save_slot_ele(mode, slot, vrc6.saw.output);

	save_slot_ele(mode, slot, vrc6.mode);
	save_slot_ele(mode, slot, vrc6.chr_map);

	if (mode == SAVE_SLOT_READ) {
		vrc6_update_chr_and_mirroring();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_VRC6(void) {
	if (vrc6.delay && !(--vrc6.delay)) {
		irq.high |= EXT_IRQ;
	}

	if (!vrc6.enabled) {
		return;
	}

	if (!vrc6.mode) {
		if (vrc6.prescaler < 338) {
			vrc6.prescaler += 3;
			return;
		}
		vrc6.prescaler -= 338;
	}

	if (vrc6.count != 0xFF) {
		vrc6.count++;
		return;
	}

	vrc6.count = vrc6.reload;
	vrc6.delay = vrc6tmp.delay;
}
void extcl_apu_tick_VRC6(void) {
	vcr6_square_tick(S3)
	vcr6_square_tick(S4)

	if (--vrc6.saw.timer == 0) {
		vrc6.saw.timer = vrc6.saw.frequency + 1;
		vrc6.clocked = TRUE;

		if (vrc6.saw.step && !(vrc6.saw.step & 0x01)) {
			vrc6.saw.internal += vrc6.saw.accumulator;
		}
		if (++vrc6.saw.step == 14) {
			vrc6.saw.internal = vrc6.saw.step = 0;
		}
		if (vrc6.saw.enabled) {
			vrc6.saw.output = vrc6.saw.internal;
		}
	}
}

INLINE static void vrc6_update_chr_and_mirroring(void) {
	DBWORD bank;
	BYTE value;

	// [$B003] & $07 →   0, 6, or 7 | 1 or 5 | 2, 3, or 4
	// Nametable bank             Register used
	// $2000-$23FF           R6        R4         R6
	// $2400-$27FF           R6        R5         R7
	// $2800-$2BFF           R7        R6         R6
	// $2C00-$2FFF           R7        R7         R7

	// [$B003] & $03 →   0 | 1 | 2 or 3
	//   CHR bank      Register used
	// $0000-$03FF      R0  R0    R0
	// $0400-$07FF      R1  R0    R1
	// $0800-$0BFF      R2  R1    R2
	// $0C00-$0FFF      R3  R1    R3
	// $1000-$13FF      R4  R2    R4
	// $1400-$17FF      R5  R2    R4
	// $1800-$1BFF      R6  R3    R5
	// $1C00-$1FFF      R7  R3    R5

	// Mirroring
	if (!(vrc6.b003 & 0x10)) {
		switch (vrc6.b003 & 0x2F) {
			case 0x20:
			case 0x27:
				mirroring_V();
				break;
			case 0x24:
			case 0x23:
				mirroring_H();
				break;
			case 0x28:
			case 0x2F:
				mirroring_SCR0();
				break;
			case 0x2C:
			case 0x2B:
				mirroring_SCR1();
				break;
			default:
				switch (vrc6.b003 & 0x07) {
					case 0x00:
					case 0x06:
					case 0x07:
						ntbl.bank_1k[0] = ntbl.bank_1k[1] = &ntbl.data[(vrc6.chr_map[6] & 0x01) << 10];
						ntbl.bank_1k[2] = ntbl.bank_1k[3] = &ntbl.data[(vrc6.chr_map[7] & 0x01) << 10];
						break;
					case 0x01:
					case 0x05:
						ntbl.bank_1k[0] = &ntbl.data[(vrc6.chr_map[4] & 0x01) << 10];
						ntbl.bank_1k[1] = &ntbl.data[(vrc6.chr_map[5] & 0x01) << 10];
						ntbl.bank_1k[2] = &ntbl.data[(vrc6.chr_map[6] & 0x01) << 10];
						ntbl.bank_1k[3] = &ntbl.data[(vrc6.chr_map[7] & 0x01) << 10];
						break;
					case 0x02:
					case 0x03:
					case 0x04:
						ntbl.bank_1k[0] = ntbl.bank_1k[2] = &ntbl.data[(vrc6.chr_map[6] & 0x01) << 10];
						ntbl.bank_1k[1] = ntbl.bank_1k[3] = &ntbl.data[(vrc6.chr_map[7] & 0x01) << 10];
						break;
				}
				break;
		}
	} else {
		switch (vrc6.b003 & 0x2F) {
			case 0x20:
			case 0x27:
				value = vrc6.chr_map[6] >> 1;
				control_bank(info.chr.rom.max.banks_2k)
				bank = value << 11;
				ntbl.bank_1k[0] = chr_pnt(bank);
				ntbl.bank_1k[1] = chr_pnt(bank | 0x400);

				value = vrc6.chr_map[7] >> 1;
				control_bank(info.chr.rom.max.banks_2k)
				bank = value << 11;
				ntbl.bank_1k[2] = chr_pnt(bank);
				ntbl.bank_1k[3] = chr_pnt(bank | 0x400);
				break;
			case 0x24:
			case 0x23:
				value = vrc6.chr_map[6] >> 1;
				control_bank(info.chr.rom.max.banks_2k)
				bank = value << 11;
				ntbl.bank_1k[0] = chr_pnt(bank);
				ntbl.bank_1k[2] = chr_pnt(bank | 0x400);

				value = vrc6.chr_map[7] >> 1;
				control_bank(info.chr.rom.max.banks_2k)
				bank = value << 11;
				ntbl.bank_1k[1] = chr_pnt(bank);
				ntbl.bank_1k[3] = chr_pnt(bank | 0x400);
				break;
			case 0x28:
			case 0x2F:
				value = vrc6.chr_map[6] >> 1;
				control_bank(info.chr.rom.max.banks_2k)
				bank = value << 11;
				ntbl.bank_1k[0] = ntbl.bank_1k[1] = chr_pnt(bank);

				value = vrc6.chr_map[7] >> 1;
				control_bank(info.chr.rom.max.banks_2k)
				bank = value << 11;
				ntbl.bank_1k[2] = ntbl.bank_1k[3] = chr_pnt(bank);
				break;
			case 0x2C:
			case 0x2B:
				value = vrc6.chr_map[6] >> 1;
				control_bank(info.chr.rom.max.banks_2k)
				bank = value << 11;
				ntbl.bank_1k[0] = ntbl.bank_1k[2] = chr_pnt(bank | 0x400);

				value = vrc6.chr_map[7] >> 1;
				control_bank(info.chr.rom.max.banks_2k)
				bank = value << 11;
				ntbl.bank_1k[1] = ntbl.bank_1k[3] = chr_pnt(bank | 0x400);
				break;
			default:
				switch (vrc6.b003 & 0x07) {
					case 0x00:
					case 0x06:
					case 0x07:
						value = vrc6.chr_map[6];
						control_bank(info.chr.rom.max.banks_1k)
						ntbl.bank_1k[0] = ntbl.bank_1k[1] = chr_pnt(value << 10);

						value = vrc6.chr_map[7];
						control_bank(info.chr.rom.max.banks_1k)
						ntbl.bank_1k[2] = ntbl.bank_1k[3] = chr_pnt(value << 10);
						break;
					case 0x01:
					case 0x05:
						value = vrc6.chr_map[4];
						control_bank(info.chr.rom.max.banks_1k)
						ntbl.bank_1k[0] = chr_pnt(value << 10);

						value = vrc6.chr_map[5];
						control_bank(info.chr.rom.max.banks_1k)
						ntbl.bank_1k[1] = chr_pnt(value << 10);

						value = vrc6.chr_map[6];
						control_bank(info.chr.rom.max.banks_1k)
						ntbl.bank_1k[2] = chr_pnt(value << 10);

						value = vrc6.chr_map[7];
						control_bank(info.chr.rom.max.banks_1k)
						ntbl.bank_1k[3] = chr_pnt(value << 10);
						break;
					case 0x02:
					case 0x03:
					case 0x04:
						value = vrc6.chr_map[6];
						control_bank(info.chr.rom.max.banks_1k)
						ntbl.bank_1k[0] = ntbl.bank_1k[2] = chr_pnt(value << 10);

						value = vrc6.chr_map[7];
						control_bank(info.chr.rom.max.banks_1k)
						ntbl.bank_1k[1] = ntbl.bank_1k[3] = chr_pnt(value << 10);
						break;
				}
				break;
			}
	}

	// [$B003] & $03 →  0 | 1 | 2 or 3
	//  CHR bank      Register used
	// $0000-$03FF     R0  R0    R0
	// $0400-$07FF     R1  R0    R1
	// $0800-$0BFF     R2  R1    R2
	// $0C00-$0FFF     R3  R1    R3
	// $1000-$13FF     R4  R2    R4
	// $1400-$17FF     R5  R2    R4
	// $1800-$1BFF     R6  R3    R5
	// $1C00-$1FFF     R7  R3    R5

	// CHR
	switch (vrc6.b003 & 0x03) {
		case 0:
			value = vrc6.chr_map[0];
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[0] = chr_pnt(value << 10);

			value = vrc6.chr_map[1];
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[1] = chr_pnt(value << 10);

			value = vrc6.chr_map[2];
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[2] = chr_pnt(value << 10);

			value = vrc6.chr_map[3];
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[3] = chr_pnt(value << 10);

			value = vrc6.chr_map[4];
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[4] = chr_pnt(value << 10);

			value = vrc6.chr_map[5];
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[5] = chr_pnt(value << 10);

			value = vrc6.chr_map[6];
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[6] = chr_pnt(value << 10);

			value = vrc6.chr_map[7];
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[7] = chr_pnt(value << 10);
			break;
		case 1:
			if (vrc6.b003 & 0x20) {
				value = vrc6.chr_map[0] >> 1;
				control_bank(info.chr.rom.max.banks_2k)
				bank = value << 11;
				chr.bank_1k[0] = chr_pnt(bank);
				chr.bank_1k[1] = chr_pnt(bank | 0x0400);

				value = vrc6.chr_map[1] >> 1;
				control_bank(info.chr.rom.max.banks_2k)
				bank = value << 11;
				chr.bank_1k[2] = chr_pnt(bank);
				chr.bank_1k[3] = chr_pnt(bank | 0x0400);

				value = vrc6.chr_map[2] >> 1;
				control_bank(info.chr.rom.max.banks_2k)
				bank = value << 11;
				chr.bank_1k[4] = chr_pnt(bank);
				chr.bank_1k[5] = chr_pnt(bank | 0x0400);

				value = vrc6.chr_map[3] >> 1;
				control_bank(info.chr.rom.max.banks_2k)
				bank = value << 11;
				chr.bank_1k[6] = chr_pnt(bank);
				chr.bank_1k[7] = chr_pnt(bank | 0x0400);
			} else {
				value = vrc6.chr_map[0];
				control_bank(info.chr.rom.max.banks_1k)
				bank = value << 10;
				chr.bank_1k[0] = chr.bank_1k[1] = chr_pnt(bank);

				value = vrc6.chr_map[1];
				control_bank(info.chr.rom.max.banks_1k)
				bank = value << 10;
				chr.bank_1k[2] = chr.bank_1k[3] = chr_pnt(bank);

				value = vrc6.chr_map[2];
				control_bank(info.chr.rom.max.banks_1k)
				bank = value << 10;
				chr.bank_1k[4] = chr.bank_1k[5] = chr_pnt(bank);

				value = vrc6.chr_map[3];
				control_bank(info.chr.rom.max.banks_1k)
				bank = value << 10;
				chr.bank_1k[6] = chr.bank_1k[7] = chr_pnt(bank);
			}
			break;
		case 2:
		case 3:
			value = vrc6.chr_map[0];
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[0] = chr_pnt(value << 10);

			value = vrc6.chr_map[1];
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[1] = chr_pnt(value << 10);

			value = vrc6.chr_map[2];
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[2] = chr_pnt(value << 10);

			value = vrc6.chr_map[3];
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[3] = chr_pnt(value << 10);

			if (vrc6.b003 & 0x20) {
				value = vrc6.chr_map[4] >> 1;
				control_bank(info.chr.rom.max.banks_2k)
				bank = value << 11;
				chr.bank_1k[4] = chr_pnt(bank);
				chr.bank_1k[5] = chr_pnt(bank | 0x0400);

				value = vrc6.chr_map[5] >> 1;
				control_bank(info.chr.rom.max.banks_2k)
				bank = value << 11;
				chr.bank_1k[6] = chr_pnt(bank);
				chr.bank_1k[7] = chr_pnt(bank | 0x0400);
			} else {
				value = vrc6.chr_map[4];
				control_bank(info.chr.rom.max.banks_1k)
				bank = value << 10;
				chr.bank_1k[4] = chr.bank_1k[5] = chr_pnt(bank);

				value = vrc6.chr_map[5];
				control_bank(info.chr.rom.max.banks_1k)
				bank = value << 10;
				chr.bank_1k[6] = chr.bank_1k[7] = chr_pnt(bank);
			}
			break;
	}
}

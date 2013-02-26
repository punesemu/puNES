/*
 * mapperSunsoft.c
 *
 *  Created on: 13/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

#define mirroring(data)\
	switch (data & 0x03) {\
		case 0:\
			mirroring_V();\
			break;\
		case 1:\
			mirroring_H();\
			break;\
		case 2:\
			mirroring_SCR0();\
			break;\
		case 3:\
			mirroring_SCR1();\
			break;\
		}
#define chrRom2kSwap(slot)\
{\
	DBWORD bank;\
	control_bank(chrRom2kMax)\
	bank = value << 11;\
	chr.bank_1k[slot] = &chr.data[bank];\
	chr.bank_1k[slot + 1] = &chr.data[bank | 0x400];\
}
#define sunsoftS4mirroring()\
	if (!s4.mode) {\
		switch (s4.mirroring & 0x03) {\
		case 0:\
			mirroring_V();\
			break;\
		case 1:\
			mirroring_H();\
			break;\
		case 2:\
			mirroring_SCR0();\
			break;\
		case 3:\
			mirroring_SCR1();\
			break;\
		}\
	} else {\
		switch (s4.mirroring & 0x03) {\
		case 0:\
			ntbl.bank_1k[0] = ntbl.bank_1k[2] = &chr.data[s4.chrNmt[0]];\
			ntbl.bank_1k[1] = ntbl.bank_1k[3] = &chr.data[s4.chrNmt[1]];\
			break;\
		case 1:\
			ntbl.bank_1k[0] = ntbl.bank_1k[1] = &chr.data[s4.chrNmt[0]];\
			ntbl.bank_1k[2] = ntbl.bank_1k[3] = &chr.data[s4.chrNmt[1]];\
			break;\
		case 2:\
			ntbl.bank_1k[0] = ntbl.bank_1k[1] = &chr.data[s4.chrNmt[0]];\
			ntbl.bank_1k[2] = ntbl.bank_1k[3] = &chr.data[s4.chrNmt[0]];\
			break;\
		case 3:\
			ntbl.bank_1k[0] = ntbl.bank_1k[1] = &chr.data[s4.chrNmt[1]];\
			ntbl.bank_1k[2] = ntbl.bank_1k[3] = &chr.data[s4.chrNmt[1]];\
			break;\
		}\
	}
#define fm7SquareTick(sq)\
	fm7.square[sq].output = 0;\
	if (--fm7.square[sq].timer == 0) {\
		fm7.square[sq].step = (fm7.square[sq].step + 1) & 0x1F;\
		fm7.square[sq].timer = fm7.square[sq].frequency + 1;\
		fm7.square[sq].clocked = TRUE;\
	}\
	if (!fm7.square[sq].disable) {\
		/*fm7.square[sq].output = -fm7.square[sq].volume * ((fm7.square[sq].step & 0x10) ? 2 : -2);*/\
		fm7.square[sq].output = fm7.square[sq].volume * ((fm7.square[sq].step & 0x10) ? 1 : 0);\
	}

WORD prgRom16kMax, prgRom8kMax, chrRom8kMax, chrRom4kMax, chrRom2kMax, chrRom1kMax;
BYTE type;

void map_init_Sunsoft(BYTE model) {
	prgRom16kMax = info.prg_rom_16k_count - 1;
	prgRom8kMax = info.prg_rom_8k_count - 1;
	chrRom8kMax = info.chr_rom_8k_count - 1;
	chrRom4kMax = info.chr_rom_4k_count - 1;
	chrRom2kMax = (info.chr_rom_1k_count >> 1) - 1;
	chrRom1kMax = info.chr_rom_1k_count - 1;

	switch (model) {
		case SUN1:
			EXTCL_CPU_WR_MEM(Sunsoft_S1);
			info.mapper_extend_wr = TRUE;
			break;
		case SUN2A:
		case SUN2B:
			EXTCL_CPU_WR_MEM(Sunsoft_S2);
			break;
		case SUN3:
			EXTCL_CPU_WR_MEM(Sunsoft_S3);
			EXTCL_SAVE_MAPPER(Sunsoft_S3);
			EXTCL_CPU_EVERY_CYCLE(Sunsoft_S3);
			mapper.internal_struct[0] = (BYTE *) &s3;
			mapper.internal_struct_size[0] = sizeof(s3);

			if (info.reset >= HARD) {
				memset(&s3, 0x00, sizeof(s3));
			}
			break;
		case SUN4:
			EXTCL_CPU_WR_MEM(Sunsoft_S4);
			EXTCL_SAVE_MAPPER(Sunsoft_S4);
			mapper.internal_struct[0] = (BYTE *) &s4;
			mapper.internal_struct_size[0] = sizeof(s4);

			if (info.reset >= HARD) {
				memset(&s4, 0x00, sizeof(s4));
				s4.chrNmt[0] = 0x80 << 10;
				s4.chrNmt[1] = 0x80 << 10;
			}

			if (info.id == MAHARAJA) {
				info.prg_ram_plus_8k_count = 1;
				info.prg_ram_bat_banks = 1;
			}
			break;
		case FM7:
			EXTCL_CPU_WR_MEM(Sunsoft_FM7);
			EXTCL_CPU_RD_MEM(Sunsoft_FM7);
			EXTCL_SAVE_MAPPER(Sunsoft_FM7);
			EXTCL_CPU_EVERY_CYCLE(Sunsoft_FM7);
			EXTCL_APU_TICK(Sunsoft_FM7);
			mapper.internal_struct[0] = (BYTE *) &fm7;
			mapper.internal_struct_size[0] = sizeof(fm7);

			if (info.reset >= HARD) {
				memset(&fm7, 0x00, sizeof(fm7));
			}

			info.prg_ram_plus_8k_count = 1;

			if ((info.id == BARCODEWORLD) || (info.id == DODGEDANPEI2)) {
				info.prg_ram_bat_banks = 1;
			}

			fm7.square[0].timer = 1;
			fm7.square[1].timer = 1;
			fm7.square[2].timer = 1;
			break;
	}

	type = model;
}

void extcl_cpu_wr_mem_Sunsoft_S1(WORD address, BYTE value) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return;
	}

	{
		const BYTE save = value;
		DBWORD bank;

		control_bank_with_AND(0x0F, chrRom4kMax)
		bank = value << 12;
		chr.bank_1k[0] = &chr.data[bank];
		chr.bank_1k[1] = &chr.data[bank | 0x0400];
		chr.bank_1k[2] = &chr.data[bank | 0x0800];
		chr.bank_1k[3] = &chr.data[bank | 0x0C00];

		value = save >> 4;
		control_bank(chrRom4kMax)
		bank = value << 12;
		chr.bank_1k[4] = &chr.data[bank];
		chr.bank_1k[5] = &chr.data[bank | 0x0400];
		chr.bank_1k[6] = &chr.data[bank | 0x0800];
		chr.bank_1k[7] = &chr.data[bank | 0x0C00];

	}
}

void extcl_cpu_wr_mem_Sunsoft_S2(WORD address, BYTE value) {
	const BYTE save = value;
	DBWORD bank;

	if (type == SUN2B) {
		if (value & 0x08) {
			mirroring_SCR1();
		} else {
			mirroring_SCR0();
		}
	}

	value = (save >> 4) & 0x07;
	control_bank(prgRom16kMax)
	map_prg_rom_8k(2, 0, value);
	map_prg_rom_8k_update();

	value = ((save & 0x80) >> 4) | (save & 0x07);
	control_bank(chrRom8kMax)
	bank = value << 13;
	chr.bank_1k[0] = &chr.data[bank];
	chr.bank_1k[1] = &chr.data[bank | 0x0400];
	chr.bank_1k[2] = &chr.data[bank | 0x0800];
	chr.bank_1k[3] = &chr.data[bank | 0x0C00];
	chr.bank_1k[4] = &chr.data[bank | 0x1000];
	chr.bank_1k[5] = &chr.data[bank | 0x1400];
	chr.bank_1k[6] = &chr.data[bank | 0x1800];
	chr.bank_1k[7] = &chr.data[bank | 0x1C00];
}

void extcl_cpu_wr_mem_Sunsoft_S3(WORD address, BYTE value) {
	switch (address & 0xF800) {
		case 0x8800:
			chrRom2kSwap(0)
			return;
		case 0x9800:
			chrRom2kSwap(2)
			return;
		case 0xA800:
			chrRom2kSwap(4)
			return;
		case 0xB800:
			chrRom2kSwap(6)
			return;
		case 0xC000:
		case 0xC800:
			if (s3.toggle ^= 1) {
				s3.count = (s3.count & 0x00FF) | (value << 8);
			} else {
				s3.count = (s3.count & 0xFF00) | value;
			}
			return;
		case 0xD800:
			s3.toggle = 0;
			s3.enable = value & 0x10;
			irq.high &= ~EXT_IRQ;
			return;
		case 0xE800:
			mirroring(value)
			return;
		case 0xF800:
			control_bank(prgRom16kMax)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k_update();
			return;
	}
}
BYTE extcl_save_mapper_Sunsoft_S3(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, s3.enable);
	save_slot_ele(mode, slot, s3.toggle);
	save_slot_ele(mode, slot, s3.count);
	save_slot_ele(mode, slot, s3.delay);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_Sunsoft_S3(void) {
	if (s3.delay && !(--s3.delay)) {
		irq.high |= EXT_IRQ;
	}

	if (s3.enable && s3.count && !(--s3.count)) {
		s3.enable = FALSE;
		s3.count = 0xFFFF;
		s3.delay = 1;
	}
}

void extcl_cpu_wr_mem_Sunsoft_S4(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
			chrRom2kSwap(0)
			return;
		case 0x9000:
			chrRom2kSwap(2)
			return;
		case 0xA000:
			chrRom2kSwap(4)
			return;
		case 0xB000:
			chrRom2kSwap(6)
			return;
		case 0xC000:
			s4.chrNmt[0] = (value | 0x80) << 10;
			sunsoftS4mirroring()
			return;
		case 0xD000:
			s4.chrNmt[1] = (value | 0x80) << 10;
			sunsoftS4mirroring()
			return;
		case 0xE000:
			s4.mode = value & 0x10;
			s4.mirroring = value & 0x03;
			sunsoftS4mirroring()
			return;
		case 0xF000:
			control_bank(prgRom16kMax)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k_update();
			return;
	}
}
BYTE extcl_save_mapper_Sunsoft_S4(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, s4.chrNmt);
	save_slot_ele(mode, slot, s4.mirroring);
	save_slot_ele(mode, slot, s4.mode);
	if ((mode == SAVE_SLOT_READ) && s4.mode) {
		sunsoftS4mirroring()
	}

	return (EXIT_OK);
}

void extcl_cpu_wr_mem_Sunsoft_FM7(WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x4000:
			if (cpu.prg_ram_wr_active) {
				return;
			}
			prg.ram[address & 0x1FFF] = value;
			return;
		case 0x6000:
			return;
		case 0x8000:
			fm7.address = value;
			return;
		case 0xA000: {
			const BYTE bank = fm7.address & 0x0F;

			switch (bank) {
				case 0x00:
				case 0x01:
				case 0x02:
				case 0x03:
				case 0x04:
				case 0x05:
				case 0x06:
				case 0x07:
					control_bank(chrRom1kMax)
					chr.bank_1k[bank] = &chr.data[value << 10];
					return;
				case 0x08: {
					fm7.prgRamMode = value & 0x40;
					fm7.prgRamEnable = value & 0x80;
					switch (value & 0xC0) {
						case 0x00:
						case 0x80:
							cpu.prg_ram_rd_active = TRUE;
							cpu.prg_ram_wr_active = FALSE;
							control_bank_with_AND(0x3F, prgRom8kMax)
							fm7.prgRamAddress = value << 13;
							prg.ram_plus_8k = &prg.rom[fm7.prgRamAddress];
							return;
						case 0x40:
							cpu.prg_ram_rd_active = FALSE;
							cpu.prg_ram_wr_active = TRUE;
							prg.ram_plus_8k = &prg.ram_plus[0];
							return;
						case 0xC0:
							cpu.prg_ram_rd_active = TRUE;
							cpu.prg_ram_wr_active = TRUE;
							prg.ram_plus_8k = &prg.ram_plus[0];
							return;
					}
					return;
				}
				case 0x09:
				case 0x0A:
				case 0x0B:
					control_bank(prgRom8kMax)
					map_prg_rom_8k(1, (bank & 0x03) - 1, value);
					map_prg_rom_8k_update();
					return;
				case 0x0C:
					mirroring(value)
					return;
				case 0x0D:
					fm7.irqEnableTrig = value & 0x01;
					fm7.irqEnableCount = value & 0x80;
					if (!fm7.irqEnableCount) {
						irq.high &= ~EXT_IRQ;
					}
					return;
				case 0x0E:
					fm7.irqCount = (fm7.irqCount & 0xFF00) | value;
					return;
				case 0x0F:
					fm7.irqCount = (fm7.irqCount & 0x00FF) | (value << 8);
					return;
			}
			return;
		}
		case 0xC000:
			fm7.sndReg = value & 0x0F;
			return;
		case 0xE000:
			switch (fm7.sndReg) {
				case 0x00:
				case 0x02:
				case 0x04: {
					BYTE index = fm7.sndReg >> 1;
					fm7.square[index].frequency = (fm7.square[index].frequency & 0x0F00) | value;
					return;
				}
				case 0x01:
				case 0x03:
				case 0x05: {
					BYTE index = fm7.sndReg >> 1;
					fm7.square[index].frequency = (fm7.square[index].frequency & 0x00FF)
					        | ((value & 0x0F) << 8);
					return;
				}
				case 0x07:
					fm7.square[0].disable = value & 0x01;
					fm7.square[1].disable = value & 0x02;
					fm7.square[2].disable = value & 0x04;
					return;
				case 0x08:
				case 0x09:
				case 0x0A: {
					BYTE index = fm7.sndReg & 0x03;
					fm7.square[index].volume = value & 0x0F;
					return;
				}

			}
			return;
	}
}
BYTE extcl_cpu_rd_mem_Sunsoft_FM7(WORD address, BYTE openbus, BYTE before) {
	if (fm7.prgRamEnable) {
		return (openbus);
	}

	if (address < 0x6000) {
		return (prg.ram[address & 0x1FFF]);
	}

	return (openbus);
}
BYTE extcl_save_mapper_Sunsoft_FM7(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, fm7.address);
	save_slot_ele(mode, slot, fm7.prgRamEnable);
	save_slot_ele(mode, slot, fm7.prgRamMode);
	save_slot_ele(mode, slot, fm7.prgRamAddress);
	if ((mode == SAVE_SLOT_READ) && !fm7.prgRamMode) {
		prg.ram_plus_8k = &prg.rom[fm7.prgRamAddress];
	}
	save_slot_ele(mode, slot, fm7.irqEnableTrig);
	save_slot_ele(mode, slot, fm7.irqEnableCount);
	save_slot_ele(mode, slot, fm7.irqCount);
	save_slot_ele(mode, slot, fm7.irqDelay);

	/*
	 * nelle versioni 1 e 2 dei files di save non salvavo
	 * i dati delle snd square perche' non avevo ancora
	 * implementato la loro emulazione.
	 */
	if (save_slot.version > 2) {
		BYTE i;

		for (i = 0; i < LENGTH(fm7.square); i++) {
			save_slot_ele(mode, slot, fm7.square[i].disable);
			save_slot_ele(mode, slot, fm7.square[i].step);
			save_slot_ele(mode, slot, fm7.square[i].frequency);
			save_slot_ele(mode, slot, fm7.square[i].timer);
			save_slot_ele(mode, slot, fm7.square[i].volume);
			save_slot_ele(mode, slot, fm7.square[i].output);
		}
	}
	return (EXIT_OK);
}
void extcl_cpu_every_cycle_Sunsoft_FM7(void) {
	if (fm7.irqDelay && !(--fm7.irqDelay)) {
		irq.high |= EXT_IRQ;
	}

	if (!fm7.irqEnableCount) {
		return;
	}

	if (!(--fm7.irqCount) && fm7.irqEnableTrig) {
		fm7.irqDelay = 1;
	}
}
void extcl_apu_tick_Sunsoft_FM7(void) {
	fm7SquareTick(0)
	fm7SquareTick(1)
	fm7SquareTick(2)
}

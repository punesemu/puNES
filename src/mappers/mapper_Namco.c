/*
 * mapper_Namco.c
 *
 *  Created on: 13/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "cpu.h"
#include "apu.h"
#include "save_slot.h"

#define n163_prg_rom_8k_update(slot)\
	control_bank_with_AND(0x3F, info.prg.rom.max.banks_8k)\
	map_prg_rom_8k(1, slot, value);\
	map_prg_rom_8k_update()
#define _n163_nmt_update(slot)\
	ntbl.bank_1k[slot] = chr_chip_byte_pnt(0, n163.nmt_bank[slot][1])
#define n163_nmt_update(slot)\
	if (hardwired) {\
		return;\
	}\
	if (value >= 0xE0) {\
		n163.nmt_bank[slot][0] = FALSE;\
		ntbl.bank_1k[slot] = &ntbl.data[(value & 0x03) << 10];\
	} else {\
		control_bank(info.chr.rom.max.banks_1k)\
		n163.nmt_bank[slot][0] = TRUE;\
		n163.nmt_bank[slot][1] = value << 10;\
		_n163_nmt_update(slot);\
	}

#define _n163_ch_freq(operation, channel)\
{\
	const DBWORD freq = operation;\
	if (n163.ch[channel].freq != freq) {\
		n163.ch[channel].freq = freq;\
		if (n163.ch[channel].freq) {\
			n163.ch[channel].cycles_reload = (0xF0000 * (8 - n163.snd_ch_start))\
				/ n163.ch[channel].freq;\
			n163.ch[channel].cycles = n163.ch[channel].cycles_reload;\
		}\
	}\
}
#define n163_ch_freq_high(channel)\
	_n163_ch_freq((n163.ch[channel].freq & 0x0FFFF) | ((value & 0x03) << 16), channel)
#define n163_ch_freq_middle(channel)\
	_n163_ch_freq((n163.ch[channel].freq & 0x300FF) | (value << 8), channel)
#define n163_ch_freq_low(channel)\
	_n163_ch_freq((n163.ch[channel].freq & 0x3FF00) | value, channel)

#define n3425_nmt_update()\
	if (type == N3425) {\
		ntbl.bank_1k[n3425.bank_to_update >> 1] = &ntbl.data[((value >> 5) & 0x01) << 10];\
	}

BYTE hardwired, type;

void map_init_Namco(BYTE model) {
	switch (model) {
		case N163:
			EXTCL_CPU_WR_MEM(Namco_163);
			EXTCL_CPU_RD_MEM(Namco_163);
			EXTCL_SAVE_MAPPER(Namco_163);
			EXTCL_CPU_EVERY_CYCLE(Namco_163);
			EXTCL_APU_TICK(Namco_163);
			mapper.internal_struct[0] = (BYTE *) &n163;
			mapper.internal_struct_size[0] = sizeof(n163);

			if (info.reset >= HARD) {
				memset(&n163, 0x00, sizeof(n163));
				n163.snd_ch_start = 7;
				n163.snd_auto_inc = 1;
			} else {
				memset(&n163.ch, 0x00, sizeof(n163.ch));
				memset(&n163.snd_ram, 0x00, sizeof(n163.snd_ram));
				memset(&n163.snd_wave, 0x00, sizeof(n163.snd_wave));
				n163.irq_delay = FALSE;
				n163.snd_ch_start = 7;
				n163.snd_adr = 0;
				n163.snd_auto_inc = 1;
			}

			info.mapper.extend_wr = TRUE;
			hardwired = FALSE;

			switch (info.id) {
				case NAMCO_HARD_WIRED_V:
					hardwired = TRUE;
					mirroring_V();
					break;
				case NAMCO_HARD_WIRED_H:
					hardwired = TRUE;
					mirroring_H();
					break;
				case MINDSEEKER:
					info.prg.ram.banks_8k_plus = 1;
					info.prg.ram.bat.banks = 1;
					break;
			}
			break;
		case N3416:
		case N3425:
		case N3433:
		case N3453:
			EXTCL_CPU_WR_MEM(Namco_3425);
			EXTCL_SAVE_MAPPER(Namco_3425);
			mapper.internal_struct[0] = (BYTE *) &n3425;
			mapper.internal_struct_size[0] = sizeof(n3425);

			if (info.reset >= HARD) {
				memset(&n3425, 0x00, sizeof(n3425));
			}

			if (model == N3453) {
				mirroring_SCR0();
			}
			break;
		case N3446:
			EXTCL_CPU_WR_MEM(Namco_3446);
			EXTCL_SAVE_MAPPER(Namco_3446);
			mapper.internal_struct[0] = (BYTE *) &n3446;
			mapper.internal_struct_size[0] = sizeof(n3446);

			if (info.reset >= HARD) {
				memset(&n3446, 0x00, sizeof(n3446));
			}

			mapper.rom_map_to[2] = info.prg.rom.max.banks_8k - 1;
			break;
	}

	type = model;
}

void extcl_cpu_wr_mem_Namco_163(WORD address, BYTE value) {
	if (address < 0x4800) {
		return;
	}

	switch (address & 0xF800) {
		case 0x4800: {
			const BYTE index = address << 1;

			n163.snd_ram[n163.snd_adr] = value;

			{
				BYTE a;

				/* taglio le frequenze troppo basse */
				n163.snd_wave[index] = ((a = (value & 0x0F)) < 0x08 ? 0x08 : a);
				n163.snd_wave[index + 1] = ((a = (value & 0xF0)) < 0x80 ? 0x08 : a >> 4);
			}

			if (n163.snd_adr >= 0x40) {
				const BYTE chan = (n163.snd_adr - 0x40) >> 3;

				n163.ch[chan].active = FALSE;

				switch (n163.snd_adr & 0x7) {
					case 0x00:
						n163_ch_freq_low(chan);
						break;
					case 0x02:
						n163_ch_freq_middle(chan);
						break;
					case 0x04: {
						const BYTE length = (8 - ((value >> 2) & 0x07)) << 2;

						n163_ch_freq_high(chan);

						if (n163.ch[chan].length != length) {
							n163.ch[chan].length = length;
							n163.ch[chan].step = 0;
						}
						break;
					}
					case 0x06:
						n163.ch[chan].address = value;
						break;
					case 0x07:
						n163.ch[chan].volume = value & 0x0F;
						if (chan == 7) {
							BYTE i;

							value = (value >> 4) & 0x07;
							n163.snd_ch_start = 7 - value;
							for (i = 0; i < 8; i++) {
								n163.ch[i].enabled = TRUE;
								if (i < n163.snd_ch_start) {
									n163.ch[i].enabled = FALSE;
								}
							}
						}
						break;
				}
				/* se sono vere queste condizioni allora il canale e' attivo */
				if (n163.ch[chan].enabled && n163.ch[chan].freq && n163.ch[chan].volume) {
					n163.ch[chan].active = TRUE;
				}
			}
			n163.snd_adr = (n163.snd_adr + n163.snd_auto_inc) & 0x07F;
			return;
		}
		case 0x8000:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[0] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0x8800:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[1] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0x9000:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[2] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0x9800:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[3] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0xA000:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[4] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0xA800:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[5] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0xB000:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[6] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0xB800:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[7] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0xC000:
			n163_nmt_update(0)
			return;
		case 0xC800:
			n163_nmt_update(1)
			return;
		case 0xD000:
			n163_nmt_update(2)
			return;
		case 0xD800:
			n163_nmt_update(3)
			return;
		case 0xE000:
			n163_prg_rom_8k_update(0);
			return;
		case 0xE800:
			n163_prg_rom_8k_update(1);
			return;
		case 0xF000:
			n163_prg_rom_8k_update(2);
			return;
		case 0x5000:
			n163.irq_count = (n163.irq_count & 0xFF00) | value;
			irq.high &= ~EXT_IRQ;
			return;
		case 0x5800:
			n163.irq_count = (value << 8) | (n163.irq_count & 0x00FF);
			irq.high &= ~EXT_IRQ;
			return;
		case 0xF800:
			n163.snd_auto_inc = (value & 0x80) >> 7;
			n163.snd_adr = value & 0x7F;
			return;
	}
}
BYTE extcl_cpu_rd_mem_Namco_163(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x4800) || (address >= 0x6000)) {
		return (openbus);
	}

	switch (address & 0xF800) {
		case 0x4800:
			openbus = n163.snd_ram[n163.snd_adr];
			n163.snd_adr = (n163.snd_adr + n163.snd_auto_inc) & 0x07F;
			return (openbus);
		case 0x5000:
			return (n163.irq_count & 0xFF);
		case 0x5800:
			return ((n163.irq_count >> 8) & 0xFF);
		default:
			return (openbus);
	}
}
BYTE extcl_save_mapper_Namco_163(BYTE mode, BYTE slot, FILE *fp) {
	BYTE i;

	for (i = 0; i < LENGTH(n163.nmt_bank); i++) {
		save_slot_int(mode, slot, n163.nmt_bank[i][0]);
		save_slot_int(mode, slot, n163.nmt_bank[i][1]);
		if ((mode == SAVE_SLOT_READ) && (n163.nmt_bank[i][0])) {
			_n163_nmt_update(i);
		}
	}
	save_slot_ele(mode, slot, n163.irq_delay);
	save_slot_ele(mode, slot, n163.irq_count);
	save_slot_ele(mode, slot, n163.snd_ram);
	save_slot_ele(mode, slot, n163.snd_adr);
	save_slot_ele(mode, slot, n163.snd_auto_inc);
	save_slot_ele(mode, slot, n163.snd_ch_start);
	save_slot_ele(mode, slot, n163.snd_wave);
	for (i = 0; i < LENGTH(n163.ch); i++) {
		save_slot_ele(mode, slot, n163.ch[i].enabled);
		save_slot_ele(mode, slot, n163.ch[i].active);
		save_slot_ele(mode, slot, n163.ch[i].address);
		save_slot_ele(mode, slot, n163.ch[i].freq);
		save_slot_ele(mode, slot, n163.ch[i].cycles_reload);
		save_slot_ele(mode, slot, n163.ch[i].cycles);
		save_slot_ele(mode, slot, n163.ch[i].length);
		save_slot_ele(mode, slot, n163.ch[i].step);
		save_slot_ele(mode, slot, n163.ch[i].volume);
		save_slot_ele(mode, slot, n163.ch[i].output);
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_Namco_163(void) {
	if (n163.irq_delay) {
		n163.irq_delay = FALSE;
		irq.high |= EXT_IRQ;
	}
	if (((n163.irq_count - 0x8000) < 0x7FFF) && (++n163.irq_count == 0xFFFF)) {
		/* vale sempre il solito discorso di un ciclo di delay */
		n163.irq_delay = TRUE;
	}
}
void extcl_apu_tick_Namco_163(void) {
	BYTE i;

	for (i = n163.snd_ch_start; i < 8; i++) {
		if ((n163.ch[i].active) && !(--n163.ch[i].cycles)) {

			n163.ch[i].output = n163.snd_wave[(n163.ch[i].address + n163.ch[i].step) & 0xFF];

			n163.ch[i].cycles = n163.ch[i].cycles_reload;

			if (++n163.ch[i].step == n163.ch[i].length) {
				n163.ch[i].step = 0;
			}
		}
	}
}

void extcl_cpu_wr_mem_Namco_3425(WORD address, BYTE value) {
	switch (address & 0x8001) {
		case 0x8000:
			n3425.bank_to_update = value & 0x7;
			if (type == N3453) {
				if (value & 0x40) {
					mirroring_SCR1();
				} else {
					mirroring_SCR0();
				}
			}
			return;
		case 0x8001: {
			switch (n3425.bank_to_update) {
				case 0x00:
				case 0x01: {
					const BYTE slot = n3425.bank_to_update << 1;
					DBWORD bank;

					n3425_nmt_update()
					value >>= 1;
					control_bank(info.chr.rom.max.banks_2k)
					bank = value << 11;
					chr.bank_1k[slot] = chr_chip_byte_pnt(0, bank);
					chr.bank_1k[slot | 0x01] = chr_chip_byte_pnt(0, bank | 0x400);
					return;
				}
				case 0x02:
				case 0x03:
				case 0x04:
				case 0x05:
					n3425_nmt_update()
					value |= 0x40;
					control_bank(info.chr.rom.max.banks_1k)
					chr.bank_1k[n3425.bank_to_update + 2] = chr_chip_byte_pnt(0, value << 10);
					return;
				case 0x06:
					control_bank(info.prg.rom.max.banks_8k)
					map_prg_rom_8k(1, 0, value);
					map_prg_rom_8k_update();
					return;
				case 0x07:
					control_bank(info.prg.rom.max.banks_8k)
					map_prg_rom_8k(1, 1, value);
					map_prg_rom_8k_update();
					return;
			}
			return;
		}
	}
}
BYTE extcl_save_mapper_Namco_3425(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, n3425.bank_to_update);

	return (EXIT_OK);
}

void extcl_cpu_wr_mem_Namco_3446(WORD address, BYTE value) {
	switch (address & 0x8001) {
		case 0x8000:
			n3446.bank_to_update = value & 0x7;
			n3446.prg_rom_mode = (value & 0x40) >> 5;
			return;
		case 0x8001: {
			switch (n3446.bank_to_update) {
				case 0x02:
				case 0x03:
				case 0x04:
				case 0x05: {
					const BYTE slot = (n3446.bank_to_update - 2) << 1;
					DBWORD bank;

					control_bank(info.chr.rom.max.banks_2k)
					bank = value << 11;
					chr.bank_1k[slot] = chr_chip_byte_pnt(0, bank);
					chr.bank_1k[slot | 0x01] = chr_chip_byte_pnt(0, bank | 0x400);
					return;
				}
				case 0x06:
					control_bank(info.prg.rom.max.banks_8k)
					map_prg_rom_8k(1, n3446.prg_rom_mode, value);
					map_prg_rom_8k_update();
					return;
				case 0x07:
					control_bank(info.prg.rom.max.banks_8k)
					map_prg_rom_8k(1, 1, value);
					map_prg_rom_8k_update();
					return;
			}
			return;
		}
	}
}
BYTE extcl_save_mapper_Namco_3446(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, n3446.bank_to_update);
	save_slot_ele(mode, slot, n3446.prg_rom_mode);

	return (EXIT_OK);
}

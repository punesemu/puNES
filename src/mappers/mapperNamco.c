/*
 * mapperNamco.c
 *
 *  Created on: 13/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "cpu.h"
#include "apu.h"
#include "savestate.h"

#define namco163PrgRom8kUpdate(slot)\
	control_bank_with_AND(0x3F, prgRom8kMax)\
	map_prg_rom_8k(1, slot, value);\
	map_prg_rom_8k_update()
#define namco163NmtUpdate(slot)\
	if (hardwired) { return; }\
	if (value >= 0xE0) {\
		n163.nmtBank[slot][0] = FALSE;\
		ntbl.bank_1k[slot] = &ntbl.data[(value & 0x03) << 10];\
	} else {\
		control_bank(chrRom1kMax)\
		n163.nmtBank[slot][0] = TRUE;\
		n163.nmtBank[slot][1] = value << 10;\
		_namco163NmtUpdate(slot);\
	}
#define _namco163NmtUpdate(slot)\
	ntbl.bank_1k[slot] = &chr.data[n163.nmtBank[slot][1]]
#define n163ChFreqHigh(channel)\
	_n163ChFreq((n163.ch[channel].freq & 0x0FFFF) | ((value & 0x03) << 16), channel)
#define n163ChFreqMiddle(channel)\
	_n163ChFreq((n163.ch[channel].freq & 0x300FF) | (value << 8), channel)
#define n163ChFreqLow(channel)\
	_n163ChFreq((n163.ch[channel].freq & 0x3FF00) | value, channel)
#define _n163ChFreq(operation, channel)\
{\
	const DBWORD freq = operation;\
	if (n163.ch[channel].freq != freq) {\
		n163.ch[channel].freq = freq;\
		if (n163.ch[channel].freq) {\
			n163.ch[channel].cyclesReload = (0xF0000 * (8 - n163.sndChStart))\
				/ n163.ch[channel].freq;\
			n163.ch[channel].cycles = n163.ch[channel].cyclesReload;\
		}\
	}\
}

#define n3425NmtUpdate()\
	if (type == N3425) {\
		ntbl.bank_1k[n3425.bankToUpdate >> 1] = \
			&ntbl.data[((value >> 5) & 0x01) << 10];\
	}

WORD prgRom8kMax, chrRom2kMax, chrRom1kMax;
BYTE hardwired, type;

void map_init_Namco(BYTE model) {
	prgRom8kMax = info.prg_rom_8k_count - 1;
	chrRom2kMax = (info.chr_rom_1k_count >> 1) - 1;
	chrRom1kMax = info.chr_rom_1k_count - 1;

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
				n163.sndChStart = 7;
				n163.sndAutoInc = 1;
			} else {
				memset(&n163.ch, 0x00, sizeof(n163.ch));
				memset(&n163.sndRam, 0x00, sizeof(n163.sndRam));
				memset(&n163.sndWave, 0x00, sizeof(n163.sndWave));
				n163.irqDelay = FALSE;
				n163.sndChStart = 7;
				n163.sndAdr = 0;
				n163.sndAutoInc = 1;
			}

			info.mapper_extend_wr = TRUE;
			hardwired = FALSE;

			switch (info.id) {
				case NHARDWIREDV:
					hardwired = TRUE;
					mirroring_V();
					break;
				case NHARDWIREDH:
					hardwired = TRUE;
					mirroring_H();
					break;
				case MINDSEEKER:
					info.prg_ram_plus_8k_count = 1;
					info.prg_ram_bat_banks = 1;
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

			mapper.rom_map_to[2] = prgRom8kMax - 1;
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

			n163.sndRam[n163.sndAdr] = value;

			{
				BYTE a;

				/* taglio le frequenze troppo basse */
				n163.sndWave[index] = ((a = (value & 0x0F)) < 0x08 ? 0x08 : a);
				n163.sndWave[index + 1] = ((a = (value & 0xF0)) < 0x80 ? 0x08 : a >> 4);
			}

			if (n163.sndAdr >= 0x40) {
				const BYTE chan = (n163.sndAdr - 0x40) >> 3;

				n163.ch[chan].active = FALSE;

				switch (n163.sndAdr & 0x7) {
					case 0x00:
						n163ChFreqLow(chan);
						break;
					case 0x02:
						n163ChFreqMiddle(chan);
						break;
					case 0x04: {
						const BYTE length = (8 - ((value >> 2) & 0x07)) << 2;

						n163ChFreqHigh(chan);

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
							n163.sndChStart = 7 - value;
							for (i = 0; i < 8; i++) {
								n163.ch[i].enabled = TRUE;
								if (i < n163.sndChStart) {
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
			n163.sndAdr = (n163.sndAdr + n163.sndAutoInc) & 0x07F;
			return;
		}
		case 0x8000:
			control_bank(chrRom1kMax)
			chr.bank_1k[0] = &chr.data[value << 10];
			return;
		case 0x8800:
			control_bank(chrRom1kMax)
			chr.bank_1k[1] = &chr.data[value << 10];
			return;
		case 0x9000:
			control_bank(chrRom1kMax)
			chr.bank_1k[2] = &chr.data[value << 10];
			return;
		case 0x9800:
			control_bank(chrRom1kMax)
			chr.bank_1k[3] = &chr.data[value << 10];
			return;
		case 0xA000:
			control_bank(chrRom1kMax)
			chr.bank_1k[4] = &chr.data[value << 10];
			return;
		case 0xA800:
			control_bank(chrRom1kMax)
			chr.bank_1k[5] = &chr.data[value << 10];
			return;
		case 0xB000:
			control_bank(chrRom1kMax)
			chr.bank_1k[6] = &chr.data[value << 10];
			return;
		case 0xB800:
			control_bank(chrRom1kMax)
			chr.bank_1k[7] = &chr.data[value << 10];
			return;
		case 0xC000:
			namco163NmtUpdate(0)
			return;
		case 0xC800:
			namco163NmtUpdate(1)
			return;
		case 0xD000:
			namco163NmtUpdate(2)
			return;
		case 0xD800:
			namco163NmtUpdate(3)
			return;
		case 0xE000:
			namco163PrgRom8kUpdate(0);
			return;
		case 0xE800:
			namco163PrgRom8kUpdate(1);
			return;
		case 0xF000:
			namco163PrgRom8kUpdate(2);
			return;
		case 0x5000:
			n163.irqCount = (n163.irqCount & 0xFF00) | value;
			irq.high &= ~EXTIRQ;
			return;
		case 0x5800:
			n163.irqCount = (value << 8) | (n163.irqCount & 0x00FF);
			irq.high &= ~EXTIRQ;
			return;
		case 0xF800:
			n163.sndAutoInc = (value & 0x80) >> 7;
			n163.sndAdr = value & 0x7F;
			return;
	}
}
BYTE extcl_cpu_rd_mem_Namco_163(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x4800) || (address >= 0x6000)) {
		return (openbus);
	}

	switch (address & 0xF800) {
		case 0x4800:
			openbus = n163.sndRam[n163.sndAdr];
			n163.sndAdr = (n163.sndAdr + n163.sndAutoInc) & 0x07F;
			return (openbus);
		case 0x5000:
			return (n163.irqCount & 0xFF);
		case 0x5800:
			return ((n163.irqCount >> 8) & 0xFF);
		default:
			return (openbus);
	}
}
BYTE extcl_save_mapper_Namco_163(BYTE mode, BYTE slot, FILE *fp) {
	BYTE i;

	for (i = 0; i < LENGTH(n163.nmtBank); i++) {
		savestateInt(mode, slot, n163.nmtBank[i][0]);
		savestateInt(mode, slot, n163.nmtBank[i][1]);
		if ((mode == SSREAD) && (n163.nmtBank[i][0])) {
			_namco163NmtUpdate(i);
		}
	}
	savestateEle(mode, slot, n163.irqDelay);
	savestateEle(mode, slot, n163.irqCount);
	savestateEle(mode, slot, n163.sndRam);
	savestateEle(mode, slot, n163.sndAdr);
	savestateEle(mode, slot, n163.sndAutoInc);
	savestateEle(mode, slot, n163.sndChStart);
	savestateEle(mode, slot, n163.sndWave);
	for (i = 0; i < LENGTH(n163.ch); i++) {
		savestateEle(mode, slot, n163.ch[i].enabled);
		savestateEle(mode, slot, n163.ch[i].active);
		savestateEle(mode, slot, n163.ch[i].address);
		savestateEle(mode, slot, n163.ch[i].freq);
		savestateEle(mode, slot, n163.ch[i].cyclesReload);
		savestateEle(mode, slot, n163.ch[i].cycles);
		savestateEle(mode, slot, n163.ch[i].length);
		savestateEle(mode, slot, n163.ch[i].step);
		savestateEle(mode, slot, n163.ch[i].volume);
		savestateEle(mode, slot, n163.ch[i].output);
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_Namco_163(void) {
	if (n163.irqDelay) {
		n163.irqDelay = FALSE;
		irq.high |= EXTIRQ;
	}
	if (((n163.irqCount - 0x8000) < 0x7FFF) && (++n163.irqCount == 0xFFFF)) {
		/* vale sempre il solito discorso di un ciclo di delay */
		n163.irqDelay = TRUE;
	}
}
void extcl_apu_tick_Namco_163(void) {
	BYTE i;

	for (i = n163.sndChStart; i < 8; i++) {
		if ((n163.ch[i].active) && !(--n163.ch[i].cycles)) {

			n163.ch[i].output = n163.sndWave[(n163.ch[i].address + n163.ch[i].step) & 0xFF];

			n163.ch[i].cycles = n163.ch[i].cyclesReload;

			if (++n163.ch[i].step == n163.ch[i].length) {
				n163.ch[i].step = 0;
			}
		}
	}
}

void extcl_cpu_wr_mem_Namco_3425(WORD address, BYTE value) {
	switch (address & 0x8001) {
		case 0x8000:
			n3425.bankToUpdate = value & 0x7;
			if (type == N3453) {
				if (value & 0x40) {
					mirroring_SCR1();
				} else {
					mirroring_SCR0();
				}
			}
			return;
		case 0x8001: {
			switch (n3425.bankToUpdate) {
				case 0x00:
				case 0x01: {
					const BYTE slot = n3425.bankToUpdate << 1;
					DBWORD bank;

					n3425NmtUpdate()
					value >>= 1;
					control_bank(chrRom2kMax)
					bank = value << 11;
					chr.bank_1k[slot] = &chr.data[bank];
					chr.bank_1k[slot | 0x01] = &chr.data[bank | 0x400];
					return;
				}
				case 0x02:
				case 0x03:
				case 0x04:
				case 0x05:
					n3425NmtUpdate()
					value |= 0x40;
					control_bank(chrRom1kMax)
					chr.bank_1k[n3425.bankToUpdate + 2] = &chr.data[value << 10];
					return;
				case 0x06:
					control_bank(prgRom8kMax)
					map_prg_rom_8k(1, 0, value);
					map_prg_rom_8k_update();
					return;
				case 0x07:
					control_bank(prgRom8kMax)
					map_prg_rom_8k(1, 1, value);
					map_prg_rom_8k_update();
					return;
			}
			return;
		}
	}
}
BYTE extcl_save_mapper_Namco_3425(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, n3425.bankToUpdate);

	return (EXIT_OK);
}

void extcl_cpu_wr_mem_Namco_3446(WORD address, BYTE value) {
	switch (address & 0x8001) {
		case 0x8000:
			n3446.bankToUpdate = value & 0x7;
			n3446.prgRomMode = (value & 0x40) >> 5;
			return;
		case 0x8001: {
			switch (n3446.bankToUpdate) {
				case 0x02:
				case 0x03:
				case 0x04:
				case 0x05: {
					const BYTE slot = (n3446.bankToUpdate - 2) << 1;
					DBWORD bank;

					control_bank(chrRom2kMax)
					bank = value << 11;
					chr.bank_1k[slot] = &chr.data[bank];
					chr.bank_1k[slot | 0x01] = &chr.data[bank | 0x400];
					return;
				}
				case 0x06:
					control_bank(prgRom8kMax)
					map_prg_rom_8k(1, n3446.prgRomMode, value);
					map_prg_rom_8k_update();
					return;
				case 0x07:
					control_bank(prgRom8kMax)
					map_prg_rom_8k(1, 1, value);
					map_prg_rom_8k_update();
					return;
			}
			return;
		}
	}
}
BYTE extcl_save_mapper_Namco_3446(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, n3446.bankToUpdate);
	savestateEle(mode, slot, n3446.prgRomMode);

	return (EXIT_OK);
}

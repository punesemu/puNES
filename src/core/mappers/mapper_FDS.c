/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

/*
 *  Many thanks to Sour (author of Mesen) for the auto insert logic.
 *  https://github.com/SourMesen/Mesen/blob/master/Core/FDS.cpp
 */

#include <string.h>
#include "mappers.h"
#include "info.h"
#include "fds.h"
#include "conf.h"
#include "gui.h"

static const SBYTE modulation_table[8] = { 0, 1, 2, 4, 8, -4, -2, -1 };
static const BYTE volume_wave[4] = { 36, 24, 17, 14 };

void map_init_FDS(void) {
	EXTCL_AFTER_MAPPER_INIT(FDS);
	EXTCL_CPU_RD_MEM(FDS);
	EXTCL_CPU_EVERY_CYCLE(FDS);
	EXTCL_APU_TICK(FDS);

	fds.auto_insert.r4032.checks = 0;
	fds.auto_insert.r4032.frames = 0;
	fds.auto_insert.delay.dummy = 0;
	fds.auto_insert.rE445.in_run = FALSE;
	fds.auto_insert.in_game = FALSE;

	fds.drive.mirroring = 0x08;
	fds.drive.io_mode = 0x04;
	fds.drive.drive_ready = 0x40;

	if (cfg->fds_disk1sideA_at_reset) {
		fds_disk_op(FDS_DISK_EJECT, 0, TRUE);
		if (fds.drive.side_inserted != 0) {
			fds_disk_op(FDS_DISK_SELECT_AND_INSERT, 0, FALSE);
		} else {
			fds_disk_op(FDS_DISK_INSERT, 0, TRUE);
		}
	}

	nes[0].c.cpu.SP = 0xFF;
	nes[0].c.cpu.SR = 0x30;
	// disassemblo il Processor Status Register
	disassemble_SR(0);
	// setto il flag di disabilitazione dell'irq
	nes[0].c.irq.inhibit = nes[0].c.cpu.im;
}
void map_init_NSF_FDS(void) {
	memset(&fds, 0x00, sizeof(fds));

	fds.snd.modulation.counter = 0xFFFF;
	fds.snd.wave.counter = 0xFFFF;

	fds.drive.enabled_snd_reg = 0x02;
}
void extcl_after_mapper_init_FDS(void) {
	memmap_wram_8k(0, MMCPU(0x6000), 0);
	memmap_wram_8k(0, MMCPU(0x8000), 1);
	memmap_wram_8k(0, MMCPU(0xA000), 2);
	memmap_wram_8k(0, MMCPU(0xC000), 3);
	memmap_prgrom_8k(0, MMCPU(0xE000), 0);
	if (fds.drive.mirroring) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
BYTE extcl_cpu_rd_mem_FDS(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	switch (address) {
		case 0xE188:
			// 0xE18B : NMI entry point
			// [$0100]: PC action on NMI. set to $C0 on reset
			// When NMI occurs while $100 & $C0 != 0, it typically means that the game is starting.
			if (!fds.auto_insert.in_game & ((cpu_rd_mem_dbg(nidx, 0x100) & 0xC0) != 0)) {
				fds.auto_insert.in_game = TRUE;
			}
			break;
		case 0xE445:
			// Address          : 0xE445
			// Name             : CheckDiskHeader
			// Input parameters : Pointer to 10 byte string at $00
			// Description      : Compares the first 10 bytes on the disk coming after the FDS string, to 10
			//                    bytes pointed to by Ptr($00). To bypass the checking of any byte, a -1 can be placed in the
			//                    equivelant place in the compare string. Otherwise, if the comparison fails, an appropriate error
			//                    will be generated.
			if (fds_auto_insert_enabled() & !fds.auto_insert.rE445.in_run) {
				WORD adr = cpu_rd_mem_dbg(nidx, 0) | (cpu_rd_mem_dbg(nidx, 1) << 8);
				BYTE string[10], side = 0xFF;
				uint32_t position = 0;
				unsigned int a, count = 0;

				for (a = 0; a < sizeof(string); a++) {
					if ((adr + a) == address) {
						string[a] = 0;
						continue;
					}
					string[a] = cpu_rd_mem_dbg(nidx, adr + a);
				}
				for (a = 0; a < fds.info.total_sides; a++) {
					BYTE finded = TRUE;

					position = (a * fds_disk_side_size(fds.info.format));
					if (fds.info.type == FDS_TYPE_FDS) {
						position += 16;
					}
					if ((position + fds_disk_side_size(fds.info.format)) > fds.info.total_size) {
						finded = FALSE;
					} else {
						unsigned int b = 0;

						for (b = 0; b < 10; b++) {
							// To bypass the checking of any byte, a -1 (0xFF) can be placed
							// in the equivelant place in the compare string.
							if ((string[b] != 0xFF) & (string[b] != fds.info.data[position + 1 + 14 + b])) {
								finded = FALSE;
								break;
							}
						}
					}
					if (finded) {
						count++;
						side = a;
					}
				}
				// casi :
				// - count == 0 : non faccio nulla
				//   Mr. Gold - Kinsan in the Space (19xx)()(J).fds
				// - count  > 1 : disabilito l'auto insert
				//   Akumajou Dracula (Japan) (Rev 1).fds
				//   Akumajou Dracula v1.0 (1986)(Konami)(J).fds
				//   Akumajou Dracula *.fds
				// - count == 1 : avvio il meccanismo di auto insert
				if (count > 1) {
					fds.auto_insert.disabled = TRUE;
					gui_overlay_info_append_msg_precompiled(29, &count);
				} else if (count == 1) {
					if ((side != fds.drive.side_inserted) || fds.drive.disk_ejected) {
						fds.auto_insert.rE445.in_run = TRUE;
						fds.side.change.new_side = side;
						fds.side.change.delay = emu_ms_to_cpu_cycles(1);
						fds.auto_insert.delay.dummy = 0;
						if (!fds.drive.disk_ejected) {
							fds_disk_op(FDS_DISK_EJECT, 0, TRUE);
							gui_update_fds_menu();
						}
					}
					if (side > 0) {
						fds.auto_insert.in_game = TRUE;
					}
					fds.auto_insert.delay.dummy = 0;
				}
			}
			break;
		case 0xEF44:
			// Wait after disk insertion
			if (cfg->fds_fast_forward) {
				nes[nidx].c.cpu.PC.w += 2;
				address = nes[nidx].c.cpu.PC.w - 1;
			}
			break;
		case 0xEFAF:
			// Wait license
			if (cfg->fds_fast_forward) {
				nes[nidx].c.cpu.AR = 0;
				nes[nidx].c.cpu.PC.w += 2;
				address = nes[nidx].c.cpu.PC.w - 1;
			}
			break;
//		case 0xF46E:
//			// License check
//			if (cfg->fds_fast_forward) {
//				nes[nidx].c.cpu.PC.w += 2;
//				address = nes[nidx].c.cpu.PC.w - 1;
//			}
//			break;
	}
	return (prgrom_rd(nidx, address));
}
void extcl_cpu_every_cycle_FDS(BYTE nidx) {
	BYTE max_speed = cfg->fds_fast_forward &
		((fds.side.change.delay | fds.drive.delay_insert) || !fds.auto_insert.in_game ||
		(fds.drive.scan & (info.lag_frame.consecutive > FDS_MIN_LAG_FRAMES)) ||
		(fds.auto_insert.r4032.checks > 5));

	// auto insert
	if (fds_auto_insert_enabled() && (fds.auto_insert.delay.dummy > 0)) {
		if (!(--fds.auto_insert.delay.dummy)) {
			fds_disk_op(FDS_DISK_INSERT, fds.drive.side_inserted, TRUE);
			gui_update_fds_menu();
		} else {
			max_speed = cfg->fds_fast_forward && fds.auto_insert.end_of_head.disabled;
		}
	}

	if (max_speed) {
		gui_max_speed_start();
	} else {
		gui_max_speed_stop();
	}

	// IRQ handler
//	if (fds.drive.irq_timer_delay && !(--fds.drive.irq_timer_delay)) {
//		fds.drive.irq_timer_high = 0x01;
//		nes[nidx].c.irq.high |= FDS_TIMER_IRQ;
//	}
	if (fds.drive.enabled_dsk_reg && fds.drive.irq_timer_enabled) {
		if (fds.drive.irq_timer_counter) {
			fds.drive.irq_timer_counter--;
		}
		if (!fds.drive.irq_timer_counter) {
			if (fds.drive.irq_timer_reload_enabled) {
				fds.drive.irq_timer_counter = fds.drive.irq_timer_reload;
			} else {
				fds.drive.irq_timer_enabled = FALSE;
			}
//			fds.drive.irq_timer_delay = 1;
			fds.drive.irq_timer_high = 0x01;
			nes[nidx].c.irq.high |= FDS_TIMER_IRQ;
		}
	}

	// se c'e' un delay aspetto
	if (fds.side.change.delay > 0) {
		if (!(--fds.side.change.delay)) {
			fds_disk_op(FDS_DISK_SELECT_AND_INSERT, fds.side.change.new_side, FALSE);
			gui_update_fds_menu();
		}
		return;
	}

	// no disco, no party
	if (fds.drive.disk_ejected) {
		return;
	}

	if (!fds.drive.motor_on) {
		fds.drive.transfer_reset = 0x02;
	}

	if (fds.drive.delay_insert) {
		--fds.drive.delay_insert;
		if (fds.drive.transfer_reset) {
			fds.drive.motor_started = FALSE;
		}
		if (fds.drive.delay_insert) {
			return;
		}
		fds.drive.disk_position = 0;
		fds.drive.mark_finded = FALSE;
		fds.drive.end_of_head = FALSE;
		fds.drive.data_available = FALSE;
		fds.drive.delay_8bit = fds.info.cycles_8bit_delay;
	}

	if (!fds.drive.motor_started && !fds.drive.transfer_reset && fds.drive.drive_ready) {
		// "Akuu Senki Raijin (Japan) (Disk Writer)" (sporcare lo screen).
		fds.drive.delay_8bit = fds.info.cycles_8bit_delay;
		fds.drive.motor_started = TRUE;
	}

	// se c'e' un delay aspetto
	if (fds.drive.delay_8bit && --fds.drive.delay_8bit) {
		return;
	}

	fds.drive.scan = FALSE;
	fds.info.last_operation = FDS_OP_NONE;

	if (fds.drive.motor_started) {
		fds.drive.scan = !fds.drive.transfer_reset;

		if (fds.drive.scan) {
			BYTE data = 0, transfer = FALSE;

			data = fds.side.info->data[fds.drive.disk_position];

			if (fds.drive.io_mode) {
				// read
				if (fds.drive.drive_ready) {
					if (fds.drive.mark_finded) {
						transfer = TRUE;
						fds.drive.crc = fds_crc_byte(fds.drive.crc, data);
					} else if (data == FDS_DISK_BLOCK_MARK) {
						fds.drive.mark_finded = TRUE;
						fds.drive.crc = 0;
						fds.drive.crc = fds_crc_byte(fds.drive.crc, data);
					}
				}
			} else {
				// write
				if (!fds.drive.drive_ready) {
					data = FDS_DISK_GAP;
					fds.drive.crc = 0;
				} else {
					if (fds.drive.crc_control) {
						data = fds.drive.crc >> 0;
						fds.drive.crc >>= 8;
					} else {
						data = fds.drive.data_io;
						fds.drive.crc = fds_crc_byte(fds.drive.crc, data);
						fds.drive.data_io = FDS_DISK_GAP;
					}
				}
				transfer = TRUE;
			}

			fds.auto_insert.r4032.frames = 0;
			fds.auto_insert.r4032.checks = 0;

			if (transfer) {
				if (fds.drive.irq_disk_enabled) {
					nes[nidx].c.irq.high |= FDS_DISK_IRQ;
				}
				if (fds.drive.io_mode) {
					fds.drive.data_io = data;
					if (!fds.drive.data_available) {
						fds.info.last_operation = FDS_OP_READ;
					}
				} else {
					fds.side.info->data[fds.drive.disk_position] = data;
					fds.info.writings_occurred = TRUE;
					if (!fds.drive.data_available) {
						fds.info.last_operation = FDS_OP_WRITE;
					}
				}
				fds.drive.data_available = 0x80;
				fds.drive.transfer_flag = 0x02;
			}
		}
		if (++fds.drive.disk_position >= fds.info.sides[fds.drive.side_inserted].size) {
			fds.drive.delay_insert = fds.info.cycles_insert_delay;
			fds.drive.end_of_head = 0x40;
			// Bishoujo Alien Battle (Japan) (Unl).fds non esegue la routine a 0xE188
			fds.auto_insert.in_game = TRUE;
			// signal "disk is full" if end track was reached during writing
			if (!fds.drive.io_mode) {
				fds.drive.transfer_reset = 0x02;
			}
			if (fds_auto_insert_enabled() && !fds.auto_insert.end_of_head.disabled && !fds.auto_insert.delay.dummy) {
				fds_disk_op(FDS_DISK_EJECT, 0, TRUE);
				gui_update_fds_menu();
				fds.auto_insert.delay.dummy = fds.info.cycles_dummy_delay;
			}
		} else {
			// il delay per riuscire a leggere i prossimi 8 bit
			fds.drive.delay_8bit = fds.info.cycles_8bit_delay;
		}
	}
}
void extcl_apu_tick_FDS(void) {
	int32_t freq = fds.snd.main.frequency;

	if (!fds.snd.envelope.disabled && !fds.snd.main.silence && (fds.snd.envelope.speed > 0)) {
		// volume unit
		if (!fds.snd.volume.mode) {
			if (--fds.snd.volume.counter == 0) {
				fds.snd.volume.counter = fds_reset_envelope_counter(volume);
				if (fds.snd.volume.increase) {
					if (fds.snd.volume.gain < 32) {
						fds.snd.volume.gain++;
					}
				} else if (fds.snd.volume.gain) {
					fds.snd.volume.gain--;
				}
			}
		}
		// sweep unit
		if (!fds.snd.sweep.mode) {
			if (--fds.snd.sweep.counter == 0) {
				fds.snd.sweep.counter = fds_reset_envelope_counter(sweep);
				if (fds.snd.sweep.increase) {
					if (fds.snd.sweep.gain < 32) {
						fds.snd.sweep.gain++;
					}
				} else if (fds.snd.sweep.gain) {
					fds.snd.sweep.gain--;
				}
			}
		}
	}

	// modulation unit
	if (!fds.snd.modulation.disabled && fds.snd.modulation.frequency) {
		fds.snd.modulation.counter -= fds.snd.modulation.frequency;
		if (fds.snd.modulation.counter < 0) {
			SBYTE adj = modulation_table[fds.snd.modulation.data[fds.snd.modulation.index]];

			fds.snd.modulation.counter += 65536;
			fds.snd.modulation.index = (fds.snd.modulation.index + 1) & 0x3F;

			fds.snd.sweep.bias += ((BYTE)adj == 8 ? 0 : adj);
			fds.snd.sweep.bias = fds_sweep_bias(fds.snd.sweep.bias)

//			// vecchia gestione
//			{
//				SWORD temp, temp2, a, d;
//				temp = fds.snd.sweep.bias * fds.snd.sweep.gain;
//
//				a = 64;
//				d = 0;
//
//				if (temp <= 0) {
//					d = 15;
//				} else if (temp < 3040) { //95 * 32
//					a = 66;
//					d = -31;
//				}
//
//				temp2 = a + (SBYTE)((temp - d) / 16 - a);
//
//				fds.snd.modulation.mod = freq * temp2 / 64;
//			}

			// from https://www.nesdev.org/wiki/FDS_audio
			// and https://forums.nesdev.org/viewtopic.php?p=232662#p232662

			// freq               = $4082/4083 (12-bit unsigned pitch value)
			// fds.snd.sweep.bias = $4085 (7-bit signed mod counter)
			// fds.snd.sweep.gain = $4084 (6-bit unsigned mod gain)
			{
				// 1. multiply counter by gain, lose lowest 4 bits of result but "round" in a strange way
				int32_t temp = fds.snd.sweep.bias * fds.snd.sweep.gain;
				int32_t remainder = temp & 0xF;

				temp >>= 4;
				if ((remainder > 0) && ((temp & 0x80) == 0))
				{
					if (fds.snd.sweep.bias < 0) {
						temp -= 1;
					} else {
						temp += 2;
					}
				}
				// 2. wrap if a certain range is exceeded
				if (temp >= 192) {
					temp -= 256;
				} else if (temp < -64) {
					temp += 256;
				}
				// 3. multiply result by pitch, then round to nearest while dropping 6 bits
				temp = freq * temp;
				remainder = temp & 0x3F;
				temp >>= 6;
				if (remainder >= 32) {
					temp += 1;
				}
				// final mod result is in temp
				fds.snd.modulation.mod = temp;
			}
		}
		if (freq) {
			freq += fds.snd.modulation.mod;
		}
	}

	// main unit
	if (fds.snd.main.silence) {
		fds.snd.main.output = 0;
		fds.snd.wave.index = 0;
		return;
	}

	if ((freq > 0) && !fds.snd.wave.writable) {
		fds.snd.wave.counter -= freq;
		if (fds.snd.wave.counter < 0) {
			WORD level = (fds.snd.volume.gain < 32 ? fds.snd.volume.gain : 32) * volume_wave[fds.snd.wave.volume];

			// valore massimo dell'output (63 * (39 * 32)) = 78624
//			fds.snd.main.output = (fds.snd.wave.data[fds.snd.wave.index] * level) >> 4;
			fds.snd.main.output = (fds.snd.wave.data[fds.snd.wave.index] * level) >> 3;

			fds.snd.wave.counter += 65536;
			fds.snd.wave.index = (fds.snd.wave.index + 1) & 0x3F;
			fds.snd.wave.clocked = TRUE;
		}
	}
}

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

/*
 *  Many thanks to Sour (author of Mesen) for the auto insert logic.
 *  https://github.com/SourMesen/Mesen/blob/master/Core/FDS.cpp
 */

#include <string.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "fds.h"
#include "cpu.h"
#include "conf.h"
#include "tas.h"
#include "gui.h"

enum { TRANSFERED_8BIT = 0x02, END_OF_HEAD = 0x40 };

static const SBYTE modulation_table[8] = { 0, 1, 2, 4, 8, -4, -2, -1 };
static const BYTE volume_wave[4] = { 36, 24, 17, 14 };

void map_init_FDS(void) {
	EXTCL_CPU_RD_MEM(FDS);
	EXTCL_CPU_EVERY_CYCLE(FDS);
	EXTCL_APU_TICK(FDS);

	memset (&fds.auto_insert, 0x00, sizeof(fds.auto_insert));
	fds.auto_insert.delay.eject = -1;
	fds.auto_insert.delay.dummy = -1;
	fds.auto_insert.delay.side = -1;

	if (cfg->fds_disk1sideA_at_reset) {
		fds_disk_op(FDS_DISK_EJECT, 0, TRUE);
		if (fds.drive.side_inserted != 0) {
			fds_disk_op(FDS_DISK_SELECT_AND_INSERT, 0, FALSE);
		} else {
			fds_disk_op(FDS_DISK_INSERT, 0, TRUE);
		}
	}

	mapper.write_vram = TRUE;
	info.chr.rom.banks_8k = 1;

	mirroring_H();

	cpu.SP = 0xFF;

	cpu.SR = 0x30;
	/* disassemblo il Processor Status Register */
	disassemble_SR();
	/* setto il flag di disabilitazione dell'irq */
	irq.inhibit = cpu.im;
}
void map_init_NSF_FDS(void) {
	memset(&fds, 0x00, sizeof(fds));
	fds.snd.modulation.counter = 0xFFFF;
	fds.snd.wave.counter = 0xFFFF;

	fds.drive.enabled_snd_reg = 0x02;
}
BYTE extcl_cpu_rd_mem_FDS(WORD address, UNUSED(BYTE openbus), UNUSED(BYTE before)) {
	// 0xE18B : NMI entry point
	// [$0100]: PC action on NMI. set to $C0 on reset
	// When NMI occurs while $100 & $C0 != 0, it typically means that the game is starting.
	if ((address == 0xE18B) & !fds.auto_insert.in_game & ((cpu_rd_mem_dbg(0x100) & 0xC0) != 0)) {
		fds.auto_insert.in_game = TRUE;
	} else if (address == 0xE445) {
		// Address          : 0xE445
		// Name             : CheckDiskHeader
		// Input parameters : Pointer to 10 byte string at $00
		// Description      : Compares the first 10 bytes on the disk coming after the FDS string, to 10
		//                    bytes pointed to by Ptr($00). To bypass the checking of any byte, a -1 can be placed in the
		//                    equivelant place in the compare string. Otherwise, if the comparison fails, an appropriate error
		//                    will be generated.
		if (fds_auto_insert_enabled() & !fds.auto_insert.rE445.in_run) {
			// i primi due passaggi sono del bios e li ignoro
			if (fds.auto_insert.rE445.count > 1) {
				WORD adr = cpu_rd_mem_dbg(0) | (cpu_rd_mem_dbg(1) << 8);
				BYTE string[10], side = 0xFF;
				uint32_t position = 0;
				unsigned int a, count = 0;

				for (a = 0; a < sizeof(string); a++) {
					if ((adr + a) == address) {
						string[a] = 0;
						continue;
					}
					string[a] = cpu_rd_mem_dbg(adr + a);
				}
				for (a = 0; a < fds.info.total_sides; a++) {
					BYTE finded = TRUE;

					position = (a * DISK_SIDE_SIZE);

					if (fds.info.type == FDS_FORMAT_FDS) {
						position += 16;
					}
					if ((position + DISK_SIDE_SIZE) > fds.info.total_size) {
						finded = FALSE;
					} else {
						unsigned int b;

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
				// * count == 0
				//   non faccio nulla ("Mr. Gold - Kinsan in the Space (19xx)()(J).fds")
				// * count == 1
				//   avvio il meccanismo di auto insert
				// * count > 1
				//   disabilito l'auto insert
				if (count > 1) {
					fds.auto_insert.disabled = TRUE;
					gui_overlay_info_append_msg_precompiled(29, &count);
				} else if (count == 1) {
					if (side != fds.drive.side_inserted) {
						fds.auto_insert.rE445.in_run = TRUE;
						fds.auto_insert.new_side = side;
						fds.auto_insert.delay.side = FDS_AUTOINSERT_OP_SIDE_DELAY;
					}
					if (side > 0) {
						fds.auto_insert.in_game = TRUE;
					}
					fds.auto_insert.delay.eject = -1;
					fds.auto_insert.delay.dummy = -1;
				}
			} else {
				fds.auto_insert.rE445.count++;
			}
		}
	}
	return (prg_byte(address & 0x1FFF));
}
void extcl_cpu_every_cycle_FDS(void) {
	BYTE max_speed = cfg->fds_fast_forward & ((fds.drive.scan & tas.lag_actual_frame) | !fds.auto_insert.in_game);
	WORD data;

	// auto insert
	if (fds_auto_insert_enabled()) {
#define _max_speed cfg->fds_fast_forward & tas.lag_actual_frame
		if (fds.auto_insert.delay.eject > 0) {
			fds.auto_insert.delay.eject--;
			max_speed = _max_speed & (fds.auto_insert.delay.eject > 0);
		} else if (fds.auto_insert.delay.dummy > 0) {
			if (--fds.auto_insert.delay.dummy == 0) {
				fds.auto_insert.delay.dummy = -1;
				fds_disk_op(FDS_DISK_INSERT, 0, TRUE);
				gui_update_fds_menu();
			}
			max_speed = _max_speed & (fds.auto_insert.delay.dummy > 0);
		} else if (fds.auto_insert.delay.side > 0) {
			if (--fds.auto_insert.delay.side == 0) {
				fds.auto_insert.delay.side = -1;
				fds.side.change.new_side = fds.auto_insert.new_side;
				fds.side.change.delay = FDS_OP_SIDE_DELAY;
				fds_disk_op(FDS_DISK_EJECT, 0, TRUE);
				gui_update_fds_menu();
			}
			max_speed = _max_speed & (fds.auto_insert.delay.side > 0);
		}
		if (!fds.auto_insert.delay.eject & (fds.auto_insert.delay.dummy == -1) & (fds.auto_insert.r4032.checks > 20)) {
			fds.auto_insert.delay.eject = -1;
			fds.auto_insert.r4032.frames = 0;
			fds.auto_insert.r4032.checks = 0;
			fds.auto_insert.delay.dummy = FDS_OP_SIDE_DELAY;
			fds_disk_op(FDS_DISK_EJECT, 0, TRUE);
			gui_update_fds_menu();
			max_speed = _max_speed;
		}
#undef _max_speed
	}

	if (max_speed & !info.fds_only_bios) {
		gui_max_speed_start();
	} else {
		gui_max_speed_stop();
	}

	/* IRQ handler */
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
			fds.drive.irq_timer_high = 0x01;
			irq.high |= FDS_TIMER_IRQ;
		}
	}

	/* se c'e' un delay aspetto */
	if (fds.side.change.delay > 0) {
		if (!(--fds.side.change.delay)) {
			fds_disk_op(FDS_DISK_SELECT_AND_INSERT, fds.side.change.new_side, FALSE);
			gui_update_fds_menu();
		}
		return;
	}

	/* no disco, no party */
	if (fds.drive.disk_ejected) {
		fds.drive.delay = 1000;
		return;
	}

	/* il motore non e' avviato */
	if (!fds.drive.motor_on) {
		fds.drive.disk_position = 0;
		fds.drive.gap_ended = FALSE;
		fds.drive.scan = FALSE;
		fds.drive.delay = 1;
		return;
	}

	/* se c'e' un delay aspetto */
	if ((fds.drive.delay > 0) && --fds.drive.delay) {
		return;
	}

	if (fds.drive.transfer_reset) {
		fds.drive.scan = TRUE;
	}
	if (!fds.drive.scan) {
		return;
	}

	/* se c'e' una richiesta di invio crc i prossimi due bytes lo saranno */
	if (!fds.drive.crc_char && fds.drive.crc_control) {
		fds.drive.crc_char = 2;
	}

	if (fds.drive.read_mode) {
		/* read */
		fds.drive.data_readed = data = fds.side.data[fds.drive.disk_position];
	} else {
		/* write */
		if (!fds.drive.drive_ready) {
			data = FDS_DISK_GAP;
		} else if (fds.drive.crc_char) {
			data = FDS_DISK_CRC_CHAR1;
		} else {
			data = fds.side.data[fds.drive.disk_position];
		}
	}

	/*
	 * se non sono piu' nel gap vuol dire che ho trasferito
	 * 8 bit di dati quindi setto il flag corrispondente e
	 * se e' abilitato l'irq del disco, lo setto.
	 */
	if (fds.drive.gap_ended) {
		if (fds.drive.irq_disk_enabled) {
			fds.drive.irq_disk_high = 0x02;
			irq.high |= FDS_DISK_IRQ;
		}

		if (!fds.drive.read_mode) {
			uint32_t position = (fds.drive.disk_position - 2);
			WORD *dst = fds.side.data + position;

			if (((*dst) == 0x0100) && (fds.drive.data_to_write == 0x00)) {
				(*dst) = 0x0100;
			} else if (((*dst) == 0x0180) && (fds.drive.data_to_write == 0x80)) {
				(*dst) = 0x0180;
			} else if (fds.drive.crc_char) {
				if (fds.drive.crc_char == 2) {
					(*dst) = FDS_DISK_CRC_CHAR1;
				} else {
					(*dst) = FDS_DISK_CRC_CHAR2;
				}
			} else {
				(*dst) = fds.drive.data_to_write;
				fds_diff_op(FDS_OP_WRITE, position, fds.drive.data_to_write);
			}
			fds.info.last_operation = FDS_OP_WRITE;
		} else if (!fds.info.last_operation) {
			fds.info.last_operation = FDS_OP_READ;
		}
	}

	if (data != FDS_DISK_GAP) {
		fds.drive.gap_ended = TRUE;
	}

	if (fds.drive.crc_char && !(--fds.drive.crc_char)) {
		fds.drive.gap_ended = fds.drive.crc_control = FALSE;
	}

	if (!fds.drive.drive_ready) {
		fds.drive.gap_ended = FALSE;
	}

	if (++fds.drive.disk_position >= fds.info.sides_size[fds.drive.side_inserted]) {
		fds.drive.end_of_head = END_OF_HEAD;
		fds.drive.disk_position = 0;
		fds.drive.gap_ended = FALSE;
		fds.drive.delay = 900000;
		fds.drive.motor_on = FALSE;
		// FDS interessati :
		// - 19 Neunzehn (1988)(Soft Pro)(J).fds
		//   visto che il controllo del r4032 e' mooooolto lento, l'eject forzato alla fine del disco
		//   costringe la rom al richiamo della funzione del bios $E445.
		// - Dandy (19xx)(Pony Canyon)(J).fds
		//   dopo aver selezionato il nome del personaggio, puo' capitare che il disco sia disinserito a causa di
		//   di eject e che dia un "error 01" che comunque verra' subito corretto dall'insert automatico seguente.
		// - Zelda no Densetsu - The Hyrule Fantasy (1986)(Nintendo)(J).fds
		//   stesso discorso fatto per Dandy (19xx)(Pony Canyon)(J).fds.
		fds.auto_insert.delay.eject = FDS_OP_SIDE_DELAY;
	} else {
		fds.drive.end_of_head = FALSE;
		/* il delay per riuscire a leggere i prossimi 8 bit */
		fds.drive.delay = 149;
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
		if ((fds.snd.modulation.counter -= fds.snd.modulation.frequency) < 0) {
			SBYTE adj = modulation_table[fds.snd.modulation.data[fds.snd.modulation.index]];

			fds.snd.modulation.counter += 65536;
			fds.snd.modulation.index = (fds.snd.modulation.index + 1) & 0x3F;

			fds.snd.sweep.bias += ((BYTE)adj == 8 ? 0 : adj);
			fds.snd.sweep.bias = fds_sweep_bias(fds.snd.sweep.bias);

			/*
			// vecchia gestione
			{
				SWORD temp, temp2, a, d;
				temp = fds.snd.sweep.bias * ((fds.snd.sweep.gain < 32) ? fds.snd.sweep.gain : 32);

				a = 64;
				d = 0;

				if (temp <= 0) {
					d = 15;
				} else if (temp < 3040) { //95 * 32
					a = 66;
					d = -31;
				}

				temp2 = a + (SBYTE)((temp - d) / 16 - a);

				fds.snd.modulation.mod = freq * temp2 / 64;
			}
			*/

			// from https://www.nesdev.org/wiki/FDS_audio
			// and https://forums.nesdev.org/viewtopic.php?p=232662#p232662

			// freq               = $4082/4083 (12-bit unsigned pitch value)
			// fds.snd.sweep.bias = $4085 (7-bit signed mod counter)
			// fds.snd.sweep.gain = $4084 (6-bit unsigned mod gain)
			{
				int32_t temp;

				temp = fds.snd.sweep.bias * fds.snd.sweep.gain;
				if ((temp & 0x0F) && !(temp & 0x800)) {
					temp += 0x20;
				}
				temp += 0x400;
				temp = (temp >> 4) & 0xFF;

				temp = freq * temp;
				temp >>= 6;
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
		if ((fds.snd.wave.counter -= freq) < 0) {
			WORD level = (fds.snd.volume.gain < 32 ? fds.snd.volume.gain : 32) * volume_wave[fds.snd.wave.volume];

			/* valore massimo dell'output (63 * (39 * 32)) = 78624 */
			/*fds.snd.main.output = (fds.snd.wave.data[fds.snd.wave.index] * level) >> 4;*/
			fds.snd.main.output = (fds.snd.wave.data[fds.snd.wave.index] * level) >> 3;

			fds.snd.wave.counter += 65536;
			fds.snd.wave.index = (fds.snd.wave.index + 1) & 0x3F;
			fds.snd.wave.clocked = TRUE;
		}
	}
}

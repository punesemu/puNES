/*
 * mapperFDS.c
 *
 *  Created on: 29/mar/2012
 *      Author: fhorse
 */

#include <stdio.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "fds.h"
#include "cpu.h"

enum {
	TRANSFERED_8BIT = 0x02,
	END_OF_HEAD = 0x40
};

static const BYTE volume_wave[4] = { 39, 26, 19, 15 };

void map_init_FDS(void) {
	EXTCL_CPU_EVERY_CYCLE(FDS);
	EXTCL_APU_TICK(FDS);

	info.chr_rom_8k_count = 1;
	mapper.write_vram = TRUE;

	mirroring_H();

	cpu.SP = 0xFF;

	cpu.SR = 0x30;
	/* assemblo il Processor Status Register */
	DIS_SR;
	/* setto il flag di disabilitazione dell'irq */
	irq.inhibit = cpu.im;
}
void extcl_cpu_every_cycle_FDS(void) {
	WORD data;

	if (fds.drive.irq_timer_delay && !(--fds.drive.irq_timer_delay)) {
		fds.drive.irq_timer_high = 0x01;
		irq.high |= FDSTIMERIRQ;
	}

	/* IRQ handler */
	if (fds.drive.irq_timer_enabled && fds.drive.irq_timer_counter &&
			!(--fds.drive.irq_timer_counter)) {
		/**/
		if (fds.drive.irq_timer_reload_enabled) {
			fds.drive.irq_timer_counter = fds.drive.irq_timer_reload;
		} else {
			fds.drive.irq_timer_enabled = FALSE;
		}

		/* il solito delay */
		fds.drive.irq_timer_delay = 1;
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

	/* se c'è una richiesta di invio crc i prossimi due bytes lo saranno */
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
	 * se non sono più nel gap vuol dire che ho trasferito
	 * 8 bit di dati quindi setto il flag corrispondente e
	 * se e' abilitato l'irq del disco, lo setto.
	 */
	if (fds.drive.gap_ended) {
		if (fds.drive.irq_disk_enabled) {
#ifndef RELEASE
			//fprintf(stderr, "qui ed ora\n");
#endif
			fds.drive.irq_disk_high = 0x01;
			irq.high |= FDSDISKIRQ;
		}

		fds.drive.transfer_flag = TRUE;

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
#ifndef RELEASE
			/*fprintf(stderr, "data : [%5d 0x%04X 0x%04X] 0x%04X 0x%02X %d\n",
					fds.drive.disk_position - 2,
			        prima,
			        (*src),
			        data,
			        fds.drive.data_to_write,
			        fds.drive.crc_char);*/
#endif
			fds.info.last_operation = FDS_OP_WRITE;
		} else if (!fds.info.last_operation) {
			fds.info.last_operation = FDS_OP_READ;
		}
	}

	if (data != FDS_DISK_GAP) {
		fds.drive.gap_ended = TRUE;
#ifndef RELEASE
		//fprintf(stderr, "gap ended : [%5d 0x%04X 0x%04X]\n", fds.drive.disk_position,
		//		fds.side.data[fds.drive.disk_position], data);
#endif
	}

	if (fds.drive.crc_char && !(--fds.drive.crc_char)) {
		fds.drive.gap_ended = fds.drive.crc_control = FALSE;
#ifndef RELEASE
		//fprintf(stderr, "gap ended to false : %d\n", irq.high);
#endif
	}

	if (!fds.drive.drive_ready) {
		fds.drive.gap_ended = FALSE;
	}

	if (++fds.drive.disk_position >= fds.info.sides_size[fds.drive.side_inserted]) {
		fds.drive.end_of_head = END_OF_HEAD;
		fds.drive.disk_position = 0;
		fds.drive.gap_ended = FALSE;
		fds.drive.delay = 900000;
	} else {
		/* il delay per riuscire a leggere i prossimi 8 bit */
		fds.drive.delay = 149;
	}
}
void extcl_apu_tick_FDS(void) {
	SWORD freq;

	/* volume unit */
	if (fds.snd.volume.mode) {
		fds.snd.volume.gain = fds.snd.volume.speed;
	} else if (!fds.snd.envelope.disabled && fds.snd.envelope.speed) {
		if (fds.snd.volume.counter) {
			fds.snd.volume.counter--;
		} else {
			fds.snd.volume.counter = (fds.snd.envelope.speed << 3) * (fds.snd.volume.speed + 1);
			if (fds.snd.volume.increase) {
				if (fds.snd.volume.gain < 32) {
					fds.snd.volume.gain++;
				}
			} else if (fds.snd.volume.gain) {
				fds.snd.volume.gain--;
			}
		}
	}

	/* sweep unit */
	if (fds.snd.sweep.mode) {
		fds.snd.sweep.gain = fds.snd.sweep.speed;
	} else if (!fds.snd.envelope.disabled && fds.snd.envelope.speed) {
		if (fds.snd.sweep.counter) {
			fds.snd.sweep.counter--;
		} else {
			fds.snd.sweep.counter = (fds.snd.envelope.speed << 3) * (fds.snd.sweep.speed + 1);
			if (fds.snd.sweep.increase) {
				if (fds.snd.sweep.gain < 32) {
					fds.snd.sweep.gain++;
				}
			} else if (fds.snd.sweep.gain) {
				fds.snd.sweep.gain--;
			}
		}
	}

	/* modulation unit */
	freq = fds.snd.main.frequency;

	if (!fds.snd.modulation.disabled && fds.snd.modulation.frequency) {
		if ((fds.snd.modulation.counter -= fds.snd.modulation.frequency) < 0) {
			SWORD temp, temp2, a, d;
			SBYTE adj = fds.snd.modulation.data[fds.snd.modulation.index];

			fds.snd.modulation.counter += 65536;

			if (++fds.snd.modulation.index == 64) {
				fds.snd.modulation.index = 0;
			}

			if (adj == -4) {
				fds.snd.sweep.bias = 0;
			} else {
				fds.snd.sweep.bias += adj;
			}

			temp = fds.snd.sweep.bias * ((fds.snd.sweep.gain < 32) ? fds.snd.sweep.gain : 32);

			a = 64;
			d = 0;

			if (temp <= 0) {
				d = 15;
			} else if (temp < 3040) { //95 * 32
				a = 66;
				d = -31;
			}

			temp2 = a + (SBYTE) ((temp - d) / 16 - a);

			fds.snd.modulation.mod = freq * temp2 / 64;
		}

		if (freq) {
			freq += fds.snd.modulation.mod;
		}
	}

	/* main unit */
	if (fds.snd.main.silence) {
		fds.snd.main.output = 0;
		return;
	}

	if (freq && !fds.snd.wave.writable) {
		if ((fds.snd.wave.counter -= freq) < 0) {
			WORD level;

			fds.snd.wave.counter += 65536;

			level = (fds.snd.volume.gain < 32 ? fds.snd.volume.gain : 32)
			        * volume_wave[fds.snd.wave.volume];

			/* valore massimo dell'output (63 * (39 * 32)) = 78624 */
			/*fds.snd.main.output = (fds.snd.wave.data[fds.snd.wave.index] * level) >> 4;*/
			fds.snd.main.output = (fds.snd.wave.data[fds.snd.wave.index] * level) >> 3;

			if (++fds.snd.wave.index == 64) {
				fds.snd.wave.index = 0;
			}

			fds.snd.wave.clocked = TRUE;
		}
	}
}

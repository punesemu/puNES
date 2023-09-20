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

#ifndef CPU_INLINE_H_
#define CPU_INLINE_H_

#include <stdlib.h>
#include "clock.h"
#include "cpu.h"
#include "input.h"
#include "mappers.h"
#include "ppu_inline.h"
#include "apu.h"
#include "irqA12.h"
#include "irql2f.h"
#include "tas.h"
#include "fds.h"
#include "nsf.h"
#include "cheat.h"
#include "info.h"
#include "conf.h"
#include "vs_system.h"
#include "qt.h"
#include "audio/snd.h"
#include "tape_data_recorder.h"
#include "memmap.h"

#define mod_cycles_op(op, vl) nes.c.cpu.cycles op vl
#define r2006_during_rendering()\
	if (!nes.p.ppu.vblank && nes.p.r2001.visible && (nes.p.ppu.frame_y > nes.p.ppu_sclines.vint) && (nes.p.ppu.screen_y < SCR_ROWS)) {\
		_r2006_during_rendering()\
	} else {\
		nes.p.r2006.value += nes.p.r2000.r2006_inc;\
	}
#define _r2006_during_rendering()\
	r2006_inc()\
	if ((nes.p.r2006.value & 0x1F) == 0x1F) {\
		nes.p.r2006.value ^= 0x41F;\
	} else {\
		nes.p.r2006.value++;\
	}
#define ppu_openbus_wr(bit) nes.p.ppu_openbus.bit = nes.p.ppu.frames
#define ppu_openbus_wr_all()\
	ppu_openbus_wr(bit0);\
	ppu_openbus_wr(bit1);\
	ppu_openbus_wr(bit2);\
	ppu_openbus_wr(bit3);\
	ppu_openbus_wr(bit4);\
	ppu_openbus_wr(bit5);\
	ppu_openbus_wr(bit6);\
	ppu_openbus_wr(bit7)
#define ppu_openbus_rd(bit, mask)\
	if ((nes.p.ppu.frames - nes.p.ppu_openbus.bit) > machine.ppu_openbus_frames) {\
		nes.p.ppu.openbus &= (mask);\
	}
#define ppu_openbus_rd_all()\
	ppu_openbus_rd(bit0, 0x01);\
	ppu_openbus_rd(bit1, 0x02);\
	ppu_openbus_rd(bit2, 0x04);\
	ppu_openbus_rd(bit3, 0x08);\
	ppu_openbus_rd(bit4, 0x10);\
	ppu_openbus_rd(bit5, 0x20);\
	ppu_openbus_rd(bit6, 0x40);\
	ppu_openbus_rd(bit7, 0x80)
#define look_cheats_list(lst, lng, adr)\
	if ((lst).counter) {\
		int i;\
		for (i = 0; i < (lng); i++) {\
			if (!(lst).cheat[i].disabled && ((lst).cheat[i].address == (adr))) {\
				if ((lst).cheat[i].enabled_compare) {\
					if ((lst).cheat[i].compare == nes.c.cpu.openbus) {\
						nes.c.cpu.openbus = (lst).cheat[i].replace;\
					}\
				} else {\
					nes.c.cpu.openbus = (lst).cheat[i].replace;\
				}\
			}\
		}\
	}

INLINE static BYTE ppu_rd_reg(WORD address);
INLINE static BYTE apu_rd_reg(WORD address);
INLINE static void nsf_rd_mem(WORD address, BYTE made_tick);
INLINE static BYTE fds_rd_mem(WORD address, BYTE made_tick);

INLINE static void ppu_wr_mem(WORD address, BYTE value);
INLINE static void ppu_wr_reg(WORD address, BYTE value);
INLINE static void apu_wr_reg(WORD address, BYTE value);
INLINE static void nsf_wr_mem(WORD address, BYTE value);
INLINE static BYTE fds_wr_mem(WORD address, BYTE value);

INLINE static WORD lend_word(WORD address, BYTE indirect, BYTE make_last_tick_hw);
INLINE static void tick_hw(BYTE value);

/* ------------------------------------ READ ROUTINE ------------------------------------------- */

BYTE cpu_rd_mem_dbg(WORD address) {
	BYTE cpu_openbus = nes.c.cpu.openbus;
	BYTE read = 0;

	info.disable_tick_hw = TRUE;
	read = cpu_rd_mem(address, FALSE);
	nes.c.cpu.openbus = cpu_openbus;
	info.disable_tick_hw = FALSE;
	return (read);
}
BYTE cpu_rd_mem(WORD address, BYTE made_tick) {
	if (info.cpu_rw_extern) {
		if (nsf.enabled) {
			nsf_rd_mem(address, made_tick);
			return (nes.c.cpu.openbus);
		} else if (fds.info.enabled) {
			if (fds_rd_mem(address, made_tick)) {
				return (nes.c.cpu.openbus);
			}
		}
	} else if (address >= 0x8000) {
		/* eseguo un tick hardware */
		if (made_tick) {
			tick_hw(1);
		}
		// nella mapper 413 extcl_cpu_rd_mem e' utilizzato anche dal DMC
		// (in apu.c : DMC.buffer = prgrom_rd(DMC.address) per leggere
		// i dati dal miscrom.
		nes.c.cpu.openbus = info.mapper.extend_rd ? extcl_cpu_rd_mem(address, nes.c.cpu.openbus) : prgrom_rd(address);

		/* cheat */
		if (cfg->cheat_mode != NOCHEAT_MODE) {
			if (cfg->cheat_mode == GAMEGENIE_MODE) {
				look_cheats_list(gamegenie, (int)LENGTH(gamegenie.cheat), address)
			} else if (cfg->cheat_mode == CHEATSLIST_MODE) {
				look_cheats_list(cheats_list, cheats_list.counter, address)
			}
		}

		return (nes.c.cpu.openbus);
	}
	if (address <= 0x4017) {
		/* Ram */
		if (address < 0x2000) {
			/* eseguo un tick hardware */
			if (made_tick) {
				tick_hw(1);
			}

			nes.c.cpu.openbus = extcl_cpu_rd_ram ? extcl_cpu_rd_ram(address, nes.c.cpu.openbus) : ram_rd(address);

			/* cheat */
			if (cfg->cheat_mode == CHEATSLIST_MODE) {
				look_cheats_list(cheats_list, cheats_list.counter, (address & 0x7FF))
			}

			return (nes.c.cpu.openbus);
		}
		/* PPU */
		if (address < 0x4000) {
			/* eseguo un tick hardware */
			tick_hw(1);
			/* leggo */
			nes.c.cpu.openbus = ppu_rd_reg(address & 0x2007);
			return (nes.c.cpu.openbus);
		}
		/* APU */
		if (address <= 0x4015) {
			/* leggo */
			nes.c.cpu.openbus = apu_rd_reg(address);
			/* eseguo un tick hardware ed e' importante che sia fatto dopo */
			tick_hw(1);
			return (nes.c.cpu.openbus);
		}
		/* Controller port 1 */
		if (address == 0x4016) {
			info.lag_frame.next = FALSE;
			/* eseguo un tick hardware */
			tick_hw(1);
			/* leggo dal controller */
			nes.c.cpu.openbus = input_rd_reg[PORT1](nes.c.cpu.openbus, PORT1);
			return (nes.c.cpu.openbus);
		}
		/* Controller port 2 */
		if (address == 0x4017) {
			info.lag_frame.next = FALSE;
			/* eseguo un tick hardware */
			tick_hw(1);
			/* leggo dal controller */
			nes.c.cpu.openbus = input_rd_reg[PORT2](nes.c.cpu.openbus, PORT2);
			return (nes.c.cpu.openbus);
		}
	}
	// WRam (normale ed extra (battery packed o meno)
	if (address < 0x8000) {
		// eseguo un tick hardware
		/*
		 * la mancanza del controllo del made_tick l'ho notata grazie alla rom
		 * "Tetris 2 + BomBliss (J) [!].nes". Questa utilizza la ram extra per eseguire
		 * codice tra cui l'accesso ai registri della PPU quindi, utilizzo il lend_word()
		 * e quindi devo controllare se mi trovo alla lettura del secondo BYTE e se si
		 * non devo eseguire il tick_hw() esattamente come faccio quando
		 * eseguo codice dal 0x8000 in su.
		 */
		if (made_tick) {
			tick_hw(1);
		}

		nes.c.cpu.openbus = extcl_cpu_rd_mem ? extcl_cpu_rd_mem(address, nes.c.cpu.openbus) : wram_rd(address);

		/* cheat */
		if (cfg->cheat_mode == CHEATSLIST_MODE) {
			look_cheats_list(cheats_list, cheats_list.counter, (address & 0x1FFF))
		}

		return (nes.c.cpu.openbus);
	}

	/* eseguo un tick hardware */
	tick_hw(1);
	/* qualsiasi altra cosa */
	return (nes.c.cpu.openbus);
}
INLINE static BYTE ppu_rd_reg(WORD address) {
	BYTE value = 0;

	ppu_openbus_rd_all()

	if (address == 0x2002) {
		/* Situazioni particolari --- */
		if (!info.r2002_race_condition_disabled && !(nes.p.ppu.frame_y | nes.c.nmi.before)) {
			/* situazione di contesa (race condition)
			 *
			 * se la lettura avviene esattamente all'inizio
			 * del vblank allora il bit 7 verra' restituito
			 * a 0.
			 */
			if (!nes.p.ppu.frame_x) {
				/*
				 * Nota: quando e' abilitato questo controllo
				 * la demo cuter.nes (Who's Cuter?) ha un problema
				 * di rallentamento e sfarfallio. Il problema e'
				 * della demo e non dell'emulatore (confermato
				 * dall'autore stesso della demo).
				 */
				nes.p.r2002.vblank = FALSE;
			}
			/*
			 * leggendo questo registro nei primi tre
			 * cicli PPU del vblank, se presente, viene
			 * disabilitato l'NMI pendente.
			 */
			nes.c.nmi.high = nes.c.nmi.delay = FALSE;
		}
		if ((nes.p.ppu.frame_y == nes.p.ppu_sclines.vint) && !nes.p.ppu.frame_x) {
			/* situazione di contesa (race condition)
			 *
			 * se la lettura avviene esattamente alla fine
			 * del vblank allora i bit 5 e 6 verranno restituiti
			 * a 0.
			 */
			nes.p.r2002.sprite_overflow = nes.p.r2002.sprite0_hit = FALSE;
		}
		/* -------------------------- */

		value = nes.p.r2002.vblank | nes.p.r2002.sprite0_hit | (nes.p.r2002.race.sprite_overflow ? 0 : nes.p.r2002.sprite_overflow);
		/* azzero il VBlank */
		nes.p.r2002.vblank = FALSE;
		/*
		 * azzero il bit toggle (1°/2° write flipflop
		 * usato da $2005 e $2006)
		 */
		nes.p.r2002.toggle = 0;
		/* open bus */
		value = nes.p.ppu.openbus = (value & 0xE0) | (nes.p.ppu.openbus & 0x1F);
		ppu_openbus_wr(bit5);
		ppu_openbus_wr(bit6);
		ppu_openbus_wr(bit7);

		// Vs. System
		if (vs_system.rc2c05.enabled) {
			value = (value & 0xE0) | vs_system.rc2c05.r2002;
		}

		return (value);
	}
	if (address == 0x2004) {
		value = nes.p.oam.data[nes.p.r2003.value];

		if (nes.p.ppu.vblank) {
			if ((machine.type == PAL) && (nes.p.ppu.frame_y > 23)) {
				value = nes.p.r2004.value;
			}
		} else {
			if (nes.p.r2001.visible && (nes.p.ppu.screen_y < SCR_ROWS)) {
				value = nes.p.r2004.value;
			}
		}

		/* ppu open bus */
		nes.p.ppu.openbus = value;
		ppu_openbus_wr_all();
		return (value);
	}
	if (address == 0x2007) {
		WORD old_r2006 = nes.p.r2006.value;
		BYTE repeat = 1;

		/*
		 * utilizzato dalle mappers :
		 * MMC5
		 */
		if (extcl_rd_r2007) {
			extcl_rd_r2007();
		}

		if (DMC.dma_cycle == 2) {
			repeat = 3;
		} else if (nes.c.cpu.double_rd) {
			WORD random = (WORD)emu_irand(10);

			value = ppu_rd_mem(nes.p.r2006.value - (nes.p.r2000.r2006_inc << 1));
			if (random > 5) {
				nes.p.r2007.value = ppu_rd_mem(nes.p.r2006.value);
				r2006_during_rendering()
			}
			/* ppu open bus */
			nes.p.ppu.openbus = value;
			ppu_openbus_wr_all();
			return (value);
		}

		while (repeat > 0) {
			/* Read da $2007
			 * When reading Address $0000 - $3EFF from $2007
			 * the value return is the *last* value read from the
			 * PPU. So if you were to set the Address to $2000 and
			 * start reading the nametable bytes, the first byte would
			 * be whatever you laset *read* and the next read will be
			 * the byte at $2000. However, if the Read if
			 * $3F00 - $3FFF (palette), you will get the palette
			 * byte right away, the value at $2xxx will replace the
			 * pipe value wich you will receive the next time
			 * you read from 0-3EFF.
			 * Sembra che il registro utilizzi un buffer interno
			 * che ha questo ritardo nel leggere da $0000 - $3EFF.
			 */
			if ((nes.p.r2006.value & 0x3FFF) < 0x3F00) {
				value = nes.p.r2007.value;
				nes.p.r2007.value = ppu_rd_mem(nes.p.r2006.value);
				/* ppu open bus */
				nes.p.ppu.openbus = value;
				ppu_openbus_wr_all();
			} else {
				value = ppu_rd_mem(nes.p.r2006.value);
				nes.p.r2007.value = ppu_rd_mem(nes.p.r2006.value & 0x2FFF);
				/* ppu open bus */
				value = nes.p.ppu.openbus = (value & 0x3F) | (nes.p.ppu.openbus & 0xC0);
				ppu_openbus_wr(bit0);
				ppu_openbus_wr(bit1);
				ppu_openbus_wr(bit2);
				ppu_openbus_wr(bit3);
				ppu_openbus_wr(bit4);
				ppu_openbus_wr(bit5);
			}
			r2006_during_rendering()
			repeat--;
			if (extcl_update_r2006) {
				/*
				 * utilizzato dalle mappers :
				 * MMC3
				 * Rex (DBZ)
				 * Taito (TC0690)
				 * Tengen (Rambo)
				 */
				if (!nes.p.ppu.vblank && nes.p.r2001.visible && (nes.p.ppu.frame_y > nes.p.ppu_sclines.vint) && (nes.p.ppu.screen_y < SCR_ROWS)) {
					extcl_update_r2006(nes.p.r2006.value & 0x2FFF, old_r2006);
				} else {
					extcl_update_r2006(nes.p.r2006.value, old_r2006);
				}
			}
		}
		return (value);
	}

#if defined (DEBUG)
	//printf("Alert: Attempt to read PPU port %04X\n", address);
#endif

	/* ppu open bus */
	return (nes.p.ppu.openbus);
}
INLINE static BYTE apu_rd_reg(WORD address) {
	BYTE value = nes.c.cpu.openbus;

	if (address == 0x4015) {
		/* azzero la varibile d'uscita */
		value = 0;
		/*
		 * per ogni canale controllo se il length counter
		 * non e' a 0 e se si setto a 1 il bit corrispondente
		 * nel byte di ritorno.
		 */
		if (S1.length.value) {
			value |= 0x01;
		}
		if (S2.length.value) {
			value |= 0x02;
		}
		if (TR.length.value) {
			value |= 0x04;
		}
		if (NS.length.value) {
			value |= 0x08;
		}
		/* indico se ci sono bytes da processare ancora */
		if (DMC.remain) {
			value |= 0x10;
		}
		/* APU status register */
		value |= (r4015.value & 0xC0);
		/* azzero il bit 6 (interrupt flag) */
		r4015.value &= 0xBF;
		/*
		 * disabilito l'IRQ del frame counter ma solo se
		 * l'irq non e' stato raggiunto nel tick precedente
		 * (che in questo caso corrisponde allo stesso della
		 * lettura.
		 * rom interessate :
		 * test_cpu_flag_concurrency.nes (by Bisqwit)
		 */
		if ((nes.c.irq.high & APU_IRQ) && nes.c.irq.before) {
			nes.c.irq.high &= ~APU_IRQ;
		}
#if defined (DEBUG)
	//} else {
	//	printf("Alert: Attempt to read APU port %04X\n", address);
#endif
	}

	if (extcl_rd_apu) {
		/*
		 * utilizzato dalle mappers :
		 * OneBus
		 */
		value = extcl_rd_apu(address, value);
	}

	return (value);
}
INLINE static void nsf_rd_mem(WORD address, BYTE made_tick) {
	// Rom
	if (address >= 0x8000) {
		if (made_tick) {
			tick_hw(1);
		}
		switch (address) {
			case 0xFFFA:
				if (nsf.routine.INT_NMI) {
					nsf.routine.INT_NMI--;
					if (nsf.state & NSF_CHANGE_SONG) {
						nes.c.cpu.openbus = 0x00;
						snd_reset_buffers();
						nsf_reset();
					} else {
						nes.c.cpu.openbus = 0x0E;
					}
					return;
				}
				break;
			case 0xFFFB:
				if (nsf.routine.INT_NMI) {
					nsf.routine.INT_NMI--;
					nes.c.cpu.openbus = 0x25;
					snd_reset_buffers();
					nsf_reset();
					return;
				}
				break;
			case 0xFFFC:
				if (nsf.routine.INT_RESET) {
					nsf.routine.INT_RESET--;
					nes.c.cpu.openbus = 0x08;
					snd_reset_buffers();
					nsf_reset();
					return;
				}
				break;
			case 0xFFFD:
				if (nsf.routine.INT_RESET) {
					nsf.routine.INT_RESET--;
					nes.c.cpu.openbus = 0x25;
					snd_reset_buffers();
					nsf_reset();
					return;
				}
				break;
			default:
				break;
		}
		nes.c.cpu.openbus = prgrom_rd(address);
		return;
	}
	// Ram
	if (address >= 0x6000) {
		if (made_tick) {
			tick_hw(1);
		}
		nes.c.cpu.openbus = wram_rd(address);
		return;
	}
	// APU
	if (address == 0x4015) {
		nes.c.cpu.openbus = apu_rd_reg(address);
		tick_hw(1);
		return;
	}
	// FDS
	if (nsf.sound_chips.fds) {
		if ((address >= 0x4040) && (address <= 0x407F)) {
			fds_rd_mem(address, made_tick);
			return;
		}
		if (address == 0x4090) {
			fds_rd_mem(address, made_tick);
			return;
		}
		if (address == 0x4092) {
			fds_rd_mem(address, made_tick);
			return;
		}
	}
	// MMC5
	if (nsf.sound_chips.mmc5) {
		if ((address >= 0x5C00) && (address <= 0x5FF5)) {
			address &= 0x03FF;
			nes.c.cpu.openbus = m005.ext_ram[address];
			return;
		}
		switch (address) {
			case 0x5015:
			case 0x5205:
			case 0x5206:
				nes.c.cpu.openbus = extcl_cpu_rd_mem_005(address, nes.c.cpu.openbus);
				return;
			default:
				break;
		}
	}
	// Namco 163
	if (nsf.sound_chips.namco163 && (address == 0x4800)) {
		nes.c.cpu.openbus = extcl_cpu_rd_mem_019(address, nes.c.cpu.openbus);
		return;
	}
	// RAM
	if (address < 0x2000) {
		if (made_tick) {
			tick_hw(1);
		}
		nes.c.cpu.openbus = ram_rd(address & 0x7FF);
		return;
	}
	// NSF Player Routine
	if ((address >= NSF_R_START) && (address <= NSF_R_END)) {
		if (made_tick) {
			tick_hw(1);
		}

		switch (address) {
			case 0x2500:
				nsf_init_tune();
				break;
			case NSF_R_PLAY_INST:
				nsf.routine.prg[NSF_R_JMP_PLAY] = NSF_R_LOOP;
				break;
			default:
				break;
		}

		nes.c.cpu.openbus = nsf.routine.prg[address & NSF_R_MASK];
		return;
	}
}
INLINE static BYTE fds_rd_mem(WORD address, BYTE made_tick) {
	if (address >= 0x8000) {
		/* eseguo un tick hardware */
		if (made_tick) {
			tick_hw(1);
		}
		/* leggo */
		nes.c.cpu.openbus = extcl_cpu_rd_mem(address, nes.c.cpu.openbus);
		return (TRUE);
	}
	if (address >= 0x6000) {
		/* eseguo un tick hardware */
		if (made_tick) {
			tick_hw(1);
		}
		/* leggo */
		nes.c.cpu.openbus = wram_rd(address);
		return (TRUE);
	}
	if (fds.drive.enabled_dsk_reg && ((address >= 0x4030) && (address <= 0x4033))) {
		/* eseguo un tick hardware */
		tick_hw(1);

		if (address == 0x4030) {
			/*
			 * 7  bit  0
			 * ---------
			 * IExB xxTD
			 * ||||   ||
			 * ||||   |+- Timer Interrupt (1: an IRQ occurred)
			 * ||||   +-- Byte transfer flag. Set every time 8 bits
			 * ||||         have been transfered between the RAM adaptor & disk
			 * ||||         drive (service $4024/$4031).
			 * ||||         Reset when $4024, $4031, or $4030 has been serviced.
			 * |||+------ CRC control (0: CRC passed; 1: CRC error)
			 * |+-------- End of Head (1 when disk head is on the most inner track)
			 * +--------- Disk Data Read/Write Enable (1 when disk is readable/writable)
			 */
			/* azzero */
			nes.c.cpu.openbus = 0;
			/* bit 0 (timer irq) */
			nes.c.cpu.openbus |= fds.drive.irq_timer_high;
			/* bit 1 (trasfer flag) */
			nes.c.cpu.openbus |= fds.drive.irq_disk_high;
			/* bit 2 e 3 non settati */
			/* TODO : bit 4 (CRC control : 0 passato, 1 errore) */
			/* bit 5 non settato */
			/* TODO : bit 6 (end of head) */
			nes.c.cpu.openbus |= fds.drive.end_of_head;
			//fds.drive.end_of_head = FALSE;
			/* TODO : bit 7 (disk data read/write enable (1 when disk is readable/writable) */
			/* devo disabilitare sia il timer IRQ ... */
			fds.drive.irq_timer_high = FALSE;
			nes.c.irq.high &= ~FDS_TIMER_IRQ;
			/* che il disk IRQ */
			fds.drive.irq_disk_high = FALSE;
			nes.c.irq.high &= ~FDS_DISK_IRQ;
#if !defined (RELEASE)
			//printf("0x%04X 0x%02X %d\n", address, nes.c.cpu.openbus, nes.c.irq.high);
#endif
			return (TRUE);
		}
		if (address == 0x4031) {
			nes.c.cpu.openbus = fds.drive.data_readed;
#if !defined (RELEASE)
			/*
			printf("0x%04X 0x%02X [0x%04X] 0x%04X %d %d %d\n", address, nes.c.cpu.openbus,
				fds.side.data[fds.drive.disk_position], nes.c.cpu.opcode_PC, fds.drive.disk_position,
				fds.info.sides_size[fds.drive.side_inserted], nes.c.irq.high);
			*/
#endif
			/* devo disabilitare il disk IRQ */
			fds.drive.irq_disk_high = FALSE;
			nes.c.irq.high &= ~FDS_DISK_IRQ;
			return (TRUE);
		}
		if (address == 0x4032) {
			/*
			 * 7  bit  0
			 * ---------
			 * xxxx xPRS
			 *       |||
			 *       ||+- Disk flag (0: Disk inserted; 1: Disk not inserted)
			 *       |+-- Ready flag (0: Disk read; 1: Disk not ready)
			 *       +--- Protect flag (0: Not write protected; 1: Write protected or disk ejected)
			 */
			nes.c.cpu.openbus &= ~0x07;

			if (fds.drive.disk_ejected) {
				nes.c.cpu.openbus |= 0x07;
			} else if (!fds.drive.scan) {
				nes.c.cpu.openbus |= 0x02;
			}

			if (fds_auto_insert_enabled()) {
				if ((nes.p.ppu.frames - fds.auto_insert.r4032.frames) < 100) {
					if ((++fds.auto_insert.r4032.checks > FDS_AUTOINSERT_R4032_MAX_CHECKS) &&
						(fds.auto_insert.delay.side == -1) &&
						(fds.auto_insert.delay.eject == -1) &&
						(fds.auto_insert.delay.dummy == -1)) {
						// FDS interessati :
						// - Ao no Senritsu (1987)(Gakken)(J).fds
						// - Bishojou SF Alien Battle (19xx)(Hacker International)(J)(Unl)[b].fds
						// senza questo controllo entrambi gli fds potrebbero rimanere in attesa
						// del cambio side senza che l'auto insert avvenga visto che l'END_OF_HEAD
						// è stato raggiunto prima.
						fds.auto_insert.r4032.checks = 0;
						fds.auto_insert.delay.eject = FDS_OP_SIDE_DELAY;
					}
				} else {
					fds.auto_insert.r4032.checks = 0;
				}
				fds.auto_insert.r4032.frames = nes.p.ppu.frames;
			}

			return (TRUE);
		}
		if (address == 0x4033) {
			/*
			 * 7  bit  0
			 * ---------
			 * BIII IIII
			 * |||| ||||
			 * |+++-++++- Input from expansion terminal where there's a shutter
			 * |            on the back of the ram card.
			 * +--------- Battery status (0: Good; 1: Voltage is low).
			 */
			nes.c.cpu.openbus = fds.drive.data_external_connector & 0x80;
			return (TRUE);
		}
	}
	if (fds.drive.enabled_snd_reg) {
		if ((address >= 0x4040) && (address <= 0x407F)) {
			/* eseguo un tick hardware */
			tick_hw(1);
			/*
			 * 7  bit  0  (read/write)
			 * ---- ----
			 * OOSS SSSS
			 * |||| ||||
			 * ||++-++++- Sample
			 * ++-------- Returns 01 on read, likely from open bus
			 */
			// When writing is disabled ($4089.7), reading anywhere in 4040-407F returns the value at the current wave position.
			nes.c.cpu.openbus = (fds.snd.wave.writable ? fds.snd.wave.data[address & 0x3F] : fds.snd.wave.data[fds.snd.wave.index]) |
				(nes.c.cpu.openbus & 0xC0);
			return (TRUE);
		}
		if (address == 0x4090) {
			/* eseguo un tick hardware */
			tick_hw(1);
			nes.c.cpu.openbus = (fds.snd.volume.gain & 0x3F) | (nes.c.cpu.openbus & 0xC0);
			return (TRUE);
		}
		if (address == 0x4092) {
			/* eseguo un tick hardware */
			tick_hw(1);
			nes.c.cpu.openbus = (fds.snd.sweep.gain & 0x3F) | (nes.c.cpu.openbus & 0xC0);
			return (TRUE);
		}
	}
	if (address > 0x4017) {
		/* eseguo un tick hardware */
		tick_hw(1);
		return (TRUE);
	}

	return (FALSE);
}

/* ------------------------------------ WRITE ROUTINE ------------------------------------------ */

void cpu_wr_mem(WORD address, BYTE value) {
	if (info.cpu_rw_extern) {
		if (nsf.enabled) {
			nsf_wr_mem(address, value);
			return;
		} else if (fds.info.enabled) {
			if (fds_wr_mem(address, value)) {
				return;
			}
		}
	}

	if (address <= 0x403F) {
		/* Ram */
		if (address < 0x2000) {
			/* eseguo un tick hardware */
			tick_hw(1);
			/* scrivo */
			ram_wr(address, value);
			return;
		}
		if (address < 0x4000) {
			if (extcl_wr_ppu_reg) {
				/*
				 * utilizzato dalle mappers :
				 * OneBus
				 */
				if (extcl_wr_ppu_reg(address, &value)) {
					/* eseguo un tick hardware */
					tick_hw(1);
					return;
				}
			}

			address &= 0x2007;

			// Vs. System
			if (vs_system.rc2c05.enabled) {
				if (address == 0x2000) {
					address = 0x2001;
				} else if (address == 0x2001) {
					address = 0x2000;
				}
			}

			/*
			 * per riuscire a far funzionare contemporaneamente
			 * Battletoads e Fighting Road (J) senza trick, devo
			 * far prima eseguire la scrittura di questo registro
			 * e poi eseguire il tick hardware. Queste due rom vanno
			 * a scrivere in questo registro verso la fine della
			 * scanline e se il tick e' eseguito prima, posso ritrovarmi
			 * con la scanline gia' finita prima della scrittura.
			 * Inoltre cosi' ho risolto anche i problemi con
			 * Incredible Crash Dummies, The (U) [!] e Rad Racer 2 (U) [!].
			 */
			if (address == 0x2005) {
				/* scrivo */
				ppu_wr_reg(address, value);
				/* eseguo un tick hardware */
				tick_hw(1);
				return;
			}

			/* eseguo un tick hardware */
			tick_hw(1);
			/* scrivo */
			ppu_wr_reg(address, value);
			return;
		}

		if (extcl_wr_apu) {
			/*
			 * utilizzato dalle mappers :
			 * OneBus
			 */
			if (extcl_wr_apu(address, &value)) {
				/* eseguo un tick hardware */
				tick_hw(1);
				return;
			}
		}

		/* Sprite memory */
		if (address == 0x4014) {
			DMC.tick_type = DMC_R4014;
			/* eseguo un tick hardware */
			tick_hw(1);
			/* scrivo */
			ppu_wr_reg(address, value);
			return;
		}
		/* Controller */
		if (address == 0x4016) {
			/* eseguo un tick hardware */
			tick_hw(1);
			if (extcl_cpu_wr_r4016) {
				/*
				 * utilizzato dalle mappers :
				 * Vs
				 */
				extcl_cpu_wr_r4016(value);
			}
			/* memorizzo il nuovo valore */
			r4016.value = input_wr_reg(value);
			return;
		}
		/* APU */
		if (address == 0x4015) {
			/*
			 * per riuscire a far funzionare contemporaneamente
			 * sprdma_and_dmc_dma.nes, devo
			 * far prima eseguire la scrittura di questo registro
			 * e poi eseguire il tick hardware.
			 */
			/* scrivo */
			apu_wr_reg(address, value);
			/* eseguo un tick hardware */
			tick_hw(1);
			return;
		}
		if (address <= 0x4017) {
			/* eseguo un tick hardware */
			tick_hw(1);
			/* scrivo */
			apu_wr_reg(address, value);
			return;
		}
	}
	// WRam (normale ed extra) */
	if (address < 0x8000) {
		/* eseguo un tick hardware */
		tick_hw(1);

		wram_wr(address, value);

		if (info.mapper.extend_wr) {
			extcl_cpu_wr_mem(address, value);
		}
		return;
	}

	prgrom_wr(address, value);

	extcl_cpu_wr_mem(address, value);

	/* su questo devo fare qualche altro esperimento */
	tick_hw(1);
}
void apu_wr_mem_mapper(WORD address, BYTE value) {
	apu_wr_reg(address, value);
}
INLINE static void ppu_wr_mem(WORD address, BYTE value) {
	address &= 0x3FFF;
	if (address < 0x2000) {
		if (extcl_wr_chr) {
			extcl_wr_chr(address, value);
		} else {
			chr_wr(address, value);
		}
		return;
	}
	if (address < 0x3F00) {
		if (extcl_wr_nmt) {
			extcl_wr_nmt(address, value);
		} else  {
			nmt_wr(address, value);
		}
		return;
	}
	address &= 0x1F;
	nes.m.memmap_palette.color[address] = value & 0x3F;
	if (!(address & 0x03)) {
		nes.m.memmap_palette.color[(address + 0x10) & 0x1F] = nes.m.memmap_palette.color[address];
	}
}
INLINE static void ppu_wr_reg(WORD address, BYTE value) {
	if (address == 0x2000) {
#if !defined (RELEASE)
		BYTE old_delay = FALSE;
#endif

		/*
		 * se l'nmi e' attivo quando scrivo nel registro r2000
		 * deve essere eseguito nell'istruzione successiva.
		 * L'ho notato con "baxter,zugzwang-adventuresoflolo.fm2"
		 * al tas.frame 53762. Se non c'e' questo vengono generati
		 * 2 nmi nello stesso frame, perche' il primo frame e' generato
		 * proprio nella scrittura di questo registro e la mancata
		 * esecuzione dell'istruzione successiva pone le condizioni
		 * per il secondo nmi che avverra' 25 scanline dopo.
		 */
		if (nes.c.nmi.high && (nes.c.nmi.cpu_cycles_from_last_nmi <= nes.c.cpu.base_opcode_cycles)) {
#if !defined (RELEASE)
			old_delay = TRUE;
#endif
			nes.c.nmi.delay = TRUE;
		}

		/* open bus */
		nes.p.ppu.openbus = value;
		ppu_openbus_wr_all();

		/*
		 * 76543210
		 * ||||||||
		 * ||||||++- Base nametable address
		 * ||||||    (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
		 * |||||+--- VRAM address increment per CPU read/write of PPUDATA
		 * |||||     (0: increment by 1, going across; 1: increment by 32, going down)
		 * ||||+---- Sprite pattern table address for 8x8 sprites
		 * ||||      (0: $0000; 1: $1000; ignored in 8x16 mode)
		 * |||+----- Background pattern table address (0: $0000; 1: $1000)
		 * ||+------ Sprite size (0: 8x8; 1: 8x16)
		 * |+------- PPU master/slave select (has no effect on the NES)
		 * +-------- Generate an NMI at the start of the
		 *           vertical blanking interval (0: off; 1: on)
		 */
		/*
		 * se il bit 7 passa da 0 a 1 e il bit 7
		 * del $2002 e' a 1 devo generare un NMI
		 * ma dopo l'istruzione successiva.
		 */
		if (!nes.p.r2000.nmi_enable && (value & 0x80)) {
			if (nes.p.r2002.vblank) {
				nes.c.nmi.high = nes.c.nmi.delay = TRUE;
				nes.c.nmi.frame_x = nes.p.ppu.frame_x;
			}
			/*
			 * se viene disabilitato l'NMI (bit 7 da 1 a 0)
			 * all'inizio del vblank, l'NMI generato deve
			 * essere disabilitato.
			 */
		} else if (nes.p.r2000.nmi_enable && !(value & 0x80)) {
			if (!(nes.p.ppu.frame_y | nes.c.nmi.before)) {
				nes.c.nmi.high = nes.c.nmi.delay = FALSE;
			}
		}
		/* valorizzo $2000 */
		nes.p.r2000.value = value;
		/* NMI abilitato */
		nes.p.r2000.nmi_enable = value & 0x80;
		/* VRAM address increment */
		(value & 0x04) ? (nes.p.r2000.r2006_inc = 32) : (nes.p.r2000.r2006_inc = 1);
		/* memorizzo la dimensione degli sprites */
		(value & 0x20) ? (nes.p.r2000.size_spr = 16) : (nes.p.r2000.size_spr = 8);
		/* Sprite pattern table address */
		nes.p.r2000.spt_adr = (value & 0x08) << 9;
		/* Background pattern table address */
		nes.p.r2000.bpt_adr = (value & 0x10) << 8;
		/*
		 * NN -> Name-table Bits. Vertical bit e Horizontal bit
		 *
		 * $2000      W %---- --NN
		 * bitsNT       %0000 00NN
		 * tmp_vram      %---- NN-- ---- ----
		 */
		if ((nes.p.ppu.frame_x == 257) && (!nes.p.ppu.vblank && (!nes.p.r2001.spr_visible && nes.p.r2001.bck_visible) && (nes.p.ppu.screen_y < SCR_ROWS))) {
			/*
			 * gestione della condizione di race del $2000 al dot 257
			 * https://forums.nesdev.com/viewtopic.php?f=3&t=18113
			 * ppu_2000_glitch.nes e ppu_2100_glitch.nes
			 */
			nes.p.r2000.race.ctrl = TRUE;
			nes.p.r2000.race.value = value;
			nes.p.ppu.tmp_vram = (nes.p.ppu.tmp_vram & 0xF3FF) | ((nes.c.cpu.openbus & 0x03) << 10);
			r2006_end_scanline();
		} else {
			nes.p.ppu.tmp_vram = (nes.p.ppu.tmp_vram & 0xF3FF) | ((value & 0x03) << 10);
		}

		/*
		 * per questo registro il tick_hw e' gia' stato effettuato, quindi
		 * la PPU ha gia' fatto 3 cicli. Se sono nel range 253 - 255 del
		 * nes.p.ppu.frame_x, il registro $2006 e' gia' stato aggiornato dalla PPU
		 * (cosa che avviene nel ciclo 253). Questa variazione e' direttamente
		 * controllata anche dal valore di nes.p.ppu.tmp_vram, quindi una scrittura
		 * in questo registro nella PPU che ha gia' fatto il ciclo 253, non
		 * influenzerebbe il $2006 e questo e' sbagliato. Quindi lo ricalcolo io,
		 * ma solo so sono in fase di rendering.
		 * Rom interessate:
		 * Road Runner (Tengen) [!].nes
		 * (premuto il tasto start due volte ed avviato il gioco, una riga
		 * piu' scura sfarfalla nello schermo).
		 */
		if (((nes.p.ppu.frame_x >= 253) && (nes.p.ppu.frame_x <= 255)) && (!nes.p.ppu.vblank && nes.p.r2001.visible && (nes.p.ppu.screen_y < SCR_ROWS))) {
			r2006_end_scanline();
		}

#if !defined (RELEASE)
		if (old_delay && nes.c.nmi.high) {
			log_warning(uL("cpu_inline;r2000 nmi high, set delay nes.c.nmi.before, %d %d %d - %d %d - 0x%02X %d"),
				nes.p.ppu.frames, nes.p.ppu.frame_y, nes.p.ppu.frame_x, nes.c.nmi.frame_x, nes.c.nmi.cpu_cycles_from_last_nmi,
				nes.c.cpu.opcode, nes.c.cpu.base_opcode_cycles);
		}
#endif
		return;
	}
	if (address == 0x2001) {
		/* open bus */
		nes.p.ppu.openbus = value;
		ppu_openbus_wr_all();

		/*
		 * se viene scritto esattamente nel nes.p.ppu.frame_x compreso tra (326 e 328)
		 * e tra (334 e 336) azzerando l'nes.p.r2001.visible, inibirebbe il
		 * "FETCH TILE 0 e 1 SCANLINE+1" non permettendo piu' l'incremento del
		 * nes.p.r2006.value (glitch grafici in "Micro Machines (Camerica) [!].nes".
		 */
		if (((nes.p.ppu.frame_x >= 326) && (nes.p.ppu.frame_x <= 328)) ||
			((nes.p.ppu.frame_x >= 334) && (nes.p.ppu.frame_x <= 336))) {
			if (nes.p.r2001.visible) {
				nes.p.r2001.race.ctrl = TRUE;
				nes.p.r2001.race.value = nes.p.r2001.visible;
			}
		} else if ((nes.p.ppu.frame_x >= 338) && (nes.p.ppu.frame_x <= 339)) {
			if (machine.type == NTSC) {
				nes.p.r2001.race.ctrl = TRUE;
				nes.p.r2001.race.value = nes.p.r2001.visible;
			}
		}

		/* valorizzo $2001 */
		nes.p.r2001.value = value;
		/*
		 * con il bit 0 settato viene indicata
		 * la modalita' scale di grigio per l'output.
		 * Sembra proprio che questo bit abbia effetto due pixel dopo essere stato settato
		 * quindi al terzo pixel.
		 * http://tasvideos.org/forum/viewtopic.php?p=440662&sid=6a9e6eadb7600425864d6c4e96170223#440662
		 * nmi_sync test ros (demo_ntsc.nes e demo_pal.nes).
		 */
		if (value & 0x01) {
			nes.p.r2001.grayscale_bit.delay = 2 + 1;
		} else {
			nes.p.r2001.grayscale_bit.delay = 0;
			nes.p.r2001.color_mode = PPU_CM_NORMAL;
		}
		/* visibilita' del background */
		nes.p.r2001.bck_visible = value & 0x08;
		/* visibilita' degli sprites */
		nes.p.r2001.spr_visible = value & 0x10;
		/* basta che uno dei due sia visibile */
		nes.p.r2001.visible = nes.p.r2001.bck_visible | nes.p.r2001.spr_visible;
		/* questo per ora mi serve solo per l'A12 */
		/* MMC3 and Taito*/
		if (nes.p.r2001.visible) {
			if (irqA12.present) {
				irqA12.s_adr_old = irqA12.b_adr_old = 0;
			}
		} else {
			if (irql2f.present) {
				irql2f.in_frame = FALSE;
			}
		}
		/* clipping del background */
		nes.p.r2001.bck_clipping = value & 0x02;
		/* clipping degli sprites */
		nes.p.r2001.spr_clipping = value & 0x04;
		/* salvo la maschera di enfatizzazione del colore */
		nes.p.r2001.emphasis = (value << 1) & 0x1C0;
		return;
	}
	if (address == 0x2003) {
		/* open bus */
		nes.p.ppu.openbus = value;
		ppu_openbus_wr_all();
		nes.p.r2003.value = value;
		return;
	}
	if (address == 0x2004) {
		/* open bus */
		nes.p.ppu.openbus = value;
		ppu_openbus_wr_all();
		/*
		 * il 3° byte dei quattro che compongono un elemento
		 * dell'oam hai i bit 2,3 e 4 non sono implementati quindi
		 * non devo salvarli.
		 *
		 * 76543210
		 * ||||||||
		 * ||||||++- Palette (4 to 7) of sprite
		 * |||+++--- Unimplemented
		 * ||+------ Priority (0: in front of background; 1: behind background)
		 * |+------- Flip sprite horizontally
		 * +-------- Flip sprite vertically
		 */
		if ((nes.p.r2003.value & 0x03) == 0x02) {
			value &= 0xE3;
		}
		nes.p.oam.data[nes.p.r2003.value++] = value;
		return;
	}
	if (address == 0x2005) {
		/* open bus */
		nes.p.ppu.openbus = value;
		ppu_openbus_wr_all();

		/*
		 * Bit totali manipolati con $2005:
		 * tmpAdrVRAM %0yyy --YY YYYX XXXX
		 */
		if (!nes.p.r2002.toggle) {
			/*
			 * XXXXX -> Tile X
			 * xxx   -> Fine X
			 *
			 * $2005       W %XXXX Xxxx (toggle e' 0)
			 * fine_x        %0000 0xxx
			 * tmp_vram      %0--- ---- ---X XXXX
			 * toggle = 1
			 */
			nes.p.ppu.fine_x = (value & 0x07);
			nes.p.ppu.tmp_vram = (nes.p.ppu.tmp_vram & 0x7FE0) | (value >> 3);
		} else {
			/*
			 * YYYYY -> Tile Y
			 * yyy   -> Fine Y
			 *
			 * $2005       W %YYYY Yyyy (toggle e' 1)
			 * tmpAdrVRAM    %0yyy --YY YYY- ----
			 * toggle = 0
			 */
			nes.p.ppu.tmp_vram = (nes.p.ppu.tmp_vram & 0x0C1F) | ((value & 0xF8) << 2) | ((value & 0x07) << 12);
		}

		nes.p.r2002.toggle = !nes.p.r2002.toggle;
		return;
	}
	if (address == 0x2006) {
		/* open bus */
		nes.p.ppu.openbus = value;
		ppu_openbus_wr_all();

		/*
		 * Bit totali manipolati con $2006:
		 * tmpAdrVRAM  %00yy NNYY YYYX XXXX
		 */
		if (!nes.p.r2002.toggle) {
			/*
			 * YYYYY -> Tile Y
			 * yyy   -> Fine Y
			 * NN    -> Name-table Bits. Vertical bit e Horizontal bit
			 *
			 * $2006       W %--yy NNYY (toggle e' 0)
			 * tmpAdrVRAM    %00yy NNYY ---- ----
			 * toggle = 1
			 */
			nes.p.ppu.tmp_vram = (nes.p.ppu.tmp_vram & 0x00FF) | ((value & 0x3F) << 8);
		} else {
			/*
			 * YYYYY -> Tile Y
			 * XXXXX -> Tile X
			 *
			 * $2006        W %YYYX XXXX (toggle e' 1)
			 * tmpAdrVRAM     %0--- ---- YYYX XXXX
			 * toggle = 0
			 * addressVRAM = tmpAdrVRAM
			 */

			// http://forums.nesdev.com/viewtopic.php?p=189463#p189463
			// sembra che la seconda scrittura del $2006 avvenga con qualche
			// ciclo ppu di ritardo.
			nes.p.r2006.second_write.value = (nes.p.ppu.tmp_vram & 0x7F00) | value;
			nes.p.r2006.second_write.delay = 3;
		}

		nes.p.r2002.toggle = !nes.p.r2002.toggle;
		return;
	}
	if (address == 0x2007) {
		const WORD old_r2006 = nes.p.r2006.value;

		/* open bus */
		nes.p.ppu.openbus = value;
		ppu_openbus_wr_all();

		if (!nes.p.ppu.vblank && nes.p.r2001.visible && (nes.p.ppu.frame_y > nes.p.ppu_sclines.vint) && (nes.p.ppu.screen_y < SCR_ROWS)) {
			ppu_wr_mem(nes.p.ppu.rnd_adr, nes.p.ppu.rnd_adr & 0x00FF);
			_r2006_during_rendering()
		} else {
			ppu_wr_mem(nes.p.r2006.value, value);
			nes.p.r2006.value += nes.p.r2000.r2006_inc;
		}

		if (extcl_update_r2006) {
			 /*
			  * utilizzato dalle mappers :
			  * MMC3
			  * Rex (DBZ)
			  * Taito (TC0690)
			  * Tengen (Rambo)
			  */
			extcl_update_r2006(nes.p.r2006.value, old_r2006);
		}
		return;
	}
	if (address == 0x4014) {
		/*
		 * se durante l'ultimo ciclo dell'istruzione
		 * e' stato richiesto un IRQ, deve essere ritardato
		 * all'istruzione successiva.
		 */
		if (nes.c.irq.high && !nes.c.cpu.cycles && !nes.c.irq.before) {
			nes.c.irq.delay = TRUE;
		}
		/* DMA transfer source address */
		address = value << 8;
		{
			WORD index = 0;
			BYTE save_irq = nes.c.irq.high;
			BYTE save_cpu_cycles = nes.c.cpu.cycles;

			if (info.r4014_precise_timing_disabled) {
				mod_cycles_op(+=, 512);
			} else {
				/*
			 	 * su un 2A03 reale, questo ciclo viene
			 	 * lasciato per completare la scrittura nel registro.
			 	 */
				tick_hw(1);
				/* sono 513 i cicli CPU che il trasferimento si prende */
				mod_cycles_op(+=, 513);
				/*
				 * se l'avvio del trasferimento avviene
				 * in un ciclo dispari allora c'e' un ritardo
				 * di un altro ciclo (quindi in totale diventano
				 * 514).
				 */
				if ((machine.type == NTSC) && nes.c.cpu.odd_cycle) {
					tick_hw(1);
					mod_cycles_op(+=, 1);
				}
			}
			for (index = 0; index < 256; ++index) {
				/*
				 * ogni trasferimento prende due cicli, uno di
				 * lettura (contenuto nel cpu_rd_mem e l'altro di
				 * scrittura che faccio prima di valorizzare
				 * l'nes.p.oam.
				 */
				if (index == 253) {
					cpu_rd_mem(address++, TRUE);
					DMC.tick_type = DMC_NNL_DMA;
				} else if (index == 254) {
					cpu_rd_mem(address++, TRUE);
					DMC.tick_type = DMC_R4014;
				} else if (index == 255) {
					DMC.tick_type = DMC_CPU_WRITE;
					cpu_rd_mem(address++, TRUE);
				} else {
					cpu_rd_mem(address++, TRUE);
				}
				tick_hw(1);
				if ((nes.p.r2003.value & 0x03) == 0x02) {
					nes.c.cpu.openbus &= 0xE3;
				}
				nes.p.oam.data[nes.p.r2003.value++] = nes.c.cpu.openbus;
			}
			/*
			 * se sopraggiunge un IRQ durante i 513/514 cicli
			 * e non ci sono altri cicli dell'istruzione,
			 * l'IRQ deve essere ritardato di una istruzione.
			 */
			if (nes.c.irq.high && !(save_irq | save_cpu_cycles)) {
				nes.c.irq.delay = TRUE;
			}
		}
		return;
	}

#if defined (DEBUG)
	/* non si puo' scrivere nel registro $2002 */
	//printf("Alert: Attempt to write PPU port %04X\n", address);
#endif

	/* open bus */
	nes.p.ppu.openbus = value;
	ppu_openbus_wr_all();
}
INLINE static void apu_wr_reg(WORD address, BYTE value) {
	if (!(address & 0x0010)) {
		/* -------------------- square 1 --------------------*/
		if (address <= 0x4003) {
			if (address == 0x4000) {
				square_reg0(S1);
				return;
			}
			if (address == 0x4001) {
				square_reg1(S1);
				sweep_silence(S1)
				return;
			}
			if (address == 0x4002) {
				square_reg2(S1);
				sweep_silence(S1)
				return;
			}
			if (address == 0x4003) {
				square_reg3(S1);
				sweep_silence(S1)
				return;
			}
			return;
		}
		/* -------------------- square 2 --------------------*/
		if (address <= 0x4007) {
			if (address == 0x4004) {
				square_reg0(S2);
				return;
			}
			if (address == 0x4005) {
				square_reg1(S2);
				sweep_silence(S2)
				return;
			}
			if (address == 0x4006) {
				square_reg2(S2);
				sweep_silence(S2)
				return;
			}
			if (address == 0x4007) {
				square_reg3(S2);
				sweep_silence(S2)
				return;
			}
			return;
		}
		/* -------------------- triangle --------------------*/
		if (address <= 0x400B) {
			if (address == 0x4008) {
				/* length counter */
				/*
				 * il triangle ha una posizione diversa per il
				 * flag LCHalt.
				 */
				TR.length.halt = value & 0x80;
				/* linear counter */
				TR.linear.reload = value & 0x7F;
				return;
			}
			if (address == 0x400A) {
				/* timer (low 8 bits) */
				TR.timer = (TR.timer & 0x0700) | value;
				return;
			}
			if (address == 0x400B) {
				/* length counter */
				/*
				 * se non disabilitato, una scrittura in
				 * questo registro, carica immediatamente il
				 * length counter del canale, tranne nel caso
				 * in cui la scrittura avvenga nello stesso
				 * momento del clock di un length counter e
				 * con il length diverso da zero.
				 */
				if (TR.length.enabled && !(apu.length_clocked && TR.length.value)) {
					TR.length.value = length_table[value >> 3];
				}
				/* timer (high 3 bits) */
				TR.timer = (TR.timer & 0x00FF) | ((value & 0x07) << 8);
				/*
				 * scrivendo in questo registro si setta
				 * automaticamente l'halt flag del triangle.
				 */
				TR.linear.halt = TRUE;
				return;
			}
			return;
		}
		/* --------------------- noise ----------------------*/
		if (address <= 0x400F) {
			if (address == 0x400C) {
				NS.length.halt = value & 0x20;
				/* envelope */
				NS.envelope.constant_volume = value & 0x10;
				NS.envelope.divider = value & 0x0F;
				return;
			}
			if (address == 0x400E) {
				NS.mode = value & 0x80;
				NS.timer = value & 0x0F;
				return;
			}
			if (address == 0x400F) {
				/*
				 * se non disabilitato, una scrittura in
				 * questo registro, carica immediatamente il
				 * length counter del canale, tranne nel caso
				 * in cui la scrittura avvenga nello stesso
				 * momento del clock di un length counter e
				 * con il length diverso da zero.
				 */
				if (NS.length.enabled && !(apu.length_clocked && NS.length.value)) {
					NS.length.value = length_table[value >> 3];
				}
				/* envelope */
				NS.envelope.enabled = TRUE;
				return;
			}
			return;
		}
		return;
	} else {
		/* ---------------------- DMC -----------------------*/
		if (address <= 0x4013) {
			if (address == 0x4010) {
				DMC.irq_enabled = value & 0x80;
				/* se l'irq viene disabilitato allora... */
				if (!DMC.irq_enabled) {
					/* ...azzero l'interrupt flag del DMC */
					r4015.value &= 0x7F;
					/* disabilito l'IRQ del DMC */
					nes.c.irq.high &= ~DMC_IRQ;
				}
				DMC.loop = value & 0x40;
				DMC.rate_index = value & 0x0F;
				return;
			}
			if (address == 0x4011) {
				BYTE save = DMC.counter;

				value &= 0x7F;

				/*
				 * questa lo faccio perche' in alcuni giochi come Batman,
				 * Ninja Gaiden 3, Castlevania II ed altri, producono
				 * un popping del suono fastidioso;
				 * from Fceu doc:
				 * Why do some games make a popping sound (Batman, Ninja Gaiden 3,
				 * Castlevania II etc.)? These games do a very crude drum imitation
				 * by causing a large jump in the output level for a short period of
				 * time via the register at $4011. The analog filters on a real
				 * Famicom make it sound decent(better). I have not completely
				 * emulated these filters.
				 * (Xodnizel)
				 */
				if (r4011.frames > 1) {
					r4011.output = (SWORD)((value - save) >> 3);
					DMC.counter = DMC.output = (SWORD)(save + r4011.output);
				} else {
					DMC.counter = DMC.output = value;
				}
				apu.clocked = TRUE;

				r4011.cycles = r4011.frames = 0;
				r4011.value = value;

				if (!nsf.enabled && cfg->ppu_overclock && !cfg->ppu_overclock_dmc_control_disabled && value) {
					nes.p.overclock.DMC_in_use = TRUE;
					nes.p.ppu_sclines.total = machine.total_lines;
					nes.p.ppu_sclines.vint = machine.vint_lines;
					ppu_overclock_control()
				}
				return;
			}
			if (address == 0x4012) {
				DMC.address_start = (value << 6) | 0xC000;

				if (!nsf.enabled && cfg->ppu_overclock && !cfg->ppu_overclock_dmc_control_disabled && value) {
					nes.p.overclock.DMC_in_use = FALSE;
					ppu_overclock_update()
					ppu_overclock_control()
				}
				return;
			}
			if (address == 0x4013) {
				/* sample length */
				DMC.length = (value << 4) | 0x01;

				if (!nsf.enabled && cfg->ppu_overclock && !cfg->ppu_overclock_dmc_control_disabled && value) {
					nes.p.overclock.DMC_in_use = FALSE;
					ppu_overclock_update()
					ppu_overclock_control()
				}
				return;
			}
			return;
		}
		/* --------------------------------------------------*/
		if (address == 0x4015) {
			/*
			 * 76543210
			 * || |||||
			 * || ||||+- Pulse channel 1's length counter enabled flag
			 * || |||+-- Pulse channel 2's length counter enabled flag
			 * || ||+--- Triangle channel's length counter enabled flag
			 * || |+---- Noise channel's length counter enabled flag
			 * || +----- If clear, the DMC's bytes remaining is set to 0,
			 * ||        otherwise the DMC sample is restarted only if the
			 * ||        DMC's bytes remaining is 0
			 * |+------- Frame interrupt flag
			 * +-------- DMC interrupt flag
			 */
			/*
			 * dopo la write il bit 7 (dmc flag) deve
			 * essere azzerato mentre lascio inalterati
			 * i bit 5 e 6.
			 */
			r4015.value = (r4015.value & 0x60) | (value & 0x1F);
			/* disabilito l'IRQ del DMC */
			nes.c.irq.high &= ~DMC_IRQ;
			/*
			 * quando il flag di abilitazione del length
			 * counter di ogni canale e' a 0, il counter
			 * dello stesso canale e' immediatamente azzerato.
			 */
			S1.length.enabled = r4015.value & 0x01;
			if (!S1.length.enabled) {
				S1.length.value = 0;
			}
			S2.length.enabled = r4015.value & 0x02;
			if (!S2.length.enabled) {
				S2.length.value = 0;
			}
			TR.length.enabled = r4015.value & 0x04;
			if (!TR.length.enabled) {
				TR.length.value = 0;
			}
			NS.length.enabled = r4015.value & 0x08;
			if (!NS.length.enabled) {
				NS.length.value = 0;
			}
			/*
			 * se il bit 4 e' 0 allora devo azzerare i bytes
			 * rimanenti del DMC, alrimenti devo riavviare
			 * la lettura dei sample DMC solo nel caso che
			 * in cui i bytes rimanenti siano a 0.
			 */
			if (!(r4015.value & 0x10)) {
				DMC.remain = 0;
				DMC.empty = TRUE;
			} else if (!DMC.remain) {
				DMC.remain = DMC.length;
				DMC.address = DMC.address_start;
			}
			return;
		}
		if (address == 0x4017) {
			/* APU frame counter */
			r4017.jitter.value = value;
			/*
			 * nell'2A03 se la scrittura del $4017 avviene
			 * in un ciclo pari, allora l'effettiva modifica
			 * avverra' nel ciclo successivo.
			 */
			if (nes.c.cpu.odd_cycle) {
				r4017.jitter.delay = TRUE;
			} else {
				r4017.jitter.delay = FALSE;
				r4017_jitter(1)
				r4017_reset_frame()
			}
			return;
		}
	}

#if defined (DEBUG)
	//printf("Alert: Attempt to write APU port %04X\n", address);
#endif
}
INLINE static void nsf_wr_mem(WORD address, BYTE value) {
	// Ram
	if (address >= 0x8000) {
		tick_hw(1);

		if (nsf.sound_chips.vrc6) {
			switch (address) {
				case 0x9000:
				case 0x9001:
				case 0x9002:
				case 0x9003:
				case 0xA000:
				case 0xA001:
				case 0xA002:
				case 0xB000:
				case 0xB001:
				case 0xB002:
					extcl_cpu_wr_mem_VRC6(address, value);
					return;
				default:
					break;
			}
		}
		if (nsf.sound_chips.vrc7) {
			switch (address) {
				case 0x9010:
				case 0x9030:
					extcl_cpu_wr_mem_VRC7(address, value);
					return;
				default:
					break;
			}
		}
		if (nsf.sound_chips.namco163 && (address == 0xF800)) {
			extcl_cpu_wr_mem_019(address, value);
			return;
		}
		if (nsf.sound_chips.sunsoft5b) {
			switch (address) {
				case 0xC000:
				case 0xE000:
					extcl_cpu_wr_mem_FME7(address, value);
					return;
				default:
					break;
			}
		}
		if (nsf.sound_chips.fds) {
			prgrom_wr(address, value);
			return;
		}
		return;
	}
	if (address >= 0x6000) {
		tick_hw(1);
		wram_wr(address, value);
		return;
	}
	if (address < 0x2000) {
		tick_hw(1);
		ram_wr(address, value);
		return;
	}
	// APU
	if (address == 0x4015) {
		apu_wr_reg(address, value);
		tick_hw(1);
		return;
	}
	if (address <= 0x4017) {
		tick_hw(1);
		apu_wr_reg(address, value);
		return;
	}
	// FDS
	if (nsf.sound_chips.fds && (address >= 0x4040) && (address <= 0x408A)) {
		fds_wr_mem(address, value);
		return;
	}
	// MMC5
	if (nsf.sound_chips.mmc5) {
		if ((address >= 0x5C00) && (address <= 0x5FF5)) {
			address &= 0x03FF;
			m005.ext_ram[address] = value;
			return;
		}
		switch (address) {
			case 0x5000:
			case 0x5001:
			case 0x5002:
			case 0x5003:
			case 0x5004:
			case 0x5005:
			case 0x5006:
			case 0x5007:
			case 0x5008:
			case 0x5009:
			case 0x500A:
			case 0x500B:
			case 0x500C:
			case 0x500D:
			case 0x500E:
			case 0x500F:
			case 0x5010:
			case 0x5011:
			case 0x5012:
			case 0x5013:
			case 0x5014:
			case 0x5015:
			case 0x5205:
			case 0x5206:
				extcl_cpu_wr_mem_005(address, value);
				return;
			default:
				break;
		}
	}
	// Namco 163
	if (nsf.sound_chips.namco163 && (address == 0x4800)) {
		extcl_cpu_wr_mem_019(address, value);
	}
	// Bankswitch
	if (nsf.bankswitch.enabled && (address >= 0x5FF6) && (address <= 0x5FFF)) {
		BYTE *dst = NULL;
		WORD bank = 0;

		tick_hw(1);

		switch (address) {
			case 0x5FF6:
			case 0x5FF7:
				if (nsf.sound_chips.fds) {
					value = prgrom_control_bank(S4K, value);
					bank = (address & 0x01);
					address = (address & 0x000F) << 12;
					memmap_wram_4k(MMCPU(address), bank);
					dst = memmap_chunk_pnt(address);
					if (dst) memcpy(dst, prgrom_pnt_byte(value << 12), S4K);
				}
				return;
			case 0x5FF8:
			case 0x5FF9:
			case 0x5FFA:
			case 0x5FFB:
			case 0x5FFC:
			case 0x5FFD:
			case 0x5FFE:
			case 0x5FFF:
				if (nsf.sound_chips.fds && (address <= 0x5FFD)) {
					value = prgrom_control_bank(S4K, value);
					bank = (address & 0x07);
					address = (address & 0x000F) << 12;
					memmap_wram_4k(MMCPU(address), bank + 2);
					dst = memmap_chunk_pnt(address);
					if (dst) memcpy(dst, prgrom_pnt_byte(value << 12), S4K);
					return;
				}
				address = (address & 0x000F) << 12;
				memmap_prgrom_4k(MMCPU(address), value);
				return;
			default:
				return;
		}
	}

	tick_hw(1);
}
INLINE static BYTE fds_wr_mem(WORD address, BYTE value) {
	if (address >= 0x8000) {
		/* eseguo un tick hardware */
		tick_hw(1);
		/* scrivo */
		prgrom_wr(address, value);
		return (TRUE);
	}
	if (address >= 0x6000) {
		/* eseguo un tick hardware */
		tick_hw(1);
		/* scrivo */
		wram_wr(address, value);
		return (TRUE);
	}
	if ((address >= 0x4020) && (address <= 0x4026)) {
		/* eseguo un tick hardware */
		tick_hw(1);

#if !defined (RELEASE)
		/*if (address == 0x4025) {
			printf("0x%04X 0x%02X %d\n", address, value, fds.drive.enabled_dsk_reg);
		} else {
			if (fds.drive.disk_position)
			printf("0x%04X 0x%02X 0x%04X %d 0x%02X %d\n", address, value, nes.c.cpu.opcode_PC,
				fds.drive.disk_position - 1, fds.side.data[fds.drive.disk_position - 1], nes.p.ppu.frames);
		}*/
#endif

		if (address == 0x4020) {
			/*
			 * 7  bit  0
			 * ---------
			 * LLLL LLLL
			 * |||| ||||
			 * ++++-++++- 8 LSB of IRQ timer
			 */
			fds.drive.irq_timer_reload = (fds.drive.irq_timer_reload & 0xFF00) | value;
			fds.drive.irq_timer_high = FALSE;
			nes.c.irq.high &= ~FDS_TIMER_IRQ;
			return (TRUE);
		}
		if (address == 0x4021) {
			/*
			 * 7  bit  0
			 * ---------
			 * LLLL LLLL
			 * |||| ||||
			 * ++++-++++- 8 MSB of IRQ timer
			 */
			fds.drive.irq_timer_reload = (value << 8) | (fds.drive.irq_timer_reload & 0x00FF);
			fds.drive.irq_timer_high = FALSE;
			nes.c.irq.high &= ~FDS_TIMER_IRQ;
			return (TRUE);
		}
		if (address == 0x4022) {
			/*
			 * 7  bit  0
			 * ---------
			 * xxxx xxER
			 *        ||
			 *        |+- IRQ Reload Flag
			 *        +-- IRQ Enabled
			 */
			if (fds.drive.enabled_dsk_reg) {
				fds.drive.irq_timer_reload_enabled = value & 0x01;
				fds.drive.irq_timer_enabled = value & 0x02;
			}

			if (fds.drive.irq_timer_enabled) {
				fds.drive.irq_timer_counter = fds.drive.irq_timer_reload;
			} else {
				fds.drive.irq_timer_high = FALSE;
				nes.c.irq.high &= ~FDS_TIMER_IRQ;
			}

			return (TRUE);
		}
		if (address == 0x4023) {
			/*
			 * 7  bit  0
			 * ---------
			 * xxxx xxSD
			 *        ||
			 *        |+- Enable disk I/O registers
			 *        +-- Enable sound I/O registers
			 */
			fds.drive.enabled_dsk_reg = value & 0x01;
			fds.drive.enabled_snd_reg = value & 0x02;

			if (!fds.drive.enabled_dsk_reg) {
				fds.drive.irq_timer_high = FALSE;
				nes.c.irq.high &= ~FDS_TIMER_IRQ;
			}

			return (TRUE);
		}
		if (address == 0x4024) {
			fds.drive.data_to_write = value;
			fds.drive.irq_disk_high = FALSE;
			nes.c.irq.high &= ~FDS_DISK_IRQ;
			return (TRUE);
		}
		if (address == 0x4025) {
			/*
			 * 7  bit  0
			 * ---------
			 * IS1B MRTD
			 * |||| ||||
			 * |||| |||+- Drive Motor Control
			 * |||| |||     0: Stop motor
			 * |||| |||     1: Turn on motor
			 * |||| ||+-- Transfer Reset
			 * |||| ||      Set 1 to reset transfer timing to the initial state.
			 * |||| |+--- Read / Write mode
			 * |||| |     (0: write; 1: read)
			 * |||| +---- Mirroring (0: horizontal; 1: vertical)
			 * |||+------ CRC control (set during CRC calculation of transfer)
			 * ||+------- Always set to '1'
			 * |+-------- Read/Write Start
			 * |            Turn on motor.  Set to 1 when the drive becomes ready for read/write
			 * +--------- Interrupt Transfer
			 *              0: Transfer without using IRQ
			 *              1: Enable IRQ when the drive becomes ready for
			 */

			if (!fds.drive.enabled_dsk_reg) {
				return (TRUE);
			}

			fds.drive.motor_on = value & 0x01;
			fds.drive.transfer_reset = value & 0x02;

			/* TODO : penso che sia corretto */
			if (fds.drive.read_mode != (value & 0x04)) {
				fds.drive.gap_ended = FALSE;
			}
			fds.drive.read_mode = value & 0x04;
			fds.drive.mirroring = value & 0x08;
			if (fds.drive.mirroring) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			fds.drive.crc_control = value & 0x10;
			fds.drive.unknow = value & 0x20;
			fds.drive.drive_ready = value & 0x40;
			fds.drive.irq_disk_enabled = value & 0x80;

			fds.drive.irq_disk_high = FALSE;
			nes.c.irq.high &= ~FDS_DISK_IRQ;
			return (TRUE);
		}
		if (address == 0x4026) {
			if (!fds.drive.enabled_dsk_reg) {
				return (TRUE);
			}
			fds.drive.data_external_connector = value;
			return (TRUE);
		}
	}
	if ((address >= 0x4040) && (address <= 0x408A)) {
		/* eseguo un tick hardware */
		tick_hw(1);

		if (fds.drive.enabled_snd_reg) {
			if ((address >= 0x4040) && (address <= 0x407F)) {
				if (fds.snd.wave.writable) {
					fds.snd.wave.data[address & 0x003F] = value & 0x3F;
				}
				return (TRUE);
			}
			if (address == 0x4080) {
				fds.snd.volume.speed = value & 0x3F;
				fds.snd.volume.increase = value & 0x40;
				fds.snd.volume.mode = value & 0x80;
				fds.snd.volume.counter = fds_reset_envelope_counter(volume);
				if (fds.snd.volume.mode) {
					fds.snd.volume.gain = fds.snd.volume.speed;
				}
				return (TRUE);
			}
			if (address == 0x4082) {
				fds.snd.main.frequency = (fds.snd.main.frequency & 0xFF00) | value;
				return (TRUE);
			}
			if (address == 0x4083) {
				fds.snd.main.frequency = ((value & 0x0F) << 8) | (fds.snd.main.frequency & 0x00FF);
				fds.snd.envelope.disabled = value & 0x40;
				fds.snd.main.silence = value & 0x80;
				if (fds.snd.envelope.disabled) {
					fds.snd.volume.counter = fds_reset_envelope_counter(volume);
					fds.snd.sweep.counter = fds_reset_envelope_counter(sweep);
				}
				return (TRUE);
			}
			if (address == 0x4084) {
				fds.snd.sweep.speed = value & 0x3F;
				fds.snd.sweep.increase = value & 0x40;
				fds.snd.sweep.mode = value & 0x80;
				fds.snd.sweep.counter = fds_reset_envelope_counter(sweep);
				if (fds.snd.sweep.mode) {
					fds.snd.sweep.gain = fds.snd.sweep.speed;
				}
				return (TRUE);
			}
			if (address == 0x4085) {
				fds.snd.sweep.bias = fds_sweep_bias(value)
				return (TRUE);
			}
			if (address == 0x4086) {
				fds.snd.modulation.frequency = (fds.snd.modulation.frequency & 0xFF00) | value;
				return (TRUE);
			}
			if (address == 0x4087) {
				fds.snd.modulation.frequency = ((value & 0x0F) << 8) | (fds.snd.modulation.frequency & 0x00FF);
				fds.snd.modulation.disabled = value & 0x80;
				if (fds.snd.modulation.disabled) {
					fds.snd.modulation.counter = 0xFFFF;
				}
				return (TRUE);
			}
			if (address == 0x4088) {
				if (fds.snd.modulation.disabled) {
					fds.snd.modulation.data[fds.snd.modulation.index] = (SBYTE)(value & 0x07);
					fds.snd.modulation.data[(fds.snd.modulation.index + 1) & 0x3F] = (SBYTE)(value & 0x07);
					fds.snd.modulation.index = (fds.snd.modulation.index + 2) & 0x3F;
				}
				return (TRUE);
			}
			if (address == 0x4089) {
				fds.snd.wave.writable = value & 0x80;
				fds.snd.wave.volume = value & 0x03;
				return (TRUE);
			}
			if (address == 0x408A) {
				fds.snd.envelope.speed = value;
				return (TRUE);
			}
		}
	}
	if (address > 0x4017) {
		/* eseguo un tick hardware */
		tick_hw(1);
		return (TRUE);
	}

	return (FALSE);
}

/* ------------------------------------ MISC ROUTINE ------------------------------------------- */

INLINE static WORD lend_word(WORD address, BYTE indirect, BYTE make_last_tick_hw) {
	WORD newAdr = cpu_rd_mem(address++, TRUE);

	/* 6502 Bugs :
	 * Indirect addressing modes are not able to fetch an address which
	 * crosses the page boundary
	 * LDA ($FF),Y
	 * LDX #$00
	 * LDA ($FF,X)
	 * LDX #$FF
	 * LDA ($00,X)
	 * will all fetch the low-byte from $00FF and the high-byte from $0000
	 * JMP ($12FF)
	 * will fetch the low-byte from $12FF and the high-byte from $1200
	 */
	if (indirect && !(address & 0x00FF)) {
		address -= 0x100;
	}
	newAdr |= (cpu_rd_mem(address, make_last_tick_hw) << 8);
	return (newAdr);
}
INLINE static void tick_hw(BYTE value) {
	if (info.disable_tick_hw) {
		return;
	}

	tick_hw_start:
	if (nsf.enabled) {
		if (nsf.made_tick) {
			nes.c.cpu.opcode_cycle++;
			nes.c.nmi.before = nes.c.nmi.high;
			nes.c.irq.before = nes.c.irq.high;
			nsf_tick();

			apu_tick(&value);
			nes.c.cpu.odd_cycle = !nes.c.cpu.odd_cycle;
			value--;
			mod_cycles_op(-=, 1);
		}
		return;
	}
	nes.c.cpu.opcode_cycle++;
	nes.c.nmi.before = nes.c.nmi.high;
	nes.c.irq.before = nes.c.irq.high;
	ppu_tick();

	if (!nes.p.overclock.in_extra_sclines) {
		apu_tick(&value);
	}

	if (tape_data_recorder.enabled) {
		tape_data_recorder_tick();
	}

	// microfono
	if (mic.enable) {
		if (mic.mode == MIC_STOP) {
			mic.enable = FALSE;
			mic.mode = MIC_NONE;
			mic.cycles = 0;
			mic.data = 0x00;
		} if (mic.mode == MIC_RESET) {
			mic.mode = MIC_NONE;
			if (!mic.cycles) {
				mic.cycles = 0x3FFFF;
				mic.data = 0x00;
			}
		}
		if (mic.cycles) {
			mic.cycles--;
			if (!mic.cycles) {
				mic.mode = MIC_STOP;
			} else if (!(mic.cycles & 0x000F)) {
				mic.data ^= 0x04;
			}
		}
	}

	if (extcl_cpu_every_cycle) {
		extcl_cpu_every_cycle();
	}
	nes.c.cpu.odd_cycle = !nes.c.cpu.odd_cycle;
	value--;
	mod_cycles_op(-=, 1);

	nes.p.r2000.race.ctrl = FALSE;
	nes.p.r2001.race.ctrl = FALSE;
	nes.p.r2006.race.ctrl = FALSE;

	if (irqA12.present) {
		irqA12.cycles++;
		irqA12.race.C001 = FALSE;
	}

	if (vs_system.enabled) {
		if (vs_system.coins.left) {
			vs_system.coins.left--;
		}
		if (vs_system.coins.right) {
			vs_system.coins.right--;
		}
		if (vs_system.coins.service) {
			vs_system.coins.service--;
		}
		if (++vs_system.watchdog.timer == vs_system.watchdog.next) {
			vs_system.watchdog.reset = TRUE;
		}
		vs_system_r4020_timer(rd)
		vs_system_r4020_timer(wr)
	}

	if (value > 0) {
		goto tick_hw_start;
	}
}

/* --------------------------------------------------------------------------------------------- */

#endif /* CPU_INLINE_H_ */

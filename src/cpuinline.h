/*
 * cpuinline.h
 *
 *  Created on: 23/apr/2010
 *      Author: fhorse
 */

#ifndef CPUINLINE_H_
#define CPUINLINE_H_

#include <stdlib.h>
#include <stdio.h>
#include "clock.h"
#include "cpu6502.h"
#include "input.h"
#include "mappers.h"
#include "ppuinline.h"
#include "apu.h"
#include "irqA12.h"
#include "irql2f.h"
#include "tas.h"
#include "fds.h"
#include "gamegenie.h"

#define modCyclesOP(op, value) cpu.cycles op value
#define r2006duringRendering()\
{\
	if (!r2002.vblank && r2001.visible &&\
		(ppu.frameY > machine.vintLines) &&\
		(ppu.screenY < SCRLINES)) {\
		if (r2000.r2006Inc == 32) {\
			r2006inc();\
		} else {\
			r2006.value++;\
		}\
	} else {\
		r2006.value += r2000.r2006Inc;\
	}\
}
#define ppuObWr(bit) ppuOpenbus.bit = ppu.frames
#define ppuObWrAll()\
	ppuObWr(bit0);\
	ppuObWr(bit1);\
	ppuObWr(bit2);\
	ppuObWr(bit3);\
	ppuObWr(bit4);\
	ppuObWr(bit5);\
	ppuObWr(bit6);\
	ppuObWr(bit7)
#define ppuObRd(bit, mask)\
	if ((ppu.frames - ppuOpenbus.bit) > machine.ppuOpenbusFrames) {\
		ppu.openbus &= mask;\
	}
#define ppuObRdAll()\
	ppuObRd(bit0, 0x01);\
	ppuObRd(bit1, 0x02);\
	ppuObRd(bit2, 0x04);\
	ppuObRd(bit3, 0x08);\
	ppuObRd(bit4, 0x10);\
	ppuObRd(bit5, 0x20);\
	ppuObRd(bit6, 0x40);\
	ppuObRd(bit7, 0x80)

static BYTE cpuRdMem(WORD address, BYTE madeTick);
static BYTE INLINE ppuRdReg(WORD address);
static BYTE INLINE apuRdReg(WORD address);
static BYTE INLINE fdsRdMem(WORD address, BYTE madeTick);

static void cpuWrMem(WORD address, BYTE value);
static void INLINE ppuWrMem(WORD address, BYTE value);
static void INLINE ppuWrReg(WORD address, BYTE value);
static void INLINE apuWrReg(WORD address, BYTE value);
static BYTE INLINE fdsWrMem(WORD address, BYTE value);

static WORD INLINE lendWord(WORD address, BYTE indirect, BYTE makeLastTickHW);
static void INLINE tickHW(BYTE value);

/* ------------------------------------ READ ROUTINE ------------------------------------------- */

static BYTE cpuRdMem(WORD address, BYTE madeTick) {
	if (fds.info.enabled) {
		if (fdsRdMem(address, madeTick)) {
			return (cpu.openbus);
		}
	} else if (address >= 0x8000) {
		/* PRG Rom */
		BYTE before = cpu.openbus;

		/* eseguo un tick hardware */
		if (madeTick) {
			tickHW(1);
		}
		/* leggo */
		cpu.openbus = prgRomRd(address);

		if (info.mapperExtendRead) {
			cpu.openbus = extclCpuRdMem(address, cpu.openbus, before);
		}

		if (gamegenie.counter) {
			BYTE i;

			for (i = 0; i < LENGTH(gamegenie.cheat); i++) {
				if (!gamegenie.cheat[i].disabled &&  (gamegenie.cheat[i].address == address)) {
					if (gamegenie.cheat[i].enabled_compare) {
						if (gamegenie.cheat[i].compare == cpu.openbus) {
							cpu.openbus = gamegenie.cheat[i].replace;
						}
					} else {
						cpu.openbus = gamegenie.cheat[i].replace;
					}
				}
			}
		}

		return (cpu.openbus);
	}
	if (address <= 0x4017) {
		/* Ram */
		if (address < 0x2000) {
			/* eseguo un tick hardware */
			if (madeTick) {
				tickHW(1);
			}
			/* leggo */
			cpu.openbus = mmcpu.ram[address & 0x7FF];
			return (cpu.openbus);
		}
		/* PPU */
		if (address < 0x4000) {
			/* eseguo un tick hardware */
			tickHW(1);
			/* leggo */
			cpu.openbus = ppuRdReg(address & 0x2007);
			return (cpu.openbus);
		}
		/* APU */
		if (address <= 0x4015) {
			/* leggo */
			cpu.openbus = apuRdReg(address);
			/* eseguo un tick hardware ed e' importante che sia fatto dopo */
			tickHW(1);
			return (cpu.openbus);
		}
		/* Controller port 1 */
		if (address == 0x4016) {

			tas.lag_frame = FALSE;
			/* eseguo un tick hardware */
			tickHW(1);
			/* leggo dal controller */
			cpu.openbus = inputReadReg1(cpu.openbus, screen.line, &port1);
			return (cpu.openbus);
		}
		/* Controller port 2 */
		if (address == 0x4017) {
			tas.lag_frame = FALSE;
			/* eseguo un tick hardware */
			tickHW(1);
			/* leggo dal controller */
			cpu.openbus = inputReadReg2(cpu.openbus, screen.line, &port2);
			return (cpu.openbus);
		}
	}
	/* Prg Ram (normale ed extra (battery packed o meno) */
	if (address < 0x8000) {
		BYTE before = cpu.openbus;

		/* eseguo un tick hardware */
		tickHW(1);
		/* controllo se e' consentita la lettura dalla PRG Ram */
		if (cpu.prgRamRdActive) {
			if (address < 0x6000) {
				/* leggo */
				cpu.openbus = prg.ram[address & 0x1FFF];
			} else {
				/*
				 * se la rom ha una PRG Ram extra allora
				 * la utilizzo, altrimenti leggo dalla PRG
				 * Ram normale.
				 */
				if (!prg.ramPlus) {
					cpu.openbus = prg.ram[address & 0x1FFF];
				} else {
					cpu.openbus = prg.ramPlus8k[address & 0x1FFF];
				}
			}
		}
		if (extclCpuRdMem) {
			/*
			 * utilizzato dalle mappers :
			 * MMC5
			 * Namcot (163)
			 * Rex (DBZ)
			 * Sunsoft (FM7)
			 */
			/* Mappers */
			cpu.openbus = extclCpuRdMem(address, cpu.openbus, before);
		}
		return (cpu.openbus);
	}

	/* eseguo un tick hardware */
	tickHW(1);
	/* qualsiasi altra cosa */
	return (cpu.openbus);
}
static BYTE INLINE ppuRdReg(WORD address) {
	BYTE value = 0;

	ppuObRdAll();

	if (address == 0x2002) {
		/* Situazioni particolari */
		if (!info.r2002_race_condition_disabled && !(ppu.frameY | nmi.before)) {
			/* situazione di contesa (race condition)
			 *
			 * se la lettura avviene esattamente all'inizio
			 * del vblank allora il bit 7 verra' restituito
			 * a 0.
			 */
			if (!ppu.frameX) {
				/*
				 * Nota: quando e' abilitato questo controllo
				 * la demo cuter.nes (Who's Cuter?) ha un problema
				 * di rallentamento e sfarfallio. Il problema e'
				 * della demo e non dell'emulatore (confermato
				 * dall'autore stesso della demo).
				 */
				r2002.vblank = FALSE;
			}
			/*
			 * leggendo questo registro nei primi tre
			 * cicli PPU del vblank, se presente, viene
			 * disabilitato l'NMI pendente.
			 */
			nmi.high = nmi.delay = FALSE;
		}
		value = r2002.vblank | r2002.sprite0Hit | r2002.spriteOverflow;
		/* azzero il VBlank */
		r2002.vblank = FALSE;
		/*
		 * azzero il bit toggle (1°/2° write flipflop
		 * usato da $2005 e $2006)
		 */
		r2002.toggle = 0;
		/* open bus */
		value = ppu.openbus = (value & 0xE0) | (ppu.openbus & 0x1F);
		ppuObWr(bit5);
		ppuObWr(bit6);
		ppuObWr(bit7);
		return (value);
	}
	if (address == 0x2004) {
		if ((!r2002.vblank && r2001.visible && (ppu.screenY < SCRLINES))) {
			value = r2004.value;
		} else {
			value = oam.data[r2003.value];
		}
		/* ppu open bus */
		ppu.openbus = value;
		ppuObWrAll();
		return (value);
	}
	if (address == 0x2007) {
		WORD r2006Old = r2006.value;
		BYTE repeat = 1;

		if (DMC.dmaCycle == 2) {
			repeat = 3;
		} else if (cpu.dblRd) {
			WORD random = (WORD) rand() % 10;
			value = ppuRdMem(r2006.value - (r2000.r2006Inc << 1));
			if (random > 5) {
				r2007.value = ppuRdMem(r2006.value);
				r2006duringRendering();
			}
			/* ppu open bus */
			ppu.openbus = value;
			ppuObWrAll();
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
			if ((r2006.value & 0x3FFF) < 0x3F00) {
				value = r2007.value;
				r2007.value = ppuRdMem(r2006.value);
				/* ppu open bus */
				ppu.openbus = value;
				ppuObWrAll();
			} else {
				value = ppuRdMem(r2006.value);
				r2007.value = ppuRdMem(r2006.value & 0x2FFF);
				/* ppu open bus */
				value = ppu.openbus = (value & 0x3F) | (ppu.openbus & 0xC0);
				ppuObWr(bit0);
				ppuObWr(bit1);
				ppuObWr(bit2);
				ppuObWr(bit3);
				ppuObWr(bit4);
				ppuObWr(bit5);
			}
			r2006duringRendering();
			repeat--;
			if (extcl2006Update) {
				/*
				 * utilizzato dalle mappers :
				 * MMC3
				 * Rex (DBZ)
				 * Taito (TC0190FMCPAL16R4)
				 * Tengen (Rambo)
				 */
				extcl2006Update(r2006Old);
			}
		}
		return (value);
	}

#ifdef DEBUG
	fprintf(stderr, "Alert: Attempt to read PPU port %04X\n", address);
#endif

	/* ppu open bus */
	return (ppu.openbus);
}
static BYTE INLINE apuRdReg(WORD address) {
	BYTE value = cpu.openbus;

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
		if ((irq.high & APUIRQ) && irq.before) {
			irq.high &= ~APUIRQ;
		}
#ifdef DEBUG
	} else {
		fprintf(stderr, "Alert: Attempt to read APU port %04X\n", address);
#endif
	}

	return (value);
}
static BYTE INLINE fdsRdMem(WORD address, BYTE madeTick) {
	if (address >= 0xE000) {
		/* eseguo un tick hardware */
		if (madeTick) {
			tickHW(1);
		}
		/* leggo */
		cpu.openbus = prg.rom[address & 0x1FFF];
		return (TRUE);
	}
	if (address >= 0x6000) {
		/* eseguo un tick hardware */
		if (madeTick) {
			tickHW(1);
		}
		/* leggo */
		cpu.openbus = prg.ram[address - 0x6000];
		return (TRUE);
	}
	if (fds.drive.enabled_dsk_reg && ((address >= 0x4030) && (address <= 0x4033))) {
		/* eseguo un tick hardware */
		tickHW(1);

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
			cpu.openbus = 0;
			/* bit 0  (timer irq) */
			cpu.openbus |= fds.drive.irq_timer_high;
			/* bit 1 (trasfer flag) */
			cpu.openbus |= fds.drive.transfer_flag;
			/* bit 2 e 3 non settati (??) */
			/* TODO : bit 4 (CRC control : 0 passato, 1 errore) */
			/* TODO : bit 5 (end of head) */
			cpu.openbus |= fds.drive.end_of_head;
			fds.drive.end_of_head = FALSE;

			/* TODO : bit 6  (disk data read/write enable (1 when disk is readable/writable) */
			/* azzero il transfer flag */
			fds.drive.transfer_flag = FALSE;
			/* devo disabilitare sia il timer IRQ ... */
			fds.drive.irq_timer_high = FALSE;
			irq.high &= ~FDSTIMERIRQ;
			/* che il disk IRQ */
			fds.drive.irq_disk_high = FALSE;
			irq.high &= ~FDSDISKIRQ;

#ifndef RELEASE
			//fprintf(stderr, "0x%04X 0x%02X %d\n", address, cpu.openbus, irq.high);
#endif

			return (TRUE);
		}
		if (address == 0x4031) {
			cpu.openbus = fds.drive.data_readed;
			/* azzero il transfer flag */
			fds.drive.transfer_flag = FALSE;

#ifndef RELEASE
			/*fprintf(stderr, "0x%04X 0x%02X [0x%04X] 0x%04X %d %d %d\n", address, cpu.openbus,
			        fds.side.data[fds.drive.disk_position], cpu.codeopPC, fds.drive.disk_position,
			        fds.info.sides_size[fds.drive.side_inserted], irq.high);*/
#endif

			/* devo disabilitare il disk IRQ */
			fds.drive.irq_disk_high = FALSE;
			irq.high &= ~FDSDISKIRQ;

			return (TRUE);
		}
		if (address == 0x4032) {
			/*
			 * 7  bit  0
			 * ---------
			 * xxxx xPRS
			 *       |||
			 *       ||+- Disk flag  (0: Disk inserted; 1: Disk not inserted)
			 *       |+-- Ready flag (0: Disk read; 1: Disk not ready)
			 *       +--- Protect flag (0: Not write protected; 1: Write protected or disk ejected)
			 */
			cpu.openbus &= ~0x07;

			if (fds.drive.disk_ejected) {
				cpu.openbus |= 0x07;
			} else if (!fds.drive.scan) {
				cpu.openbus |= 0x02;
			}

#ifndef RELEASE
			//fprintf(stderr, "0x%04X 0x%02X\n", address, cpu.openbus);
#endif

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
			cpu.openbus = fds.drive.data_external_connector & 0x80;

			return (TRUE);
		}
	}
	if (fds.drive.enabled_snd_reg) {
		if ((address >= 0x4040) && (address <= 0x407F)) {
			/* eseguo un tick hardware */
			tickHW(1);

			/*
			 * 7  bit  0  (read/write)
			 * ---- ----
			 * OOSS SSSS
			 * |||| ||||
			 * ||++-++++- Sample
			 * ++-------- Returns 01 on read, likely from open bus
			 */
			cpu.openbus = fds.snd.wave.data[address & 0x3F] | (cpu.openbus & 0xC0);

			return (TRUE);
		}
		if (address == 0x4090) {
			/* eseguo un tick hardware */
			tickHW(1);

			cpu.openbus = (fds.snd.volume.gain & 0x3F) | (cpu.openbus & 0xC0);

			return (TRUE);
		}
		if (address == 0x4092) {
			/* eseguo un tick hardware */
			tickHW(1);

			cpu.openbus = (fds.snd.sweep.gain & 0x3F) | (cpu.openbus & 0xC0);

			return (TRUE);
		}
	}
	if (address > 0x4017) {
		/* eseguo un tick hardware */
		tickHW(1);
		return (TRUE);
	}

	return (FALSE);
}
/* ------------------------------------ WRITE ROUTINE ------------------------------------------ */

static void cpuWrMem(WORD address, BYTE value) {
	if (fds.info.enabled) {
		if (fdsWrMem(address, value)) {
			return;
		}
	}

	if (address <= 0x4017) {
		/* Ram */
		if (address < 0x2000) {
			/* eseguo un tick hardware */
			tickHW(1);
			/* scrivo */
			mmcpu.ram[(address & 0x7FF)] = value;
			return;
		}
		if (address < 0x4000) {
			address &= 0x2007;
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
				ppuWrReg(address, value);
				/* eseguo un tick hardware */
				tickHW(1);
				return;
			}
			/* eseguo un tick hardware */
			tickHW(1);
			/* scrivo */
			ppuWrReg(address, value);
			return;
		}
		/* Sprite memory */
		if (address == 0x4014) {
			DMC.tickType = DMCR4014;
			/* eseguo un tick hardware */
			tickHW(1);
			/* scrivo */
			ppuWrReg(address, value);
			return;
		}
		/* Controller */
		if (address == 0x4016) {
			/* eseguo un tick hardware */
			tickHW(1);
			if (extclCPUWr4016) {
				/*
				 * utilizzato dalle mappers :
				 * Vs
				 */
				extclCPUWr4016(value);
			}
			/* in caso di strobe azzero l'indice */
			if (r4016.value && !(value & 0x01)) {
				port1.index = port2.index = 0;
			}
			/* memorizzo il valore */
			r4016.value = value & 0x01;
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
			apuWrReg(address, value);
			/* eseguo un tick hardware */
			tickHW(1);
			return;
		}
		if (address <= 0x4017) {
			/* eseguo un tick hardware */
			tickHW(1);
			/* scrivo */
			apuWrReg(address, value);
			return;
		}
	}
	/* PRG Ram (normale ed extra) */
	if (address < 0x8000) {
		/* eseguo un tick hardware */
		tickHW(1);
		/* controllo se e' attiva la PRG Ram */
		if (cpu.prgRamWrActive) {
			if (address < 0x6000) {
				/* scrivo */
				prg.ram[address & 0x1FFF] = value;
			} else {
				/*
				 * se la rom ha una PRG Ram extra allora
				 * la utilizzo, altrimenti uso la PRG Ram
				 * normale.
				 */
				if (!prg.ramPlus) {
					prg.ram[address & 0x1FFF] = value;
				} else {
					prg.ramPlus8k[address & 0x1FFF] = value;
				}
			}
		}
		if (info.mapperExtendWrite) {
			/*
			 * utilizzato dalle mappers :
			 * Active
			 * 74x138x161
			 * Ave (NINA06)
			 * BxROM (AVENINA001)
			 * Caltron
			 * Jaleco (JF05 e JF11)
			 * MMC5
			 * REX (DBZ)
			 */
			extclCpuWrMem(address, value);
		}
		return;
	}

	/* Mappers */
	extclCpuWrMem(address, value);
	/* su questo devo fare qualche altro esperimento */
	tickHW(1);
	return;
}
static void INLINE ppuWrMem(WORD address, BYTE value) {
	address &= 0x3FFF;
	if (address < 0x2000) {
		if (extclWrChr) {
			/*
			 * utilizzato dalle mappers :
			 * Irem (LROG017)
			 */
			extclWrChr(address, value);
			return;
		}
		if (mapper.writeVRAM) {
			chr.bank1k[address >> 10][address & 0x3FF] = value;
		}
		return;
	}
	if (address < 0x3F00) {
		address &= 0x0FFF;
		ntbl.bank1k[address >> 10][address & 0x3FF] = value;
		return;
	}
	address &= 0x1F;
	if (!(address & 0x03)) {
		palette.color[address] = palette.color[(address + 0x10) & 0x1F] = value & 0x3F;
	} else {
		palette.color[address] = value & 0x3F;
	}
	return;
}
static void INLINE ppuWrReg(WORD address, BYTE value) {
	if (address == 0x2000) {
#ifndef RELEASE
		BYTE old_delay = FALSE;
#endif

		/*
		 * se l'nmi e' attivo quando scrivo nel registro r2000
		 * deve essere eseguito nell'istruzione successiva.
		 * L'ho notato con "baxter,zugzwang-adventuresoflolo.fm2"
		 * al tas.frame 53762. Se non c'è questo vengono generati
		 * 2 nmi nello stesso frame, perche' il primo frame e' generato
		 * proprio nella scrittura di questo registro e la mancata
		 * esecuzione dell'istruzione successiva pone le condizioni
		 * per il secondo nmi che avverra' 25 scanline dopo.
		 */
		if (nmi.high && !nmi.delay) {
#ifndef RELEASE
			old_delay = TRUE;
#endif
			nmi.delay = TRUE;
		}

		/* open bus */
		ppu.openbus = value;
		ppuObWrAll();

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
		if (!r2000.NMIenable && (value & 0x80)) {
			if (r2002.vblank) {
				nmi.high = nmi.delay = TRUE;
				nmi.frameX = ppu.frameX;
			}
			/*
			 * se viene disabilitato l'NMI (bit 7 da 1 a 0)
			 * all'inizio del vblank, l'NMI generato deve
			 * essere disabilitato.
			 */
		} else if (r2000.NMIenable && !(value & 0x80)) {
			if (!(ppu.frameY | nmi.before)) {
				nmi.high = nmi.delay = FALSE;
			}
		}
		/* valorizzo $2000 */
		r2000.value = value;
		/* NMI abilitato */
		r2000.NMIenable = value & 0x80;
		/* VRAM address increment */
		(value & 0x04) ? (r2000.r2006Inc = 32) : (r2000.r2006Inc = 1);
		/* memorizzo la dimensione degli sprites */
		(value & 0x20) ? (r2000.sizeSPR = 16) : (r2000.sizeSPR = 8);
		/* Sprite pattern table address */
		r2000.SPTadr = (value & 0x08) << 9;
		/* Background pattern table address */
		r2000.BPTadr = (value & 0x10) << 8;
		/*
		 * NN -> Name-table Bits. Vertical bit e Horizontal bit
		 *
		 * $2000      W %---- --NN
		 * bitsNT       %0000 00NN
		 * tmpAdrVRAM   %---- NN-- ---- ----
		 */
		ppu.tmpVRAM = (ppu.tmpVRAM & 0xF3FF) | ((value & 0x03) << 10);

		/*
		 * per questo registro il tickHw e' gia' stato effettuato, quindi
		 * la PPU ha gia' fatto 3 cicli. Se sono nel range 253 - 255 del
		 * ppu.frameX, il registro $2006 e' gia' stato aggiornato dalla PPU
		 * (cosa che avviene nel ciclo 253). Questa variazione e' direttamente
		 * controllata anche dal valore di ppu.tmpVRAM, quindi una scrittura
		 * in questo registro nella PPU che ha gia' fatto il ciclo 253, non
		 * influenzerebbe il $2006 e questo e' sbagliato. Quindi lo ricalcolo io,
		 * ma solo so sono in fase di rendering.
		 * Rom interessate:
		 * Road Runner (Tengen) [!].nes
		 * (premuto il tasto start due volte ed avviato il gioco, una riga
		 * piu' scura sfarfalla nello schermo).
		 */
		if (!r2002.vblank && r2001.visible && (ppu.screenY < SCRLINES)) {
			if ((ppu.frameX >= 253) && (ppu.frameX <= 255)) {
				r2006EndScanline();
			}
		}

#ifndef RELEASE
		if (old_delay && nmi.high) {
			fprintf(stderr, "r2000 nmi high, set delay\n");
		}
#endif
		return;
	}
	if (address == 0x2001) {
		/* open bus */
		ppu.openbus = value;
		ppuObWrAll();
		/* valorizzo $2001 */
		r2001.value = value;
		/*
		 * con il bit 0 settato viene indicata
		 * la modalita' scale di grigio per l'output.
		 */
		(value & 0x01) ? (r2001.colorMode = GRAYSCALE) : (r2001.colorMode = NORMAL);
		/* visibilita' del background */
		r2001.bckVisible = value & 0x08;
		/* visibilita' degli sprites */
		r2001.sprVisible = value & 0x10;
		/* basta che uno dei due sia visibile */
		r2001.visible = r2001.bckVisible | r2001.sprVisible;
		/* questo per ora mi serve solo per l'A12 */
		/* MMC3 and Taito*/
		if (r2001.visible) {
			if (irqA12.present) {
				irqA12.sAdrOld = irqA12.bAdrOld = 0;
			}
		} else {
			if (irql2f.present) {
				irql2f.inFrame = FALSE;
			}
		}
		/* clipping del background */
		r2001.bckClipping = value & 0x02;
		/* clipping degli sprites */
		r2001.sprClipping = value & 0x04;
		/* salvo la maschera di enfatizzazione del colore */
		r2001.emphasis = (value << 1) & 0x1C0;
		return;
	}
	if (address == 0x2003) {
		/* open bus */
		ppu.openbus = value;
		ppuObWrAll();
		r2003.value = value;
		return;
	}
	if (address == 0x2004) {
		/* open bus */
		ppu.openbus = value;
		ppuObWrAll();
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
		if ((r2003.value & 0x03) == 0x02) {
			value &= 0xE3;
		}
		oam.data[r2003.value++] = value;
		return;
	}
	if (address == 0x2005) {
		/* open bus */
		ppu.openbus = value;
		ppuObWrAll();
		/*
		 * Bit totali manipolati con $2005:
		 * tmpAdrVRAM  %0yyy --YY YYYX XXXX
		 */
		if (!r2002.toggle) {
			/*
			 * XXXXX -> Tile X
			 * xxx   -> Fine X
			 *
			 * $2005       W %XXXX Xxxx (toggle e' 0)
			 * fineX         %0000 0xxx
			 * tmpAdrVRAM    %0--- ---- ---X XXXX
			 * toggle = 1
			 */
			ppu.fineX = (value & 0x07);
			//ppu.tmpVRAM = (ppu.tmpVRAM & 0x7FE0) | ((value & 0xF8) >> 3);
			ppu.tmpVRAM = (ppu.tmpVRAM & 0x7FE0) | (value >> 3);
		} else {
			/*
			 * YYYYY -> Tile Y
			 * yyy   -> Fine Y
			 *
			 * $2005       W %YYYY Yyyy (toggle e' 1)
			 * tmpAdrVRAM    %0yyy --YY YYY- ----
			 * toggle = 0
			 */
			ppu.tmpVRAM = (ppu.tmpVRAM & 0x0C1F) | ((value & 0xF8) << 2)
			        		| ((value & 0x07) << 12);
		}
		r2002.toggle = !r2002.toggle;
		return;
	}
	if (address == 0x2006) {
		WORD r2006Old = r2006.value;

		/* open bus */
		ppu.openbus = value;
		ppuObWrAll();

		/*
		 * Bit totali manipolati con $2006:
		 * tmpAdrVRAM  %00yy NNYY YYYX XXXX
		 */
		if (!r2002.toggle) {
			/*
			 * YYYYY -> Tile Y
			 * yyy   -> Fine Y
			 * NN    -> Name-table Bits. Vertical bit e Horizontal bit
			 *
			 * $2006       W %--yy NNYY (toggle e' 0)
			 * tmpAdrVRAM    %00yy NNYY ---- ----
			 * toggle = 1
			 */
			ppu.tmpVRAM = (ppu.tmpVRAM & 0x00FF) | ((value & 0x3F) << 8);
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
			ppu.tmpVRAM = (ppu.tmpVRAM & 0x7F00) | value;
			/* aggiorno l'r2006 */
			r2006.value = ppu.tmpVRAM;
			/*
			 * se l'$2006 viene aggiornato proprio al
			 * ciclo 253 della PPU, l'incremento che viene
			 * fatto della PPU proprio al ciclo 253 viene
			 * ignorato.
			 * Rom interessata :
			 * Cosmic Wars (J) [!].nes
			 * (avviare la rom e non premere niente. Dopo la scritta
			 * 260 iniziale e le esplosioni che seguono, si apre una
			 * schermata con la parte centrale che saltella senza
			 * questo controllo)
			 */
			r2006.changedFromOP = ppu.frameX;

			if (extcl2006Update) {
				/*
				 * utilizzato dalle mappers :
				 * MMC3
				 * Rex (DBZ)
				 * Taito (TC0190FMCPAL16R4)
				 * Tengen (Rambo)
				 */
				extcl2006Update(r2006Old);
			}
		}
		r2002.toggle = !r2002.toggle;
		return;
	}

	if (address == 0x2007) {
		WORD r2006Old = r2006.value;

		/* open bus */
		ppu.openbus = value;
		ppuObWrAll();

		ppuWrMem(r2006.value, value);
		/*
		 * e' stato scoperto che anche se normalmente
		 * durante il rendering scrittura e lettura del
		 * $2007 non viene mai fatto per non modificare
		 * il $2006, un gioco lo fa (Young Indiana Jones
		 * Chronicles, The (U) [!].nes) e quando avviene,
		 * l'incremento del $2006 avviene proprio come
		 * nel ciclo 253 della ppu se r2000.r2006Inc e'
		 * uguale a 32.
		 */
		r2006duringRendering();

		 if (extcl2006Update) {
			 /*
			  * utilizzato dalle mappers :
			  * MMC3
			  * Rex (DBZ)
			  * Taito (TC0190FMCPAL16R4)
			  * Tengen (Rambo)
			  */
			 extcl2006Update(r2006Old);
		 }
		 return;
	}
	if (address == 0x4014) {
		/*
		 * se durante l'ultimo ciclo dell'istruzione
		 * e' stata richiesto un IRQ, deve essere ritardato
		 * all'istruzione successiva.
		 */
		if (irq.high && !cpu.cycles && !irq.before) {
			irq.delay = TRUE;
		}
		/* DMA transfer source address */
		address = value << 8;
		{
			WORD index;
			BYTE saveIRQ = irq.high;
			BYTE saveCpuCycles = cpu.cycles;

			if (info.r4014_precise_timing_disabled) {
				modCyclesOP(+=, 512);
			} else {
				/*
			 	 * su un 2A03 reale, questo ciclo viene
			 	 * lasciato per completare la scrittura nel registro.
			 	 */
				tickHW(1);
				/* sono 513 i cicli CPU che il trasferimento si prende */
				modCyclesOP(+=, 513);
				/*
				 * se l'avvio del trasferimento avviene
				 * in un ciclo dispari allora c'e' un ritardo
				 * di un altro ciclo (quindi in totale diventano
				 * 514).
				 */
				if ((machine.type == NTSC) && cpu.oddCycle) {
					tickHW(1);
					modCyclesOP(+=, 1);
				}
			}
			for (index = 0; index < 256; ++index) {
				/*
				 * ogni trasferimento prende due cicli, uno di
				 * lettura (contenuto nel cpuRdMem e l'altro di
				 * scrittura che faccio prima di valorizzare
				 * l'oam.
				 */
				if (index == 253) {
					cpuRdMem(address++, TRUE);
					DMC.tickType = DMCNNLDMA;
				} else if (index == 254) {
					cpuRdMem(address++, TRUE);
					DMC.tickType = DMCR4014;
				} else if (index == 255) {
					DMC.tickType = DMCCPUWRITE;
					cpuRdMem(address++, TRUE);
				} else {
					cpuRdMem(address++, TRUE);
				}
				tickHW(1);
				if ((r2003.value & 0x03) == 0x02) {
					cpu.openbus &= 0xE3;
				}
				oam.data[r2003.value++] = cpu.openbus;
			}
			/*
			 * se sopraggiunge un IRQ durante i 513/514 cicli
			 * e non ci sono altri cicli dell'istruzione,
			 * l'IRQ deve essere ritardato di una istruzione.
			 */
			if (irq.high && !(saveIRQ | saveCpuCycles)) {
				irq.delay = TRUE;
			}
		}
		return;
	}

#ifdef DEBUG
	/* non si puo' scrivere nel registro $2002 */
	fprintf(stderr, "Alert: Attempt to write PPU port %04X\n", address);
#endif

	/* open bus */
	ppu.openbus = value;
	ppuObWrAll();
	return;
}
static void INLINE apuWrReg(WORD address, BYTE value) {
	if (!(address & 0x0010)) {
		/* -------------------- square 1 --------------------*/
		if (address <= 0x4003) {
			if (address == 0x4000) {
				squareReg0(S1);
				return;
			}
			if (address == 0x4001) {
				squareReg1(S1);
				return;
			}
			if (address == 0x4002) {
				squareReg2(S1);
				return;
			}
			if (address == 0x4003) {
				squareReg3(S1);
				return;
			}
			return;
		}
		/* -------------------- square 2 --------------------*/
		if (address <= 0x4007) {
			if (address == 0x4004) {
				squareReg0(S2);
				return;
			}
			if (address == 0x4005) {
				squareReg1(S2);
				return;
			}
			if (address == 0x4006) {
				squareReg2(S2);
				return;
			}
			if (address == 0x4007) {
				squareReg3(S2);
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
					TR.length.value = lengthTable[value >> 3];
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
					NS.length.value = lengthTable[value >> 3];
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
				DMC.irqEnabled = value & 0x80;
				/* se l'irq viene disabilitato allora... */
				if (!DMC.irqEnabled) {
					/* ...azzero l'interrupt flag del DMC */
					r4015.value &= 0x7F;
					/* disabilito l'IRQ del DMC */
					irq.high &= ~DMCIRQ;
				}
				DMC.loop = value & 0x40;
				DMC.rateIndex = value & 0x0F;
				return;
			}
			if (address == 0x4011) {
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

				/* applico un low pass filter */
				DMC.counter = DMC.output = 0.6 * value + (1.0 - 0.6) * DMC.output;
				//DMC.counter = DMC.output = value;

				DMC.clocked = TRUE;

				r4011.value = value;
				return;
			}
			if (address == 0x4012) {
				DMC.addressStart = (value << 6) | 0xC000;
				return;
			}
			if (address == 0x4013) {
				/* sample length */
				DMC.length = (value << 4) | 0x01;
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
			irq.high &= ~DMCIRQ;
			/*
			 * quando il flag di abilitazione del length
			 * counter di ogni canale e' a 0, il counter
			 * dello stesso canale e' immediatamente azzerato.
			 */
			if (!(S1.length.enabled = r4015.value & 0x01)) {
				S1.length.value = 0;
			}
			if (!(S2.length.enabled = r4015.value & 0x02)) {
				S2.length.value = 0;
			}
			if (!(TR.length.enabled = r4015.value & 0x04)) {
				TR.length.value = 0;
			}
			if (!(NS.length.enabled = r4015.value & 0x08)) {
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
				DMC.address = DMC.addressStart;
			}
			return;
		}
		if (address == 0x4017) {
			/* APU frame counter */
			r4017.jitter = value;
			/*
			 * nell'2A03 se la scrittura del $4017 avviene
			 * in un ciclo pari, allora l'effettiva modifica
			 * avverra' nel ciclo successivo.
			 */
			if (cpu.oddCycle) {
				r4017.delay = TRUE;
			} else {
				r4017.delay = FALSE;
				r4017jitter();
			}
			return;
		}
	}

#ifdef DEBUG
		//fprintf(stderr, "Alert: Attempt to write APU port %04X\n", address);
#endif

	return;
}
static BYTE INLINE fdsWrMem(WORD address, BYTE value) {
	if (address >= 0xE000) {
		/* eseguo un tick hardware */
		tickHW(1);
		/* non faccio proprio niente */
		return (TRUE);
	}
	if (address >= 0x6000) {
		/* eseguo un tick hardware */
		tickHW(1);
		/* scrivo */
		prg.ram[address - 0x6000] = value;
		return (TRUE);
	}
	if ((address >= 0x4020) && (address <= 0x4026)) {
		/* eseguo un tick hardware */
		tickHW(1);

#ifndef RELEASE
		/*if (address == 0x4025) {
			fprintf(stderr, "0x%04X 0x%02X %d\n", address, value, fds.drive.enabled_dsk_reg);
		} else {
			if (fds.drive.disk_position)
			fprintf(stderr, "0x%04X 0x%02X 0x%04X %d 0x%02X %d\n", address, value, cpu.codeopPC,
					fds.drive.disk_position - 1, fds.side.data[fds.drive.disk_position - 1], ppu.frames);
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
			irq.high &= ~FDSTIMERIRQ;
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
			irq.high &= ~FDSTIMERIRQ;
			return (TRUE);
		}
		if (address == 0x4022) {
			/*
			 * 7  bit  0
			 * ---------
			 * xxxx xxEx
			 *        |
			 *        +-- Enable IRQ timer
			 */
			/* questo l'ho preso dall'FCEUX e da nestopia */
			fds.drive.irq_timer_reload_enabled = value & 0x01;

			fds.drive.irq_timer_enabled = value & 0x02;
			fds.drive.irq_timer_counter = fds.drive.irq_timer_reload;
			fds.drive.irq_timer_high = FALSE;
			irq.high &= ~FDSTIMERIRQ;
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
			return (TRUE);
		}
		if (address == 0x4024) {
			fds.drive.data_to_write = value;

			fds.drive.transfer_flag = FALSE;

			fds.drive.irq_disk_high = FALSE;
			irq.high &= ~FDSDISKIRQ;
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

			if ((fds.drive.mirroring = value & 0x08)) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			fds.drive.crc_control = value & 0x10;
			fds.drive.unknow = value & 0x20;
			fds.drive.drive_ready = value & 0x40;
			fds.drive.irq_disk_enabled = value & 0x80;

			fds.drive.irq_disk_high = FALSE;
			irq.high &= ~FDSDISKIRQ;

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
		tickHW(1);

		if (fds.drive.enabled_snd_reg) {
			if ((address >= 0x4040) && (address <= 0x407F)) {
				fds.snd.wave.data[address & 0x003F] = value & 0x3F;

				return (TRUE);
			}
			if (address == 0x4080) {
				fds.snd.volume.speed = value & 0x3F;
				fds.snd.volume.increase = value & 0x40;
				fds.snd.volume.mode = value & 0x80;

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

				return (TRUE);
			}
			if (address == 0x4084) {
				fds.snd.sweep.speed = value & 0x3F;
				fds.snd.sweep.increase = value & 0x40;
				fds.snd.sweep.mode = value & 0x80;

				return (TRUE);
			}
			if (address == 0x4085) {
				fds.snd.sweep.bias = ((SBYTE) (value << 1)) / 2;
				fds.snd.modulation.index = 0;

				return (TRUE);
			}
			if (address == 0x4086) {
				fds.snd.modulation.frequency = (fds.snd.modulation.frequency & 0xFF00) | value;

				return (TRUE);
			}
			if (address == 0x4087) {
				fds.snd.modulation.frequency = ((value & 0x0F) << 8)
			        		| (fds.snd.modulation.frequency & 0x00FF);
				fds.snd.modulation.disabled = value & 0x80;

				return (TRUE);
			}
			if (address == 0x4088) {
				BYTE i;

				// 0,2,4,6, -8, -6,-4,-2
				for (i = 0; i < 32; i++) {
					BYTE a = i << 1;

					if (i < 31) {
						fds.snd.modulation.data[a] = fds.snd.modulation.data[a + 2];
					} else {
						BYTE tmp = ((value & 0x03) | (0x3F * (value & 0x04)));
						fds.snd.modulation.data[a] = (SBYTE) tmp;
					}
					fds.snd.modulation.data[a + 1] = fds.snd.modulation.data[a];
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
		tickHW(1);
		return (TRUE);
	}

	return (FALSE);
}
/* ------------------------------------ MISC ROUTINE ------------------------------------------- */

static WORD INLINE lendWord(WORD address, BYTE indirect, BYTE makeLastTickHW) {
	WORD newAdr;

	newAdr = cpuRdMem(address++, TRUE);
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
	newAdr |= (cpuRdMem(address, makeLastTickHW) << 8);
	return (newAdr);
}
static void INLINE tickHW(BYTE value) {
	while (value > 0) {
		cpu.opCycle++;
		nmi.before = nmi.high;
		irq.before = irq.high;
		ppuTick(1);
		apuTick(1, &value);
		if (extclCPUEveryCycle) {
			/*
			 * utilizzato dalle mappers :
			 * 183
			 * 222
			 * Bandai (FCGX)
			 * FDS
			 * Futeremedia
			 * Kaise (ks202)
			 * Jaleco (SS8806)
			 * Irem (H3000)
			 * Namco (163)
			 * Tengen (Rambo)
			 * VRC3
			 * VRC4
			 * VRC6
			 * VRC7
			 * Sunsoft (S3)
			 * Sunsoft (FM7)
			 * TxROM
			 */
			extclCPUEveryCycle();
		}
		cpu.oddCycle = !cpu.oddCycle;
		value--;
		modCyclesOP(-=, 1);
	}
}
/* --------------------------------------------------------------------------------------------- */

#endif /* CPUINLINE_H_ */

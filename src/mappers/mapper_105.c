/*
 * mapper_105.c
 *
 *  Created on: 08/gen/2014
 *      Author: fhorse
 *
 *  Note : Questa mapper e' una MMC1 modificata.
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

static void INLINE ctrl_reg_105(void);
static void INLINE swap_prg_rom_105(void);

/*
 * IRQ Counter:
 * ---------------------------
 *
 * The 'I' bit in $A000 controls the IRQ counter.  When cleared, the IRQ counter counts up
 * every cycle.  When set, the IRQ counter is reset to 0 and stays there (does not count),
 * and the pending IRQ is acknowledged.
 *
 * The cart has 4 dipswitches which control how high the counter must reach for
 * an IRQ to be generated.
 *
 * The IRQ counter is 30 bits wide.. when it reaches the following value, an IRQ is fired:
 *
 * [1D CBAx xxxx xxxx xxxx xxxx xxxx xxxx]
 *   ^ ^^^
 *   | |||
 *   either 0 or 1, depending on the corresponding dipswitch.
 *
 * So if all dipswitches are open (use '0' above), the counter must reach $20000000.
 * If all dipswitches are closed (use '1' above), the counter must reach $3E000000.
 * etc
 *
 * In the official tournament, 'C' was closed, and the others were open, so
 * the counter had to reach $2800000.
 */
#define M105_DIPSWITCH 0x14

enum MMC1_regs { CTRL, CHR0, CHR1, PRG0 };

WORD prg_rom_32k_max, prg_rom_16k_max;
uint32_t counter_must_reach;

void map_init_105(void) {
	prg_rom_32k_max = (info.prg.rom.banks_16k >> 1) - 1;
	prg_rom_16k_max = info.prg.rom.banks_16k - 1;

	EXTCL_CPU_WR_MEM(105);
	EXTCL_SAVE_MAPPER(105);
	EXTCL_CPU_EVERY_CYCLE(105);
	mapper.internal_struct[0] = (BYTE *) &m105;
	mapper.internal_struct_size[0] = sizeof(m105);

	counter_must_reach = M105_DIPSWITCH << 25;

	if (info.reset >= HARD) {
		memset(&m105, 0x00, sizeof(m105));
		m105.ctrl = 0x0C;
		m105.prg.mode = 3;
	}

	m105.prg.locked = TRUE;
	m105.prg.reg[3] = 1;
	map_prg_rom_8k(4, 0, 0);
}
void extcl_cpu_wr_mem_105(WORD address, BYTE value) {
	/*
	 * se nel tick precedente e' stato fatto un reset e
	 * sono in presenza di una doppia scrittura da parte
	 * di un'istruzione (tipo l'INC), allora l'MMC1 non
	 * la considera. Roms interessate:
	 * Advanced Dungeons & Dragons - Hillsfar
	 * Bill & Ted's Excellent Video Game Adventure
	 * Snow Brothers
	 */
	if (m105.reset) {
		/* azzero il flag */
		m105.reset = FALSE;
		/* esco se necessario */
		if (cpu.double_wr) {
			return;
		}
	}
	/*
	 * A program's reset code will reset the mapper
	 * first by writing a value of $80 through $FF
	 * to any address in $8000-$FFFF.
	 */
	if (value & 0x80) {
		/* indico che e' stato fatto un reset */
		m105.reset = TRUE;
		/* azzero posizione e registro temporaneo */
		m105.pos = m105.reg = 0;
		/*
		 * reset shift register and write
		 * Control with (Control OR $0C).
		 */
		m105.ctrl |= 0x0C;
		/* reinizializzo tutto */
		ctrl_reg_105();
		/*
		 * locking PRG ROM at $C000-$FFFF
		 * to the last 16k bank.
		 */
		if (m105.prg.locked == FALSE) {
			if (m105.prg.upper) {
				map_prg_rom_8k(2, 2, m105.prg.upper | (prg_rom_16k_max & 0x0F));
				map_prg_rom_8k_update();
			}
		}
		return;
	}

	m105.reg |= ((value & 0x01) << m105.pos);

	if (m105.pos++ == 4) {
		BYTE reg = (address >> 13) & 0x03;

		switch (reg) {
			case CTRL:
				m105.ctrl = m105.reg;
				ctrl_reg_105();
				break;
			case CHR0:
				m105.prg.reg[0] = (m105.reg & 0x06) >> 1;
				m105.prg.upper = m105.reg & 0x08;
		        /*
		         * questo bit, oltre a controllare l'irq e' usato anche
		         * per sbloccare lo swap del prg.
		         */
				if (!m105.irq.reg && (m105.reg & 0x10)) {
					/* sblocco lo swap della prg */
					m105.prg.locked = FALSE;
				}
				if ((m105.irq.reg = m105.reg & 0x10)) {
					m105.irq.count = 0;
					irq.high &= ~EXT_IRQ;
		        }
				break;
			case CHR1:
				break;
			case PRG0:
				m105.prg.reg[1] = m105.reg & 0x0F;
				cpu.prg_ram_rd_active = (reg & 0x10 ? FALSE : TRUE);
				cpu.prg_ram_wr_active = cpu.prg_ram_rd_active;
				break;
		}
		swap_prg_rom_105();
		/* azzero posizione e registro temporaneo */
		m105.pos = m105.reg = 0;
	}
}
BYTE extcl_save_mapper_105(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m105.reg);
	save_slot_ele(mode, slot, m105.pos);
	save_slot_ele(mode, slot, m105.ctrl);
	save_slot_ele(mode, slot, m105.reset);
	save_slot_ele(mode, slot, m105.prg.mode);
	save_slot_ele(mode, slot, m105.prg.locked);
	save_slot_ele(mode, slot, m105.prg.upper);
	save_slot_ele(mode, slot, m105.prg.reg);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_105(void) {
	if (!m105.irq.reg) {
		if (++m105.irq.count == counter_must_reach) {
			m105.irq.count = 0;
			irq.high |= EXT_IRQ;
		}
	}
}

static void INLINE ctrl_reg_105(void) {
	m105.prg.mode = (m105.ctrl & 0x0C) >> 2;
	switch (m105.ctrl & 0x03) {
		case 0x00:
			mirroring_SCR0();
			break;
		case 0x01:
			mirroring_SCR1();
			break;
		case 0x02:
			mirroring_V();
			break;
		case 0x03:
			mirroring_H();
			break;
	}
}
static void INLINE swap_prg_rom_105(void) {
	BYTE value;

	if (m105.prg.locked == TRUE) {
		map_prg_rom_8k(4, 0, 0);
	} else if (!m105.prg.upper) {
		value = m105.prg.reg[0];
		control_bank(prg_rom_32k_max)
		map_prg_rom_8k(4, 0, value);
	} else {
		value = m105.prg.reg[1];
		switch (m105.prg.mode) {
			case 0:
			case 1: {
				BYTE bank;

				control_bank_with_AND(0x0E, prg_rom_16k_max)
				bank = m105.prg.upper | value;
				/* switch 32k at $8000, ignoring low bit of bank number */
				map_prg_rom_8k(2, 0, bank);
				map_prg_rom_8k(2, 2, bank | 0x01);
				break;
			}
			case 2:
				control_bank_with_AND(0x0F, prg_rom_16k_max)
				/* fix first 16k bank at $8000 and switch 16 KB bank at $C000 */
				map_prg_rom_8k(2, 0, m105.prg.upper);
				map_prg_rom_8k(2, 2, m105.prg.upper | value);
				break;
			case 3:
				control_bank_with_AND(0x0F, prg_rom_16k_max)
				/* fix last 16k bank at $C000 and switch 16 KB bank at $8000 */
				map_prg_rom_8k(2, 0, m105.prg.upper | value);
				map_prg_rom_8k(2, 2, m105.prg.upper | (prg_rom_16k_max & 0x0F));
				break;
		}
	}
	map_prg_rom_8k_update();
}

// license:BSD-3-Clause
// copyright-holders:Tony La Porta
	/**************************************************************************\
	*                      Microchip PIC16C5x Emulator                         *
	*                                                                          *
	*                    Copyright Tony La Porta                               *
	*                 Originally written for the MAME project.                 *
	*                                                                          *
	*                                                                          *
	*      Addressing architecture is based on the Harvard addressing scheme.  *
	*                                                                          *
	\**************************************************************************/

#pragma once

#include <stdint.h>
#include "pic16c5x.h"

enum {
	PIC16C5x_RTCC = 0
};

typedef union {
	struct {
		unsigned n0 :4;
		unsigned n1 :4;
		unsigned n2 :4;
		unsigned n3 :4;
	};
	uint8_t b[2];
	uint16_t s[1];
} uint16_n;
#define b0 b[0]
#define b1 b[1]
#define s0 s[0]

// in the mid-90s RTCC was renamed to T0CKI
#define PIC16C5x_T0CKI PIC16C5x_RTCC

// i/o ports
enum {
	PIC16C5x_PORTA = 0,
	PIC16C5x_PORTB,
	PIC16C5x_PORTC,
	PIC16C5x_PORTD
};

class PIC16C5X {
public:
	/****************************************************************************
	 *  Function to configure the CONFIG register. This is actually hard-wired
	 *  during ROM programming, so should be called in the driver INIT, with
	 *  the value if known (available in HEX dumps of the ROM).
	 */
	void	set_input(int line, int state);
	void	set_config(uint16_t data);
	void	reset(uint8_t reset_type);
	void	run();
	BYTE	save_mapper(BYTE mode, BYTE slot, FILE *fp);
protected:
		PIC16C5X(int program_width, int data_width, int picmodel, uint8_t *rom, pic16c5x_rd_funct _rd, pic16c5x_wr_funct _wr);
private:

	/******************** CPU Internal Registers *******************/
	uint16_t  m_PC;
	uint16_t  m_PREVPC;     /* previous program counter */
	uint8_t   m_W;
	uint8_t   m_OPTION;
	uint16_t  m_CONFIG;
	uint8_t   m_ALU;
	uint16_t  m_WDT;
	uint8_t   m_TRISA;
	uint8_t   m_TRISB;
	uint8_t   m_TRISC;
	uint16_t  m_STACK[2];
	uint16_t  m_prescaler;  /* Note: this is really an 8-bit register */
	uint16_n  m_opcode;
	uint8_t   m_internalram[128];
	uint8_t   *m_rom;

	int       m_icount;
	int       m_picmodel;
	int       m_delay_timer;
	uint16_t  m_temp_config;
	int       m_rtcc;
	bool      m_count_pending;
	int8_t    m_old_data;
	uint8_t   m_picRAMmask;
	uint16_t  m_picROMmask;
	int       m_inst_cycles;
	int       m_clock2cycle;

	/*address_space *m_program;
	memory_access_cache<1, -1, ENDIANNESS_LITTLE> *m_cache;
	address_space *m_data;*/

	// i/o handlers
	pic16c5x_rd_funct m_read;
	pic16c5x_wr_funct m_write;

	/* opcode table entry */
	typedef void (PIC16C5X::*PIC16C5x_ophandler)();
	struct PIC16C5x_opcode
	{
		uint8_t   cycles;
		PIC16C5x_ophandler function;
	};
	static const PIC16C5x_opcode s_opcode_main[256];
	static const PIC16C5x_opcode s_opcode_00x[16];

	void update_internalram_ptr();
	void CALCULATE_Z_FLAG();
	void CALCULATE_ADD_CARRY();
	void CALCULATE_ADD_DIGITCARRY();
	void CALCULATE_SUB_CARRY();
	void CALCULATE_SUB_DIGITCARRY();
	uint16_t POP_STACK();
	void PUSH_STACK(uint16_t data);
	uint8_t GET_REGFILE(uint32_t addr);
	void STORE_REGFILE(uint32_t addr, uint8_t data);
	void STORE_RESULT(uint32_t addr, uint8_t data);
	void illegal();
	void addwf();
	void andwf();
	void andlw();
	void bcf();
	void bsf();
	void btfss();
	void btfsc();
	void call();
	void clrw();
	void clrf();
	void clrwdt();
	void comf();
	void decf();
	void decfsz();
	void goto_op();
	void incf();
	void incfsz();
	void iorlw();
	void iorwf();
	void movf();
	void movlw();
	void movwf();
	void nop();
	void option();
	void retlw();
	void rlf();
	void rrf();
	void sleepic();
	void subwf();
	void swapf();
	void tris();
	void xorlw();
	void xorwf();
	void PIC16C5x_reset_regs();
	void PIC16C5x_soft_reset();
	void PIC16C5x_update_watchdog(int counts);
	void PIC16C5x_update_timer(int counts);
};


class PIC16C54 : public PIC16C5X {
public:
	PIC16C54(uint8_t* rom, pic16c5x_rd_funct _rd, pic16c5x_wr_funct _wr);
};


class PIC16C55 : public PIC16C5X {
public:
	PIC16C55(uint8_t* rom, pic16c5x_rd_funct _rd, pic16c5x_wr_funct _wr);
};


class PIC16C56 : public PIC16C5X {
public:
	PIC16C56(uint8_t* rom, pic16c5x_rd_funct _rd, pic16c5x_wr_funct _wr);
};


class PIC16C57 : public PIC16C5X {
public:
	PIC16C57(uint8_t* rom, pic16c5x_rd_funct _rd, pic16c5x_wr_funct _wr);
};


class PIC16C58 : public PIC16C5X {
public:
	PIC16C58(uint8_t* rom, pic16c5x_rd_funct _rd, pic16c5x_wr_funct _wr);
};


class PIC1650 : public PIC16C5X {
public:
	PIC1650(uint8_t* rom, pic16c5x_rd_funct _rd, pic16c5x_wr_funct _wr);
};


class PIC1655 : public PIC16C5X {
public:
	PIC1655(uint8_t* rom, pic16c5x_rd_funct _rd, pic16c5x_wr_funct _wr);
};

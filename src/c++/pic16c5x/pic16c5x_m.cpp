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
	*                                                                          *
	*  **** Change Log ****                                                    *
	*  TLP (06-Apr-2003)                                                       *
	*   - First Public release.                                                *
	*  BO  (07-Apr-2003) Ver 1.01                                              *
	*   - Renamed 'sleep' function to 'sleepic' to avoid C conflicts.          *
	*  TLP (09-Apr-2003) Ver 1.10                                              *
	*   - Fixed modification of file register $03 (Status).                    *
	*   - Corrected support for 7FFh (12-bit) size ROMs.                       *
	*   - The 'call' and 'goto' instructions weren't correctly handling the    *
	*     STATUS page info correctly.                                          *
	*   - The FSR register was incorrectly oring the data with 0xe0 when read. *
	*   - Prescaler masking information was set to 3 instead of 7.             *
	*   - Prescaler assign bit was set to 4 instead of 8.                      *
	*   - Timer source and edge select flags/masks were wrong.                 *
	*   - Corrected the memory bank selection in GET/SET_REGFILE and also the  *
	*     indirect register addressing.                                        *
	*  BMP (18-May-2003) Ver 1.11                                              *
	*   - PIC16C5x_get_reg functions were missing 'returns'.                   *
	*  TLP (27-May-2003) Ver 1.12                                              *
	*   - Fixed the WatchDog timer count.                                      *
	*   - The Prescaler rate was incorrectly being zeroed, instead of the      *
	*     actual Prescaler counter in the CLRWDT and SLEEP instructions.       *
	*   - Added masking to the FSR register. Upper unused bits are always 1.   *
	*  TLP (27-Aug-2009) Ver 1.13                                              *
	*   - Indirect addressing was not taking into account special purpose      *
	*     memory mapped locations.                                             *
	*   - 'iorlw' instruction was saving the result to memory instead of       *
	*     the W register.                                                      *
	*   - 'tris' instruction no longer modifies Port-C on PIC models that      *
	*     do not have Port-C implemented.                                      *
	*  TLP (07-Sep-2009) Ver 1.14                                              *
	*   - Edge sense control for the T0 count input was incorrectly reversed   *
	*  LE (05-Feb-2017) Ver 1.15                                               *
	*   - Allow writing all bits of the status register except TO and PD.      *
	*     This enables e.g. bcf, bsf or clrf to change the flags when the      *
	*     status register is the destination.                                  *
	*   - Changed rlf and rrf to update the carry flag in the last step.       *
	*     Fixes the case where the status register is the destination.         *
	*  hap (12-Feb-2017) Ver 1.16                                              *
	*   - Added basic support for the old GI PIC1650 and PIC1655.              *
	*   - Made RTCC(aka T0CKI) pin an inputline handler.                       *
	*                                                                          *
	*                                                                          *
	*  **** Notes: ****                                                        *
	*  PIC WatchDog Timer has a separate internal clock. For the moment, we're *
	*     basing the count on a 4MHz input clock, since 4MHz is the typical    *
	*     input frequency (but by no means always).                            *
	*  A single scaler is available for the Counter/Timer or WatchDog Timer.   *
	*     When connected to the Counter/Timer, it functions as a Prescaler,    *
	*     hence prescale overflows, tick the Counter/Timer.                    *
	*     When connected to the WatchDog Timer, it functions as a Postscaler   *
	*     hence WatchDog Timer overflows, tick the Postscaler. This scenario   *
	*     means that the WatchDog timeout occurs when the Postscaler has       *
	*     reached the scaler rate value, not when the WatchDog reaches zero.   *
	*  CLRWDT should prevent the WatchDog Timer from timing out and generating *
	*     a device reset, but how is not known. The manual also mentions that  *
	*     the WatchDog Timer can only be disabled during ROM programming, and  *
	*     no other means seem to exist???                                      *
	*                                                                          *
	\**************************************************************************/

#include "pic16c5x_m.h"
#include "save_slot.h"

PIC16C5X::PIC16C5X(int program_width, int data_width, int picmodel, uint8_t *rom, pic16c5x_rd_funct _rd, pic16c5x_wr_funct _wr):
	m_rom(rom),
	m_picmodel(picmodel),
	m_temp_config(0),
	m_picRAMmask((1 << data_width) - 1),
	m_picROMmask((1 << program_width) - 1),
	m_read(_rd),
	m_write(_wr) {
	m_PC = 0;
	m_PREVPC = 0;
	m_W = 0;
	m_OPTION = 0;
	m_CONFIG = 0;
	m_ALU = 0;
	m_WDT = 0;
	m_TRISA = 0;
	m_TRISB = 0;
	m_TRISC = 0;
	m_prescaler = 0;
	m_icount = 0;
	m_delay_timer = 0;
	m_rtcc = 0;
	m_count_pending = 0;
	m_old_data = 0;
	m_inst_cycles = 0;
	m_clock2cycle = 0;
}

PIC16C54::PIC16C54(uint8_t *rom, pic16c5x_rd_funct _rd, pic16c5x_wr_funct _wr): PIC16C5X( 9, 5, 0x16C54, rom, _rd, _wr) {};
PIC16C55::PIC16C55(uint8_t *rom, pic16c5x_rd_funct _rd, pic16c5x_wr_funct _wr): PIC16C5X( 9, 5, 0x16C55, rom, _rd, _wr) {};
PIC16C56::PIC16C56(uint8_t *rom, pic16c5x_rd_funct _rd, pic16c5x_wr_funct _wr): PIC16C5X(10, 5, 0x16C56, rom, _rd, _wr) {};
PIC16C57::PIC16C57(uint8_t *rom, pic16c5x_rd_funct _rd, pic16c5x_wr_funct _wr): PIC16C5X(11, 7, 0x16C57, rom, _rd, _wr) {};
PIC16C58::PIC16C58(uint8_t *rom, pic16c5x_rd_funct _rd, pic16c5x_wr_funct _wr): PIC16C5X(11, 7, 0x16C58, rom, _rd, _wr) {};
PIC1650::PIC1650  (uint8_t *rom, pic16c5x_rd_funct _rd, pic16c5x_wr_funct _wr): PIC16C5X( 9, 5, 0x1650,  rom, _rd, _wr) {};
PIC1655::PIC1655  (uint8_t *rom, pic16c5x_rd_funct _rd, pic16c5x_wr_funct _wr): PIC16C5X( 9, 5, 0x1655,  rom, _rd, _wr) {};

#define M_RDRAM(A)      m_internalram[A &m_picRAMmask]
#define M_WRTRAM(A,V)   m_internalram[A &m_picRAMmask] =(V)
#define M_RDOP(A)       m_rom[(A &m_picROMmask) <<1 |0] | m_rom[(A &m_picROMmask) <<1 |1] <<8
#define ADDR_MASK       0x7ff

#define TMR0    m_internalram[1]
#define PCL     m_internalram[2]
#define STATUS  m_internalram[3]
#define FSR     m_internalram[4]
#define PORTA   m_internalram[5]
#define PORTB   m_internalram[6]
#define PORTC   m_internalram[7]
#define PORTD   m_internalram[8]
#define INDF    M_RDRAM(FSR)

#define ADDR    (m_opcode.b0 & 0x1f)


/********  The following is the Status Flag register definition.  *********/
			/* | 7 | 6 | 5 |  4 |  3 | 2 |  1 | 0 | */
			/* |    PA     | TO | PD | Z | DC | C | */
#define PA_REG      0xe0    /* PA   Program Page Preselect - bit 8 is unused here */
#define TO_FLAG     0x10    /* TO   Time Out flag (WatchDog) */
#define PD_FLAG     0x08    /* PD   Power Down flag */
#define Z_FLAG      0x04    /* Z    Zero Flag */
#define DC_FLAG     0x02    /* DC   Digit Carry/Borrow flag (Nibble) */
#define C_FLAG      0x01    /* C    Carry/Borrow Flag (Byte) */

#define PA      (STATUS & PA_REG)
#define TO      (STATUS & TO_FLAG)
#define PD      (STATUS & PD_FLAG)
#define ZERO    (STATUS & Z_FLAG)
#define DC      (STATUS & DC_FLAG)
#define CARRY   (STATUS & C_FLAG)


/********  The following is the Option Flag register definition.  *********/
			/* | 7 | 6 |   5  |   4  |  3  | 2 | 1 | 0 | */
			/* | 0 | 0 | TOCS | TOSE | PSA |    PS     | */
#define T0CS_FLAG   0x20    /* TOCS     Timer 0 clock source select */
#define T0SE_FLAG   0x10    /* TOSE     Timer 0 clock source edge select */
#define PSA_FLAG    0x08    /* PSA      Prescaler Assignment bit */
#define PS_REG      0x07    /* PS       Prescaler Rate select */

#define T0CS    (m_OPTION & T0CS_FLAG)
#define T0SE    (m_OPTION & T0SE_FLAG)
#define PSA     (m_OPTION & PSA_FLAG)
#define PS      (m_OPTION & PS_REG)


/********  The following is the Config Flag register definition.  *********/
	/* | 11 | 10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 |   2  | 1 | 0 | */
	/* |              CP                     | WDTE |  FOSC | */
							/* CP       Code Protect (ROM read protect) */
#define WDTE_FLAG   0x04    /* WDTE     WatchDog Timer enable */
#define FOSC_FLAG   0x03    /* FOSC     Oscillator source select */

#define WDTE    (m_CONFIG & WDTE_FLAG)
#define FOSC    (m_CONFIG & FOSC_FLAG)


/************************************************************************
 *  Shortcuts
 ************************************************************************/

#define CLR(flagreg, flag) ( flagreg &= (uint8_t)(~flag) )
#define SET(flagreg, flag) ( flagreg |=  flag )


/* Easy bit position selectors */
#define POS  ((m_opcode.b0 >> 5) & 7)
static const unsigned int bit_clr[8] = { 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f };
static const unsigned int bit_set[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

void PIC16C5X::CALCULATE_Z_FLAG()
{
	if (m_ALU == 0) SET(STATUS, Z_FLAG);
	else CLR(STATUS, Z_FLAG);
}

void PIC16C5X::CALCULATE_ADD_CARRY()
{
	if ((uint8_t)(m_old_data) > (uint8_t)(m_ALU)) {
		SET(STATUS, C_FLAG);
	}
	else {
		CLR(STATUS, C_FLAG);
	}
}

void PIC16C5X::CALCULATE_ADD_DIGITCARRY()
{
	if (((uint8_t)(m_old_data) & 0x0f) > ((uint8_t)(m_ALU) & 0x0f)) {
		SET(STATUS, DC_FLAG);
	}
	else {
		CLR(STATUS, DC_FLAG);
	}
}

void PIC16C5X::CALCULATE_SUB_CARRY()
{
	if ((uint8_t)(m_old_data) < (uint8_t)(m_ALU)) {
		CLR(STATUS, C_FLAG);
	}
	else {
		SET(STATUS, C_FLAG);
	}
}

void PIC16C5X::CALCULATE_SUB_DIGITCARRY()
{
	if (((uint8_t)(m_old_data) & 0x0f) < ((uint8_t)(m_ALU) & 0x0f)) {
		CLR(STATUS, DC_FLAG);
	}
	else {
		SET(STATUS, DC_FLAG);
	}
}



uint16_t PIC16C5X::POP_STACK()
{
	uint16_t data = m_STACK[1];
	m_STACK[1] = m_STACK[0];
	return (data & ADDR_MASK);
}
void PIC16C5X::PUSH_STACK(uint16_t data)
{
	m_STACK[0] = m_STACK[1];
	m_STACK[1] = (data & ADDR_MASK);
}



uint8_t PIC16C5X::GET_REGFILE(uint32_t addr) /* Read from internal memory */
{
	uint8_t data = 0;

	if (addr == 0) {                        /* Indirect addressing  */
		addr = (FSR & m_picRAMmask);
	}

	if ((m_picmodel == 0x16C57) || (m_picmodel == 0x16C58)) {
		addr |= (FSR & 0x60);     /* FSR bits 6-5 are used for banking in direct mode */
	}

	if ((addr & 0x10) == 0) addr &= 0x0f;

	switch(addr)
	{
		case 0:     /* Not an actual register, so return 0 */
					data = 0;
					break;
		case 4:     data = (FSR | (uint8_t)(~m_picRAMmask));
					break;
		case 5:     /* read port A */
					if (m_picmodel == 0x1650) {
						data = m_read(PIC16C5x_PORTA) & PORTA;
					}
					else if (m_picmodel == 0x1655) {
						data = m_read(PIC16C5x_PORTA) & 0x0f;
					}
					else {
						data = m_read(PIC16C5x_PORTA);
						data &= m_TRISA;
						data |= ((uint8_t)(~m_TRISA) & PORTA);
						data &= 0x0f; /* 4-bit port (only lower 4 bits used) */
					}
					break;
		case 6:     /* read port B */
					if (m_picmodel == 0x1650) {
						data = m_read(PIC16C5x_PORTB) & PORTB;
					}
					else if (m_picmodel != 0x1655) { /* B is output-only on 1655 */
						data = m_read(PIC16C5x_PORTB);
						data &= m_TRISB;
						data |= ((uint8_t)(~m_TRISB) & PORTB);
					}
					break;
		case 7:     /* read port C */
					if (m_picmodel == 0x1650 || m_picmodel == 0x1655) {
						data = m_read(PIC16C5x_PORTC) & PORTC;
					}
					else if ((m_picmodel == 0x16C55) || (m_picmodel == 0x16C57)) {
						data = m_read(PIC16C5x_PORTC);
						data &= m_TRISC;
						data |= ((uint8_t)(~m_TRISC) & PORTC);
					}
					else { /* PIC16C54, PIC16C56, PIC16C58 */
						data = M_RDRAM(addr);
					}
					break;
		case 8:     /* read port D */
					if (m_picmodel == 0x1650) {
						data = m_read(PIC16C5x_PORTD) & PORTD;
					}
					else {
						data = M_RDRAM(addr);
					}
					break;
		default:    data = M_RDRAM(addr);
					break;
	}
	return data;
}

void PIC16C5X::STORE_REGFILE(uint32_t addr, uint8_t data)    /* Write to internal memory */
{
	if (addr == 0) {                        /* Indirect addressing  */
		addr = (FSR & m_picRAMmask);
	}

	if ((m_picmodel == 0x16C57) || (m_picmodel == 0x16C58)) {
		addr |= (FSR & 0x60);     /* FSR bits 6-5 are used for banking in direct mode */
	}

	if ((addr & 0x10) == 0) addr &= 0x0f;

	switch(addr)
	{
		case 0:     /* Not an actual register, nothing to save */
					break;
		case 1:     m_delay_timer = 2; /* Timer starts after next two instructions */
					if (PSA == 0) m_prescaler = 0; /* Must clear the Prescaler */
					TMR0 = data;
					break;
		case 2:     PCL = data;
					m_PC = ((STATUS & PA_REG) << 4) | data;
					break;
		case 3:     STATUS = (STATUS & (TO_FLAG | PD_FLAG)) | (data & (uint8_t)(~(TO_FLAG | PD_FLAG)));
					break;
		case 4:     FSR = (data | (uint8_t)(~m_picRAMmask));
					break;
		case 5:     /* write port A */
					if (m_picmodel == 0x1650) {
						m_write(PIC16C5x_PORTA, data);
					}
					else if (m_picmodel != 0x1655) { /* A is input-only on 1655 */
						data &= 0x0f; /* 4-bit port (only lower 4 bits used) */
						m_write(PIC16C5x_PORTA, data & (uint8_t)(~m_TRISA));
					}
					PORTA = data;
					break;
		case 6:     /* write port B */
					if (m_picmodel == 0x1650 || m_picmodel == 0x1655) {
						m_write(PIC16C5x_PORTB, data);
					}
					else {
						m_write(PIC16C5x_PORTB, data & (uint8_t)(~m_TRISB));
					}
					PORTB = data;
					break;
		case 7:     /* write port C */
					if (m_picmodel == 0x1650 || m_picmodel == 0x1655) {
						m_write(PIC16C5x_PORTC, data);
					}
					else if ((m_picmodel == 0x16C55) || (m_picmodel == 0x16C57)) {
						m_write(PIC16C5x_PORTC, data & (uint8_t)(~m_TRISC));
					}
					PORTC = data; /* also writes to RAM */
					break;
		case 8:     /* write port D */
					if (m_picmodel == 0x1650) {
						m_write(PIC16C5x_PORTD, data);
					}
					PORTD = data; /* also writes to RAM */
					break;
		default:    M_WRTRAM(addr, data);
					break;
	}
}


void PIC16C5X::STORE_RESULT(uint32_t addr, uint8_t data)
{
	if (m_opcode.b0 & 0x20)
	{
		STORE_REGFILE(addr, data);
	}
	else
	{
		m_W = data;
	}
}


/************************************************************************
 *  Emulate the Instructions
 ************************************************************************/

/* This following function is here to fill in the void for */
/* the opcode call function. This function is never called. */


void PIC16C5X::illegal()
{
	//printf("PIC16C5x:  PC=%03x,  Illegal opcode = %04x\n", (m_PC-1), m_opcode.s0);
}

/*
  Note:
  According to the manual, if the STATUS register is the destination for an instruction that affects the Z, DC or C bits
  then the write to these three bits is disabled. These bits are set or cleared according to the device logic.
  To ensure this is correctly emulated, in instructions that write to the file registers, always change the status flags
  *after* storing the result of the instruction.
  e.g. CALCULATE_*, SET(STATUS,*_FLAG) and CLR(STATUS,*_FLAG) should appear as the last steps of the instruction emulation.
*/

void PIC16C5X::addwf()
{
	m_old_data = GET_REGFILE(ADDR);
	m_ALU = m_old_data + m_W;
	STORE_RESULT(ADDR, m_ALU);
	CALCULATE_Z_FLAG();
	CALCULATE_ADD_CARRY();
	CALCULATE_ADD_DIGITCARRY();
}

void PIC16C5X::andwf()
{
	m_ALU = GET_REGFILE(ADDR) & m_W;
	STORE_RESULT(ADDR, m_ALU);
	CALCULATE_Z_FLAG();
}

void PIC16C5X::andlw()
{
	m_ALU = m_opcode.b0 & m_W;
	m_W = m_ALU;
	CALCULATE_Z_FLAG();
}

void PIC16C5X::bcf()
{
	m_ALU = GET_REGFILE(ADDR);
	m_ALU &= bit_clr[POS];
	STORE_REGFILE(ADDR, m_ALU);
}

void PIC16C5X::bsf()
{
	m_ALU = GET_REGFILE(ADDR);
	m_ALU |= bit_set[POS];
	STORE_REGFILE(ADDR, m_ALU);
}

void PIC16C5X::btfss()
{
	if ((GET_REGFILE(ADDR) & bit_set[POS]) == bit_set[POS])
	{
		m_PC++;
		PCL = m_PC & 0xff;
		m_inst_cycles += 1;     /* Add NOP cycles */
	}
}

void PIC16C5X::btfsc()
{
	if ((GET_REGFILE(ADDR) & bit_set[POS]) == 0)
	{
		m_PC++;
		PCL = m_PC & 0xff;
		m_inst_cycles += 1;     /* Add NOP cycles */
	}
}

void PIC16C5X::call()
{
	PUSH_STACK(m_PC);
	m_PC = ((STATUS & PA_REG) << 4) | m_opcode.b0;
	m_PC &= 0x6ff;
	PCL = m_PC & 0xff;
}

void PIC16C5X::clrw()
{
	m_W = 0;
	SET(STATUS, Z_FLAG);
}

void PIC16C5X::clrf()
{
	STORE_REGFILE(ADDR, 0);
	SET(STATUS, Z_FLAG);
}

void PIC16C5X::clrwdt()
{
	m_WDT = 0;
	if (PSA) m_prescaler = 0;
	SET(STATUS, TO_FLAG);
	SET(STATUS, PD_FLAG);
}

void PIC16C5X::comf()
{
	m_ALU = (uint8_t)(~(GET_REGFILE(ADDR)));
	STORE_RESULT(ADDR, m_ALU);
	CALCULATE_Z_FLAG();
}

void PIC16C5X::decf()
{
	m_ALU = GET_REGFILE(ADDR) - 1;
	STORE_RESULT(ADDR, m_ALU);
	CALCULATE_Z_FLAG();
}

void PIC16C5X::decfsz()
{
	m_ALU = GET_REGFILE(ADDR) - 1;
	STORE_RESULT(ADDR, m_ALU);
	if (m_ALU == 0)
	{
		m_PC++;
		PCL = m_PC & 0xff;
		m_inst_cycles += 1;     /* Add NOP cycles */
	}
}

void PIC16C5X::goto_op()
{
	m_PC = ((STATUS & PA_REG) << 4) | (m_opcode.s0 & 0x1ff);
	m_PC &= ADDR_MASK;
	PCL = m_PC & 0xff;
}

void PIC16C5X::incf()
{
	m_ALU = GET_REGFILE(ADDR) + 1;
	STORE_RESULT(ADDR, m_ALU);
	CALCULATE_Z_FLAG();
}

void PIC16C5X::incfsz()
{
	m_ALU = GET_REGFILE(ADDR) + 1;
	STORE_RESULT(ADDR, m_ALU);
	if (m_ALU == 0)
	{
		m_PC++;
		PCL = m_PC & 0xff;
		m_inst_cycles += 1;     /* Add NOP cycles */
	}
}

void PIC16C5X::iorlw()
{
	m_ALU = m_opcode.b0 | m_W;
	m_W = m_ALU;
	CALCULATE_Z_FLAG();
}

void PIC16C5X::iorwf()
{
	m_ALU = GET_REGFILE(ADDR) | m_W;
	STORE_RESULT(ADDR, m_ALU);
	CALCULATE_Z_FLAG();
}

void PIC16C5X::movf()
{
	m_ALU = GET_REGFILE(ADDR);
	STORE_RESULT(ADDR, m_ALU);
	CALCULATE_Z_FLAG();
}

void PIC16C5X::movlw()
{
	m_W = m_opcode.b0;
}

void PIC16C5X::movwf()
{
	STORE_REGFILE(ADDR, m_W);
}

void PIC16C5X::nop()
{
	/* Do nothing */
}

void PIC16C5X::option()
{
	m_OPTION = m_W & (T0CS_FLAG | T0SE_FLAG | PSA_FLAG | PS_REG);
}

void PIC16C5X::retlw()
{
	m_W = m_opcode.b0;
	m_PC = POP_STACK();
	PCL = m_PC & 0xff;
}

void PIC16C5X::rlf()
{
	m_ALU = GET_REGFILE(ADDR);
	uint8_t bit7 = m_ALU & 0x80;
	m_ALU <<= 1;
	if (STATUS & C_FLAG) m_ALU |= 1;
	STORE_RESULT(ADDR, m_ALU);
	if (bit7) SET(STATUS, C_FLAG);
	else CLR(STATUS, C_FLAG);
}

void PIC16C5X::rrf()
{
	m_ALU = GET_REGFILE(ADDR);
	uint8_t bit0 = m_ALU & 1;
	m_ALU >>= 1;
	if (STATUS & C_FLAG) m_ALU |= 0x80;
	STORE_RESULT(ADDR, m_ALU);
	if (bit0) SET(STATUS, C_FLAG);
	else CLR(STATUS, C_FLAG);
}

void PIC16C5X::sleepic()
{
	if (WDTE) m_WDT = 0;
	if (PSA) m_prescaler = 0;
	SET(STATUS, TO_FLAG);
	CLR(STATUS, PD_FLAG);
}

void PIC16C5X::subwf()
{
	m_old_data = GET_REGFILE(ADDR);
	m_ALU = m_old_data - m_W;
	STORE_RESULT(ADDR, m_ALU);
	CALCULATE_Z_FLAG();
	CALCULATE_SUB_CARRY();
	CALCULATE_SUB_DIGITCARRY();
}

void PIC16C5X::swapf()
{
	m_ALU  = ((GET_REGFILE(ADDR) << 4) & 0xf0);
	m_ALU |= ((GET_REGFILE(ADDR) >> 4) & 0x0f);
	STORE_RESULT(ADDR, m_ALU);
}

void PIC16C5X::tris()
{
	switch(m_opcode.b0 & 0x7)
	{
		case 5:     if   (m_TRISA == m_W) break;
					else { m_TRISA = m_W | 0xf0; m_write(PIC16C5x_PORTA, 0x1000 | (PORTA & (uint8_t)(~m_TRISA) & 0x0f)); break; }
		case 6:     if   (m_TRISB == m_W) break;
					else { m_TRISB = m_W; m_write(PIC16C5x_PORTB, 0x1000 | (PORTB & (uint8_t)(~m_TRISB))); break; }
		case 7:     if ((m_picmodel == 0x16C55) || (m_picmodel == 0x16C57)) {
						if   (m_TRISC == m_W) break;
						else { m_TRISC = m_W; m_write(PIC16C5x_PORTC, 0x1000 | (PORTC & (uint8_t)(~m_TRISC))); break; }
					}
					else {
						illegal(); break;
					}
		default:    illegal(); break;
	}
}

void PIC16C5X::xorlw()
{
	m_ALU = m_W ^ m_opcode.b0;
	m_W = m_ALU;
	CALCULATE_Z_FLAG();
}

void PIC16C5X::xorwf()
{
	m_ALU = GET_REGFILE(ADDR) ^ m_W;
	STORE_RESULT(ADDR, m_ALU);
	CALCULATE_Z_FLAG();
}




/***********************************************************************
 *  Opcode Table (Cycles, Instruction)
 ***********************************************************************/

const PIC16C5X::PIC16C5x_opcode PIC16C5X::s_opcode_main[256]=
{
/*00*/  {1, &PIC16C5X::nop     },{1, &PIC16C5X::illegal   },{1, &PIC16C5X::movwf     },{1, &PIC16C5X::movwf     },
		{1, &PIC16C5X::clrw    },{1, &PIC16C5X::illegal   },{1, &PIC16C5X::clrf      },{1, &PIC16C5X::clrf      },
/*08*/  {1, &PIC16C5X::subwf   },{1, &PIC16C5X::subwf     },{1, &PIC16C5X::subwf     },{1, &PIC16C5X::subwf     },
		{1, &PIC16C5X::decf    },{1, &PIC16C5X::decf      },{1, &PIC16C5X::decf      },{1, &PIC16C5X::decf      },
/*10*/  {1, &PIC16C5X::iorwf   },{1, &PIC16C5X::iorwf     },{1, &PIC16C5X::iorwf     },{1, &PIC16C5X::iorwf     },
		{1, &PIC16C5X::andwf   },{1, &PIC16C5X::andwf     },{1, &PIC16C5X::andwf     },{1, &PIC16C5X::andwf     },
/*18*/  {1, &PIC16C5X::xorwf   },{1, &PIC16C5X::xorwf     },{1, &PIC16C5X::xorwf     },{1, &PIC16C5X::xorwf     },
		{1, &PIC16C5X::addwf   },{1, &PIC16C5X::addwf     },{1, &PIC16C5X::addwf     },{1, &PIC16C5X::addwf     },
/*20*/  {1, &PIC16C5X::movf    },{1, &PIC16C5X::movf      },{1, &PIC16C5X::movf      },{1, &PIC16C5X::movf      },
		{1, &PIC16C5X::comf    },{1, &PIC16C5X::comf      },{1, &PIC16C5X::comf      },{1, &PIC16C5X::comf      },
/*28*/  {1, &PIC16C5X::incf    },{1, &PIC16C5X::incf      },{1, &PIC16C5X::incf      },{1, &PIC16C5X::incf      },
		{1, &PIC16C5X::decfsz  },{1, &PIC16C5X::decfsz    },{1, &PIC16C5X::decfsz    },{1, &PIC16C5X::decfsz    },
/*30*/  {1, &PIC16C5X::rrf     },{1, &PIC16C5X::rrf       },{1, &PIC16C5X::rrf       },{1, &PIC16C5X::rrf       },
		{1, &PIC16C5X::rlf     },{1, &PIC16C5X::rlf       },{1, &PIC16C5X::rlf       },{1, &PIC16C5X::rlf       },
/*38*/  {1, &PIC16C5X::swapf   },{1, &PIC16C5X::swapf     },{1, &PIC16C5X::swapf     },{1, &PIC16C5X::swapf     },
		{1, &PIC16C5X::incfsz  },{1, &PIC16C5X::incfsz    },{1, &PIC16C5X::incfsz    },{1, &PIC16C5X::incfsz    },
/*40*/  {1, &PIC16C5X::bcf     },{1, &PIC16C5X::bcf       },{1, &PIC16C5X::bcf       },{1, &PIC16C5X::bcf       },
		{1, &PIC16C5X::bcf     },{1, &PIC16C5X::bcf       },{1, &PIC16C5X::bcf       },{1, &PIC16C5X::bcf       },
/*48*/  {1, &PIC16C5X::bcf     },{1, &PIC16C5X::bcf       },{1, &PIC16C5X::bcf       },{1, &PIC16C5X::bcf       },
		{1, &PIC16C5X::bcf     },{1, &PIC16C5X::bcf       },{1, &PIC16C5X::bcf       },{1, &PIC16C5X::bcf       },
/*50*/  {1, &PIC16C5X::bsf     },{1, &PIC16C5X::bsf       },{1, &PIC16C5X::bsf       },{1, &PIC16C5X::bsf       },
		{1, &PIC16C5X::bsf     },{1, &PIC16C5X::bsf       },{1, &PIC16C5X::bsf       },{1, &PIC16C5X::bsf       },
/*58*/  {1, &PIC16C5X::bsf     },{1, &PIC16C5X::bsf       },{1, &PIC16C5X::bsf       },{1, &PIC16C5X::bsf       },
		{1, &PIC16C5X::bsf     },{1, &PIC16C5X::bsf       },{1, &PIC16C5X::bsf       },{1, &PIC16C5X::bsf       },
/*60*/  {1, &PIC16C5X::btfsc   },{1, &PIC16C5X::btfsc     },{1, &PIC16C5X::btfsc     },{1, &PIC16C5X::btfsc     },
		{1, &PIC16C5X::btfsc   },{1, &PIC16C5X::btfsc     },{1, &PIC16C5X::btfsc     },{1, &PIC16C5X::btfsc     },
/*68*/  {1, &PIC16C5X::btfsc   },{1, &PIC16C5X::btfsc     },{1, &PIC16C5X::btfsc     },{1, &PIC16C5X::btfsc     },
		{1, &PIC16C5X::btfsc   },{1, &PIC16C5X::btfsc     },{1, &PIC16C5X::btfsc     },{1, &PIC16C5X::btfsc     },
/*70*/  {1, &PIC16C5X::btfss   },{1, &PIC16C5X::btfss     },{1, &PIC16C5X::btfss     },{1, &PIC16C5X::btfss     },
		{1, &PIC16C5X::btfss   },{1, &PIC16C5X::btfss     },{1, &PIC16C5X::btfss     },{1, &PIC16C5X::btfss     },
/*78*/  {1, &PIC16C5X::btfss   },{1, &PIC16C5X::btfss     },{1, &PIC16C5X::btfss     },{1, &PIC16C5X::btfss     },
		{1, &PIC16C5X::btfss   },{1, &PIC16C5X::btfss     },{1, &PIC16C5X::btfss     },{1, &PIC16C5X::btfss     },
/*80*/  {2, &PIC16C5X::retlw   },{2, &PIC16C5X::retlw     },{2, &PIC16C5X::retlw     },{2, &PIC16C5X::retlw     },
		{2, &PIC16C5X::retlw   },{2, &PIC16C5X::retlw     },{2, &PIC16C5X::retlw     },{2, &PIC16C5X::retlw     },
/*88*/  {2, &PIC16C5X::retlw   },{2, &PIC16C5X::retlw     },{2, &PIC16C5X::retlw     },{2, &PIC16C5X::retlw     },
		{2, &PIC16C5X::retlw   },{2, &PIC16C5X::retlw     },{2, &PIC16C5X::retlw     },{2, &PIC16C5X::retlw     },
/*90*/  {2, &PIC16C5X::call    },{2, &PIC16C5X::call      },{2, &PIC16C5X::call      },{2, &PIC16C5X::call      },
		{2, &PIC16C5X::call    },{2, &PIC16C5X::call      },{2, &PIC16C5X::call      },{2, &PIC16C5X::call      },
/*98*/  {2, &PIC16C5X::call    },{2, &PIC16C5X::call      },{2, &PIC16C5X::call      },{2, &PIC16C5X::call      },
		{2, &PIC16C5X::call    },{2, &PIC16C5X::call      },{2, &PIC16C5X::call      },{2, &PIC16C5X::call      },
/*A0*/  {2, &PIC16C5X::goto_op },{2, &PIC16C5X::goto_op   },{2, &PIC16C5X::goto_op   },{2, &PIC16C5X::goto_op   },
		{2, &PIC16C5X::goto_op },{2, &PIC16C5X::goto_op   },{2, &PIC16C5X::goto_op   },{2, &PIC16C5X::goto_op   },
/*A8*/  {2, &PIC16C5X::goto_op },{2, &PIC16C5X::goto_op   },{2, &PIC16C5X::goto_op   },{2, &PIC16C5X::goto_op   },
		{2, &PIC16C5X::goto_op },{2, &PIC16C5X::goto_op   },{2, &PIC16C5X::goto_op   },{2, &PIC16C5X::goto_op   },
/*B0*/  {2, &PIC16C5X::goto_op },{2, &PIC16C5X::goto_op   },{2, &PIC16C5X::goto_op   },{2, &PIC16C5X::goto_op   },
		{2, &PIC16C5X::goto_op },{2, &PIC16C5X::goto_op   },{2, &PIC16C5X::goto_op   },{2, &PIC16C5X::goto_op   },
/*B8*/  {2, &PIC16C5X::goto_op },{2, &PIC16C5X::goto_op   },{2, &PIC16C5X::goto_op   },{2, &PIC16C5X::goto_op   },
		{2, &PIC16C5X::goto_op },{2, &PIC16C5X::goto_op   },{2, &PIC16C5X::goto_op   },{2, &PIC16C5X::goto_op   },
/*C0*/  {1, &PIC16C5X::movlw   },{1, &PIC16C5X::movlw     },{1, &PIC16C5X::movlw     },{1, &PIC16C5X::movlw     },
		{1, &PIC16C5X::movlw   },{1, &PIC16C5X::movlw     },{1, &PIC16C5X::movlw     },{1, &PIC16C5X::movlw     },
/*C8*/  {1, &PIC16C5X::movlw   },{1, &PIC16C5X::movlw     },{1, &PIC16C5X::movlw     },{1, &PIC16C5X::movlw     },
		{1, &PIC16C5X::movlw   },{1, &PIC16C5X::movlw     },{1, &PIC16C5X::movlw     },{1, &PIC16C5X::movlw     },
/*D0*/  {1, &PIC16C5X::iorlw   },{1, &PIC16C5X::iorlw     },{1, &PIC16C5X::iorlw     },{1, &PIC16C5X::iorlw     },
		{1, &PIC16C5X::iorlw   },{1, &PIC16C5X::iorlw     },{1, &PIC16C5X::iorlw     },{1, &PIC16C5X::iorlw     },
/*D8*/  {1, &PIC16C5X::iorlw   },{1, &PIC16C5X::iorlw     },{1, &PIC16C5X::iorlw     },{1, &PIC16C5X::iorlw     },
		{1, &PIC16C5X::iorlw   },{1, &PIC16C5X::iorlw     },{1, &PIC16C5X::iorlw     },{1, &PIC16C5X::iorlw     },
/*E0*/  {1, &PIC16C5X::andlw   },{1, &PIC16C5X::andlw     },{1, &PIC16C5X::andlw     },{1, &PIC16C5X::andlw     },
		{1, &PIC16C5X::andlw   },{1, &PIC16C5X::andlw     },{1, &PIC16C5X::andlw     },{1, &PIC16C5X::andlw     },
/*E8*/  {1, &PIC16C5X::andlw   },{1, &PIC16C5X::andlw     },{1, &PIC16C5X::andlw     },{1, &PIC16C5X::andlw     },
		{1, &PIC16C5X::andlw   },{1, &PIC16C5X::andlw     },{1, &PIC16C5X::andlw     },{1, &PIC16C5X::andlw     },
/*F0*/  {1, &PIC16C5X::xorlw   },{1, &PIC16C5X::xorlw     },{1, &PIC16C5X::xorlw     },{1, &PIC16C5X::xorlw     },
		{1, &PIC16C5X::xorlw   },{1, &PIC16C5X::xorlw     },{1, &PIC16C5X::xorlw     },{1, &PIC16C5X::xorlw     },
/*F8*/  {1, &PIC16C5X::xorlw   },{1, &PIC16C5X::xorlw     },{1, &PIC16C5X::xorlw     },{1, &PIC16C5X::xorlw     },
		{1, &PIC16C5X::xorlw   },{1, &PIC16C5X::xorlw     },{1, &PIC16C5X::xorlw     },{1, &PIC16C5X::xorlw     }
};


const PIC16C5X::PIC16C5x_opcode PIC16C5X::s_opcode_00x[16]=
{
/*00*/  {1, &PIC16C5X::nop     },{1, &PIC16C5X::illegal   },{1, &PIC16C5X::option    },{1, &PIC16C5X::sleepic   },
		{1, &PIC16C5X::clrwdt  },{1, &PIC16C5X::tris      },{1, &PIC16C5X::tris      },{1, &PIC16C5X::tris      },
/*08*/  {1, &PIC16C5X::illegal },{1, &PIC16C5X::illegal   },{1, &PIC16C5X::illegal   },{1, &PIC16C5X::illegal   },
		{1, &PIC16C5X::illegal },{1, &PIC16C5X::illegal   },{1, &PIC16C5X::illegal   },{1, &PIC16C5X::illegal   }
};



/****************************************************************************
 *  Inits CPU emulation
 ****************************************************************************/

enum
{
	PIC16C5x_PC=1, PIC16C5x_STK0, PIC16C5x_STK1, PIC16C5x_FSR,
	PIC16C5x_W,    PIC16C5x_ALU,  PIC16C5x_STR,  PIC16C5x_OPT,
	PIC16C5x_TMR0, PIC16C5x_PRTA, PIC16C5x_PRTB, PIC16C5x_PRTC, PIC16C5x_PRTD,
	PIC16C5x_WDT,  PIC16C5x_TRSA, PIC16C5x_TRSB, PIC16C5x_TRSC,
	PIC16C5x_PSCL
};

/****************************************************************************
 *  Reset registers to their initial values
 ****************************************************************************/

void PIC16C5X::PIC16C5x_reset_regs()
{
	m_PC     = m_picROMmask;
	m_CONFIG = m_temp_config;
	m_TRISA  = 0xff;
	m_TRISB  = 0xff;
	m_TRISC  = 0xff;
	m_OPTION = (T0CS_FLAG | T0SE_FLAG | PSA_FLAG | PS_REG);
	PCL    = 0xff;
	FSR   |= (uint8_t)(~m_picRAMmask);
	m_prescaler = 0;
	m_delay_timer = 0;
	m_inst_cycles = 0;
	m_count_pending = false;
}

void PIC16C5X::reset(uint8_t reset_type)
{
	if (reset_type >= HARD) {
		for (auto &c: m_internalram) c = 0;
		PIC16C5x_reset_regs();
		CLR(STATUS, PA_REG);
		SET(STATUS, (TO_FLAG | PD_FLAG));
		m_icount = 0;
		m_clock2cycle = 0;
	} else {
		SET(STATUS, (TO_FLAG | PD_FLAG | Z_FLAG | DC_FLAG | C_FLAG));
		PIC16C5x_reset_regs();
	}
}

void PIC16C5X::set_config(uint16_t data)
{
	m_temp_config = data;
}



/****************************************************************************
 *  WatchDog
 ****************************************************************************/

void PIC16C5X::PIC16C5x_update_watchdog(int counts)
{
	/* WatchDog is set up to count 18,000 (0x464f hex) ticks to provide */
	/* the timeout period of 0.018ms based on a 4MHz input clock. */
	/* Note: the 4MHz clock should be divided by the PIC16C5x_CLOCK_DIVIDER */
	/* which effectively makes the PIC run at 1MHz internally. */

	/* If the current instruction is CLRWDT or SLEEP, don't update the WDT */

	if ((m_opcode.s0 != 3) && (m_opcode.s0 != 4))
	{
		uint16_t old_WDT = m_WDT;

		m_WDT -= counts;

		if (m_WDT > 0x464f) {
			m_WDT = 0x464f - (0xffff - m_WDT);
		}

		if (((old_WDT != 0) && (old_WDT < m_WDT)) || (m_WDT == 0))
		{
			if (PSA) {
				m_prescaler++;
				if (m_prescaler >= (1 << PS)) { /* Prescale values from 1 to 128 */
					m_prescaler = 0;
					CLR(STATUS, TO_FLAG);
					//PIC16C5x_soft_reset();
				}
			}
			else {
				CLR(STATUS, TO_FLAG);
				//PIC16C5x_soft_reset();
			}
		}
	}
}


/****************************************************************************
 *  Update Timer
 ****************************************************************************/

void PIC16C5X::PIC16C5x_update_timer(int counts)
{
	if (PSA == 0) {
		m_prescaler += counts;
		if (m_prescaler >= (2 << PS)) { /* Prescale values from 2 to 256 */
			TMR0 += (m_prescaler / (2 << PS));
			m_prescaler %= (2 << PS);   /* Overflow prescaler */
		}
	}
	else {
		TMR0 += counts;
	}
}

void PIC16C5X::set_input(int line, int state)
{
	switch (line)
	{
		/* RTCC/T0CKI pin */
		case PIC16C5x_RTCC:
			if (T0CS && state != m_rtcc) /* Count mode, edge triggered */
				if ((T0SE && !state) || (!T0SE && state))
					m_count_pending = true;

			m_rtcc = state;
			break;

		default:
			break;
	}
}


void PIC16C5X::run()
{
	if ((++m_clock2cycle &3) ==0) m_icount++;

	while (m_icount > 0) {
		if (PD == 0) /* Sleep Mode */
		{
			m_count_pending = false;
			m_inst_cycles = 1;
			if (WDTE) {
				PIC16C5x_update_watchdog(1);
			}
		}
		else
		{
			if (m_count_pending) { /* RTCC/T0CKI clocked while in Count mode */
				m_count_pending = false;
				PIC16C5x_update_timer(1);
			}

			m_PREVPC = m_PC;

			m_opcode.s0 = M_RDOP(m_PC);
			//printf("%04X: %03X %03X\n", m_PC, m_opcode.s0, m_opcode.b0);
			m_PC++;
			PCL++;

			if (m_picmodel == 0x1650 || m_picmodel == 0x1655 || (m_opcode.s0 & 0xff0) != 0x000) { /* Do all opcodes except the 00? ones */
				m_inst_cycles = s_opcode_main[((m_opcode.s0 >> 4) & 0xff)].cycles;
				(this->*s_opcode_main[((m_opcode.s0 >> 4) & 0xff)].function)();
			}
			else {  /* Opcode 0x00? has many opcodes in its minor nibble */
				m_inst_cycles = s_opcode_00x[(m_opcode.b0 & 0x1f)].cycles;
				(this->*s_opcode_00x[(m_opcode.b0 & 0x1f)].function)();
			}

			if (!T0CS) { /* Timer mode */
				if (m_delay_timer) {
					m_delay_timer--;
				}
				else {
					PIC16C5x_update_timer(m_inst_cycles);
				}
			}
			if (WDTE) {
				PIC16C5x_update_watchdog(m_inst_cycles);
			}
		}

		m_icount -= m_inst_cycles;
	} 
}

BYTE PIC16C5X::save_mapper(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m_PC);
	save_slot_ele(mode, slot, m_PREVPC);
	save_slot_ele(mode, slot, m_W);
	save_slot_ele(mode, slot, m_OPTION);
	save_slot_ele(mode, slot, m_CONFIG);
	save_slot_ele(mode, slot, m_ALU);
	save_slot_ele(mode, slot, m_WDT);
	save_slot_ele(mode, slot, m_TRISA);
	save_slot_ele(mode, slot, m_TRISB);
	save_slot_ele(mode, slot, m_TRISC);
	save_slot_ele(mode, slot, m_STACK);
	save_slot_ele(mode, slot, m_prescaler);
	save_slot_ele(mode, slot, m_opcode.s0);
	save_slot_ele(mode, slot, m_internalram);
	save_slot_ele(mode, slot, m_icount);
	save_slot_ele(mode, slot, m_delay_timer);
	save_slot_ele(mode, slot, m_rtcc);
	save_slot_ele(mode, slot, m_count_pending);
	save_slot_ele(mode, slot, m_inst_cycles);
	save_slot_ele(mode, slot, m_clock2cycle);
	return (EXIT_OK);
}

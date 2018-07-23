/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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
 *  based on:
 *  s_VRC7.cpp -- Nintendulator Mapper DLLs by 2002-2011 QMT Productions
 *  based on:
 *  emu2413.c  -- YM2413 emulator written by Mitsutaka Okazaki 2001
 *
 ***********************************************************************************
 */

#include <math.h>
#include <float.h>
#include "mapper_VRC7_snd.h"
#include "save_slot.h"

/* Mask */
#define OPLL_MASK_CH(x) (1<<(x))

#define PI 3.14159265358979323846

#define UPDATE_PG(S) (S)->dphase = dphaseTable[(S)->fnum][(S)->block][(S)->patch.ML]
#define UPDATE_TLL(S)\
	(((S)->type==0)?\
	((S)->tll = tllTable[((S)->fnum)>>5][(S)->block][(S)->patch.TL][(S)->patch.KL]):\
	((S)->tll = tllTable[((S)->fnum)>>5][(S)->block][(S)->volume][(S)->patch.KL]))
#define UPDATE_RKS(S) (S)->rks = rksTable[((S)->fnum)>>8][(S)->block][(S)->patch.KR]
#define UPDATE_WF(S) (S)->sintbl = waveform[(S)->patch.WF]
#define UPDATE_EG(S) (S)->eg_dphase = calc_eg_dphase(S)
#define UPDATE_ALL(S)\
	UPDATE_PG(S);\
	UPDATE_TLL(S);\
	UPDATE_RKS(S);\
	UPDATE_WF(S);\
	UPDATE_EG(S)		/* EG should be updated last. */

/* Size of Sintable ( 8 -- 18 can be used. 9 recommended.)*/
#define PG_BITS 9
#define PG_WIDTH (1<<PG_BITS)

/* Phase increment counter */
#define DP_BITS 18
#define DP_WIDTH (1<<DP_BITS)
#define DP_BASE_BITS (DP_BITS - PG_BITS)

/* Dynamic range (Accuracy of sin table) */
#define DB_BITS 8
#define DB_STEP (48.0/(1<<DB_BITS))
#define DB_MUTE (1<<DB_BITS)

/* Dynamic range of envelope */
#define EG_STEP 0.375
#define EG_BITS 7
#define EG_MUTE (1<<EG_BITS)

/* Dynamic range of total level */
#define TL_STEP 0.750
#define TL_BITS 6
#define TL_MUTE (1<<TL_BITS)

/* Dynamic range of sustine level */
#define SL_STEP 3.0
#define SL_BITS 4
#define SL_MUTE (1<<SL_BITS)

#define EG2DB(d) ((d)*(int32_t)(EG_STEP/DB_STEP))
#define TL2EG(d) ((d)*(int32_t)(TL_STEP/EG_STEP))
#define SL2EG(d) ((d)*(int32_t)(SL_STEP/EG_STEP))

#define DB_POS(x) (uint32_t)((x)/DB_STEP)
#define DB_NEG(x) (uint32_t)(DB_MUTE+DB_MUTE+(x)/DB_STEP)

/* Bits for liner value */
#define DB2LIN_AMP_BITS 11
#define SLOT_AMP_BITS (DB2LIN_AMP_BITS)

/* Bits for envelope phase incremental counter */
#define EG_DP_BITS 22
#define EG_DP_WIDTH (1<<EG_DP_BITS)

/* Bits for Pitch and Amp modulator */
#define PM_PG_BITS 8
#define PM_PG_WIDTH (1<<PM_PG_BITS)
#define PM_DP_BITS 16
#define PM_DP_WIDTH (1<<PM_DP_BITS)
#define AM_PG_BITS 8
#define AM_PG_WIDTH (1<<AM_PG_BITS)
#define AM_DP_BITS 16
#define AM_DP_WIDTH (1<<AM_DP_BITS)

/* PM table is calcurated by PM_AMP * pow(2,PM_DEPTH*sin(x)/1200) */
#define PM_AMP_BITS 8
#define PM_AMP (1<<PM_AMP_BITS)

/* PM speed(Hz) and depth(cent) */
#define PM_SPEED 6.4
#define PM_DEPTH 13.75

/* AM speed(Hz) and depth(dB) */
#define AM_SPEED 3.7
#define AM_DEPTH 4.8

/* Cut the lower b bit(s) off. */
#define HIGHBITS(c,b) ((c)>>(b))

/* Leave the lower b bit(s). */
#define LOWBITS(c,b) ((c)&((1<<(b))-1))

/* Expand x which is s bits to d bits. */
#define EXPAND_BITS(x,s,d) ((x)<<((d)-(s)))

/* Expand x which is s bits to d bits and fill expanded bits '1' */
#define EXPAND_BITS_X(x,s,d) (((x)<<((d)-(s)))|((1<<((d)-(s)))-1))

/* Adjust envelope speed which depends on sampling rate. */
/* added 0.5 to round the value*/
#define rate_adjust(x) (rate==49716?x:(uint32_t)((double)(x)*clk/72/rate + 0.5))

#define MOD(x) (&opll.slot[(x)<<1])
#define CAR(x) (&opll.slot[((x)<<1)|1])

#define BIT(s,b) (((s)>>(b))&1)

/* Convert Amp(0 to EG_HEIGHT) to Phase(0 to 2PI). */
#if ( SLOT_AMP_BITS - PG_BITS ) > 0
#define wave2_2pi(e)	( (e) >> ( SLOT_AMP_BITS - PG_BITS ))
#else
#define wave2_2pi(e)	( (e) << ( PG_BITS - SLOT_AMP_BITS ))
#endif

/* Convert Amp(0 to EG_HEIGHT) to Phase(0 to 4PI). */
#if ( SLOT_AMP_BITS - PG_BITS - 1 ) == 0
#define wave2_4pi(e)	(e)
#elif ( SLOT_AMP_BITS - PG_BITS - 1 ) > 0
#define wave2_4pi(e)	( (e) >> ( SLOT_AMP_BITS - PG_BITS - 1 ))
#else
#define wave2_4pi(e)	( (e) << ( 1 + PG_BITS - SLOT_AMP_BITS ))
#endif

/* Convert Amp(0 to EG_HEIGHT) to Phase(0 to 8PI). */
#if ( SLOT_AMP_BITS - PG_BITS - 2 ) == 0
#define wave2_8pi(e)	(e)
#elif ( SLOT_AMP_BITS - PG_BITS - 2 ) > 0
#define wave2_8pi(e)	( (e) >> ( SLOT_AMP_BITS - PG_BITS - 2 ))
#else
#define wave2_8pi(e)	( (e) << ( 2 + PG_BITS - SLOT_AMP_BITS ))
#endif

enum { OPLL_VRC7_TONE = 0 };
/* Definition of envelope mode */
enum {
	SETTLE,
	ATTACK,
	DECAY,
	SUSHOLD,
	SUSTINE,
	RLEASE,
	FINISH
};

/* voice data */
typedef struct {
	uint32_t TL;
	uint32_t FB;
	uint32_t EG;
	uint32_t ML;
	uint32_t AR;
	uint32_t DR;
	uint32_t SL;
	uint32_t RR;
	uint32_t KR;
	uint32_t KL;
	uint32_t AM;
	uint32_t PM;
	uint32_t WF;
} _patch;
/* slot */
typedef struct {
	int32_t type;           /* 0 : modulator 1 : carrier */
	/* OUTPUT */
	int32_t feedback;
	int32_t output[2];      /* Output value of slot */
	/* for Phase Generator (PG) */
	uint16_t *sintbl;       /* Wavetable */
	uint32_t phase;         /* Phase */
	uint32_t dphase;        /* Phase increment amount */
	uint32_t pgout;         /* output */
	/* for Envelope Generator (EG) */
	int32_t fnum;           /* F-Number */
	int32_t block;          /* Block */
	int32_t volume;         /* Current volume */
	int32_t sustine;        /* Sustine 1 = ON, 0 = OFF */
	uint32_t tll;	        /* Total Level + Key scale level*/
	uint32_t rks;           /* Key scale offset (Rks) */
	int32_t eg_mode;        /* Current state */
	uint32_t eg_phase;      /* Phase */
	uint32_t eg_dphase;     /* Phase increment amount */
	uint32_t egout;         /* output */
	_patch patch;
} _slot;
/* opll */
typedef struct {
	int32_t out;
	uint32_t adr;
	uint32_t mask;
	uint32_t real_step;
	uint32_t opll_time;
	uint32_t opll_step;
	int32_t prev;
	int32_t next;
	/* Register */
	uint8_t low_freq[6];
	uint8_t hi_freq[6];
	uint8_t inst_vol[6];
	/* Pitch Modulator */
	uint32_t pm_phase;
	int32_t lfo_pm;
	/* Amp Modulator */
	int32_t am_phase;
	int32_t lfo_am;
	/* Channel Data */
	int32_t patch_number[6];
	int32_t key_status[6];

	int32_t slot_on_flag[6 * 2];

	uint8_t custom_inst[8];
	/* Slot */
	_slot slot[6 * 2];
} _opll;

static void INLINE slot_reset(_slot *slot, int type);
static void INLINE make_tables(uint32_t c, uint32_t r);
static void INLINE internal_refresh(void);
static void INLINE set_instrument(uint8_t i, uint8_t inst);
static void INLINE update_ampm(void);
static int32_t INLINE min(int32_t i, int32_t j);
static int32_t INLINE lin2db(double d);
static uint32_t INLINE calc_eg_dphase(_slot * slot);
static void INLINE slot_on(_slot *slot);
static void INLINE slot_off(_slot *slot);
static void INLINE key_on(int32_t i);
static void INLINE key_off(int32_t i);
static void INLINE set_sustine(int32_t c, int32_t sustine);
static void INLINE set_volume(int32_t c, int32_t volume);
static void INLINE set_fnumber(int32_t c, int32_t fnum);
static void INLINE set_block(int32_t c, int32_t block);
static void INLINE update_key_status(void);
static int32_t INLINE calc(void);
static void INLINE calc_phase(_slot *slot, int32_t lfo);
static void INLINE calc_envelope(_slot *slot, int32_t lfo);
static int32_t INLINE calc_slot_car(_slot *slot, int32_t fm);
static int32_t INLINE calc_slot_mod(_slot *slot);

static const unsigned char default_inst[15][8] = {
	{0x03, 0x21, 0x04, 0x06, 0x8D, 0xF2, 0x42, 0x17}, // Violin
	{0x13, 0x41, 0x05, 0x0E, 0x99, 0x96, 0x63, 0x12}, // Guitar
	{0x31, 0x11, 0x10, 0x0A, 0xF0, 0x9C, 0x32, 0x02}, // Piano
	{0x21, 0x61, 0x1D, 0x07, 0x9F, 0x64, 0x20, 0x27}, // Flute
	{0x22, 0x21, 0x1E, 0x06, 0xF0, 0x76, 0x08, 0x28}, // Clarinet
	{0x02, 0x01, 0x06, 0x00, 0xF0, 0xF2, 0x03, 0x95}, // Oboe
	{0x21, 0x61, 0x1C, 0x07, 0x82, 0x81, 0x16, 0x07}, // Trumpet
	{0x23, 0x21, 0x1A, 0x17, 0xEF, 0x82, 0x25, 0x15}, // Organ
	{0x25, 0x11, 0x1F, 0x00, 0x86, 0x41, 0x20, 0x11}, // Horn
	{0x85, 0x01, 0x1F, 0x0F, 0xE4, 0xA2, 0x11, 0x12}, // Synthesizer
	{0x07, 0xC1, 0x2B, 0x45, 0xB4, 0xF1, 0x24, 0xF4}, // Harpsichord
	{0x61, 0x23, 0x11, 0x06, 0x96, 0x96, 0x13, 0x16}, // Vibraphone
	{0x01, 0x02, 0xD3, 0x05, 0x82, 0xA2, 0x31, 0x51}, // Synthesizer Bass
	{0x61, 0x22, 0x0D, 0x02, 0xC3, 0x7F, 0x24, 0x05}, // Acoustic Bass
	{0x21, 0x62, 0x0E, 0x00, 0xA1, 0xA0, 0x44, 0x17}  // Electric Guitar
};
/* Input clock */
static uint32_t clk = 844451141;
/* Sampling rate */
static uint32_t rate = 3354932;
/* WaveTable for each envelope amp */
//static uint16_t fullsintable[PG_WIDTH];
//static uint16_t halfsintable[PG_WIDTH];
//static uint16_t *waveform[2] = { fullsintable, halfsintable };

enum { fullsintable, halfsintable };
static uint16_t waveform[2][PG_WIDTH];

/* LFO Table */
static int32_t pmtable[PM_PG_WIDTH];
static int32_t amtable[AM_PG_WIDTH];
/* Phase delta for LFO */
static uint32_t pm_dphase;
static uint32_t am_dphase;
/* dB to Liner table */
static int16_t DB2LIN_TABLE[(DB_MUTE + DB_MUTE) * 2];
/* Liner to Log curve conversion table (for Attack rate). */
static uint16_t AR_ADJUST_TABLE[1 << EG_BITS];
/* Phase incr table for Attack */
static uint32_t dphaseARTable[16][16];
/* Phase incr table for Decay and Release */
static uint32_t dphaseDRTable[16][16];
/* KSL + TL Table */
static uint32_t tllTable[16][8][1 << TL_BITS][4];
static int32_t rksTable[2][8][2];
/* Phase incr table for PG */
static uint32_t dphaseTable[512][8][16];

uint8_t initialized = 0;

_opll opll;

/* Reset whole of OPLL except patch datas. */
void opll_reset(uint32_t clk, uint32_t rate) {
	int32_t i;

	make_tables(clk, rate);

	if (initialized == FALSE) {
		opll.mask = 0;

		opll.adr = 0;
		opll.out = 0;

		opll.pm_phase = 0;
		opll.am_phase = 0;

		opll.mask = 0;

		for (i = 0; i < 12; i++) {
			slot_reset(&opll.slot[i], i % 2);
		}

		for (i = 0; i < 6; i++) {
			opll.key_status[i] = 0;
			//setPatch(&OPLL, i, 0);
		}

		for (i = 0; i < 0x40; i++) {
			opll_write_reg(i, 0);
		}
		initialized = TRUE;
	}

	opll.real_step = (uint32_t) ((1 << 31) / rate);
	opll.opll_step = (uint32_t) ((1 << 31) / (clk / 72));
	opll.opll_time = 0;
}
void opll_write_reg(uint32_t reg, uint8_t value) {
	int32_t i, v, ch;

	reg = reg & 0x3f;

	switch (reg) {
		case 0x00:
			opll.custom_inst[0] = value;
			for (i = 0; i < 6; i++) {
				if (opll.patch_number[i] == 0) {
					set_instrument(i, 0);
					UPDATE_PG(MOD(i));
					UPDATE_RKS(MOD(i));
					UPDATE_EG(MOD(i));
				}
			}
			break;
		case 0x01:
			opll.custom_inst[1] = value;
			for (i = 0; i < 6; i++) {
				if (opll.patch_number[i] == 0) {
					set_instrument(i, 0);
					UPDATE_PG(CAR(i));
					UPDATE_RKS(CAR(i));
					UPDATE_EG(CAR(i));
				}
			}
			break;
		case 0x02:
			opll.custom_inst[2] = value;
			for (i = 0; i < 6; i++) {
				if (opll.patch_number[i] == 0) {
					set_instrument(i, 0);
					UPDATE_TLL(MOD(i));
				}
			}
			break;
		case 0x03:
			opll.custom_inst[3] = value;
			for (i = 0; i < 6; i++) {
				if (opll.patch_number[i] == 0) {
					set_instrument(i, 0);
					UPDATE_WF(MOD(i));
					UPDATE_WF(CAR(i));
				}
			}
			break;
		case 0x04:
			opll.custom_inst[4] = value;
			for (i = 0; i < 6; i++) {
				if (opll.patch_number[i] == 0) {
					set_instrument(i, 0);
					UPDATE_EG(MOD(i));
				}
			}
			break;
		case 0x05:
			opll.custom_inst[5] = value;
			for (i = 0; i < 6; i++) {
				if (opll.patch_number[i] == 0) {
					set_instrument(i, 0);
					UPDATE_EG(CAR(i));
				}
			}
			break;
		case 0x06:
			opll.custom_inst[6] = value;
			for (i = 0; i < 6; i++) {
				if (opll.patch_number[i] == 0) {
					set_instrument(i, 0);
					UPDATE_EG(MOD(i));
				}
			}
			break;
		case 0x07:
			opll.custom_inst[7] = value;
			for (i = 0; i < 6; i++) {
				if (opll.patch_number[i] == 0) {
					set_instrument(i, 0);
					UPDATE_EG(CAR(i));
				}
			}
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
			ch = reg - 0x10;
			opll.low_freq[ch] = value;
			set_fnumber(ch, value + ((opll.hi_freq[ch] & 1) << 8));
			UPDATE_ALL(MOD(ch));
			UPDATE_ALL(CAR(ch));
			break;
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
			ch = reg - 0x20;
			opll.hi_freq[ch] = value;

			set_fnumber(ch, ((value & 1) << 8) + opll.low_freq[ch]);
			set_block(ch, (value >> 1) & 7);
			set_sustine(ch, (value >> 5) & 1);
			if (value & 0x10) {
				key_on(ch);
			} else {
				key_off(ch);
			}
			UPDATE_ALL(MOD(ch));
			UPDATE_ALL(CAR(ch));
			update_key_status();
			break;
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
			ch = reg - 0x30;
			opll.inst_vol[ch] = value;
			i = (value >> 4) & 15;
			v = value & 15;
			set_instrument(ch, i);
			set_volume(ch, v << 2);
			UPDATE_ALL(MOD(ch));
			UPDATE_ALL(CAR(ch));
			break;
		default:
			break;
	}
}
BYTE opll_save(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, opll.out);
	save_slot_ele(mode, slot, opll.adr);
	save_slot_ele(mode, slot, opll.mask);
	save_slot_ele(mode, slot, opll.real_step);
	save_slot_ele(mode, slot, opll.opll_time);
	save_slot_ele(mode, slot, opll.opll_step);
	save_slot_ele(mode, slot, opll.prev);
	save_slot_ele(mode, slot, opll.next);
	save_slot_ele(mode, slot, opll.low_freq);
	save_slot_ele(mode, slot, opll.hi_freq);
	save_slot_ele(mode, slot, opll.inst_vol);
	save_slot_ele(mode, slot, opll.pm_phase);
	save_slot_ele(mode, slot, opll.lfo_pm);
	save_slot_ele(mode, slot, opll.am_phase);
	save_slot_ele(mode, slot, opll.lfo_am);
	save_slot_ele(mode, slot, opll.patch_number);
	save_slot_ele(mode, slot, opll.key_status);
	save_slot_ele(mode, slot, opll.slot_on_flag);
	save_slot_ele(mode, slot, opll.custom_inst);
	{
		BYTE i;

		for (i = 0; i < LENGTH(opll.slot); i++) {
			save_slot_ele(mode, slot, opll.slot[i].type);
			save_slot_ele(mode, slot, opll.slot[i].feedback);
			save_slot_ele(mode, slot, opll.slot[i].output);
			save_slot_pos(mode, slot, waveform[0], opll.slot[i].sintbl);
			save_slot_ele(mode, slot, opll.slot[i].phase);
			save_slot_ele(mode, slot, opll.slot[i].dphase);
			save_slot_ele(mode, slot, opll.slot[i].pgout);
			save_slot_ele(mode, slot, opll.slot[i].fnum);
			save_slot_ele(mode, slot, opll.slot[i].block);
			save_slot_ele(mode, slot, opll.slot[i].volume);
			save_slot_ele(mode, slot, opll.slot[i].sustine);
			save_slot_ele(mode, slot, opll.slot[i].tll);
			save_slot_ele(mode, slot, opll.slot[i].rks);
			save_slot_ele(mode, slot, opll.slot[i].eg_mode);
			save_slot_ele(mode, slot, opll.slot[i].eg_phase);
			save_slot_ele(mode, slot, opll.slot[i].eg_dphase);
			save_slot_ele(mode, slot, opll.slot[i].egout);
			save_slot_ele(mode, slot, opll.slot[i].patch.TL);
			save_slot_ele(mode, slot, opll.slot[i].patch.FB);
			save_slot_ele(mode, slot, opll.slot[i].patch.EG);
			save_slot_ele(mode, slot, opll.slot[i].patch.ML);
			save_slot_ele(mode, slot, opll.slot[i].patch.AR);
			save_slot_ele(mode, slot, opll.slot[i].patch.DR);
			save_slot_ele(mode, slot, opll.slot[i].patch.SL);
			save_slot_ele(mode, slot, opll.slot[i].patch.RR);
			save_slot_ele(mode, slot, opll.slot[i].patch.KR);
			save_slot_ele(mode, slot, opll.slot[i].patch.KL);
			save_slot_ele(mode, slot, opll.slot[i].patch.AM);
			save_slot_ele(mode, slot, opll.slot[i].patch.PM);
			save_slot_ele(mode, slot, opll.slot[i].patch.WF);
		}
	}

	return (EXIT_OK);
}
SWORD opll_calc(void) {
	while (opll.real_step > opll.opll_time) {
		opll.opll_time += opll.opll_step;
		opll.prev = opll.next;
		opll.next = calc();
	}

	opll.opll_time -= opll.real_step;
	opll.out = ((double) opll.next * (opll.opll_step - opll.opll_time)
			+ (double) opll.prev * opll.opll_time) / opll.opll_step;

	return ((int16_t) opll.out);
}

static void INLINE slot_reset(_slot *slot, int type) {
	slot->type = type;
	slot->sintbl = waveform[0];
	slot->phase = 0;
	slot->dphase = 0;
	slot->output[0] = 0;
	slot->output[1] = 0;
	slot->feedback = 0;
	slot->eg_mode = SETTLE;
	slot->eg_phase = EG_DP_WIDTH;
	slot->eg_dphase = 0;
	slot->rks = 0;
	slot->tll = 0;
	slot->sustine = 0;
	slot->fnum = 0;
	slot->block = 0;
	slot->volume = 0;
	slot->pgout = 0;
	slot->egout = 0;
}
static void INLINE make_tables(uint32_t c, uint32_t r) {
	int32_t i;

	if (c != clk) {
		clk = c;

		/* Table for Pitch Modulator */
		for (i = 0; i < PM_PG_WIDTH; i++) {
			pmtable[i] = (int32_t)((double) PM_AMP * pow(2.0, (double) PM_DEPTH *
					sin(2.0 * PI * i / PM_PG_WIDTH) / 1200));
		}
		/* Table for Amp Modulator */
		for (i = 0; i < AM_PG_WIDTH; i++) {
			amtable[i] = (int32_t)((double) AM_DEPTH / 2 / DB_STEP * (1.0 +
					sin(2.0 * PI * i / PM_PG_WIDTH)));
		}
		/* Table for dB(0 -- (1<<DB_BITS)-1) to Liner(0 -- DB2LIN_AMP_WIDTH) */
		for (i = 0; i < DB_MUTE + DB_MUTE; i++) {
			DB2LIN_TABLE[i] = (int16_t)((double) ((1 << DB2LIN_AMP_BITS) - 1) *
					pow(10.0, -(double) i * DB_STEP / 20));
			if (i >= DB_MUTE) {
				DB2LIN_TABLE[i] = 0;
			}
			DB2LIN_TABLE[i + DB_MUTE + DB_MUTE] = (int16_t)(-DB2LIN_TABLE[i]);
		}
		/* Table for AR to LogCurve. */
		AR_ADJUST_TABLE[0] = (1 << EG_BITS);
		for (i = 1; i < 128; i++) {
			AR_ADJUST_TABLE[i] = (uint16_t)((double) (1 << EG_BITS) - 1 - (1 << EG_BITS) *
					log((double) i) / log(128.0));
		}
		/* Table of Ttl */
		{
			#define dB2(x) ((x)*2)

			static double kltable[16] = {
				dB2(0.000),  dB2(9.000),  dB2(12.000), dB2(13.875),
				dB2(15.000), dB2(16.125), dB2(16.875), dB2(17.625),
				dB2(18.000), dB2(18.750), dB2(19.125), dB2(19.500),
				dB2(19.875), dB2(20.250), dB2(20.625), dB2(21.000)
			};

			int32_t tmp;
			int32_t fnum, block, TL, KL;

			for (fnum = 0; fnum < 16; fnum++) {
				for (block = 0; block < 8; block++) {
					for (TL = 0; TL < 64; TL++) {
						for (KL = 0; KL < 4; KL++) {
							if (KL == 0) {
								tllTable[fnum][block][TL][KL] = TL2EG(TL);
							} else {
								tmp = (int32_t)(kltable[fnum] - dB2(3.000) * (7 - block));
								if (tmp <= 0) {
									tllTable[fnum][block][TL][KL] = TL2EG(TL);
								} else {
									tllTable[fnum][block][TL][KL] = (uint32_t)(
											(tmp >> (3 - KL)) / EG_STEP) + TL2EG(TL);
								}
							}
						}
					}
				}
			}
		}

		/* RksTable */
		{
			int32_t fnum8, block, KR;

			for (fnum8 = 0; fnum8 < 2; fnum8++) {
				for (block = 0; block < 8; block++) {
					for (KR = 0; KR < 2; KR++) {
						if (KR != 0) {
							rksTable[fnum8][block][KR] = (block << 1) + fnum8;
						} else {
							rksTable[fnum8][block][KR] = block >> 1;
						}
					}
				}
			}
		}
		/* Sin Table */
		for (i = 0; i < PG_WIDTH / 4; i++) {
			waveform[fullsintable][i] = (uint16_t) lin2db(sin(2.0 * PI * i / PG_WIDTH));
		}
		for (i = 0; i < PG_WIDTH / 4; i++) {
			waveform[fullsintable][PG_WIDTH / 2 - 1 - i] = waveform[fullsintable][i];
		}
		for (i = 0; i < PG_WIDTH / 2; i++) {
			waveform[fullsintable][PG_WIDTH / 2 + i] = (uint16_t) (DB_MUTE + DB_MUTE
				+ waveform[fullsintable][i]);
		}
		for (i = 0; i < PG_WIDTH / 2; i++) {
			waveform[halfsintable][i] = waveform[fullsintable][i];
		}
		for (i = PG_WIDTH / 2; i < PG_WIDTH; i++) {
			waveform[halfsintable][i] = waveform[fullsintable][0];
		}
		//makeDefaultPatch ();
	}

	if (r != rate) {
		rate = r;
		internal_refresh();
	}
}
static void INLINE internal_refresh(void) {
	/* Phase increment counter table */
	{
		uint32_t fnum, block, ML;
		uint32_t mltable[16] = {
			 1,      1 * 2,  2 * 2,  3 * 2,
			 4 * 2,  5 * 2,  6 * 2,  7 * 2,
			 8 * 2,  9 * 2, 10 * 2, 10 * 2,
			12 * 2, 12 * 2, 15 * 2, 15 * 2
		};

		for (fnum = 0; fnum < 512; fnum++) {
			for (block = 0; block < 8; block++) {
				for (ML = 0; ML < 16; ML++) {
					dphaseTable[fnum][block][ML] =
							rate_adjust(((fnum * mltable[ML]) << block) >> (20 - DP_BITS));
				}
			}
		}
	}

	/* Rate Table for Attack */
	{
		int32_t AR, Rks, RM, RL;
#ifdef USE_SPEC_ENV_SPEED
		uint32_t attacktable[16][4];

		for (RM = 0; RM < 16; RM++)
			for (RL = 0; RL < 4; RL++) {
				if (RM == 0) {
					attacktable[RM][RL] = 0;
				} else if (RM == 15) {
					attacktable[RM][RL] = EG_DP_WIDTH;
				} else {
					attacktable[RM][RL] = (uint32_t) ((double) (1 << EG_DP_BITS) /
							(attacktime[RM][RL] * 3579545 / 72000));
				}

			}
#endif

		for (AR = 0; AR < 16; AR++) {
			for (Rks = 0; Rks < 16; Rks++) {
				RM = AR + (Rks >> 2);
				RL = Rks & 3;
				if (RM > 15) {
					RM = 15;
				}
				switch (AR) {
					case 0:
						dphaseARTable[AR][Rks] = 0;
						break;
					case 15:
						dphaseARTable[AR][Rks] = 0;/*EG_DP_WIDTH;*/
						break;
					default:
#ifdef USE_SPEC_ENV_SPEED
						dphaseARTable[AR][Rks] = rate_adjust(attacktable[RM][RL]);
#else
						dphaseARTable[AR][Rks] = rate_adjust((3 * (RL + 4) << (RM + 1)));
#endif
						break;
				}
			}
		}
	}
	/* Rate Table for Decay and Release */
	{
		int32_t DR, Rks, RM, RL;

#ifdef USE_SPEC_ENV_SPEED
		uint32_t decaytable[16][4];

		for (RM = 0; RM < 16; RM++) {
			for (RL = 0; RL < 4; RL++) {
				if (RM == 0) {
					decaytable[RM][RL] = 0;
				} else {
					decaytable[RM][RL] = (uint32_t) ((double) (1 << EG_DP_BITS) /
							(decaytime[RM][RL] * 3579545 / 72000));
				}
			}
		}
#endif

		for (DR = 0; DR < 16; DR++) {
			for (Rks = 0; Rks < 16; Rks++) {
				RM = DR + (Rks >> 2);
				RL = Rks & 3;
				if (RM > 15) {
					RM = 15;
				}
				switch (DR) {
					case 0:
						dphaseDRTable[DR][Rks] = 0;
						break;
					default:
#ifdef USE_SPEC_ENV_SPEED
						dphaseDRTable[DR][Rks] = rate_adjust(decaytable[RM][RL]);
#else
						dphaseDRTable[DR][Rks] = rate_adjust((RL + 4) << (RM - 1));
#endif
						break;
				}
			}
		}
	}

	pm_dphase = (uint32_t) rate_adjust(PM_SPEED * PM_DP_WIDTH / (clk / 72));
	am_dphase = (uint32_t) rate_adjust(AM_SPEED * AM_DP_WIDTH / (clk / 72));
}
static void INLINE set_instrument(uint8_t i, uint8_t inst) {
	const uint8_t *src;
	_patch *modp, *carp;

	opll.patch_number[i] = inst;

	if (inst) {
		src = default_inst[inst - 1];
	} else {
		src = opll.custom_inst;
	}

	modp = &MOD(i)->patch;
	carp = &CAR(i)->patch;

	modp->AM = (src[0] >> 7) & 1;
	modp->PM = (src[0] >> 6) & 1;
	modp->EG = (src[0] >> 5) & 1;
	modp->KR = (src[0] >> 4) & 1;
	modp->ML = (src[0] & 0xF);

	carp->AM = (src[1] >> 7) & 1;
	carp->PM = (src[1] >> 6) & 1;
	carp->EG = (src[1] >> 5) & 1;
	carp->KR = (src[1] >> 4) & 1;
	carp->ML = (src[1] & 0xF);

	modp->KL = (src[2] >> 6) & 3;
	modp->TL = (src[2] & 0x3F);

	carp->KL = (src[3] >> 6) & 3;
	carp->WF = (src[3] >> 4) & 1;

	modp->WF = (src[3] >> 3) & 1;

	modp->FB = (src[3]) & 7;

	modp->AR = (src[4] >> 4) & 0xF;
	modp->DR = (src[4] & 0xF);

	carp->AR = (src[5] >> 4) & 0xF;
	carp->DR = (src[5] & 0xF);

	modp->SL = (src[6] >> 4) & 0xF;
	modp->RR = (src[6] & 0xF);

	carp->SL = (src[7] >> 4) & 0xF;
	carp->RR = (src[7] & 0xF);
}
/* Update AM, PM unit */
static void INLINE update_ampm(void) {
	opll.pm_phase = (opll.pm_phase + pm_dphase) & (PM_DP_WIDTH - 1);
	opll.am_phase = (opll.am_phase + am_dphase) & (AM_DP_WIDTH - 1);
	opll.lfo_am = amtable[HIGHBITS (opll.am_phase, AM_DP_BITS - AM_PG_BITS)];
	opll.lfo_pm = pmtable[HIGHBITS (opll.pm_phase, PM_DP_BITS - PM_PG_BITS)];
}
static int32_t INLINE min(int32_t i, int32_t j) {
	if (i < j) {
		return (i);
	} else {
		return (j);
	}
}
/* Liner(+0.0 - +1.0) to dB((1<<DB_BITS) - 1 -- 0) */
static int32_t INLINE lin2db(double d) {
	if (d == 0) {
		return (DB_MUTE - 1);
	} else {
		return min(-(int32_t) (20.0 * log10(d) / DB_STEP), DB_MUTE - 1); /* 0 -- 127 */
	}
}
static uint32_t INLINE calc_eg_dphase(_slot * slot) {
	switch (slot->eg_mode) {
		case ATTACK:
			return (dphaseARTable[slot->patch.AR][slot->rks]);
		case DECAY:
			return (dphaseDRTable[slot->patch.DR][slot->rks]);
		case SUSHOLD:
			return (0);
		case SUSTINE:
			return (dphaseDRTable[slot->patch.RR][slot->rks]);
		case RLEASE:
			if (slot->sustine) {
				return (dphaseDRTable[5][slot->rks]);
			} else if (slot->patch.EG) {
				return (dphaseDRTable[slot->patch.RR][slot->rks]);
			} else {
				return (dphaseDRTable[7][slot->rks]);
			}
		case FINISH:
			return (0);
		default:
			return (0);
	}
}
/* Slot key on	*/
static void INLINE slot_on(_slot *slot) {
	slot->eg_mode = ATTACK;
	slot->eg_phase = 0;
	slot->phase = 0;
}
/* Slot key off */
static void INLINE slot_off(_slot *slot) {
	if (slot->eg_mode == ATTACK) {
		slot->eg_phase = EXPAND_BITS(AR_ADJUST_TABLE[HIGHBITS(slot->eg_phase,
				EG_DP_BITS - EG_BITS)], EG_BITS, EG_DP_BITS);
	}
	slot->eg_mode = RLEASE;
}
/* Channel key on */
static void INLINE key_on(int32_t i) {
	if (!opll.slot_on_flag[i * 2]) {
		slot_on(MOD(i));
	}
	if (!opll.slot_on_flag[i * 2 + 1]) {
		slot_on(CAR(i));
	}
	opll.key_status[i] = 1;
}
/* Channel key off */
static void INLINE key_off(int32_t i) {
	if (opll.slot_on_flag[i * 2 + 1]) {
		slot_off(CAR(i));
	}
	opll.key_status[i] = 0;
}
/* Set sustine parameter */
static void INLINE set_sustine(int32_t c, int32_t sustine) {
	CAR(c)->sustine = sustine;
	if (MOD(c)->type) {
		MOD(c)->sustine = sustine;
	}
}
/* Volume : 6bit ( Volume register << 2 ) */
static void INLINE set_volume(int32_t c, int32_t volume) {
	CAR(c)->volume = volume;
}
/* Set F-Number ( fnum : 9bit ) */
static void INLINE set_fnumber(int32_t c, int32_t fnum) {
	CAR(c)->fnum = fnum;
	MOD(c)->fnum = fnum;
}
/* Set Block data (block : 3bit ) */
static void INLINE set_block(int32_t c, int32_t block) {
	CAR(c)->block = block;
	MOD(c)->block = block;
}
static void INLINE update_key_status(void) {
	int ch;

	for (ch = 0; ch < 6; ch++) {
		opll.slot_on_flag[ch * 2] = opll.slot_on_flag[ch * 2 + 1] = (opll.hi_freq[ch]) & 0x10;
	}
}
static int32_t INLINE calc(void) {
	int32_t inst = 0, i;

	update_ampm();

	for (i = 0; i < 12; i++) {
		calc_phase(&opll.slot[i], opll.lfo_pm);
		calc_envelope(&opll.slot[i], opll.lfo_am);
	}

	for (i = 0; i < 6; i++) {
		if (!(opll.mask & OPLL_MASK_CH(i)) && ((CAR(i)->eg_mode != FINISH))) {
			inst += calc_slot_car(CAR(i), calc_slot_mod(MOD(i)));
		}
	}

	return (inst);
}
/* PG */
static void INLINE calc_phase(_slot *slot, int32_t lfo) {
	if (slot->patch.PM) {
		slot->phase += (slot->dphase * lfo) >> PM_AMP_BITS;
	} else {
		slot->phase += slot->dphase;
	}
	slot->phase &= (DP_WIDTH - 1);
	slot->pgout = HIGHBITS(slot->phase, DP_BASE_BITS);
}
/* EG */
static void calc_envelope(_slot *slot, int32_t lfo) {

#define S2E(x) (SL2EG((int32_t)(x/SL_STEP))<<(EG_DP_BITS-EG_BITS))

	static uint32_t SL[16] = {
		S2E(0.0),  S2E(3.0),  S2E(6.0),  S2E(9.0),
		S2E(12.0), S2E(15.0), S2E(18.0), S2E(21.0),
		S2E(24.0), S2E(27.0), S2E(30.0), S2E(33.0),
		S2E(36.0), S2E(39.0), S2E(42.0), S2E(48.0)
	};

	uint32_t egout;

	switch (slot->eg_mode) {
		case ATTACK:
			egout = AR_ADJUST_TABLE[HIGHBITS (slot->eg_phase, EG_DP_BITS - EG_BITS)];
			slot->eg_phase += slot->eg_dphase;
			if ((EG_DP_WIDTH & slot->eg_phase) || (slot->patch.AR == 15)) {
				egout = 0;
				slot->eg_phase = 0;
				slot->eg_mode = DECAY;
				UPDATE_EG(slot);
			}
			break;
		case DECAY:
			egout = HIGHBITS (slot->eg_phase, EG_DP_BITS - EG_BITS);
			slot->eg_phase += slot->eg_dphase;
			if (slot->eg_phase >= SL[slot->patch.SL]) {
				if (slot->patch.EG) {
					slot->eg_phase = SL[slot->patch.SL];
					slot->eg_mode = SUSHOLD;
					UPDATE_EG(slot);
				} else {
					slot->eg_phase = SL[slot->patch.SL];
					slot->eg_mode = SUSTINE;
					UPDATE_EG(slot);
				}
			}
			break;
		case SUSHOLD:
			egout = HIGHBITS (slot->eg_phase, EG_DP_BITS - EG_BITS);
			if (slot->patch.EG == 0) {
				slot->eg_mode = SUSTINE;
				UPDATE_EG(slot);
			}
			break;
		case SUSTINE:
		case RLEASE:
			egout = HIGHBITS (slot->eg_phase, EG_DP_BITS - EG_BITS);
			slot->eg_phase += slot->eg_dphase;
			if (egout >= (1 << EG_BITS)) {
				slot->eg_mode = FINISH;
				egout = (1 << EG_BITS) - 1;
			}
			break;
		case FINISH:
			egout = (1 << EG_BITS) - 1;
			break;
		default:
			egout = (1 << EG_BITS) - 1;
			break;
	}

	if (slot->patch.AM) {
		egout = EG2DB(egout + slot->tll) + lfo;
	} else {
		egout = EG2DB(egout + slot->tll);
	}

	if (egout >= DB_MUTE) {
		egout = DB_MUTE - 1;
	}

	slot->egout = egout;
}
/* CARRIOR */
static int32_t INLINE calc_slot_car(_slot *slot, int32_t fm) {
	slot->output[1] = slot->output[0];

	if (slot->egout >= (DB_MUTE - 1)) {
		slot->output[0] = 0;
	} else {
		slot->output[0] = DB2LIN_TABLE[slot->sintbl[(slot->pgout + wave2_8pi(fm)) & (PG_WIDTH - 1)]
			+ slot->egout];
	}

	return (slot->output[1] + slot->output[0]) >> 1;
}
/* MODULATOR */
static int32_t INLINE calc_slot_mod(_slot *slot) {
	int32_t fm;

	slot->output[1] = slot->output[0];

	if (slot->egout >= (DB_MUTE - 1)) {
		slot->output[0] = 0;
	} else if (slot->patch.FB != 0) {
		fm = wave2_4pi(slot->feedback) >> (7 - slot->patch.FB);
		slot->output[0] = DB2LIN_TABLE[slot->sintbl[(slot->pgout + fm) & (PG_WIDTH - 1)]
			+ slot->egout];
	} else {
		slot->output[0] = DB2LIN_TABLE[slot->sintbl[slot->pgout] + slot->egout];
	}

	slot->feedback = (slot->output[1] + slot->output[0]) >> 1;

	return (slot->feedback);
}

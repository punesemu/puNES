/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

#ifndef APU_H_
#define APU_H_

#include "common.h"
#include "external_calls.h"

enum dmc_types_of_dma { DMC_NORMAL, DMC_CPU_WRITE, DMC_R4014, DMC_NNL_DMA };
enum apu_channels { APU_S1, APU_S2, APU_TR, APU_NS, APU_DMC, APU_EXTRA, APU_MASTER };
enum apu_mode { APU_60HZ, APU_48HZ };

#define apu_pre_amp 1.4f

/* length counter */
#define length_run(channel)\
	/*\
	 * se non e' settato il flag halt e il length\
	 * counter non e' 0 allora devo decrementarlo.\
	 */\
	if (!channel.length.halt && channel.length.value) {\
		channel.length.value--;\
	}
#define length_clock()\
	apu.length_clocked = TRUE;\
	length_run(S1)\
	length_run(S2)\
	length_run(TR)\
	length_run(NS)\
	if (extcl_length_clock) {\
		/*\
		 * utilizzato dalle mappers :\
		 * MMC5\
		 */\
		extcl_length_clock();\
	}
/* envelope */
#define envelope_run(channel)\
	if (channel.envelope.enabled) {\
		channel.envelope.enabled = FALSE;\
		channel.envelope.counter = 15;\
		channel.envelope.delay = (channel.envelope.divider + 1);\
	} else if (!(--channel.envelope.delay)) {\
		channel.envelope.delay = (channel.envelope.divider + 1);\
		if (channel.envelope.counter | channel.length.halt) {\
			channel.envelope.counter = (channel.envelope.counter - 1) & 0x0F;\
		}\
	}
#define envelope_volume(channel)\
	/* setto il volume */\
	if (!channel.length.value) {\
		channel.volume = 0;\
	} else if (channel.envelope.constant_volume) {\
		channel.volume = channel.envelope.divider;\
	} else {\
		channel.volume = channel.envelope.counter;\
	}
#define envelope_clock()\
	envelope_run(S1)\
	envelope_run(S2)\
	envelope_run(NS)\
	if (extcl_envelope_clock) {\
		/*\
		 * utilizzato dalle mappers :\
		 * MMC5\
		 */\
		extcl_envelope_clock();\
	}
/* sweep */
#define sweep_run(channel, negative_adjust)\
	if (!(--channel.sweep.delay)) {\
		channel.sweep.delay = (channel.sweep.divider + 1);\
		if (channel.sweep.enabled && channel.sweep.shift && (channel.timer >= 8)) {\
			SWORD offset = channel.timer >> channel.sweep.shift;\
			if (channel.sweep.negate) {\
				channel.timer += ((SWORD)negative_adjust - offset);\
			} else if ((channel.timer + offset) <= 0x800) {\
				channel.timer += offset;\
			}\
		}\
		sweep_silence(channel)\
	}\
	if (channel.sweep.reload) {\
		channel.sweep.reload = FALSE;\
		channel.sweep.delay = (channel.sweep.divider + 1);\
	}
#define sweep_silence(channel)\
{\
	WORD offset = channel.timer >> channel.sweep.shift;\
	channel.sweep.silence = FALSE;\
	if ((channel.timer < 8) || (!channel.sweep.negate && ((channel.timer + offset) >= 0x800))) {\
		channel.sweep.silence = TRUE;\
	}\
}
#define sweep_clock()\
	sweep_run(S1, -1)\
	sweep_run(S2,  0)
/* linear counter */
#define linear_clock()\
	if (TR.linear.halt) {\
		TR.linear.value = TR.linear.reload;\
	} else if (TR.linear.value) {\
		TR.linear.value--;\
	}\
	if (!TR.length.halt) {\
		TR.linear.halt = FALSE;\
	}
/* output */
#define square_output(square, swap)\
{\
	envelope_volume(square)\
	if (square.sweep.silence) {\
		square.output = 0;\
	} else {\
		square.output = square_duty[swap][square.duty][square.sequencer] * square.volume;\
	}\
}
#define triangle_output()\
	/*\
	 * ai 2 cicli piu' bassi del timer, la frequenza\
	 * risultante e' troppo alta (oltre i 20 kHz,\
	 * quindi non udibile), percio' la taglio.\
	 */\
	TR.output = triangle_duty[TR.sequencer];\
	if (TR.timer < 2) {\
		TR.output = triangle_duty[8];\
	}
#define noise_output()\
	envelope_volume(NS)\
	NS.output = 0;\
	if (NS.length.value && !(NS.shift & 0x0001)) {\
		NS.output = NS.volume;\
	}
#define dmc_output()\
	DMC.output = DMC.counter & 0x7F
/* tick */
#define square_tick(square, swap, type)\
	if (!(--square.frequency)) {\
		square_output(square, swap)\
		square.frequency = (square.timer + 1) << 1;\
		square.sequencer = (square.sequencer + 1) & 0x07;\
		type.clocked = TRUE;\
	}
#define triangle_tick()\
	if (!(--TR.frequency)) {\
		TR.frequency = TR.timer + 1;\
		if (TR.length.value && TR.linear.value) {\
			TR.sequencer = (TR.sequencer + 1) & 0x1F;\
			triangle_output()\
			apu.clocked = TRUE;\
		}\
	}
#define noise_tick()\
	if (!(--NS.frequency)) {\
		if (NS.mode) {\
			NS.shift = (NS.shift >> 1) | (((NS.shift ^ (NS.shift >> 6)) & 0x0001) << 14);\
		} else {\
			NS.shift = (NS.shift >> 1) | (((NS.shift ^ (NS.shift >> 1)) & 0x0001) << 14);\
		}\
		NS.shift &= 0x7FFF;\
		noise_output()\
		NS.frequency = noise_timer[apu.type][NS.timer];\
		apu.clocked = TRUE;\
	}
#define dmc_tick()\
	if (!(--DMC.frequency)) {\
		if (!DMC.silence) {\
			if (!(DMC.shift & 0x01)) {\
				if (DMC.counter > 1) {\
					DMC.counter -= 2;\
				}\
			} else {\
				if (DMC.counter < 126) {\
					DMC.counter += 2;\
				}\
			}\
		}\
		DMC.shift >>= 1;\
		dmc_output();\
		if (!(--DMC.counter_out)) {\
			DMC.counter_out = 8;\
			if (!DMC.empty) {\
				DMC.shift = DMC.buffer;\
				DMC.empty = TRUE;\
				DMC.silence = FALSE;\
			} else {\
				DMC.silence = TRUE;\
			}\
		}\
		DMC.frequency = dmc_rate[apu.type][DMC.rate_index];\
		apu.clocked = TRUE;\
	}\
	if (DMC.empty && DMC.remain) {\
		BYTE tick = 4;\
		switch (DMC.tick_type) {\
			case DMC_CPU_WRITE:\
				tick = 3;\
				break;\
			case DMC_R4014:\
				tick = 2;\
				break;\
			case DMC_NNL_DMA:\
				tick = 1;\
				break;\
		}\
		DMC.buffer = prgrom_rd(0, DMC.address);\
		if (!fds.info.enabled && info.mapper.extend_rd) {\
			DMC.buffer = extcl_cpu_rd_mem(0, DMC.address, DMC.buffer);\
		}\
		if (cfg->reverse_bits_dpcm) DMC.buffer = dmc_reverse_buffer_bits[DMC.buffer];\
		/* incremento gli hwtick da compiere */\
		if (hwtick) { hwtick[0] += tick; }\
		/* e naturalmente incremento anche quelli eseguiti dall'opcode */\
		nes[0].c.cpu.cycles += tick;\
		/* salvo a che ciclo dell'istruzione avviene il dma */\
		DMC.dma_cycle = nes[0].c.cpu.opcode_cycle;\
		/* il DMC non e' vuoto */\
		DMC.empty = FALSE;\
		if (++DMC.address > 0xFFFF) {\
			DMC.address = 0x8000;\
		}\
		if (!(--DMC.remain)) {\
			if (DMC.loop) {\
				DMC.remain = DMC.length;\
				DMC.address = DMC.address_start;\
			} else if (DMC.irq_enabled) {\
				r4015.value |= 0x80;\
				nes[0].c.irq.high |= DMC_IRQ;\
			}\
		}\
	}

#define apu_change_step(index)\
	apu.cycles += apuPeriod[apu.mode][apu.type][index]
#define r4017_jitter(apc)\
	r4017.value = (r4017.jitter.value & 0xC0);\
	r4017.reset_frame_delay = 1;\
	if (apu.cycles == apc) {\
		if (apu.mode == APU_48HZ) {\
			r4017.reset_frame_delay += 1;\
		} else {\
			r4017.reset_frame_delay += 2;\
		}\
	}\
	/*\
	 * se il bit 7 e' a zero, devo attivare la\
	 * modalita' NTSC, se a uno quella PAL.\
	 */\
	if (r4017.value & 0x80) {\
		apu.mode = APU_48HZ;\
	} else {\
		apu.mode = APU_60HZ;\
	}\
	if (r4017.value & 0x40) {\
		/* azzero il bit 6 del $4015 */\
		r4015.value &= 0xBF;\
		/* disabilito l'IRQ del frame counter */\
		nes[0].c.irq.high &= ~APU_IRQ;\
	}
#define r4017_reset_frame()\
	if (r4017.reset_frame_delay && (--r4017.reset_frame_delay == 0)) {\
		/* riavvio il frame audio */\
		apu.step = apu.cycles = 0;\
		apu_change_step(apu.step);\
	}
#define square_reg0(square)\
	/* duty */\
	square.duty = value >> 6;\
	/* length counter */\
	square.length.halt = value & 0x20;\
	/* envelope */\
	square.envelope.constant_volume = value & 0x10;\
	square.envelope.divider = value & 0x0F
#define square_reg1(square)\
	/* sweep */\
	square.sweep.reload = TRUE;\
	square.sweep.divider = (value >> 4) & 0x07;\
	square.sweep.shift = value & 0x07;\
	square.sweep.enabled = value & 0x80;\
	square.sweep.negate = value & 0x08
#define square_reg2(square)\
	/* timer (low 8 bits) */\
	square.timer = (square.timer & 0x0700) | value
#define square_reg3(square)\
	/* length counter */\
	/*\
	 * se non disabilitato, una scrittura in\
	 * questo registro, carica immediatamente il\
	 * length counter del canale, tranne nel caso\
	 * in cui la scrittura avvenga nello stesso\
	 * momento del clock di un length counter e\
	 * con il length diverso da zero.\
	 */\
	if (square.length.enabled && !(apu.length_clocked && square.length.value)) {\
		square.length.value = length_table[value >> 3];\
	}\
	/* envelope */\
	square.envelope.enabled = TRUE;\
	/* timer (high 3 bits) */\
	square.timer = (square.timer & 0x00FF) | ((value & 0x07) << 8);\
	/*The correct behaviour is to reset the duty cycle sequencers but not the clock dividers*/\
	/*square.frequency = 1;*/\
	/* sequencer */\
	square.sequencer = 0
#define init_nla_table(p, t)\
{\
	unsigned int i;\
	for (i = 0; i < LENGTH(nla_table.pulse); i++) {\
		double vl = 95.52 / (8128.0 / (double)i + 100.0);\
		nla_table.pulse[i] = (vl * p);\
	}\
	for (i = 0; i < LENGTH(nla_table.tnd); i++) {\
		double vl = 163.67 / (24329.0 / (double)i + 100.0);\
		nla_table.tnd[i] = (vl * t);\
	}\
}
#define _apu_channel_volume_adjust(ch, index)\
	((ch * cfg->apu.channel[index]) * ch_gain_ptnd(index))
#define s1_out\
	_apu_channel_volume_adjust(S1.output, APU_S1)
#define s2_out\
	_apu_channel_volume_adjust(S2.output, APU_S2)
#define tr_out\
	_apu_channel_volume_adjust(TR.output, APU_TR)
#define ns_out\
	_apu_channel_volume_adjust(NS.output, APU_NS)
#define dmc_out\
	_apu_channel_volume_adjust(DMC.output, APU_DMC)
#define extra_out(ch)\
	(ch * cfg->apu.channel[APU_EXTRA])
#define pulse_output()\
	nla_table.pulse[(int)(s1_out + s2_out)]
#define tnd_output()\
	nla_table.tnd[(int)((tr_out * 3) + (ns_out * 2) + dmc_out)]

typedef struct _config_apu {
	BYTE channel[APU_MASTER + 1];
	double volume[APU_MASTER + 1];
} _config_apu;
typedef struct _nla_table {
	SWORD pulse[32];
	SWORD tnd[203];
} _nla_table;
typedef struct _apu {
	BYTE mode;
	BYTE type;
	BYTE step;
	BYTE length_clocked;
	BYTE DMC;
	SWORD cycles;

	/* ------------------------------------------------------- */
	/* questi valori non e' necessario salvarli nei savestates */
	/* ------------------------------------------------------- */
	/* */ BYTE clocked;                                     /* */
	/* ------------------------------------------------------- */
} _apu;
typedef struct _r4011 {
	BYTE value;
	DBWORD frames;
	DBWORD cycles;
	SWORD output;
} _r4011;
typedef struct _r4015 {
	BYTE value;
} _r4015;
typedef struct _r4017 {
	BYTE value;
	struct _r4017_litter {
		BYTE value;
		BYTE delay;
	} jitter;
	BYTE reset_frame_delay;
} _r4017;
typedef struct _envelope {
	BYTE enabled;
	BYTE divider;
	BYTE counter;
	BYTE constant_volume;
	SBYTE delay;
} _envelope;
typedef struct _sweep {
	BYTE enabled;
	BYTE negate;
	BYTE divider;
	BYTE shift;
	BYTE reload;
	BYTE silence;
	SBYTE delay;
} _sweep;
typedef struct _length_counter {
	BYTE value;
	BYTE enabled;
	BYTE halt;
} _length_counter;
typedef struct _linear_counter {
	BYTE value;
	BYTE reload;
	BYTE halt;
} _linear_counter;
typedef struct _apuSquare {
	/* timer */
	DBWORD timer;
	/* ogni quanti cicli devo generare un output */
	WORD frequency;
	/* duty */
	BYTE duty;
	/* envelope */
	_envelope envelope;
	/* volume */
	BYTE volume;
	/* sequencer */
	BYTE sequencer;
	/* sweep */
	_sweep sweep;
	/* length counter */
	_length_counter length;
	/* output */
	SWORD output;
} _apuSquare;
typedef struct _apuTriangle {
	/* timer */
	DBWORD timer;
	/* ogni quanti cicli devo generare un output */
	WORD frequency;
	/* linear counter */
	_linear_counter linear;
	/* length counter */
	_length_counter length;
	/* sequencer */
	BYTE sequencer;
	/* output */
	SWORD output;
} _apuTriangle;
typedef struct _apuNoise {
	/* timer */
	DBWORD timer;
	/* ogni quanti cicli devo generare un output */
	WORD frequency;
	/* envelope */
	_envelope envelope;
	/* specifico del noise */
	BYTE mode;
	/* volume */
	BYTE volume;
	/* shift register */
	WORD shift;
	/* length counter */
	_length_counter length;
	/* sequencer */
	BYTE sequencer;
	/* output */
	SWORD output;
} _apuNoise;
typedef struct _apuDMC {
	/* ogni quanti cicli devo generare un output */
	WORD frequency;

	WORD remain;
	BYTE irq_enabled;
	BYTE loop;
	BYTE rate_index;
	WORD address_start;
	DBWORD address;
	WORD length;
	BYTE counter;
	BYTE empty;
	BYTE buffer;

	/* DMA */
	BYTE dma_cycle;

	/* output unit */
	BYTE silence;
	BYTE shift;
	BYTE counter_out;

	/* output */
	SWORD output;

	/* misc */
	BYTE tick_type;
} _apuDMC;

/* apuPeriod[mode][type][cycles] */
static const WORD apuPeriod[2][3][7] = {
	/*
	 * Mode 0: 4-step sequence
	 * Action      Envelopes &     Length Counter& Interrupt   Delay to next
	 *             Linear Counter  Sweep Units     Flag        NTSC     PAL   Dendy
	 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	 * $4017=$00   -               -               -           7459    8315    7459
	 * Step 1      Clock           -               -           7456    8314    7456
	 * Step 2      Clock           Clock           -           7458    8312    7458
	 * Step 3      Clock           -               -           7458    8314    7458
	 * Step 4      Clock           Clock       Set if enabled  7458    8314    7458
	 */
	{
		{7459, 7456, 7458, 7457, 1, 1, 7457},
		{8315, 8314, 8312, 8313, 1, 1, 8313},
		{7459, 7456, 7458, 7457, 1, 1, 7457}
	},
	/*
	 * Mode 1: 5-step sequence
	 * Action      Envelopes &     Length Counter& Interrupt   Delay to next
	 *             Linear Counter  Sweep Units     Flag        NTSC     PAL   Dendy
	 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	 * $4017=$80   -               -               -              1       1       1
	 * Step 1      Clock           Clock           -           7458    8314    7458
	 * Step 2      Clock           -               -           7456    8314    7456
	 * Step 3      Clock           Clock           -           7458    8312    7458
	 * Step 4      Clock           -               -           7458    8314    7458
	 * Step 5      -               -               -           7452    8312    7452
	 *
	 * Note:
	 * il 7452 e il 8312 dello step 5 diventano 7451 e 8311
	 * nella mia tabella perche' il ciclo mancante lo eseguo
	 * all'inizio del ciclo successivo.
	 */
	{
		{1, 7458, 7456, 7458, 7458, 7451, 0},
		{1, 8314, 8314, 8312, 8314, 8311, 0},
		{1, 7458, 7456, 7458, 7458, 7451, 0}
	}
};

/* la tabella con i valori da caricare nel length counter del canale */
static const BYTE length_table[32] = {
	0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06,
	0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E,
	0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16,
	0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E
};

static const BYTE square_duty[2][4][8] = {
	{
		{ 1,  0,  0,  0,  0,  0,  0,  0},
		{ 1,  1,  0,  0,  0,  0,  0,  0},
		{ 1,  1,  1,  1,  0,  0,  0,  0},
		{ 0,  0,  1,  1,  1,  1,  1,  1}
	},
	{
		{ 1,  0,  0,  0,  0,  0,  0,  0},
		{ 1,  1,  1,  1,  0,  0,  0,  0},
		{ 1,  1,  0,  0,  0,  0,  0,  0},
		{ 0,  0,  1,  1,  1,  1,  1,  1}
	},
};

static const BYTE triangle_duty[32] = {
	0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08,
	0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
};

static const WORD noise_timer[3][16] = {
	{
		0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0060, 0x0080, 0x00A0,
		0x00CA, 0x00FE, 0x017C, 0x01FC, 0x02FA, 0x03F8, 0x07F2, 0x0FE4
	},
	{
		0x0004, 0x0007, 0x000E, 0x001E, 0x003C, 0x0058, 0x0076, 0x0094,
		0x00BC, 0x00EC, 0x0162, 0x01D8, 0x02C4, 0x03B0, 0x0762, 0x0EC2
	},
	{
		0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0060, 0x0080, 0x00A0,
		0x00CA, 0x00FE, 0x017C, 0x01FC, 0x02FA, 0x03F8, 0x07F2, 0x0FE4
	}
};

static const WORD dmc_rate[3][16] = {
	{
		0x01AC, 0x017C, 0x0154, 0x0140, 0x011E, 0x00FE, 0x00E2, 0x00D6,
		0x00BE, 0x00A0, 0x008E, 0x0080, 0x006A, 0x0054, 0x0048, 0x0036
	},
	{
		0x018E, 0x0162, 0x013C, 0x012A, 0x0114, 0x00EC, 0x00D2, 0x00C6,
		0x00B0, 0x0094, 0x0084, 0x0076, 0x0062, 0x004E, 0x0042, 0x0032
	},
	{
		0x01AC, 0x017C, 0x0154, 0x0140, 0x011E, 0x00FE, 0x00E2, 0x00D6,
		0x00BE, 0x00A0, 0x008E, 0x0080, 0x006A, 0x0054, 0x0048, 0x0036
	}
};

static const BYTE dmc_reverse_buffer_bits[256] = {
	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF,
};

extern _nla_table nla_table;
extern _apu apu;
extern _r4011 r4011;
extern _r4015 r4015;
extern _r4017 r4017;
extern _apuSquare S1, S2;
extern _apuTriangle TR;
extern _apuNoise NS;
extern _apuDMC DMC;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void apu_tick(BYTE *hwtick);
EXTERNC void apu_turn_on(void);

#undef EXTERNC

#endif /* APU_H_ */

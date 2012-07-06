/*
 * apu.h
 *
 *  Created on: 06/set/2010
 *      Author: fhorse
 */

#ifndef APU_H_
#define APU_H_

#include "common.h"
#include "externalcalls.h"

/*
 * questa viene chiamata in ogni extclApuMixer chiamata dalle mappers
 * tranne che nel VRC7.
 */
#define apuMixerCutAndHigh()\
	/* taglio il risultato */\
	if (mixer > 255) {\
		mixer = 255;\
	} else if (mixer < -255) {\
		mixer = -255;\
	}\
	/* ne aumento il volume */\
	mixer <<= 7

/* length counter */
#define lengthRun(channel)\
	/*\
	 * se non e' settato il flag halt e il length\
	 * counter non e' 0 allora devo decrementarlo.\
	 */\
	if (!channel.lengthHalt && channel.length) {\
		channel.length--;\
	}
#define lengthClock()\
	apu.lengthClocked = TRUE;\
	lengthRun(S1)\
	lengthRun(S2)\
	lengthRun(TR)\
	lengthRun(NS)\
	if (extclLengthClock) {\
		/*\
		 * utilizzato dalle mappers :\
		 * MMC5\
		 */\
		extclLengthClock();\
	}
/* envelope */
#define envelopeRun(channel)\
	if (channel.envelope) {\
		channel.envelope = FALSE;\
		channel.envelopeCounter = 15;\
		channel.envelopeDelay = channel.envelopeDivider << 1;\
	} else if (--channel.envelopeDelay < 0) {\
		channel.envelopeDelay = channel.envelopeDivider << 1;\
		if (channel.envelopeCounter | channel.lengthHalt) {\
			channel.envelopeCounter = (channel.envelopeCounter - 1) & 0x0E;\
		}\
	}\
	/* setto il volume */\
	if (!channel.length) {\
		channel.volume = 0;\
	} else if (channel.constantVolume) {\
		channel.volume = channel.envelopeDivider;\
	} else {\
		channel.volume = channel.envelopeCounter;\
	}
#define envelopeClock()\
	envelopeRun(S1)\
	envelopeRun(S2)\
	envelopeRun(NS)\
	if (extclEnvelopeClock) {\
		/*\
		 * utilizzato dalle mappers :\
		 * MMC5\
		 */\
		extclEnvelopeClock();\
	}
/* sweep */
#define sweepRun(channel, negativeAdjust)\
	if (--channel.sweepDelay < 0) {\
		channel.sweepReload = TRUE;\
		if (channel.sweepShift && channel.sweepEnabled\
				&& (channel.timer >= 8)) {\
			DBWORD offset = channel.timer >> channel.sweepShift;\
			if (channel.sweepNegate) {\
				channel.timer += negativeAdjust - offset;\
			} else if ((channel.timer + offset) <= 0x800) {\
				channel.timer += offset;\
			}\
		}\
	}\
	if (channel.sweepReload) {\
		channel.sweepReload = FALSE;\
		channel.sweepDelay = channel.sweepDivider;\
	}
#define sweepClock()\
	sweepRun(S1, -1)\
	sweepRun(S2,  0)\
	if (extclSweepClock) {\
		/*\
		 * utilizzato dalle mappers :\
		 */\
		extclSweepClock();\
	}
/* linear counter */
#define linearClock()\
	if (TR.linearHalt) {\
		TR.linear = TR.linearReload;\
	} else if (TR.linear) {\
		TR.linear--;\
	}\
	if (!TR.lengthHalt) {\
		TR.linearHalt = FALSE;\
	}
/* output */
#define squareOutput(square)\
{\
	DBWORD offset = 0;\
	if (!square.sweepNegate) {\
		offset = square.timer >> square.sweepShift;\
	}\
	if (!square.volume || (square.timer <= 8)\
				|| ((square.timer + offset) >= 0x800)) {\
		square.output = 0;\
	} else {\
		square.output = squareDuty[square.duty][square.sequencer]\
				* square.volume;\
	}\
}
#define triangleOutput()\
	if (TR.length && TR.linear) {\
		/*\
		 * ai 2 cicli piu' bassi del timer, la frequenza\
		 * risultante e' troppo alta (oltre i 20 kHz,\
		 * quindi non udibile), percio' la taglio.\
		 */\
		if (TR.timer < 2) {\
			TR.output = 0;\
		} else {\
			/*TR.output = triangleDuty[TR.sequencer] << 1;*/\
			TR.output = triangleDuty[TR.sequencer];\
		}\
	}
#define noiseOutput()\
	if (NS.length && !(NS.shift & 0x0001)) {\
		NS.output = NS.volume;\
	} else {\
		NS.output = 0;\
	}
#define dmcOutput()\
	DMC.output = DMC.counter
/* ticket */
#define squareTick(square)\
	if (!(--square.frequency)) {\
		squareOutput(square)\
		square.frequency = (square.timer + 1) << 1;\
		square.sequencer = (square.sequencer + 1) & 0x07;\
	}
#define triangleTick()\
	if (!(--TR.frequency)) {\
		triangleOutput()\
		TR.frequency = TR.timer + 1;\
		TR.sequencer = (TR.sequencer + 1) & 0x1F;\
	}
#define noiseTick()\
	if (!(--NS.frequency)) {\
		NS.frequency = noiseTimer[apu.type][NS.timer];\
		if (NS.mode) {\
			NS.shift = (NS.shift >> 1) | (((NS.shift ^ (NS.shift >> 6)) & 0x0001) << 14);\
		} else {\
			NS.shift = (NS.shift >> 1) | (((NS.shift ^ (NS.shift >> 1)) & 0x0001) << 14);\
		}\
		NS.shift &= 0x7FFF;\
		noiseOutput()\
	}
#define dmcTick()\
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
			DMC.shift >>= 1;\
			dmcOutput();\
		}\
		if (!(--DMC.counterOut)) {\
			DMC.counterOut = 8;\
			if (!DMC.empty) {\
				DMC.shift = DMC.buffer;\
				DMC.empty = TRUE;\
				DMC.silence = FALSE;\
			} else {\
				DMC.silence = TRUE;\
			}\
		}\
		DMC.frequency = dmcRate[apu.type][DMC.rateIndex];\
	}\
	if (DMC.empty && DMC.remain) {\
		BYTE tick = 4;\
		switch (DMC.tickType) {\
		case DMCCPUWRITE:\
			tick = 3;\
			break;\
		case DMCR4014:\
			tick = 2;\
			break;\
		case DMCNNLDMA:\
			tick = 1;\
			break;\
		}\
		if (fds.info.enabled) {\
			if (DMC.address < 0xE000) {\
				DMC.buffer = prg.ram[DMC.address - 0x6000];\
			} else {\
				DMC.buffer = prg.rom[DMC.address & 0x1FFF];\
			}\
		} else {\
			DMC.buffer = prg.rom8k[(DMC.address >> 13) & 0x03][DMC.address & 0x1FFF];\
		}\
		/* incremento gli hwtick da compiere */\
		if (hwtick) { hwtick[0] += tick; }\
		/* e naturalmente incremento anche quelli eseguiti dall'opcode */\
		cpu.cycles += tick;\
		/* salvo a che ciclo dell'istruzione avviene il dma */\
		DMC.dmaCycle = cpu.opCycle;\
		/* il DMC non e' vuoto */\
		DMC.empty = FALSE;\
		if (++DMC.address > 0xFFFF) {\
			DMC.address = 0x8000;\
		}\
		if (!(--DMC.remain)) {\
			if (DMC.loop) {\
				DMC.remain = DMC.length;\
				DMC.address = DMC.addressStart;\
			} else if (DMC.irqEnabled) {\
				r4015.value |= 0x80;\
				irq.high |= DMCIRQ;\
			}\
		}\
	}

#define apuChangeStep(index)\
	apu.cycles += apuPeriod[apu.mode][apu.type][index]
#define r4017jitter()\
	r4017.value = (r4017.jitter & 0xC0);\
	/*\
	 * se il bit 7 e' a zero, devo attivare la\
	 * modalita' NTSC, se a uno quella PAL.\
	 */\
	if (r4017.value & 0x80) {\
		apu.mode = APU48HZ;\
	} else {\
		apu.mode = APU60HZ;\
	}\
	if (r4017.value & 0x40) {\
		/* azzero il bit 6 del $4015 */\
		r4015.value &= 0xBF;\
		/* disabilito l'IRQ del frame counter */\
		irq.high &= ~APUIRQ;\
	}\
	/* riavvio il frame audio */\
	apu.step = apu.cycles = 0;\
	apuChangeStep(apu.step)
#define squareReg0(square)\
	/* duty */\
	square.duty = value >> 6;\
	/* length counter */\
	square.lengthHalt = value & 0x20;\
	/* envelope */\
	square.constantVolume = value & 0x10;\
	square.envelopeDivider = value & 0x0F
#define squareReg1(square)\
	/* sweep */\
	square.sweepReload = TRUE;\
	square.sweepDivider = (value >> 4) & 0x07;\
	square.sweepShift = value & 0x07;\
	square.sweepEnabled = value & 0x80;\
	square.sweepNegate = value & 0x08
#define squareReg2(square)\
	/* timer (low 8 bits) */\
	square.timer = (square.timer & 0x0700) | value
#define squareReg3(square)\
	/* length counter */\
	/*\
	 * se non disabilitato, una scrittura in\
	 * questo registro, carica immediatamente il\
	 * length counter del canale, tranne nel caso\
	 * in cui la scrittura avvenga nello stesso\
	 * momento del clock di un length counter e\
	 * con il length diverso da zero.\
	 */\
	if (square.lengthEnabled && !(apu.lengthClocked && square.length)) {\
		/*square.length = lengthTable[(value & 0xF8) >> 3];*/\
		square.length = lengthTable[value >> 3];\
	}\
	/* envelope */\
	square.envelope = TRUE;\
	/* timer (high 3 bits) */\
	square.timer = (square.timer & 0x00FF) | ((value & 0x07) << 8);\
	/* sequencer */\
	square.sequencer = 0

typedef struct {
	BYTE mode;
	BYTE type;
	BYTE step;
	BYTE lengthClocked;
	BYTE DMC;
	SWORD cycles;
} _apu;
typedef struct {
	BYTE value;
	uint32_t frames;
} _r4xxx;
typedef struct {
	BYTE value;
	BYTE jitter;
	BYTE delay;
} _r4017;
typedef struct {
	/* timer */
	SDBWORD timer;
	/* ogni quanti cicli devo generare un output */
	WORD frequency;
	/* duty */
	BYTE duty;
	/* envelope */
	BYTE envelope;
	BYTE envelopeDivider;
	BYTE envelopeCounter;
	BYTE constantVolume;
	SBYTE envelopeDelay;
	/* volume */
	BYTE volume;
	/* sequencer */
	BYTE sequencer;
	/* sweep */
	BYTE sweepEnabled;
	BYTE sweepNegate;
	BYTE sweepDivider;
	BYTE sweepShift;
	BYTE sweepReload;
	SBYTE sweepDelay;
	/* length counter */
	BYTE length;
	BYTE lengthEnabled;
	BYTE lengthHalt;
	/* output */
	SWORD output;
} _apuSquare;
typedef struct {
	/* timer */
	SDBWORD timer;
	/* ogni quanti cicli devo generare un output */
	WORD frequency;
	/* linear counter */
	BYTE linear;
	BYTE linearReload;
	BYTE linearHalt;
	/* length counter */
	BYTE length;
	BYTE lengthEnabled;
	BYTE lengthHalt;
	/* sequencer */
	BYTE sequencer;
	/* output */
	SWORD output;
} _apuTriangle;
typedef struct {
	/* timer */
	SDBWORD timer;
	/* ogni quanti cicli devo generare un output */
	WORD frequency;
	/* envelope */
	BYTE envelope;
	BYTE envelopeDivider;
	BYTE envelopeCounter;
	BYTE constantVolume;
	SBYTE envelopeDelay;
	/* specifico del noise */
	BYTE mode;
	/* volume */
	BYTE volume;
	/* shift register */
	WORD shift;
	/* length counter */
	BYTE length;
	BYTE lengthEnabled;
	BYTE lengthHalt;
	/* sequencer */
	BYTE sequencer;
	/* output */
	SWORD output;
} _apuNoise;
typedef struct {
	/* ogni quanti cicli devo generare un output */
	WORD frequency;

	WORD remain;
	BYTE irqEnabled;
	BYTE loop;
	BYTE rateIndex;
	WORD addressStart;
	DBWORD address;
	WORD length;
	BYTE counter;
	BYTE empty;
	BYTE buffer;

	/* DMA */
	BYTE dmaCycle;

	/* output unit */
	BYTE silence;
	BYTE shift;
	BYTE counterOut;

	/* output */
	SWORD output;

	/* misc */
	BYTE tickType;
}  _apuDMC;

enum { DMCNORMAL, DMCCPUWRITE, DMCR4014, DMCNNLDMA };

_apu apu;
_r4xxx r4011, r4015;
_r4017 r4017;
_apuSquare S1, S2;
_apuTriangle TR;
_apuNoise NS;
_apuDMC DMC;

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

/* la tabella con i valori da caricare nel lenght counter del canale */
static const BYTE lengthTable[32] = {
	0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06,
	0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E,
	0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16,
	0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E
};

/*static const SWORD squareDuty[4][8] = {
	{-1, +1, -1, -1, -1, -1, -1, -1},
	{-1, +1, +1, -1, -1, -1, -1, -1},
	{-1, +1, +1, +1, +1, -1, -1, -1},
	{+1, -1, -1, +1, +1, +1, +1, +1}
};*/

static const SWORD squareDuty[4][8] = {
	{0, 1, 0, 0, 0, 0, 0, 0},
	{0, 1, 1, 0, 0, 0, 0, 0},
	{0, 1, 1, 1, 1, 0, 0, 0},
	{1, 0, 0, 1, 1, 1, 1, 1}
};

static const SWORD triangleDuty[32] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08,
	0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00
};

static const WORD noiseTimer[3][16] = {
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

static const WORD dmcRate[3][16] = {
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

void apuTick(SWORD cyclesCPU, BYTE *hwtick);
SWORD apuMixer(void);
void apuTurnON(void);


#endif /* APU_H_ */

/*
 * none.c
 *
 *  Created on: 11/lug/2012
 *      Author: fhorse
 */

#include "audio_filter.h"
#include "apu.h"
#include "mappers.h"
#include "mappers/mapperVRC7snd.h"
#include "fds.h"
#include "none.h"

SWORD (*extra_audio_mixer_none)(SWORD mixer);
SWORD audio_filters_none_FDS(SWORD mixer);
SWORD audio_filters_none_MMC5(SWORD mixer);
SWORD audio_filters_none_Namco_N163(SWORD mixer);
SWORD audio_filters_none_Sunsoft_FM7(SWORD mixer);
SWORD audio_filters_none_VRC6(SWORD mixer);
SWORD audio_filters_none_VRC7(SWORD mixer);

void audio_filter_init_none(void) {

	audio_filter_apu_tick = audio_filter_apu_tick_none;
	audio_filter_apu_mixer = audio_filter_apu_mixer_none;

	switch (info.mapper) {
		case FDS_MAPPER:
			/* FDS */
			extra_audio_mixer_none = audio_filters_none_FDS;
			break;
		case 5:
			/* MMC5 */
			extra_audio_mixer_none = audio_filters_none_MMC5;
			break;
		case 19:
			/* Namcot N163 */
			extra_audio_mixer_none = audio_filters_none_Namco_N163;
			break;
		case 69:
			/* Sunsoft FM7 */
			extra_audio_mixer_none = audio_filters_none_Sunsoft_FM7;
			break;
		case 24:
		case 26:
			/* VRC6 */
			extra_audio_mixer_none = audio_filters_none_VRC6;
			break;
		case 85:
			/* VRC7 */
			extra_audio_mixer_none = audio_filters_none_VRC7;
			break;
		default:
			extra_audio_mixer_none = NULL;
			break;
	}
	return;
}
void audio_filter_apu_tick_none(void) {
	return;
}
SWORD audio_filter_apu_mixer_none(void) {
	SWORD mixer = (S1.output + S2.output) + ((TR.output << 1) + NS.output + DMC.output);

	/* approsimazione lineare */
	/*SWORD pulse_out = 0.752 * (S1.output + S2.output);
	SWORD tnd_out = 0.851 * TR.output + 0.494 * NS.output + 0.335 * DMC.output;
	mixer = pulse_out + tnd_out;
	*/

	if (extra_audio_mixer_none) {
		mixer = extra_audio_mixer_none(mixer);
	} else {
		apuMixerCutAndHigh();
	}

	return (mixer);
}

/* --------------------------------------------------------------------------------------- */
/* Mappers                                                                                 */
/* --------------------------------------------------------------------------------------- */
SWORD audio_filters_none_FDS(SWORD mixer) {
	apuMixerCutAndHigh();

	return (mixer + (fds.snd.main.output + (fds.snd.main.output >> 1)));
}
SWORD audio_filters_none_MMC5(SWORD mixer) {
	mixer += (mmc5.S3.output + mmc5.S4.output) + mmc5.pcmSample;
	apuMixerCutAndHigh();
	return (mixer);
}
SWORD audio_filters_none_Namco_N163(SWORD mixer) {
	BYTE i;
	SWORD a = 0;

	for (i = n163.sndChStart; i < 8; i++) {
		if (n163.ch[i].active) {
			a += (n163.ch[i].output * (n163.ch[i].volume >> 2));
		}
	}

	mixer += a;

	apuMixerCutAndHigh();

	return (mixer);
}
SWORD audio_filters_none_Sunsoft_FM7(SWORD mixer) {
	mixer += (fm7.square[0].output + fm7.square[1].output + fm7.square[2].output);

	apuMixerCutAndHigh();

	return (mixer);
}
SWORD audio_filters_none_VRC6(SWORD mixer) {
	mixer += (vrc6.S3.output + vrc6.S4.output) + vrc6.saw.output;

	apuMixerCutAndHigh();

	return (mixer);
}
SWORD audio_filters_none_VRC7(SWORD mixer) {
	apuMixerCutAndHigh();

	return (mixer + (opll_calc() << 2));
}

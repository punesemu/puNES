/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

#include <string.h>
#include <pthread.h>
#include <alsa/asoundlib.h>
#include "info.h"
#include "snd.h"
#include "clock.h"
#include "conf.h"
#include "fps.h"
#include "audio/quality.h"
#include "audio/channels.h"
#include "gui.h"
#include "wave.h"

#define SND_LIST_DEVICES_V1

enum alsa_thread_loop_actions {
	AT_UNINITIALIZED,
	AT_RUN,
	AT_STOP,
	AT_PAUSE
};

typedef struct _alsa {
	snd_pcm_t *playback;
	snd_pcm_sframes_t (*snd_writei) (snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size);

	snd_pcm_t *capture;
	snd_pcm_sframes_t (*snd_readi) (snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size);

	snd_pcm_uframes_t bsize;
	snd_pcm_uframes_t psize;
} _alsa;
typedef struct _thread {
	pthread_t thread;
	pthread_mutex_t lock;

	BYTE action;
	BYTE in_run;

	void *cache;
	_alsa *alsa;

#if !defined (RELEASE)
	double tick;
#endif
} _thread;

static int alsa_find_index_id(_snd_list_dev *list, uTCHAR *id, int size);
static void alsa_enum_cards(_snd_list_dev *list, snd_pcm_stream_t stream);
static void alsa_device_add(_snd_list_dev *list, uTCHAR *id, uTCHAR *desc);
static void alsa_list_devices_quit(void);
static void alsa_list_devices_free(_snd_list_dev *list);
#if defined (REALLYDEBUG)
static void alsa_hwparams_print(snd_pcm_hw_params_t *hwp);
#endif

static BYTE alsa_playback_hwparams_set(void);
static BYTE alsa_playback_swparams_set(void);
static void *alsa_playback_loop(void *data);
static void alsa_playback_loop_in_pause(void);
static BYTE INLINE alsa_xrun_recovery(snd_pcm_t *handle, int err);
static void INLINE alsa_wr_buf(_alsa *handle, void *buffer, snd_pcm_sframes_t avail);

static _thread loop;
static _alsa alsa;
static _callback_data cbd;

BYTE snd_init(void) {
	memset(&snd, 0x00, sizeof(_snd));
	memset(&alsa, 0x00, sizeof(_alsa));
	memset(&cbd, 0x00, sizeof(_callback_data));
	memset(&loop, 0x00, sizeof(_thread));

	snd_apu_tick = NULL;
	snd_end_frame = NULL;

	snd_list_devices();

	// apro e avvio la riproduzione
	if (snd_playback_start()) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
void snd_quit(void) {
	// se e' in corso una registrazione, la concludo
	wave_close();

	if (loop.action != AT_UNINITIALIZED) {
		loop.action = AT_STOP;

		pthread_join(loop.thread, NULL);
		pthread_mutex_destroy(&loop.lock);
		memset(&loop, 0x00, sizeof(_thread));
	}

	snd_playback_stop();

	alsa_list_devices_quit();

#if !defined (RELEASE)
	fprintf(stderr, "\n");
#endif
}

BYTE snd_playback_start(void) {
	int rc;

	if (!cfg->apu.channel[APU_MASTER]) {
		return (EXIT_OK);
	}

	// se il thread del loop e' gia' in funzione lo metto in pausa
	if (loop.action == AT_RUN) {
		alsa_playback_loop_in_pause();
		snd_playback_lock(NULL);
	}

	// come prima cosa blocco eventuali riproduzioni
	snd_playback_stop();

	memset(&snd, 0x00, sizeof(_snd));
	memset(&alsa, 0x00, sizeof(_alsa));
	memset(&cbd, 0x00, sizeof(_callback_data));
	snd.cache = &cbd;

	audio_channels(cfg->channels_mode);

	switch (cfg->samplerate) {
		case S48000:
			snd.samplerate = 48000;
			break;
		case S44100:
			snd.samplerate = 44100;
			break;
		case S22050:
			snd.samplerate = 22050;
			break;
		case S11025:
			snd.samplerate = 11025;
			break;
	}

	{
		static int factor[10] = { 90, 80, 70, 60, 50, 40, 30, 20, 10, 5 };
		_snd_dev *dev = &snd_list.playback.devices[alsa_find_index_id(&snd_list.playback,
				cfg->audio_output, usizeof(cfg->audio_output))];
		int tries;

		// snd.samplarate / 50 = 20 ms
		alsa.psize = (snd.samplerate / factor[cfg->audio_buffer_factor]);

		rc = snd_pcm_open(&alsa.playback, (uTCHAR *) dev->id, SND_PCM_STREAM_PLAYBACK, 0);

		for (tries = 0; (tries < 100) && (rc == -EBUSY); ++tries) {
			gui_sleep(100);
			rc = snd_pcm_open(&alsa.playback, (uTCHAR *) dev->id, SND_PCM_STREAM_PLAYBACK, 0);
		}

		if (rc < 0) {
			fprintf(stderr, "Playback open error: %s\n", snd_strerror(rc));
			goto snd_playback_start_error;
		}

		if (alsa_playback_hwparams_set() != EXIT_OK) {
			goto snd_playback_start_error;
		}

		if (alsa_playback_swparams_set() != EXIT_OK) {
			goto snd_playback_start_error;
		}
	}

	snd.samples = alsa.bsize * 2;
	snd.frequency = machine.cpu_hz / (double) snd.samplerate;

	{
		// dimensione in bytes del buffer software
		snd.buffer.size = (alsa.bsize * snd.channels * sizeof(*cbd.write)) * 10;

		snd.buffer.limit.low = (snd.buffer.size / 100) * 25;
		snd.buffer.limit.high = (snd.buffer.size / 100) * 55;

#if !defined (RELEASE)
		printf("softw bsize    : %10d - %10d\n", snd.buffer.size, snd.samples);
		printf("softw limit    : %10d - %10d\n", snd.buffer.limit.high, snd.buffer.limit.low);
#endif

		// alloco il buffer in memoria
		if (!(cbd.start = (SWORD *) malloc(snd.buffer.size))) {
			fprintf(stderr, "Unable to allocate audio buffers\n");
			goto snd_playback_start_error;
		}

		if (!(cbd.silence = (SWORD *) malloc(snd.buffer.size))) {
			fprintf(stderr, "Unable to allocate silence buffer\n");
			goto snd_playback_start_error;
		}

		// inizializzo il frame di scrittura
		cbd.write = cbd.start;
		// inizializzo il frame di lettura
		cbd.read = (SBYTE *) cbd.start;
		// punto alla fine del buffer
		cbd.end = cbd.read + snd.buffer.size;
		// creo il lock
		if (pthread_mutex_init(&loop.lock, NULL) != 0) {
			fprintf(stderr, "Unable to allocate the mutex\n");
			goto snd_playback_start_error;
		}
		// azzero completamente i buffers
		memset(cbd.start, 0x00, snd.buffer.size);
		// azzero completamente il buffer del silenzio
		memset(cbd.silence, 0x00, snd.buffer.size);

		cbd.lock = (void *) &loop.lock;
	}

	if (extcl_snd_playback_start) {
		extcl_snd_playback_start((WORD) snd.samplerate);
	}

	audio_channels_init_mode();

	audio_quality(cfg->audio_quality);

	if (loop.action == AT_UNINITIALIZED) {
		loop.alsa = &alsa;
		loop.action = AT_PAUSE;

		// creo il lock
		if (pthread_mutex_init(&loop.lock, NULL) != 0) {
			fprintf(stderr, "Unable to allocate the thread mutex\n");
			goto snd_playback_start_error;
		}

		snd_playback_lock(NULL);

		if ((rc = pthread_create(&loop.thread, NULL, alsa_playback_loop, (void *) &loop))) {
			fprintf(stderr, "Error - pthread_create() return code: %d\n", rc);
			goto snd_playback_start_error;;
		}
	}

	loop.cache = &cbd;

	if ((rc = snd_pcm_prepare(alsa.playback)) < 0) {
		fprintf(stderr, "cannot prepare audio interface for use (%s)\n", snd_strerror(rc));
		goto snd_playback_start_error;
	}

	snd_playback_unlock(NULL);
	gui_sleep(50);

	loop.action = AT_RUN;

	return (EXIT_OK);

	snd_playback_start_error:
	snd_playback_stop();
	return (EXIT_ERROR);
}
void snd_playback_stop(void) {
	alsa_playback_loop_in_pause();

	if (snd.cache) {
		if (SNDCACHE->start) {
			free(SNDCACHE->start);
		}

		if (SNDCACHE->silence) {
			free(SNDCACHE->silence);
		}

		snd.cache = NULL;
	}

	if (audio_channels_quit) {
		audio_channels_quit();
	}

	if (audio_quality_quit) {
		audio_quality_quit();
	}

	if (alsa.playback) {
		snd_pcm_close(alsa.playback);
		alsa.playback = NULL;
	}
}
void snd_playback_lock(_callback_data *cache) {
	pthread_mutex_lock(&loop.lock);
}
void snd_playback_unlock(_callback_data *cache) {
	pthread_mutex_unlock(&loop.lock);
}
uTCHAR *snd_playback_device_desc(int dev) {
	if (dev >= snd_list.playback.count) {
		return (NULL);
	}
	return (snd_list.playback.devices[dev].desc);
}
uTCHAR *snd_playback_device_id(int dev) {
	if (dev >= snd_list.playback.count) {
		return (NULL);
	}
	return ((uTCHAR *) snd_list.playback.devices[dev].id);
}

uTCHAR *snd_capture_device_desc(int dev) {
	if (dev >= snd_list.capture.count) {
		return (NULL);
	}
	return (snd_list.capture.devices[dev].desc);
}
uTCHAR *snd_capture_device_id(int dev) {
	if (dev >= snd_list.capture.count) {
		return (NULL);
	}
	return ((uTCHAR *) snd_list.capture.devices[dev].id);
}

#if !defined (SND_LIST_DEVICES_V2)
void snd_list_devices(void) {
	alsa_list_devices_quit();

	// Playback devices
	alsa_device_add(&snd_list.playback, uL("default"), uL("System Default"));
	alsa_enum_cards(&snd_list.playback, SND_PCM_STREAM_PLAYBACK);

	// Capture devices
	alsa_device_add(&snd_list.capture, uL("default"), uL("System Default"));
	alsa_enum_cards(&snd_list.capture, SND_PCM_STREAM_CAPTURE);

#if defined (DEBUG)
	int i;

	printf("PLAYBACK devices\n");
	for (i = 0; i < snd_list.playback.count; i++) {
		_snd_dev *dev = &snd_list.playback.devices[i];

		printf(uL("  %3d : " uPERCENTs "\n"), i, (uTCHAR *) dev->id);
	}

	printf("CAPTURE devices\n");
	for (i = 0; i < snd_list.capture.count; i++) {
		_snd_dev *dev = &snd_list.capture.devices[i];

		printf(uL("  %3d : " uPERCENTs "\n"), i, (uTCHAR *) dev->id);
	}
#endif
}
#else
void snd_list_devices(void) {
	void **pcm_hints;
	int i;

	alsa_list_devices_quit();

	// Playback devices
	alsa_device_add(&snd_list.playback, uL("default"), uL("System Default"));

	if (snd_device_name_hint(-1, "pcm", &pcm_hints) < 0) {
		return;
	}

	{
		uTCHAR buf[100];
		void **pcm_str = pcm_hints;
		static const uTCHAR *exclude[] = {
			uL("null"),     uL("pulse"),
			uL("default"),  uL("sysdefault"),
			uL("surround")
		};

		for (; (*pcm_str); pcm_str++) {
			uTCHAR *pcm_hint_name, *pcm_hint_name_strip;
			uTCHAR *pcm_hint_ioid;
			uTCHAR *pcm_hint_desc, *end;
			BYTE is_good = TRUE;

			if ((pcm_hint_name = snd_device_name_get_hint((*pcm_str), "NAME")) == NULL) {
				continue;
			}
			if ((pcm_hint_ioid = snd_device_name_get_hint((*pcm_str), "IOID")) &&
				(strcmp(pcm_hint_ioid, "Playback") != 0) &&
				(strcmp(pcm_hint_ioid, "Output") != 0)) {
				free(pcm_hint_name);
				free(pcm_hint_ioid);
				continue;
			}

			free(pcm_hint_ioid);

			ustrncpy(buf, pcm_hint_name, usizeof(buf));

			// examples : front:CARD=PCH,DEV=0
			if ((pcm_hint_name_strip = ustrchr(buf, ':'))) {
				(*pcm_hint_name_strip) = 0;
			}

			for (i = 0; i < LENGTH(exclude); i++) {
				if (ustrncmp(exclude[i], buf, ustrlen(exclude[i])) == 0) {
					is_good = FALSE;
					break;
				}
			}

			if (is_good == FALSE) {
				free(pcm_hint_name);
				continue;
			}

			umemset(buf, 0, usizeof(buf));

			if ((pcm_hint_desc = snd_device_name_get_hint((*pcm_str), "DESC"))) {
				if ((end = ustrchr(pcm_hint_desc, '\n'))) {
					(*end) = '\0';
				}
				ustrncpy(buf, pcm_hint_desc, usizeof(buf));
				free(pcm_hint_desc);
			} else {
				ustrncpy(buf, pcm_hint_name, usizeof(buf));
			}

			alsa_device_add(&snd_list.playback, pcm_hint_name, buf);

		    free(pcm_hint_name);
        }
		snd_device_name_free_hint(pcm_hints);
	}

	// Capture devices
	alsa_device_add(&snd_list.capture, uL("default"), uL("System Default"));
	alsa_enum_cards(&snd_list.capture, SND_PCM_STREAM_CAPTURE);

#if defined (DEBUG)
	printf("PLAYBACK devices\n");
	for (i = 0; i < snd_list.playback.count; i++) {
		_snd_dev *dev = &snd_list.playback.devices[i];

		printf(uL("  %3d : " uPERCENTs "\n"), i, dev->id);
	}

	printf("CAPTURE devices\n");
	for (i = 0; i < snd_list.capture.count; i++) {
		_snd_dev *dev = &snd_list.capture.devices[i];

		printf(uL("  %3d : " uPERCENTs "\n"), i, dev->id);
	}
#endif
}
#endif

static int alsa_find_index_id(_snd_list_dev *list, uTCHAR *id, int size) {
	int i, index = -1;

	for (i = 0; i < list->count; i++) {
		if (ustrcmp(id, (uTCHAR *) list->devices[i].id) == 0) {
			index = i;
			break;
		}
	}

	if (index == -1) {
		index = 0;
		ustrncpy(id, uL("default"), size);
	}

	return (index);
}
static void alsa_enum_cards(_snd_list_dev *list, snd_pcm_stream_t stream) {
	int a = -1, b = -1;
	uTCHAR buf[100];

	while (!snd_card_next(&a) && (a != -1)) {
		uTCHAR ctl_id[20];
		snd_ctl_card_info_t *cinfo;
		snd_pcm_info_t *pinfo;
		snd_ctl_t *ctl;
		uTCHAR *card_name;

		snd_card_get_name(a, &card_name);

		usnprintf(ctl_id, usizeof(ctl_id), "hw:%d", a);
		snd_ctl_open(&ctl, ctl_id, 0);
		snd_ctl_card_info_alloca(&cinfo);
		snd_ctl_card_info(ctl, cinfo);

		snd_pcm_info_alloca(&pinfo);

		while (!snd_ctl_pcm_next_device(ctl, &b) && (b >= 0)) {
			uTCHAR id[20];

			usnprintf(id, sizeof(id), "plughw:%d,%d", a, b);

			snd_pcm_info_set_device(pinfo, b);
			snd_pcm_info_set_subdevice(pinfo, 0);

			snd_pcm_info_set_stream(pinfo, stream);

			if (snd_ctl_pcm_info(ctl, pinfo) < 0) {
				continue;
			}

			usnprintf(buf, usizeof(buf), uL("" uPERCENTs " : " uPERCENTs), card_name, id);
			alsa_device_add(list, id, buf);
		}

		snd_ctl_close(ctl);
		free(card_name);
	}

}
static void alsa_device_add(_snd_list_dev *list, uTCHAR *id, uTCHAR *desc) {
	_snd_dev *dev, *devs;

	if ((devs = (_snd_dev *) realloc(list->devices, (list->count + 1) * sizeof(_snd_dev)))) {
		list->devices = devs;
		dev = &list->devices[list->count];
		dev->id = ustrdup(id);
		dev->desc = ustrdup(desc);
		list->count++;
	}
}
static void alsa_list_devices_quit(void) {
	alsa_list_devices_free(&snd_list.playback);
	alsa_list_devices_free(&snd_list.capture);
}
static void alsa_list_devices_free(_snd_list_dev *list) {
	if (list->devices) {
		int i;

		for (i = 0; i < list->count; i++) {
			_snd_dev *dev = &list->devices[i];

			if (dev->id) {
				free(dev->id);
			}
			if (dev->desc) {
				free(dev->desc);
			}
		}
		free(list->devices);
	}
	list->count = 0;
	list->devices = NULL;
}
#if defined (REALLYDEBUG)
static void alsa_hwparams_print(snd_pcm_hw_params_t *hwp) {
	snd_pcm_uframes_t ufmin, ufmax;
	unsigned int min, max;
	int dir;

	snd_pcm_hw_params_get_channels_min(hwp, &min);
	snd_pcm_hw_params_get_channels_max(hwp, &max);
	printf("channels       : %10d - %10d\n", min, max);

	snd_pcm_hw_params_get_rate_min(hwp, &min, &dir);
	snd_pcm_hw_params_get_rate_max(hwp, &max, &dir);
	printf("rate           : %10d - %10d\n", min, max);

	snd_pcm_hw_params_get_rate_resample(alsa.playback, hwp, &min);
	printf("resample       : %10d\n", min);

	snd_pcm_hw_params_get_export_buffer(alsa.playback, hwp, &min);
	printf("export buffer  : %10d\n", min);

	snd_pcm_hw_params_get_period_wakeup(alsa.playback, hwp, &min);
	printf("period wakeup  : %10d\n", min);

	snd_pcm_hw_params_get_period_time_min(hwp, &min, &dir);
	snd_pcm_hw_params_get_period_time_max(hwp, &max, &dir);
	printf("period time    : %10d - %10d\n", min, max);

	snd_pcm_hw_params_get_period_size_min(hwp, &ufmin, &dir);
	snd_pcm_hw_params_get_period_size_max(hwp, &ufmax, &dir);
	printf("period size    : %10ld - %10ld\n", ufmin, ufmax);

	snd_pcm_hw_params_get_periods_min(hwp, &min, &dir);
	snd_pcm_hw_params_get_periods_max(hwp, &max, &dir);
	printf("period         : %10d - %10d\n", min, max);

	snd_pcm_hw_params_get_buffer_time_min(hwp, &min, &dir);
	snd_pcm_hw_params_get_buffer_time_max(hwp, &max, &dir);
	printf("buffer time    : %10d - %10d\n", min, max);

	snd_pcm_hw_params_get_buffer_size_min(hwp, &ufmin);
	snd_pcm_hw_params_get_buffer_size_max(hwp, &ufmax);
	printf("buffer size    : %10ld - %10ld\n", ufmin, ufmax);

	snd_pcm_hw_params_get_min_align(hwp, &ufmin);
	printf("align          : %10ld\n", ufmin);
}
#endif

static BYTE alsa_playback_hwparams_set(void) {
	snd_pcm_hw_params_t *params = NULL;
	unsigned int rrate;
	int rc;

	snd_pcm_hw_params_alloca(&params);

	// choose all parameters
	if ((rc = snd_pcm_hw_params_any(alsa.playback, params)) < 0) {
		fprintf(stderr, "Broken configuration for playback: no configurations available: %s\n",
				snd_strerror(rc));
		return (EXIT_ERROR);
	}

#if defined (REALLYDEBUG)
	printf("-- BEFORE --\n");
	alsa_hwparams_print(params);
	printf("\n");
#endif

	// anable hardware resampling
	if ((rc = snd_pcm_hw_params_set_rate_resample(alsa.playback, params, 1)) < 0) {
		fprintf(stderr, "Resampling setup failed for playback: %s\n", snd_strerror(rc));
		return (EXIT_ERROR);
	}

	// set the [mmap]/[rw interleaved read/write] format
	alsa.snd_writei = snd_pcm_mmap_writei;

	if ((rc = snd_pcm_hw_params_set_access(alsa.playback, params, SND_PCM_ACCESS_MMAP_INTERLEAVED))
			< 0) {
		if ((rc = snd_pcm_hw_params_set_access(alsa.playback, params, SND_PCM_ACCESS_RW_INTERLEAVED))
		        < 0) {
			fprintf(stderr, "Access type not available for playback: %s\n", snd_strerror(rc));
			return (EXIT_ERROR);
		}
		alsa.snd_writei = snd_pcm_writei;
	}

	// set the sample format
	if ((rc = snd_pcm_hw_params_set_format(alsa.playback, params, SND_PCM_FORMAT_S16)) < 0) {
		fprintf(stderr, "Sample format not available for playback: %s\n", snd_strerror(rc));
		return (EXIT_ERROR);
	}

	// set the channels
	if ((rc = snd_pcm_hw_params_set_channels(alsa.playback, params, snd.channels)) < 0) {
		fprintf(stderr, "Channels count (%i) not available for playbacks: %s\n", snd.channels,
				snd_strerror(rc));
		return (EXIT_ERROR);
	}

	// set the stream rate
	rrate = snd.samplerate;

	if ((rc = snd_pcm_hw_params_set_rate_near(alsa.playback, params, &rrate, 0)) < 0) {
		fprintf(stderr, "Rate %iHz not available for playback: %s\n", snd.samplerate,
				snd_strerror(rc));
		return (EXIT_ERROR);
	}
	if (rrate != snd.samplerate) {
		fprintf(stderr, "Rate doesn't match (requested %iHz, get %iHz)\n", snd.samplerate, rrate);
		return (EXIT_ERROR);
	}

	// set the period size
	if ((rc = snd_pcm_hw_params_set_period_size_near(alsa.playback, params, &alsa.psize, 0)) < 0) {
		fprintf(stderr, "Unable to set period size for playback: %s\n", snd_strerror(rc));
		return (EXIT_ERROR);
	}

	// set the buffer size
	alsa.bsize = alsa.psize * 3;

	if ((rc = snd_pcm_hw_params_set_buffer_size_near(alsa.playback, params, &alsa.bsize)) < 0) {
		fprintf(stderr, "Unable to set buffer size for playback: %s\n", snd_strerror(rc));
		return (EXIT_ERROR);
	}

#if defined (REALLYDEBUG)
	printf("-- AFTER --\n");
	alsa_hwparams_print(params);
	printf("\n");
#endif

	// write the parameters to device
	if ((rc = snd_pcm_hw_params(alsa.playback, params)) < 0) {
		fprintf(stderr, "Unable to set hw params for playback: %s\n", snd_strerror(rc));
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
static BYTE alsa_playback_swparams_set(void) {
	snd_pcm_sw_params_t *params = NULL;
	int rc;

	snd_pcm_sw_params_alloca(&params);

	// get the current swparams
	if ((rc = snd_pcm_sw_params_current(alsa.playback, params)) < 0) {
		fprintf(stderr, "Unable to determine current swparams for playback: %s\n", snd_strerror(rc));
		return (EXIT_ERROR);
	}

	// start the transfer when the buffer is almost full
	if ((rc = snd_pcm_sw_params_set_start_threshold(alsa.playback, params, alsa.psize)) < 0) {
		fprintf(stderr, "Unable to set start threshold mode for playback: %s\n", snd_strerror(rc));
		return (EXIT_ERROR);
	}

	// allow the transfer when at least period_size samples can be processed
	if ((rc = snd_pcm_sw_params_set_avail_min(alsa.playback, params, alsa.psize)) < 0) {
		fprintf(stderr, "Unable to set avail min for playback: %s\n", snd_strerror(rc));
		return (EXIT_ERROR);
	}

	// write the parameters to the playback device
	if ((rc = snd_pcm_sw_params(alsa.playback, params)) < 0) {
		fprintf(stderr, "Unable to set sw params for playback: %s\n", snd_strerror(rc));
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}
static void *alsa_playback_loop(void *data) {
	_thread *th = (_thread *) data;
	snd_pcm_sframes_t avail;
	uint32_t len;
	int rc;

#if !defined (RELEASE)
	th->tick = gui_get_ms();
#endif

	while (TRUE) {
		_callback_data *cache = (_callback_data *) th->cache;

		if (th->action == AT_STOP) {
			th->in_run = FALSE;
			break;
		} else if (th->action == AT_PAUSE) {
			th->in_run = FALSE;
			gui_sleep(10);
			continue;
		}

		th->in_run = TRUE;

		if ((rc = snd_pcm_wait(th->alsa->playback , 100)) < 0) {
			fprintf(stderr, "snd_pcm_wait() failed (%s)\n", snd_strerror(rc));
			alsa_xrun_recovery(th->alsa->playback, rc);
			continue;
		}

		// controllo quanti frames alsa sono richiesti
		if ((avail = snd_pcm_avail_update(th->alsa->playback)) < 0) {
			fprintf(stderr, "snd_pcm_avail_update() failed (%s)\n", snd_strerror(rc));
			alsa_xrun_recovery(th->alsa->playback, avail);
			continue;
		}

		snd_playback_lock(NULL);

		avail = (avail > alsa.psize ? alsa.psize : avail);
		len = avail * snd.channels * sizeof(*cache->write);

		if ((info.no_rom | info.turn_off | info.pause) || (snd.buffer.start == FALSE)
				|| (fps.fast_forward == TRUE)) {
			alsa_wr_buf(th->alsa, (void *) cache->silence, avail);
		} else if (cache->bytes_available < len) {
			wave_write(cache->silence, avail);
			alsa_wr_buf(th->alsa, (void *) cache->silence, avail);
			snd.out_of_sync++;
		} else {
			wave_write((SWORD *) cache->read, avail);
			alsa_wr_buf(th->alsa, (void *) cache->read, avail);

			cache->bytes_available -= len;
			cache->samples_available -= avail;

			if ((cache->read += len) >= cache->end) {
				cache->read = (SBYTE *) cache->start;
			}
		}

#if !defined (RELEASE)
		snd_pcm_sframes_t request = avail;

		if ((gui_get_ms() - th->tick) >= 250.0f) {
			th->tick = gui_get_ms();
			//if (info.snd_info == TRUE)
			fprintf(stderr, "snd : %6ld %6ld %d %6d %d %6d %6d %f %3d %f %4s\r",
				request,
				avail,
				len,
				fps.total_frames_skipped,
				cache->samples_available,
				cache->bytes_available,
				snd.out_of_sync,
				snd.frequency,
				(int) framerate.value,
				machine.ms_frame,
				" ");
		}
#endif

		snd_playback_unlock(NULL);
 	}

	pthread_exit(EXIT_OK);
}
static void alsa_playback_loop_in_pause(void) {
	if (loop.action != AT_UNINITIALIZED) {
		loop.action = AT_PAUSE;

		while (loop.in_run == TRUE) {
			gui_sleep(10);
		}
	}
}
static BYTE INLINE alsa_xrun_recovery(snd_pcm_t *handle, int err) {
	if (err == -EPIPE) {
		err = snd_pcm_prepare(handle);
		if (err < 0) {
			fprintf(stderr, "can't recovery from underrun, prepare failed: %s\n",
					snd_strerror(err));
			info.stop = TRUE;
			return (EXIT_ERROR);
		}
	} else if (err == -ESTRPIPE) {
		while ((err = snd_pcm_resume(handle)) == -EAGAIN) {
			sleep(1);
		}
		if (err < 0) {
			err = snd_pcm_prepare(handle);
			if (err < 0) {
				fprintf(stderr, "can't recovery from suspend, prepare failed: %s\n",
						snd_strerror(err));
				info.stop = TRUE;
				return (EXIT_ERROR);
			}
		}
	}
	return (EXIT_OK);
}
static void INLINE alsa_wr_buf(_alsa *alsa, void *buffer, snd_pcm_sframes_t avail) {
	int rc;

	if ((rc = alsa->snd_writei(alsa->playback, buffer, avail)) < 0) {
		fprintf(stderr, "snd_pcm_xxxx_writei failed (%s)\n", snd_strerror(rc));
	}
}

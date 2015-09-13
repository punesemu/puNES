/*
 * snd.c
 *
 *  Created on: 29/lug/2014
 *      Author: fhorse
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

enum alsa_thread_loop_actions {
	AT_UNINITIALIZED,
	AT_RUN,
	AT_STOP,
	AT_PAUSE
};

static BYTE set_hwparams(void);
static BYTE set_swparams(void);
void *alsa_loop_thread(void *data);
void alsa_loop_in_pause(void);
static int INLINE xrun_recovery(snd_pcm_t *handle, int err);
static void INLINE wrbuf(snd_pcm_t *handle, void *buffer, snd_pcm_sframes_t avail);

typedef struct {
	snd_pcm_t *handle;

	snd_pcm_uframes_t bsize;
	snd_pcm_uframes_t psize;
} _alsa;
typedef struct {
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

static _thread loop;
static _alsa alsa;

BYTE snd_init(void) {
	memset(&snd, 0x00, sizeof(_snd));
	memset(&alsa, 0x00, sizeof(_alsa));
	memset(&loop, 0x00, sizeof(_thread));

	snd_apu_tick = NULL;
	snd_end_frame = NULL;

	// apro e avvio la riproduzione
	if (snd_start()) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
BYTE snd_start(void) {
	_callback_data *cache;
	int rc;

	if (!cfg->apu.channel[APU_MASTER]) {
		return (EXIT_OK);
	}

	// se il thread del loop e' gia' in funzione lo metto in pausa
	if (loop.action == AT_RUN) {
		alsa_loop_in_pause();
		snd_lock_cache(NULL);
	}

	// come prima cosa blocco eventuali riproduzioni
	snd_stop();

	memset(&snd, 0x00, sizeof(_snd));
	memset(&alsa, 0x00, sizeof(_alsa));

	cache = (_callback_data *) malloc(sizeof(_callback_data));
	memset(cache, 0x00, sizeof(_callback_data));
	snd.cache = cache;

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
		double factor = (1.0f / 48000.0f) * (double) snd.samplerate;

		// buffer hardware
		alsa.bsize = ((1024 * factor) + ((512 * factor) * cfg->audio_buffer_factor));
		alsa.psize = alsa.bsize / snd.channels;
		//alsa.psize = alsa.bsize / 3;
	}

	snd.samples = alsa.bsize * 2;
	snd.frequency = machine.cpu_hz / (double) snd.samplerate;

	{
		// dimensione in bytes del buffer software
		snd.buffer.size = (alsa.bsize * snd.channels * sizeof(*cache->write)) * 5;

		snd.buffer.limit.low = (snd.buffer.size / 100) * 25;
		snd.buffer.limit.high = (snd.buffer.size / 100) * 55;

#if !defined (RELEASE)
		printf("softw bsize : %-6d - %-6d\n", snd.buffer.size, snd.samples);
		printf("softw limit : %-6d - %-6d\n", snd.buffer.limit.low, snd.buffer.limit.high);
#endif

		// alloco il buffer in memoria
		if (!(cache->start = (SWORD *) malloc(snd.buffer.size))) {
			fprintf(stderr, "Unable to allocate audio buffers\n");
			goto snd_start_error;
		}

		if (!(cache->silence = (SWORD *) malloc(snd.buffer.size))) {
			fprintf(stderr, "Unable to allocate silence buffer\n");
			goto snd_start_error;
		}

		// inizializzo il frame di scrittura
		cache->write = cache->start;
		// inizializzo il frame di lettura
		cache->read = (SBYTE *) cache->start;
		// punto alla fine del buffer
		cache->end = cache->read + snd.buffer.size;
		// creo il lock
		if (pthread_mutex_init(&loop.lock, NULL) != 0) {
			fprintf(stderr, "Unable to allocate the mutex\n");
			goto snd_start_error;
		}
		// azzero completamente i buffers
		memset(cache->start, 0x00, snd.buffer.size);
		// azzero completamente il buffer del silenzio
		memset(cache->silence, 0x00, snd.buffer.size);

		cache->lock = (void *) &loop.lock;
	}

	if (extcl_snd_start) {
		extcl_snd_start((WORD) snd.samplerate);
	}

	audio_channels_init_mode();

	audio_quality(cfg->audio_quality);

	if ((rc = snd_pcm_open(&alsa.handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf(stderr, "Playback open error: %s\n", snd_strerror(rc));
		goto snd_start_error;
	}
	if (set_hwparams() != EXIT_OK) {
		goto snd_start_error;
	}
	if (set_swparams() != EXIT_OK) {
		goto snd_start_error;
	}

	if (loop.action == AT_UNINITIALIZED) {
		loop.alsa = &alsa;
		loop.action = AT_PAUSE;

		// creo il lock
		if (pthread_mutex_init(&loop.lock, NULL) != 0) {
			fprintf(stderr, "Unable to allocate the thread mutex\n");
			goto snd_start_error;
		}

		snd_lock_cache(NULL);

		if ((rc = pthread_create(&loop.thread, NULL, alsa_loop_thread, (void *) &loop))) {
			fprintf(stderr, "Error - pthread_create() return code: %d\n", rc);
			goto snd_start_error;;
		}
	}

	loop.cache = cache;

	if ((rc = snd_pcm_prepare(alsa.handle)) < 0) {
		fprintf(stderr, "cannot prepare audio interface for use (%s)\n", snd_strerror(rc));
		goto snd_start_error;
	}

	snd_unlock_cache(NULL);
	gui_sleep(50);

	loop.action = AT_RUN;

	return (EXIT_OK);

	snd_start_error:
	snd_stop();
	return (EXIT_ERROR);
}
void snd_lock_cache(_callback_data *cache) {
	pthread_mutex_lock(&loop.lock);
}
void snd_unlock_cache(_callback_data *cache) {
	pthread_mutex_unlock(&loop.lock);
}
void snd_stop(void) {
	alsa_loop_in_pause();

	if (snd.cache) {
		if (SNDCACHE->start) {
			free(SNDCACHE->start);
		}

		if (SNDCACHE->silence) {
			free(SNDCACHE->silence);
		}

		free(snd.cache);
		snd.cache = NULL;
	}

	if (audio_channels_quit) {
		audio_channels_quit();
	}

	if (audio_quality_quit) {
		audio_quality_quit();
	}

	if (alsa.handle) {
		snd_pcm_close(alsa.handle);
		alsa.handle = NULL;
	}
}
void snd_quit(void) {
	if (loop.action != AT_UNINITIALIZED) {
		loop.action = AT_STOP;

		pthread_join(loop.thread, NULL);
    	pthread_mutex_destroy(&loop.lock);
    	memset(&loop, 0x00, sizeof(_thread));
    }

	snd_stop();

#if !defined (RELEASE)
	fprintf(stderr, "\n");
#endif
}

static BYTE set_hwparams(void) {
	snd_pcm_hw_params_t *params = NULL;
	unsigned int rrate;
	int rc;
#if !defined (RELEASE)
	snd_pcm_uframes_t size;
	int dir;
#endif

	snd_pcm_hw_params_alloca(&params);

	// choose all parameters
	if ((rc = snd_pcm_hw_params_any(alsa.handle, params)) < 0) {
		fprintf(stderr, "Broken configuration for playback: no configurations available: %s\n",
				snd_strerror(rc));
		return (EXIT_ERROR);
	}
	// anable hardware resampling
	if ((rc = snd_pcm_hw_params_set_rate_resample(alsa.handle, params, 1)) < 0) {
		fprintf(stderr, "Resampling setup failed for playback: %s\n", snd_strerror(rc));
		return (EXIT_ERROR);
	}
	// set the interleaved read/write format
	if ((rc = snd_pcm_hw_params_set_access(alsa.handle, params, SND_PCM_ACCESS_RW_INTERLEAVED))
			< 0) {
		fprintf(stderr, "Access type not available for playback: %s\n", snd_strerror(rc));
		return (EXIT_ERROR);
	}
	// set the sample format
	if ((rc = snd_pcm_hw_params_set_format(alsa.handle, params, SND_PCM_FORMAT_S16)) < 0) {
		fprintf(stderr, "Sample format not available for playback: %s\n", snd_strerror(rc));
		return (EXIT_ERROR);
	}
	// set the channels
	if ((rc = snd_pcm_hw_params_set_channels(alsa.handle, params, snd.channels)) < 0) {
		fprintf(stderr, "Channels count (%i) not available for playbacks: %s\n", snd.channels,
				snd_strerror(rc));
		return (EXIT_ERROR);
	}
	// set the stream rate
	rrate = snd.samplerate;

	if ((rc = snd_pcm_hw_params_set_rate_near(alsa.handle, params, &rrate, 0)) < 0) {
		fprintf(stderr, "Rate %iHz not available for playback: %s\n", snd.samplerate,
				snd_strerror(rc));
		return (EXIT_ERROR);
	}
	if (rrate != snd.samplerate) {
		fprintf(stderr, "Rate doesn't match (requested %iHz, get %iHz)\n", snd.samplerate, rrate);
		return (EXIT_ERROR);
	}
	// set the buffer size
#if !defined (RELEASE)
	printf("hardw bsize : %-6ld - ", alsa.bsize);
#endif
	if ((rc = snd_pcm_hw_params_set_buffer_size_near(alsa.handle, params, &alsa.bsize)) < 0) {
		fprintf(stderr, "Unable to set buffer size for playback: %s\n", snd_strerror(rc));
		return (EXIT_ERROR);
	}
#if !defined (RELEASE)
	printf("%-6ld - ", alsa.bsize);
	snd_pcm_hw_params_get_buffer_size_min(params, &size);
	printf("%-6ld - ", size);
	snd_pcm_hw_params_get_buffer_size_max(params, &size);
	printf("%-6ld\n", size);
#endif
	// set the period size
#if !defined (RELEASE)
	printf("period Size : %-6ld - ", alsa.psize);
#endif
	if ((rc = snd_pcm_hw_params_set_period_size_near(alsa.handle, params, &alsa.psize, 0)) < 0) {
		fprintf(stderr, "Unable to set period size for playback: %s\n", snd_strerror(rc));
		return (EXIT_ERROR);
	}
#if !defined (RELEASE)
	printf("%-6ld - ", alsa.psize);
	snd_pcm_hw_params_get_period_size_min(params, &size, &dir);
	printf("%-6ld - ", size);
	snd_pcm_hw_params_get_period_size_max(params, &size, &dir);
	printf("%-6ld\n", size);
#endif
	// write the parameters to device
	if ((rc = snd_pcm_hw_params(alsa.handle, params)) < 0) {
		fprintf(stderr, "Unable to set hw params for playback: %s\n", snd_strerror(rc));
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}
static BYTE set_swparams(void) {
	snd_pcm_sw_params_t *params = NULL;
	int rc;

	snd_pcm_sw_params_alloca(&params);

	// get the current swparams
	if ((rc = snd_pcm_sw_params_current(alsa.handle, params)) < 0) {
		fprintf(stderr, "Unable to determine current swparams for playback: %s\n", snd_strerror(rc));
		return (EXIT_ERROR);
	}
	// start the transfer when the buffer is almost full
	if ((rc = snd_pcm_sw_params_set_start_threshold(alsa.handle, params, alsa.psize)) < 0) {
		fprintf(stderr, "Unable to set start threshold mode for playback: %s\n", snd_strerror(rc));
		return (EXIT_ERROR);
	}
	// allow the transfer when at least period_size samples can be processed
	if ((rc = snd_pcm_sw_params_set_avail_min(alsa.handle, params, alsa.psize)) < 0) {
		fprintf(stderr, "Unable to set avail min for playback: %s\n", snd_strerror(rc));
		return (EXIT_ERROR);
	}
	// write the parameters to the playback device
	if ((rc = snd_pcm_sw_params(alsa.handle, params)) < 0) {
		fprintf(stderr, "Unable to set sw params for playback: %s\n", snd_strerror(rc));
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}
void *alsa_loop_thread(void *data) {
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

		if ((rc = snd_pcm_wait(th->alsa->handle , 100)) < 0) {
			fprintf(stderr, "snd_pcm_wait() failed (%s)\n", snd_strerror(rc));
			xrun_recovery(th->alsa->handle, rc);
			continue;
		}

		// controllo quanti frames alsa sono richiesti
		if ((avail = snd_pcm_avail_update(th->alsa->handle)) < 0) {
			fprintf(stderr, "snd_pcm_avail_update() failed (%s)\n", snd_strerror(rc));
			xrun_recovery(th->alsa->handle, avail);
			continue;
		}

#if !defined (RELEASE)
		snd_pcm_sframes_t request = avail;
#endif

		snd_lock_cache(NULL);

		avail = (avail > alsa.psize ? alsa.psize : avail);
		len = avail * snd.channels * sizeof(*cache->write);

		if ((info.no_rom | info.pause) || (snd.buffer.start == FALSE)
				|| (fps.fast_forward == TRUE)) {
			wrbuf(th->alsa->handle, (void *) cache->silence, avail);
		} else if (cache->bytes_available <= 0) {
			wrbuf(th->alsa->handle, (void *) cache->silence, avail);
			snd.out_of_sync++;
		} else {
			if (cache->bytes_available < len) {
				len = cache->bytes_available;
				avail = len / snd.channels / sizeof(*cache->write);
				snd.out_of_sync++;
			}

			if ((cache->read + (len - 1)) >= cache->end) {
				uint32_t l1 = 0, l2 = 0;

				l1 = cache->end - cache->read;
				wrbuf(th->alsa->handle, (void *) cache->read,
						l1 / snd.channels / sizeof(*cache->write));
				cache->read = (SBYTE *) cache->start;
				l2 = len - l1;
				wrbuf(th->alsa->handle, (void *) cache->read,
						l2 / snd.channels / sizeof(*cache->write));
				cache->read += l2;
			} else {
				wrbuf(th->alsa->handle, (void *) cache->read, avail);
				cache->read += len;
			}

			if (cache->read >= cache->end) {
				cache->read = (SBYTE *) cache->start;
			}

			cache->bytes_available -= len;
			cache->samples_available -= avail;
		}

#if !defined (RELEASE)
		if ((gui_get_ms() - th->tick) >= 250.0f) {
			th->tick = gui_get_ms();
			if (info.snd_info == TRUE)
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

		snd_unlock_cache(NULL);
 	}

	pthread_exit(EXIT_OK);
}
void alsa_loop_in_pause(void) {
	if (loop.action != AT_UNINITIALIZED) {
		loop.action = AT_PAUSE;

		while (loop.in_run == TRUE) {
			gui_sleep(10);
		}
	}
}
static int INLINE xrun_recovery(snd_pcm_t *handle, int err) {
	if (err == -EPIPE) { // under-run
		err = snd_pcm_prepare(handle);
		if (err < 0) {
			fprintf(stderr, "can't recovery from underrun, prepare failed: %s\n",
					snd_strerror(err));
			info.stop = TRUE;
			return (EXIT_ERROR);
		}
	} else if (err == -ESTRPIPE) {
		while ((err = snd_pcm_resume(handle)) == -EAGAIN) {
			sleep(1); // wait until the suspend flag is released
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
static void INLINE wrbuf(snd_pcm_t *handle, void *buffer, snd_pcm_sframes_t avail) {
	int rc;

	if ((rc = snd_pcm_writei(handle, buffer,avail)) < 0) {
		fprintf(stderr, "snd_pcm_writei failed (%s)\n", snd_strerror(rc));
	}
}

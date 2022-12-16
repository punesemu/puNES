/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

#include <signal.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "thread_def.h"
#include <sndio.h>
#include "info.h"
#include "audio/snd.h"
#include "clock.h"
#include "conf.h"
#include "fps.h"
#include "audio/blipbuf.h"
#include "audio/channels.h"
#include "gui.h"
#include "wave.h"
#include "rewind.h"

typedef struct _sndio {
	struct sio_hdl *playback;
	struct pollfd *pfds;
} _sndio;
typedef struct _snd_thread {
	thread_t thread;
	thread_mutex_t lock;

	BYTE action;
	BYTE in_run;
	int pause_calls;

#if !defined (RELEASE)
	double tick;
#endif
} _snd_thread;

static void _snd_playback_stop(void);

static thread_funct(sndio_thread_loop, void *data);
INLINE static void sndio_wr_buf(void *buffer, uint32_t avail);

static _snd_thread snd_thread;
static _sndio sndio;
static _callback_data cbd;

_snd snd;
_snd_list snd_list;

void (*snd_apu_tick)(void);
void (*snd_end_frame)(void);

BYTE snd_init(void) {
	memset(&snd, 0x00, sizeof(_snd));
	memset(&sndio, 0x00, sizeof(_sndio));
	memset(&cbd, 0x00, sizeof(_callback_data));
	memset(&snd_thread, 0x00, sizeof(_snd_thread));

	snd_apu_tick = NULL;
	snd_end_frame = NULL;

	// creo il lock
	if (thread_mutex_init(snd_thread.lock) != 0) {
		gui_critical(uL("Unable to allocate the snd mutex."));
		return (EXIT_ERROR);
	}

	// creo il thread audio
	snd_thread.action = ST_PAUSE;
	thread_create(snd_thread.thread, sndio_thread_loop, NULL);

	// apro e avvio la riproduzione
	if (snd_playback_start()) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
void snd_quit(void) {
	// se e' in corso una registrazione, la concludo
	wav_from_audio_emulator_close();

	if (snd_thread.action != ST_UNINITIALIZED) {
		snd_thread.action = ST_STOP;
		thread_join(snd_thread.thread);
		thread_mutex_destroy(snd_thread.lock);
		memset(&snd_thread, 0x00, sizeof(_snd_thread));
	}

	snd_playback_stop();
}

void snd_reset_buffers(void) {
	snd_thread_pause();

	if (snd.initialized) {
		cbd.samples_available = 0;
		cbd.bytes_available = 0;
		cbd.write = cbd.start;
		cbd.read = (SBYTE *)cbd.start;
		memset(cbd.start, 0x00, snd.buffer.size);

		audio_channels_reset();
		audio_reset_blipbuf();

		snd.buffer.start = FALSE;
	}

	snd_thread_continue();
}

void snd_thread_pause(void) {
	if (snd_thread.action == ST_UNINITIALIZED) {
		return;
	}

	snd_thread.pause_calls++;

	if (snd_thread.pause_calls == 1) {
		snd_thread.action = ST_PAUSE;

		while (snd_thread.in_run) {
			gui_sleep(1);
		}
	}
}
void snd_thread_continue(void) {
	if (snd_thread.action == ST_UNINITIALIZED) {
		return;
	}

	if (--snd_thread.pause_calls < 0) {
		snd_thread.pause_calls = 0;
	}

	if (snd_thread.pause_calls == 0) {
		snd_thread.action = ST_RUN;

		if (snd.initialized) {
			while (!snd_thread.in_run) {
				gui_sleep(1);
			}
		}
	}
}

void snd_thread_lock(void) {
	thread_mutex_lock(snd_thread.lock);
}
void snd_thread_unlock(void) {
	thread_mutex_unlock(snd_thread.lock);
}

BYTE snd_playback_start(void) {
	snd_thread_pause();

	// come prima cosa blocco eventuali riproduzioni
	_snd_playback_stop();

	memset(&snd, 0x00, sizeof(_snd));
	memset(&sndio, 0x00, sizeof(_sndio));
	memset(&cbd, 0x00, sizeof(_callback_data));
	snd.cache = &cbd;

	audio_channels(cfg->channels_mode);

	switch (cfg->samplerate) {
		case S192000:
			snd.samplerate = 192000;
			break;
		case S96000:
			snd.samplerate = 96000;
			break;
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

	if ((sndio.playback = sio_open(SIO_DEVANY, SIO_PLAY, TRUE)) == NULL) {
		log_warning(uL("sndio;sio_open() failed, audio disabled"));
		cfg->apu.channel[APU_MASTER] = 0;
		return (EXIT_OK);
	}

	if ((sndio.pfds = calloc(sio_nfds(sndio.playback), sizeof(struct pollfd))) == NULL) {
		log_error(uL("sndio;sio_nfds() failed"));
		goto snd_playback_start_error;
	}

	{
		static int factor[10] = { 90, 80, 70, 60, 50, 40, 30, 20, 10, 5 };
		struct sio_par par;

		sio_initpar(&par);

		// snd.samplerate / 50 = 20 ms
		snd.period.samples = (snd.samplerate / factor[cfg->audio_buffer_factor]);

		par.bits = 16;
		par.pchan = snd.channels;
		par.rate = snd.samplerate;
		par.round = snd.period.samples;
		par.xrun = SIO_IGNORE;

		if (!sio_setpar(sndio.playback, &par)) {
			log_error(uL("sndio;sio_setpar() failed"));
			goto snd_playback_start_error;
		}

		if (!sio_getpar(sndio.playback, &par)) {
			log_error(uL("sndio;sio_getpar() failed"));
			goto snd_playback_start_error;
		}

		snd.period.samples = par.round;
		snd.frequency = machine.cpu_hz / (double)snd.samplerate;

		// dimensione in bytes del buffer software
		snd.period.size = snd.period.samples * snd.channels * sizeof(*cbd.write);
		snd.buffer.size = snd.period.size * ((snd.samplerate / snd.period.samples) + 1);

		snd.buffer.limit.low = snd.period.size * 2;
		snd.buffer.limit.high = snd.period.size * 7;

#if !defined (RELEASE)
		//log_info(uL("psize bsize;%d %d"), snd.period.samples, par.appbufsz);
		log_info(uL("sndio;softw bsize;%d - %d"), snd.buffer.size, snd.period.samples);
		log_info(uL("sndio;softw limit;%d - %d"), snd.buffer.limit.high, snd.buffer.limit.low);
#endif

		// alloco il buffer in memoria
		if (!(cbd.start = (SWORD *)malloc(snd.buffer.size))) {
			log_error(uL("sndio;unable to allocate audio buffers"));
			goto snd_playback_start_error;
		}

		if (!(cbd.silence = (SWORD *)malloc(snd.period.size))) {
			log_error(uL("sndio;unable to allocate silence buffer"));
			goto snd_playback_start_error;
		}

		// inizializzo il frame di scrittura
		cbd.write = cbd.start;
		// inizializzo il frame di lettura
		cbd.read = (SBYTE *)cbd.start;
		// punto alla fine del buffer
		cbd.end = cbd.read + snd.buffer.size;
		// azzero completamente i buffers
		memset(cbd.start, 0x00, snd.buffer.size);
		// azzero completamente il buffer del silenzio
		memset(cbd.silence, 0x00, snd.period.size);
	}

	audio_channels_init_mode();

	audio_init_blipbuf();

	if (!sio_start(sndio.playback)) {
		log_error(uL("sndio;sio_start() failed"));
		goto snd_playback_start_error;
	}

	snd.initialized = TRUE;

	snd_thread_continue();
	gui_sleep(150);
	return (EXIT_OK);

	snd_playback_start_error:
	_snd_playback_stop();
	snd_thread_continue();
	gui_sleep(150);
	return (EXIT_ERROR);
}
void snd_playback_stop(void) {
	snd_thread_pause();
	_snd_playback_stop();
	snd_thread_continue();
}

void snd_playback_pause(void) {
	snd.pause_calls++;
}
void snd_playback_continue(void) {
	if (--snd.pause_calls < 0) {
		snd.pause_calls = 0;
	}
}

uTCHAR *snd_playback_device_desc(UNUSED(int dev)) {
	return (0);
}
uTCHAR *snd_playback_device_id(UNUSED(int dev)) {
	return (0);
}

uTCHAR *snd_capture_device_desc(UNUSED(int dev)) {
	return (0);
}
uTCHAR *snd_capture_device_id(UNUSED(int dev)) {
	return (0);
}

void snd_list_devices(void) {}

static void _snd_playback_stop(void) {
	snd.initialized = FALSE;

	if (cbd.start) {
		free(cbd.start);
		cbd.start = NULL;
	}

	if (cbd.silence) {
		free(cbd.silence);
		cbd.silence = NULL;
	}

	snd.cache = NULL;

	if (audio_channels_quit) {
		audio_channels_quit();
	}

	audio_quit_blipbuf();

	if (sndio.playback) {
		sio_close(sndio.playback);
		sndio.playback = NULL;
		gui_sleep(150);
	}

	if (sndio.pfds) {
		free(sndio.pfds);
		sndio.pfds = NULL;
	}
}

static thread_funct(sndio_thread_loop, UNUSED(void *data)) {
	int32_t avail, len;

#if !defined (RELEASE)
	snd_thread.tick = gui_get_ms();
#endif

	while (TRUE) {
		if (snd_thread.action == ST_STOP) {
			snd_thread.in_run = FALSE;
			break;
		} else if ((snd_thread.action == ST_PAUSE) || !snd.initialized) {
			snd_thread.in_run = FALSE;
			gui_sleep(1);
			continue;
		}

		snd_thread.in_run = TRUE;

		{
			int nfds, event, rc;

			nfds = sio_pollfd(sndio.playback, sndio.pfds, POLLOUT);
			rc = poll(sndio.pfds, nfds, 1);

			if (rc == 0) {
				continue;
			} else if (rc < 0) {
				if (errno == EINTR) {
					continue;
				}
				perror("poll");
				continue;
			}

			event = sio_revents(sndio.playback, sndio.pfds);

			if (event & POLLHUP) {
				continue;
			}

			if ((event & POLLOUT) == 0) {
				continue;
			}
		}

		avail = snd.period.samples;
		len = avail * snd.channels * sizeof(*cbd.write);

		if (info.no_rom | info.turn_off | info.pause | rwnd.active | fps_fast_forward_enabled() | !snd.buffer.start) {
			sndio_wr_buf((void *)cbd.silence, len);
		} else if (cbd.bytes_available < len) {
			sndio_wr_buf((void *)cbd.silence, len);
			snd.out_of_sync++;
		} else {
			void *read = (void *)cbd.read;

			snd_thread_lock();

			if (snd.pause_calls) {
				read = (void *)cbd.silence;
			}

			wave_from_audio_emulator_write((SWORD *)read, avail);
			sndio_wr_buf(read, len);

			cbd.bytes_available -= len;
			cbd.samples_available -= avail;

#if !defined (RELEASE)
			if (((void *)cbd.write > (void *)cbd.read) && ((void *)cbd.write < (void *)(cbd.read + len))) {
				snd.overlap++;
			}
#endif

			if ((cbd.read += len) >= cbd.end) {
				cbd.read = (SBYTE *)cbd.start;
			}

			snd_thread_unlock();
		}

#if !defined (RELEASE)
		if ((gui_get_ms() - snd_thread.tick) >= 250.0f) {
			snd_thread.tick = gui_get_ms();
			if (info.snd_info)
			fprintf(stderr, "snd debug : %d %d %6d %6d %4d %4d %4d %4d %3d %f %4s\r",
				avail,
				len,
				cbd.samples_available,
				cbd.bytes_available,
				fps.info.emu_too_long,
				fps.info.skipped,
				snd.overlap,
				snd.out_of_sync,
				(int)fps.gfx,
				machine.ms_frame,
				" ");
		}
#endif
 	}

	thread_funct_return();
}

INLINE static void sndio_wr_buf(void *buffer, uint32_t avail) {
	if (sio_write(sndio.playback, buffer, avail) == 0) {
		if (sio_eof(sndio.playback)) {
			log_error(uL("sndio;sio_write() failed"));
		}
	}
}

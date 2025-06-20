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

#include "audio/snd.h"
#include "thread_def.h"
#include "info.h"
#include "conf.h"
#include "audio/blipbuf.h"
#include "audio/channels.h"
#include "gui.h"
#include "fps.h"
#include "clock.h"
#include "wave.h"
#include "rewind.h"
#if defined (DEBUG)
#define __inline
#else
#define __inline inline __attribute__((always_inline))
#endif
#define INITGUID
#include <XAudio2.h>
#include <dsound.h>
#undef INITGUID

static void _snd_playback_stop(void);

static int snd_list_find_index_id(_snd_list_dev *list, uTCHAR *id, int size);
static void snd_list_device_add(_snd_list_dev *list, uTCHAR *id, GUID *guid, uTCHAR *desc);
static void snd_list_devices_quit(void);
static void snd_list_devices_free(_snd_list_dev *list);

static BOOL CALLBACK cb_enum_capture_dev(LPGUID guid, LPCWSTR desc, LPCWSTR module, LPVOID data);

static void STDMETHODCALLTYPE OnVoiceProcessPassStart(IXAudio2VoiceCallback *callback, UINT32 b);
static void STDMETHODCALLTYPE OnVoiceProcessPassEnd(IXAudio2VoiceCallback *callback);
static void STDMETHODCALLTYPE OnStreamEnd(IXAudio2VoiceCallback *callback);
static void STDMETHODCALLTYPE OnBufferStart(IXAudio2VoiceCallback *callback, void *pBufferContext);
static void STDMETHODCALLTYPE OnBufferEnd(IXAudio2VoiceCallback *callback, void* pBufferContext);
static void STDMETHODCALLTYPE OnLoopEnd(IXAudio2VoiceCallback *callback, void *pBufferContext);
static void STDMETHODCALLTYPE OnVoiceError(IXAudio2VoiceCallback *callback, void* pBufferContext, HRESULT Error);

INLINE static void xaudio2_wrbuf(IXAudio2SourceVoice *source, XAUDIO2_BUFFER *x2buf, const BYTE *buffer);

static thread_funct(snd_dummy_thread_loop, void *data);

static struct _snd_thread {
	thread_t thread;
	thread_mutex_t lock;

	BYTE action;
	BYTE in_run;
	int pause_calls;

#if !defined (RELEASE)
	double tick;
#endif
} snd_thread;
static struct _directsound8 {
	BYTE available;
	HANDLE ds8;
	HRESULT (WINAPI *DirectSoundCreate8_proc)(LPGUID, LPDIRECTSOUND*, LPUNKNOWN);
	HRESULT (WINAPI *DirectSoundCaptureEnumerateW_proc)(LPDSENUMCALLBACKW, LPVOID);
} ds8;
static struct _xaudio2 {
	IXAudio2 *engine;
	IXAudio2MasteringVoice *master;
	IXAudio2SourceVoice *source;
	XAUDIO2_BUFFER buffer;
} xaudio2;
static IXAudio2VoiceCallbackVtbl voice_callbacks_vtable = {
	OnVoiceProcessPassStart,
	OnVoiceProcessPassEnd,
	OnStreamEnd,
	OnBufferStart,
	OnBufferEnd,
	OnLoopEnd,
	OnVoiceError
};
static IXAudio2VoiceCallback voice_callbacks = { &voice_callbacks_vtable };
static _callback_data cbd;
static BYTE snd_dummy_enabled = FALSE;

_snd snd;
_snd_list snd_list;

void (*snd_apu_tick)(void);
void (*snd_end_frame)(void);

BYTE snd_init(void) {
	uTCHAR system32[MAX_PATH];

	memset(&snd, 0x00, sizeof(_snd));
	memset(&xaudio2, 0x00, sizeof(xaudio2));
	memset(&ds8, 0x00, sizeof(ds8));
	memset(&cbd, 0x00, sizeof(_callback_data));
	memset(&snd_thread, 0x00, sizeof(snd_thread));

	snd_apu_tick = NULL;
	snd_end_frame = NULL;

	if (thread_mutex_init_error(snd_thread.lock)) {
		gui_critical(uL("Unable to create XAudio2 semaphore."));
		return (EXIT_ERROR);
	}

	if (GetSystemDirectoryW(system32, MAX_PATH) != 0) {
		uTCHAR path[MAX_PATH];

		usnprintf(path, usizeof(path), uL("" uPs("") "\\DSOUND.DLL"), system32);
		ds8.ds8 = LoadLibraryW(path);
		ds8.available = FALSE;
		if (ds8.ds8 == NULL) {
			log_error(uL("directsound;failed to load DSOUND.DLL"));
		} else {
			ds8.DirectSoundCreate8_proc = (void *)GetProcAddress(ds8.ds8, "DirectSoundCreate8");
			ds8.DirectSoundCaptureEnumerateW_proc = (void *)GetProcAddress(ds8.ds8,"DirectSoundCaptureEnumerateW");
			ds8.available = (ds8.DirectSoundCreate8_proc != NULL) && (ds8.DirectSoundCaptureEnumerateW_proc != NULL);
			if (!ds8.available) {
				log_error(uL("directsound;system doesn't appear to have DS8"));
			}
		}
	}

	snd_list_devices();

	// non ho trovato dispositivi audio
	if (snd_list.playback.count == 1) {
		snd_dummy_enabled = TRUE;
	}

	// apro e avvio la riproduzione
	if (snd_playback_start()) {
		return (EXIT_ERROR);
	}

	if (snd_dummy_enabled) {
		thread_create(snd_thread.thread, snd_dummy_thread_loop, NULL);
	}

	return (EXIT_OK);
}
void snd_quit(void) {
	// se e' in corso una registrazione, la concludo
	wav_from_audio_emulator_close();

	if (snd_dummy_enabled) {
		if (snd_thread.action != ST_UNINITIALIZED) {
			snd_thread.action = ST_STOP;
			thread_join(snd_thread.thread);
		}
	}

	snd_playback_stop();

	thread_mutex_destroy(snd_thread.lock);

	if (ds8.ds8) {
		FreeLibrary(ds8.ds8);
	}
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
	if (snd_dummy_enabled && (snd_thread.action == ST_UNINITIALIZED)) {
		return;
	}

	snd_thread.pause_calls++;

	if (snd_thread.pause_calls == 1) {
		snd_thread.action = ST_PAUSE;

		while (snd_thread.in_run) {
			if (snd_dummy_enabled) {
				gui_sleep(1);
			} else {}
		}
	}
}
void snd_thread_continue(void) {
	if (snd_dummy_enabled && (snd_thread.action == ST_UNINITIALIZED)) {
		return;
	}

	if (--snd_thread.pause_calls < 0) {
		snd_thread.pause_calls = 0;
	}

	if (snd_thread.pause_calls == 0) {
		snd_thread.action = ST_RUN;

		if (snd_dummy_enabled && snd.initialized) {
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
	static BYTE first_time = TRUE;

	if (snd_dummy_enabled && !first_time) {
		snd_thread_pause();
	}

	snd_playback_restart:
	// come prima cosa blocco eventuali riproduzioni
	_snd_playback_stop();

	memset(&snd, 0x00, sizeof(_snd));
	memset(&xaudio2, 0x00, sizeof(xaudio2));
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

	if (!snd_dummy_enabled) {
		int index = snd_list_find_index_id(&snd_list.playback, cfg->audio_output, usizeof(cfg->audio_output));
		WAVEFORMATEX wfm;

		memset(&wfm, 0, sizeof(wfm));

		if (XAudio2Create(&xaudio2.engine, 0, XAUDIO2_DEFAULT_PROCESSOR) != S_OK) {
			gui_warning(uL("Unable to create XAudio2 object. Probably you\nhave an incomplete installation of DirectX 10."));
			snd_dummy_enabled = TRUE;
			goto snd_playback_restart;
		}

		if (index == 0) {
			if (IXAudio2_CreateMasteringVoice(xaudio2.engine, &xaudio2.master, snd.channels,
				snd.samplerate, 0, 0, NULL) != S_OK) {
				gui_warning(uL("Unable to create XAudio2 master voice."));
				snd_dummy_enabled = TRUE;
				goto snd_playback_restart;
			}
		} else {
			if (IXAudio2_CreateMasteringVoice(xaudio2.engine, &xaudio2.master, snd.channels,
				snd.samplerate, 0, index - 1, NULL) != S_OK) {
				gui_warning(uL("Unable to create XAudio2 master voice."));
				snd_dummy_enabled = TRUE;
				goto snd_playback_restart;
			}
		}

		wfm.wFormatTag = WAVE_FORMAT_PCM;
		wfm.nChannels = snd.channels;
		wfm.wBitsPerSample = 16;
		wfm.nSamplesPerSec = snd.samplerate;
		wfm.nBlockAlign = (wfm.nChannels * wfm.wBitsPerSample) / 8;
		wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;
		wfm.cbSize = sizeof(wfm);

		if (IXAudio2_CreateSourceVoice(xaudio2.engine,
				&xaudio2.source,
				&wfm,
				XAUDIO2_VOICE_NOSRC | XAUDIO2_VOICE_NOPITCH,
				XAUDIO2_DEFAULT_FREQ_RATIO,
				&voice_callbacks,
				NULL,
				NULL) != S_OK) {
			gui_warning(uL("Unable to create XAudio2 source voice."));
			snd_dummy_enabled = TRUE;
			goto snd_playback_restart;
		}
	}

	{
		static int factor[10] = { 90, 80, 70, 60, 50, 40, 30, 20, 10, 5 };

		// snd.samplerate / 50 = 20 ms
		snd.period.samples = (snd.samplerate / factor[cfg->audio_buffer_factor]);
		snd.frequency = machine.cpu_hz / (double)snd.samplerate;

		// dimensione in bytes del buffer
		snd.period.size = snd.period.samples * snd.channels * sizeof(*cbd.write);
		snd.buffer.size = (int32_t)snd.period.size * ((snd.samplerate / (int32_t)snd.period.samples) + 1);

		snd.buffer.limit.low = snd.period.size * 2;
		snd.buffer.limit.high = snd.period.size * 7;

#if !defined (RELEASE)
		log_info(uL("softw bsize;%d - %d"), snd.buffer.size, snd.period.samples);
		log_info(uL("softw limit;%d - %d"), snd.buffer.limit.low, snd.buffer.limit.high);
#endif

		// alloco il buffer in memoria
		if (!(cbd.start = (SWORD *)malloc(snd.buffer.size))) {
			gui_critical(uL("Unable to allocate audio buffers."));
			goto snd_playback_start_error;
		}

		if (!(cbd.silence = (SWORD *)malloc(snd.period.size))) {
			gui_critical(uL("Unable to allocate silence buffer."));
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

		// azzero completamente la struttura XAUDIO2_BUFFER
		memset(&xaudio2.buffer, 0x00, sizeof(xaudio2.buffer));

		xaudio2.buffer.AudioBytes = snd.period.samples * sizeof(*cbd.write) * snd.channels;
		xaudio2.buffer.pAudioData = (const BYTE *)cbd.silence;
		xaudio2.buffer.PlayBegin = 0;
		xaudio2.buffer.PlayLength = snd.period.samples;
		xaudio2.buffer.LoopBegin = 0;
		xaudio2.buffer.LoopLength = 0;
		xaudio2.buffer.LoopCount = 0;
		xaudio2.buffer.pContext = snd.cache;

		if (!snd_dummy_enabled) {
			if (IXAudio2SourceVoice_SubmitSourceBuffer(xaudio2.source, (const XAUDIO2_BUFFER *)&xaudio2.buffer, NULL) != S_OK) {
				gui_warning(uL("Unable to set sound engine."));
				snd_dummy_enabled = TRUE;
				goto snd_playback_restart;
			}
		}
	}

	audio_channels_init_mode();

	audio_init_blipbuf();

#if !defined (RELEASE)
	snd_thread.tick = gui_get_ms();
#endif

	if (!snd_dummy_enabled) {
		if (IXAudio2_StartEngine(xaudio2.engine) != S_OK) {
			gui_warning(uL("Unable to start sound engine."));
			snd_dummy_enabled = TRUE;
			goto snd_playback_restart;
		}
		if (IXAudio2SourceVoice_Start(xaudio2.source, 0, XAUDIO2_COMMIT_NOW) != S_OK) {
			gui_warning(uL("Unable to start source voice."));
			snd_dummy_enabled = TRUE;
			goto snd_playback_restart;
		}
	}

	snd.initialized = TRUE;

	if (snd_dummy_enabled && !first_time) {
		snd_thread_continue();
	}
	first_time = FALSE;
	return (EXIT_OK);

	snd_playback_start_error:
	_snd_playback_stop();
	if (snd_dummy_enabled && !first_time) {
		snd_thread_continue();
	}
	first_time = FALSE;
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
	return ((uTCHAR *)snd_list.playback.devices[dev].id);
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
	return ((uTCHAR *)snd_list.capture.devices[dev].id);
}

void snd_list_devices(void) {
	IXAudio2 *ixa2 = NULL;
	UINT32 devcount = 0;
	UINT32 i;

	snd_list_devices_quit();

	// Playback devices
	snd_list_device_add(&snd_list.playback, uL("default"), NULL, uL("System Default"));

	if (XAudio2Create(&ixa2, 0, XAUDIO2_DEFAULT_PROCESSOR) != S_OK) {
		log_error(uL("xaudio2;error on create XAudio2 object"));
		return;
	}
	if (IXAudio2_GetDeviceCount(ixa2, &devcount) != S_OK) {
		IXAudio2_Release(ixa2);
		log_error(uL("xaudio2;error on count devices"));
		return;
	}

	if (!devcount) {
		log_error(uL("xaudio2;no devices"));
	} else {
		if (devcount == 1) {
			log_info(uL("xaudio2;%d device"), devcount);
		} else {
			log_info(uL("xaudio2;%d devices"), devcount);
		}
		for (i = 0; i < devcount; i++) {
			XAUDIO2_DEVICE_DETAILS details;

			if (IXAudio2_GetDeviceDetails(ixa2, i, &details) == S_OK) {
				if (ustrlen(details.DisplayName) == 0) {
					log_warning_box(uL("%d : DisplayName null or empty"), i);
					continue;
				}
				snd_list_device_add(&snd_list.playback, details.DeviceID, NULL, details.DisplayName);
				log_info_box(uL("%d : " uPs("")), i, details.DisplayName);
			} else {
				log_error_box(uL("%d : error on GetDeviceDetails"), i);
			}
		}
	}
	IXAudio2_Release(ixa2);

	// Capture devices
	if (ds8.available) {
		snd_list_device_add(&snd_list.capture, uL("default"), NULL, uL("System Default"));
		ds8.DirectSoundCaptureEnumerateW_proc(cb_enum_capture_dev, NULL);
	}
}

static void _snd_playback_stop(void) {
	snd.initialized = FALSE;

	if (xaudio2.source) {
		IXAudio2SourceVoice_Stop(xaudio2.source, 0, XAUDIO2_COMMIT_NOW);
		IXAudio2SourceVoice_FlushSourceBuffers(xaudio2.source);
		IXAudio2SourceVoice_DestroyVoice(xaudio2.source);
		xaudio2.source = NULL;
	}

	if (xaudio2.engine) {
		IXAudio2_StopEngine(xaudio2.engine);
	}

	if (xaudio2.master) {
		IXAudio2MasteringVoice_DestroyVoice(xaudio2.master);
		xaudio2.master = NULL;
	}

	if (xaudio2.engine) {
		IXAudio2_Release(xaudio2.engine);
		xaudio2.engine = NULL;
	}

	gui_sleep(150);

	if (cbd.start) {
		free(snd.cache->start);
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
}

static int snd_list_find_index_id(_snd_list_dev *list, uTCHAR *id, int size) {
	int i, index = -1;

	for (i = 0; i < list->count; i++) {
		if (ustrcmp(id, (uTCHAR *)list->devices[i].id) == 0) {
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
static void snd_list_device_add(_snd_list_dev *list, uTCHAR *id, GUID *guid, uTCHAR *desc) {
	_snd_dev *dev, *devs;

	if ((devs = (_snd_dev *)realloc(list->devices, (list->count + 1) * sizeof(_snd_dev)))) {
		list->devices = devs;
		dev = &list->devices[list->count];
		if (id) {
			// playback
			dev->id = ustrdup(id);
		} else {
			// capture
			dev->id = malloc(sizeof(GUID));
			memcpy(dev->id, guid, sizeof(GUID));
		}
		dev->desc = ustrdup(desc);
		list->count++;
	}
}
static void snd_list_devices_quit(void) {
	snd_list_devices_free(&snd_list.playback);
	snd_list_devices_free(&snd_list.capture);
}
static void snd_list_devices_free(_snd_list_dev *list) {
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

static BOOL CALLBACK cb_enum_capture_dev(LPGUID guid, LPCWSTR desc, UNUSED(LPCWSTR module), UNUSED(LPVOID data)) {
	if ((guid != NULL) && (desc != NULL) && (ustrlen(desc) != 0)) {
		snd_list_device_add(&snd_list.capture, NULL, guid, (uTCHAR *)desc);
	}
	return (TRUE);
}

static void STDMETHODCALLTYPE OnVoiceProcessPassStart(UNUSED(IXAudio2VoiceCallback *callback), UNUSED(UINT32 b)) {}
static void STDMETHODCALLTYPE OnVoiceProcessPassEnd(UNUSED(IXAudio2VoiceCallback *callback)) {}
static void STDMETHODCALLTYPE OnStreamEnd(UNUSED(IXAudio2VoiceCallback *callback)) {}
static void STDMETHODCALLTYPE OnBufferStart(UNUSED(IXAudio2VoiceCallback *callback), UNUSED(void *pBufferContext)) {
	WORD len = xaudio2.buffer.AudioBytes;
	int avail = (int)xaudio2.buffer.PlayLength;

	snd_thread.in_run = TRUE;

	if ((snd_thread.action == ST_STOP) || (snd_thread.action == ST_PAUSE)) {
		xaudio2_wrbuf(xaudio2.source, &xaudio2.buffer, (const BYTE *)cbd.silence);
	} else if (info.no_rom | info.turn_off | info.pause | rwnd.active | fps_fast_forward_enabled() | !snd.buffer.start) {
		xaudio2_wrbuf(xaudio2.source, &xaudio2.buffer, (const BYTE *)cbd.silence);
	} else if (cbd.bytes_available < len) {
		xaudio2_wrbuf(xaudio2.source, &xaudio2.buffer, (const BYTE *)cbd.silence);
		snd.out_of_sync++;
	} else {
		void *read = (void *)cbd.read;

		snd_thread_lock();

		if (snd.pause_calls) {
			read = (void *)cbd.silence;
		}

		wave_from_audio_emulator_write((SWORD *)read, avail);
		xaudio2_wrbuf(xaudio2.source, &xaudio2.buffer, (const BYTE *)read);

		cbd.bytes_available -= len;
		cbd.samples_available -= avail;

#if !defined (RELEASE)
		if (((void *)cbd.write > (void *)cbd.read) && ((void *)cbd.write < (void *)(cbd.read + len))) {
			snd.overlap++;
		}
#endif

		// mi preparo per i prossimi frames da inviare, sempre
	 	// che non abbia raggiunto la fine del buffer, nel
	 	// qual caso devo puntare al suo inizio.
		if ((cbd.read += len) >= cbd.end) {
			cbd.read = (SBYTE *)cbd.start;
		}

		snd_thread_unlock();
	}

#if !defined (RELEASE)
	if ((gui_get_ms() - snd_thread.tick) >= 250.0f) {
		snd_thread.tick = gui_get_ms();
		if (info.snd_info)
		fprintf(stderr, "snd debug : %d %d %6d %6d %4d %4d %4d %4d %3d %f\r",
			avail,
			len,
			cbd.samples_available,
			cbd.bytes_available,
			fps.info.emu_too_long,
			fps.info.skipped,
			snd.overlap,
			snd.out_of_sync,
			(int)fps.gfx,
			machine.ms_frame);
	}
#endif

	snd_thread.in_run = FALSE;
}
static void STDMETHODCALLTYPE OnBufferEnd(UNUSED(IXAudio2VoiceCallback *callback), UNUSED(void *pBufferContext)) {}
static void STDMETHODCALLTYPE OnLoopEnd(UNUSED(IXAudio2VoiceCallback *callback), UNUSED(void *pBufferContext)) {}
static void STDMETHODCALLTYPE OnVoiceError(UNUSED(IXAudio2VoiceCallback *callback), UNUSED(void* pBufferContext), UNUSED(HRESULT Error)) {}

INLINE static void xaudio2_wrbuf(IXAudio2SourceVoice *source, XAUDIO2_BUFFER *x2buf, const BYTE *buffer) {
	x2buf->pAudioData = buffer;

	if (IXAudio2SourceVoice_SubmitSourceBuffer(source, x2buf, NULL) != S_OK) {
		log_warning(uL("xaudio;unable to submit source buffer"));
	}
}

static thread_funct(snd_dummy_thread_loop, UNUSED(void *data)) {
#if !defined (RELEASE)
	snd_thread.tick = gui_get_ms();
#endif

	while (TRUE) {
		WORD len = xaudio2.buffer.AudioBytes;
		int avail = (int)xaudio2.buffer.PlayLength;

		if (snd_thread.action == ST_STOP) {
			snd_thread.in_run = FALSE;
			break;
		} else if ((snd_thread.action == ST_PAUSE) || !snd.initialized || (cbd.bytes_available < len)) {
			snd_thread.in_run = FALSE;
			gui_sleep(1);
			continue;
		}

		snd_thread.in_run = TRUE;

		if (info.no_rom | info.turn_off | info.pause | rwnd.active | fps_fast_forward_enabled() | !snd.buffer.start) {
		} else {
			void *read = (void *)cbd.read;

			snd_thread_lock();

			if (snd.pause_calls) {
				read = (void *)cbd.silence;
			}

			wave_from_audio_emulator_write((SWORD *)read, avail);

			cbd.bytes_available -= len;
			cbd.samples_available -= avail;

#if !defined (RELEASE)
			if (((void *)cbd.write > (void *)cbd.read) && ((void *)cbd.write < (void *)(cbd.read + len))) {
				snd.overlap++;
			}
#endif

			// mi preparo per i prossimi frames da inviare, sempre
		 	// che non abbia raggiunto la fine del buffer, nel
		 	// qual caso devo puntare al suo inizio.
			if ((cbd.read += len) >= cbd.end) {
				cbd.read = (SBYTE *)cbd.start;
			}

			snd_thread_unlock();
		}

#if !defined (RELEASE)
		if ((gui_get_ms() - snd_thread.tick) >= 250.0f) {
			snd_thread.tick = gui_get_ms();
			if (info.snd_info)
			fprintf(stderr, "snd dummy : %d %d %6d %6d %4d %4d %4d %4d %3d %f\r",
				avail,
				len,
				cbd.samples_available,
				cbd.bytes_available,
				fps.info.emu_too_long,
				fps.info.skipped,
				snd.overlap,
				snd.out_of_sync,
				(int)fps.gfx,
				machine.ms_frame);
		}
#endif

		snd_thread.in_run = FALSE;
	}

	thread_funct_return();
}

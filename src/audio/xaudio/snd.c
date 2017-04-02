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

#include "snd.h"
#include "emu.h"
#include "info.h"
#include "conf.h"
#include "audio/quality.h"
#include "audio/channels.h"
#include "gui.h"
#include "fps.h"
#include "clock.h"
#include "apu.h"
#define INITGUID
#include <XAudio2.h>
#include <dsound.h>
#undef INITGUID

#if defined (THIS)
#undef THIS
#endif
#if defined (THIS_)
#undef THIS_
#endif

#define THIS IXAudio2VoiceCallback *callback
#define THIS_ IXAudio2VoiceCallback *callback,

static int  snd_list_find_index_id(_snd_list_dev *list, uTCHAR *id, int size);
static void snd_list_device_add(_snd_list_dev *list, uTCHAR *id, GUID *guid, uTCHAR *desc);
static void snd_list_devices_quit(void);
static void snd_list_devices_free(_snd_list_dev *list);

static BOOL CALLBACK cb_enum_capture_dev(LPGUID guid, LPCWSTR desc, LPCWSTR module, LPVOID data);

static void INLINE xaudio2_wrbuf(IXAudio2SourceVoice *source, XAUDIO2_BUFFER *x2buf,
		const BYTE *buffer);

static void STDMETHODCALLTYPE OnVoiceProcessPassStart(THIS_ UINT32 b);
static void STDMETHODCALLTYPE OnVoiceProcessPassEnd(THIS);
static void STDMETHODCALLTYPE OnStreamEnd(THIS);
static void STDMETHODCALLTYPE OnBufferStart(THIS_ void *data);
static void STDMETHODCALLTYPE OnBufferEnd(THIS_ void* data);
static void STDMETHODCALLTYPE OnLoopEnd(THIS_ void *data);
static void STDMETHODCALLTYPE OnVoiceError(THIS_ void* data, HRESULT Error);



static struct _directsound8 {
	BYTE available;
	HANDLE ds8;
	HRESULT (WINAPI *DirectSoundCreate8_proc)(LPGUID, LPDIRECTSOUND*, LPUNKNOWN);
	HRESULT (WINAPI *DirectSoundCaptureEnumerateW_proc)(LPDSENUMCALLBACKW, LPVOID);
} ds8;



static struct _xaudio2 {
	BYTE opened;
	IXAudio2 *engine;
	IXAudio2MasteringVoice *master;
	IXAudio2SourceVoice *source;
	XAUDIO2_BUFFER buffer;
	HANDLE semaphore;
#if !defined (RELEASE)
	double tick;
#endif
} xaudio2;
static IXAudio2VoiceCallbackVtbl callbacks_vtable = {
	OnVoiceProcessPassStart,
	OnVoiceProcessPassEnd,
	OnStreamEnd,
	OnBufferStart,
	OnBufferEnd,
	OnLoopEnd,
	OnVoiceError
};
static IXAudio2VoiceCallback callbacks = { &callbacks_vtable };
static _callback_data cbd;

BYTE snd_init(void) {
	memset(&snd, 0x00, sizeof(_snd));
	memset(&xaudio2, 0x00, sizeof(xaudio2));
	memset(&ds8, 0x00, sizeof(ds8));
	memset(&cbd, 0x00, sizeof(_callback_data));

	snd_apu_tick = NULL;
	snd_end_frame = NULL;

	if ((ds8.ds8 = LoadLibrary("DSOUND.DLL")) == NULL) {
		fprintf(stderr, "DirectSound: failed to load DSOUND.DLL\n");
		ds8.available = FALSE;
	} else {
		ds8.available = TRUE;
		if ((ds8.DirectSoundCreate8_proc = (HRESULT (WINAPI *)(LPGUID, LPDIRECTSOUND*,LPUNKNOWN))
				GetProcAddress(ds8.ds8, "DirectSoundCreate8")) == NULL) {
			ds8.available = FALSE;
		}
		if ((ds8.DirectSoundCaptureEnumerateW_proc = (HRESULT (WINAPI *)(LPDSENUMCALLBACKW, LPVOID))
				GetProcAddress(ds8.ds8, "DirectSoundCaptureEnumerateW")) == NULL) {
			ds8.available = FALSE;
		}
		if (ds8.available == FALSE) {
			fprintf(stderr, "DirectSound: System doesn't appear to have DS8.");
		}
	}

	snd_list_devices();

	// apro e avvio la riproduzione
	if (snd_start()) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
BYTE snd_start(void) {
	int psamples;

	if (!cfg->apu.channel[APU_MASTER]) {
		return (EXIT_OK);
	}

	// come prima cosa blocco eventuali riproduzioni
	snd_stop();

	memset(&snd, 0x00, sizeof(_snd));
	memset(&xaudio2, 0x00, sizeof(xaudio2));
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

	if (XAudio2Create(&xaudio2.engine, 0, XAUDIO2_DEFAULT_PROCESSOR) != S_OK) {
		MessageBox(NULL,
			"ATTENTION: Unable to create XAudio2 object. Probably you\n"
			"have an incomplete installation of DirectX 10."   ,
			"Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	{
		int index = snd_list_find_index_id(&snd_list.playback, cfg->audio_output,
		        usizeof(cfg->audio_output));

		if (index == 0) {
			if (IXAudio2_CreateMasteringVoice(xaudio2.engine, &xaudio2.master, snd.channels,
					snd.samplerate, 0, 0, NULL) != S_OK) {
				MessageBox(NULL, "ATTENTION: Unable to create XAudio2 master voice.", "Error!",
				MB_ICONEXCLAMATION | MB_OK);
				return (EXIT_ERROR);
			}
		} else {
			if (IXAudio2_CreateMasteringVoice(xaudio2.engine, &xaudio2.master, snd.channels,
					snd.samplerate, 0, index - 1, NULL) != S_OK) {
				MessageBox(NULL, "ATTENTION: Unable to create XAudio2 master voice.", "Error!",
				MB_ICONEXCLAMATION | MB_OK);
				return (EXIT_ERROR);
			}
		}
	}

	{
		WAVEFORMATEX wfm;

		memset(&wfm, 0, sizeof(wfm));

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
				&callbacks,
				NULL,
				NULL) != S_OK) {
			MessageBox(NULL,
				"ATTENTION: Unable to create XAudio2 source voice.\n",
				"Error!",
				MB_ICONEXCLAMATION | MB_OK);
			return (EXIT_ERROR);
		}
	}

	{
		static int factor[10] = { 90, 80, 70, 60, 50, 40, 30, 20, 10, 5 };

		// snd.samplarate / 50 = 20 ms
		psamples = (snd.samplerate / factor[cfg->audio_buffer_factor]);
		psamples *= 2;
	}

	snd.samples = psamples * 2;
	snd.frequency = machine.cpu_hz / (double) snd.samplerate;

	xaudio2.opened = TRUE;

#if !defined (RELEASE)
	xaudio2.tick = gui_get_ms();
#endif

	{
		// dimensione in bytes del buffer
		snd.buffer.size = (psamples * snd.channels * sizeof(*cbd.write)) * 5;

		snd.buffer.limit.low = (snd.buffer.size / 100) * 25;
		snd.buffer.limit.high = (snd.buffer.size / 100) * 55;

#if !defined (RELEASE)
		printf("softw bsize : %-6d - %-6d\n", snd.buffer.size, snd.samples);
		printf("softw limit : %-6d - %-6d\n", snd.buffer.limit.low, snd.buffer.limit.high);
#endif

		// alloco il buffer in memoria
		if (!(cbd.start = (SWORD *) malloc(snd.buffer.size))) {
			MessageBox(NULL,
				"ATTENTION: Unable to allocate audio buffers.\n",
				"Error!",
				MB_ICONEXCLAMATION | MB_OK);
			return (EXIT_ERROR);
		}

		if (!(cbd.silence = (SWORD *) malloc(snd.buffer.size))) {
			MessageBox(NULL,
				"ATTENTION: Unable to allocate silence buffer.\n",
				"Error!",
				MB_ICONEXCLAMATION | MB_OK);
			return (EXIT_ERROR);
		}

		// inizializzo il frame di scrittura
		cbd.write = cbd.start;
		// inizializzo il frame di lettura
		cbd.read = (SBYTE *) cbd.start;
		// punto alla fine del buffer
		cbd.end = cbd.read + snd.buffer.size;
		// creo il lock
		if ((xaudio2.semaphore = CreateSemaphore(NULL, 1, 2, NULL)) == NULL) {
			MessageBox(NULL,
				"ATTENTION: Unable to create XAudio2 semaphore.\n",
				"Error!",
				MB_ICONEXCLAMATION | MB_OK);
			return (EXIT_ERROR);
		}
		// azzero completamente i buffers
		memset(cbd.start, 0x00, snd.buffer.size);
		// azzero completamente il buffer del silenzio
		memset(cbd.silence, 0x00, snd.buffer.size);

		// azzero completamente la struttura XAUDIO2_BUFFER
		memset(&xaudio2.buffer, 0x00, sizeof(xaudio2.buffer));

		xaudio2.buffer.AudioBytes = psamples * sizeof(*cbd.write) * snd.channels;
		//xaudio2.buffer.pAudioData = (const BYTE *) cbd.read;
		xaudio2.buffer.pAudioData = (const BYTE *) cbd.silence;
		xaudio2.buffer.PlayBegin = 0;
		xaudio2.buffer.PlayLength = psamples;
		xaudio2.buffer.LoopBegin = 0;
		xaudio2.buffer.LoopLength = 0;
		xaudio2.buffer.LoopCount = 0;
		xaudio2.buffer.pContext = snd.cache;

		cbd.xa2buffer = &xaudio2.buffer;
		cbd.xa2source = xaudio2.source;
		cbd.lock = xaudio2.semaphore;

		if (IXAudio2SourceVoice_SubmitSourceBuffer(xaudio2.source,
		        (const XAUDIO2_BUFFER *) cbd.xa2buffer, NULL) != S_OK) {
			MessageBox(NULL,
				"ATTENTION: Unable to set sound engine.\n",
				"Error!",
				MB_ICONEXCLAMATION | MB_OK);
			return (EXIT_ERROR);
		}
	}

	if (extcl_snd_start) {
		extcl_snd_start((WORD) snd.samplerate);
	}

	audio_channels_init_mode();

	audio_quality(cfg->audio_quality);

	if(IXAudio2_StartEngine(xaudio2.engine) != S_OK) {
		MessageBox(NULL,
			"ATTENTION: Unable to start sound engine.\n",
			"Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	if(IXAudio2SourceVoice_Start(xaudio2.source, 0, XAUDIO2_COMMIT_NOW) != S_OK) {
		MessageBox(NULL,
			"ATTENTION: Unable to start source voice.\n",
			"Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
void snd_lock_cache(_callback_data *cache) {
	WaitForSingleObject((HANDLE **) cache->lock, INFINITE);
}
void snd_unlock_cache(_callback_data *cache) {
	ReleaseSemaphore((HANDLE **) cache->lock, 1, NULL);
}
void snd_stop(void) {
	xaudio2.opened = FALSE;

    if (ds8.ds8) {
    	FreeLibrary(ds8.ds8);
    }

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

	if (snd.cache) {
		if (SNDCACHE->start) {
			free(SNDCACHE->start);
		}

		if (SNDCACHE->silence) {
			free(SNDCACHE->silence);
		}

	    if (xaudio2.semaphore) {
	        CloseHandle(xaudio2.semaphore);
	        xaudio2.semaphore = NULL;
	    }

		snd.cache = NULL;
	}

	if (audio_channels_quit) {
		audio_channels_quit();
	}

	if (audio_quality_quit) {
		audio_quality_quit();
	}
}
void snd_quit(void) {
	snd_stop();
}

void snd_list_devices(void) {
	IXAudio2 *ixa2 = NULL;
	UINT32 devcount = 0;
	UINT32 i = 0;

	snd_list_devices_quit();

	// Playback devices
	snd_list_device_add(&snd_list.playback, uL("default"), NULL, uL("System Default"));

	if (XAudio2Create(&ixa2, 0, XAUDIO2_DEFAULT_PROCESSOR) != S_OK) {
		return;
	}
	if (IXAudio2_GetDeviceCount(ixa2, &devcount) != S_OK) {
		IXAudio2_Release(ixa2);
		return;
	}

	for (i = 0; i < devcount; i++) {
		XAUDIO2_DEVICE_DETAILS details;

		if (IXAudio2_GetDeviceDetails(ixa2, i, &details) == S_OK) {
			if (ustrlen(details.DisplayName) == 0) {
				continue;
			}
			snd_list_device_add(&snd_list.playback, details.DeviceID, NULL, details.DisplayName);
		}
	}
	IXAudio2_Release(ixa2);

	// Capture devices
	if (ds8.available) {
		snd_list_device_add(&snd_list.capture, uL("default"), NULL, uL("System Default"));

		ds8.DirectSoundCaptureEnumerateW_proc(cb_enum_capture_dev, NULL);

		for (i = 0; i < snd_list.capture.count; i++) {
			wprintf(uL("%d : %s\n"), i, snd_list.capture.devices[i].desc);
		}
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


static int snd_list_find_index_id(_snd_list_dev *list, uTCHAR *id, int size) {
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
static void snd_list_device_add(_snd_list_dev *list, uTCHAR *id, GUID *guid, uTCHAR *desc) {
	_snd_dev *dev, *devs;

	if ((devs = (_snd_dev *) realloc(list->devices, (list->count + 1) * sizeof(_snd_dev)))) {
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

static BOOL CALLBACK cb_enum_capture_dev(LPGUID guid, LPCWSTR desc, LPCWSTR module, LPVOID data) {
	if ((guid != NULL) && (desc != NULL) && (ustrlen(desc) != 0)) {
		snd_list_device_add(&snd_list.capture, NULL, guid, (uTCHAR *) desc);
	}
	return (TRUE);
}

static void INLINE xaudio2_wrbuf(IXAudio2SourceVoice *source, XAUDIO2_BUFFER *x2buf,
		const BYTE *buffer) {
	x2buf->pAudioData = buffer;

	if(IXAudio2SourceVoice_SubmitSourceBuffer(source, x2buf, NULL) != S_OK) {
		fprintf(stderr, "Unable to submit source buffer\n");
	}
}

static void STDMETHODCALLTYPE OnVoiceProcessPassStart(THIS_ UINT32 b) {}
static void STDMETHODCALLTYPE OnVoiceProcessPassEnd(THIS) {}
static void STDMETHODCALLTYPE OnStreamEnd(THIS) {}
static void STDMETHODCALLTYPE OnBufferStart(THIS_ void *data) {}
static void STDMETHODCALLTYPE OnBufferEnd(THIS_ void *data) {
	_callback_data *cache = (_callback_data *) data;
	IXAudio2SourceVoice *source = (IXAudio2SourceVoice *) cache->xa2source;
	XAUDIO2_BUFFER *buffer = (XAUDIO2_BUFFER *) cache->xa2buffer;
	WORD len = buffer->AudioBytes;

	if (xaudio2.opened == FALSE) {
		return;
	}

	snd_lock_cache(cache);

	if ((info.no_rom | info.turn_off | info.pause) || (snd.buffer.start == FALSE)
			|| (fps.fast_forward == TRUE)) {
		xaudio2_wrbuf(source, buffer, (const BYTE *) cache->silence);
	} else if (cache->bytes_available < len) {
		xaudio2_wrbuf(source, buffer, (const BYTE *) cache->silence);
		snd.out_of_sync++;
	} else {
		xaudio2_wrbuf(source, buffer, (const BYTE *) cache->read);

		cache->bytes_available -= len;
		cache->samples_available -= (len / snd.channels / sizeof(*cache->write));

		// mi preparo per i prossimi frames da inviare, sempre
	 	// che non abbia raggiunto la fine del buffer, nel
	 	// qual caso devo puntare al suo inizio.
		if ((cache->read += len) >= cache->end) {
			cache->read = (SBYTE *) cache->start;
		}
	}

#if !defined (RELEASE)
	if ((gui_get_ms() - xaudio2.tick) >= 250.0f) {
		xaudio2.tick = gui_get_ms();
		if (info.snd_info == TRUE)
		fprintf(stderr, "snd : %6d %6d %6d %6d %d %f %3d %f\r",
			buffer->AudioBytes,
			fps.total_frames_skipped,
			cache->samples_available,
			cache->bytes_available,
			snd.out_of_sync,
			snd.frequency,
			(int) framerate.value,
			machine.ms_frame);
	}
#endif

	snd_unlock_cache(cache);
}
static void STDMETHODCALLTYPE OnLoopEnd(THIS_ void *data) {}
static void STDMETHODCALLTYPE OnVoiceError(THIS_ void* data, HRESULT Error) {}

#undef THIS
#undef THIS_

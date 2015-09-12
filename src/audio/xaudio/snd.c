/*
 * snd.c
 *
 *  Created on: 01/mar/2013
 *      Author: fhorse
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
#undef INITGUID

#if defined (THIS)
#undef THIS
#define THIS IXAudio2VoiceCallback *callback
#endif
#if defined (THIS_)
#undef THIS_
#define THIS_ IXAudio2VoiceCallback *callback,
#endif

static void STDMETHODCALLTYPE OnVoiceProcessPassStart(THIS_ UINT32 b);
static void STDMETHODCALLTYPE OnVoiceProcessPassEnd(THIS);
static void STDMETHODCALLTYPE OnStreamEnd(THIS);
static void STDMETHODCALLTYPE OnBufferStart(THIS_ void *data);
static void STDMETHODCALLTYPE OnBufferEnd(THIS_ void* data);
static void STDMETHODCALLTYPE OnLoopEnd(THIS_ void *data);
static void STDMETHODCALLTYPE OnVoiceError(THIS_ void* data, HRESULT Error);

static void INLINE wrbuf(IXAudio2SourceVoice *source, XAUDIO2_BUFFER *x2buf, const BYTE *buffer);

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

BYTE snd_init(void) {
	memset(&snd, 0x00, sizeof(_snd));
	memset(&xaudio2, 0x00, sizeof(xaudio2));

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
	int bsize, psize;

	if (!cfg->apu.channel[APU_MASTER]) {
		return (EXIT_OK);
	}

	// come prima cosa blocco eventuali riproduzioni
	snd_stop();

	memset(&snd, 0x00, sizeof(_snd));
	memset(&xaudio2, 0x00, sizeof(xaudio2));

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

	if (XAudio2Create(&xaudio2.engine, 0, XAUDIO2_DEFAULT_PROCESSOR) != S_OK) {
		MessageBox(NULL,
			"ATTENTION: Unable to create XAudio2 object. Probably you\n"
			"have an incomplete installation of DirectX 10."   ,
			"Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	if (IXAudio2_CreateMasteringVoice(xaudio2.engine, &xaudio2.master, snd.channels,
			snd.samplerate, 0, 0, NULL) != S_OK) {
		MessageBox(NULL,
			"ATTENTION: Unable to create XAudio2 master voice.",
			"Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
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
		double factor = (1.0f / 48000.0f) * (double) snd.samplerate;

		// buffer hardware
		bsize = ((1024 * factor) + ((512 * factor) * cfg->audio_buffer_factor));
		psize = bsize / snd.channels;
	}

	snd.samples = bsize * 2;
	snd.frequency = machine.cpu_hz / (double) snd.samplerate;

	xaudio2.opened = TRUE;

#if !defined (RELEASE)
	xaudio2.tick = gui_get_ms();
#endif

	{
		// dimensione in bytes del buffer
		snd.buffer.size = (bsize * snd.channels * sizeof(*cache->write)) * 5;

		snd.buffer.limit.low = (snd.buffer.size / 100) * 25;
		snd.buffer.limit.high = (snd.buffer.size / 100) * 55;

#if !defined (RELEASE)
		printf("softw bsize : %-6d - %-6d\n", snd.buffer.size, snd.samples);
		printf("softw limit : %-6d - %-6d\n", snd.buffer.limit.low, snd.buffer.limit.high);
#endif

		// alloco il buffer in memoria
		if (!(cache->start = (SWORD *) malloc(snd.buffer.size))) {
			MessageBox(NULL,
				"ATTENTION: Unable to allocate audio buffers.\n",
				"Error!",
				MB_ICONEXCLAMATION | MB_OK);
			return (EXIT_ERROR);
		}

		if (!(cache->silence = (SWORD *) malloc(snd.buffer.size))) {
			MessageBox(NULL,
				"ATTENTION: Unable to allocate silence buffer.\n",
				"Error!",
				MB_ICONEXCLAMATION | MB_OK);
			return (EXIT_ERROR);
		}

		// inizializzo il frame di scrittura
		cache->write = cache->start;
		// inizializzo il frame di lettura
		cache->read = (SBYTE *) cache->start;
		// punto alla fine del buffer
		cache->end = cache->read + snd.buffer.size;
		// creo il lock
		if ((xaudio2.semaphore = CreateSemaphore(NULL, 1, 2, NULL)) == NULL) {
			MessageBox(NULL,
				"ATTENTION: Unable to create XAudio2 semaphore.\n",
				"Error!",
				MB_ICONEXCLAMATION | MB_OK);
			return (EXIT_ERROR);
		}
		// azzero completamente i buffers
		memset(cache->start, 0x00, snd.buffer.size);
		// azzero completamente il buffer del silenzio
		memset(cache->silence, 0x00, snd.buffer.size);

		// azzero completamente il buffer
		memset(&xaudio2.buffer, 0x00, sizeof(xaudio2.buffer));

		xaudio2.buffer.AudioBytes = bsize * snd.channels * sizeof(*cache->write);
		xaudio2.buffer.pAudioData = (const BYTE *) cache->read;
		xaudio2.buffer.PlayBegin = 0;
		xaudio2.buffer.PlayLength = psize * snd.channels;
		xaudio2.buffer.LoopBegin = 0;
		xaudio2.buffer.LoopLength = 0;
		xaudio2.buffer.LoopCount = 0;
		xaudio2.buffer.pContext = snd.cache;

		cache->xa2buffer = &xaudio2.buffer;
		cache->xa2source = xaudio2.source;
		cache->lock = &xaudio2.semaphore;

		if (IXAudio2SourceVoice_SubmitSourceBuffer(xaudio2.source,
		        (const XAUDIO2_BUFFER *) cache->xa2buffer, NULL) != S_OK) {
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

		free(snd.cache);
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

	if ((info.no_rom | info.pause) || (snd.buffer.start == FALSE) || (fps.fast_forward == TRUE)) {
		wrbuf(source, buffer, (const BYTE *) cache->silence);
	} else if (cache->bytes_available < len) {
		wrbuf(source, buffer, (const BYTE *) cache->silence);
		snd.out_of_sync++;
	} else {
		wrbuf(source, buffer, (const BYTE *) cache->read);

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

static void INLINE wrbuf(IXAudio2SourceVoice *source, XAUDIO2_BUFFER *x2buf, const BYTE *buffer) {
	x2buf->pAudioData = buffer;

	if(IXAudio2SourceVoice_SubmitSourceBuffer(source, x2buf, NULL) != S_OK) {
		fprintf(stderr, "Unable to submit source buffer\n");
	}
}

/*
 * snd.c
 *
 *  Created on: 01/mar/2013
 *      Author: fhorse
 */

#include "snd.h"
#include "emu.h"
#include "cfg_file.h"
#include "audio_quality.h"
#include "fps.h"
#include "clock.h"
#include "apu.h"

#define INITGUID 1
#include <XAudio2.h>

#if defined THIS
#undef THIS
#define THIS IXAudio2VoiceCallback *callback
#endif

#if defined THIS_
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

struct _xaudio2 {
	IXAudio2 *engine;
	IXAudio2MasteringVoice *master;
	IXAudio2SourceVoice *source;
	XAUDIO2_BUFFER buffer;
	HANDLE semaphore;
} xaudio2;

BYTE snd_init(void) {
	_callback_data *cache;

	cfg->channels = MONO;
	//cfg->audio = 0;

	if (!cfg->audio) {
		return (EXIT_OK);
	}

	memset(&snd, 0x00, sizeof(snd));
	memset(&xaudio2, 0x00, sizeof(xaudio2));

	cache = malloc(sizeof(_callback_data));
	memset(cache, 0, sizeof(_callback_data));
	snd.cache = cache;

	switch (cfg->samplerate) {
		case S44100:
			snd.samplerate = 44100;
			snd.buffer.size = 512 * 8;
			break;
		case S22050:
			snd.samplerate = 22050;
			snd.buffer.size = 256 * 8;
			break;
		case S11025:
			snd.samplerate = 11025;
			snd.buffer.size = 128 * 8;
			break;
	}

	{
		double latency;
		double sample_latency;

		if (cfg->channels == STEREO) {
			//latency = 200.0f;
			latency = 400.0f;
		} else {
			latency = 400.0f;
		}

		sample_latency = latency * (double) snd.samplerate * (double) cfg->channels / 1000.0f;
		snd.buffer.count = sample_latency / snd.buffer.size;
	}

	if (CoInitializeEx(NULL, COINIT_MULTITHREADED) != S_OK) {
		fprintf(stderr, "Unable to initialize COM interface\n");
		return (EXIT_ERROR);
	}

	if (XAudio2Create(&xaudio2.engine, 0, XAUDIO2_DEFAULT_PROCESSOR) != S_OK) {
		fprintf(stderr, "Unable to create XAudio2 object\n");
		return (EXIT_ERROR);
	}

	if (IXAudio2_CreateMasteringVoice(xaudio2.engine, &xaudio2.master, cfg->channels,
			snd.samplerate, 0, 0, NULL) != S_OK) {
		fprintf(stderr, "Unable to create XAudio2 master voice\n");
		return (EXIT_ERROR);
	}

	{
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
		WAVEFORMATEX wfm;

		memset(&wfm, 0, sizeof(wfm));

		wfm.wFormatTag = WAVE_FORMAT_PCM;
		wfm.nChannels = cfg->channels;
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
			fprintf(stderr, "Unable to create XAudio2 source voice\n");
			return (EXIT_ERROR);
		}
	}

	snd.frequency = (fps.nominal * machine.cpu_cycles_frame) / (double) snd.samplerate;

	cache->samples = snd.buffer.size / cfg->channels;

	if (cfg->channels == STEREO) {
		BYTE i;

		snd.channel.max_pos = cache->samples * 0.300;

		snd.channel.pos = 0;

		for (i = 0; i < 2; i++) {
			DBWORD size = snd.channel.max_pos * sizeof(*cache->write);

			snd.channel.buf[i] = malloc(size);
			memset(snd.channel.buf[i], 0, size);
			snd.channel.ptr[i] = snd.channel.buf[i];
		}
	}

	snd_frequency(snd_factor[apu.type][SND_FACTOR_NORMAL])

	{
		/* dimensione in bytes del buffer */
    	DBWORD total_buffer_size = snd.buffer.size * snd.buffer.count * sizeof(*cache->write);

		printf("snd.buffer.size   : %d\n", snd.buffer.size);
		printf("snd.buffer.count  : %d\n", snd.buffer.count);
		printf("total_buffer_size : %d\n", total_buffer_size);

		/* alloco il buffer in memoria */
		cache->start = malloc(total_buffer_size);

		if (!cache->start) {
			fprintf(stderr, "Out of memory\n");
			return (EXIT_ERROR);
		}

		/* inizializzo il frame di scrittura */
		cache->write = cache->start;
		/* inizializzo il frame di lettura */
		cache->read = (SBYTE *) cache->start;
		/* punto alla fine del buffer */
		cache->end = cache->read + total_buffer_size;
		/* creo il lock */
		if ((xaudio2.semaphore = CreateSemaphore(NULL, 1, 2, NULL)) == NULL) {
			fprintf(stderr, "Unable to create XAudio2 semaphore\n");
			return (EXIT_ERROR);
		}
		/* azzero completamente il buffer */
		memset(cache->start, 0, total_buffer_size);

		/* punto all'inizio del frame di scrittura */
		snd.pos.current = snd.pos.last = 0;

		/* Submit the next filled buffer */
		memset(&xaudio2.buffer, 0x00, sizeof(xaudio2.buffer));

		xaudio2.buffer.AudioBytes = snd.buffer.size * sizeof(*cache->write);
		xaudio2.buffer.pAudioData = (const BYTE *) cache->read;
		xaudio2.buffer.PlayBegin = 0;
		xaudio2.buffer.PlayLength = cache->samples;
		xaudio2.buffer.LoopBegin = 0;
		xaudio2.buffer.LoopLength = 0;
		xaudio2.buffer.LoopCount = 0;
		xaudio2.buffer.pContext = snd.cache;

		cache->xa2buffer = &xaudio2.buffer;
		cache->xa2source = xaudio2.source;
		cache->lock = &xaudio2.semaphore;

		if(IXAudio2SourceVoice_SubmitSourceBuffer(xaudio2.source, cache->xa2buffer, NULL) != S_OK) {
			fprintf(stderr, "Unable to set sound engine\n");
			return (EXIT_ERROR);
		}
	}

	audio_quality(cfg->audio_quality);

	if(IXAudio2_StartEngine(xaudio2.engine) != S_OK) {
		fprintf(stderr, "Unable to start sound engine\n");
		return (EXIT_ERROR);
	}

	if(IXAudio2SourceVoice_Start(xaudio2.source, 0, XAUDIO2_COMMIT_NOW) != S_OK) {
		fprintf(stderr, "Unable to start source voice\n");
		return (EXIT_ERROR);
    }

	return (EXIT_OK);
}
BYTE snd_start(void) {
	return (EXIT_OK);
}
void snd_output(void *udata, BYTE *stream, int len) {
	return;
}
void snd_stop(void) {
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

	CoUninitialize();


	if (snd.cache) {
    	_callback_data *cache = snd.cache;

		if (cache->start) {
			free(cache->start);
		}

	    if (xaudio2.semaphore) {
	        CloseHandle(xaudio2.semaphore);
	        xaudio2.semaphore = NULL;
	    }

		free(snd.cache);
		snd.cache = NULL;
	}

	{
		BYTE i;

		for (i = 0; i < STEREO; i++) {
			/* rilascio la memoria */
			if (snd.channel.buf[i]) {
				free(snd.channel.buf[i]);
			}
			/* azzero i puntatori */
			snd.channel.ptr[i] =  snd.channel.buf[i] = NULL;
		}
	}

	if (audio_quality_quit) {
		audio_quality_quit();
	}
}
void snd_quit(void) {
	snd_stop();
}

void snd_lock_buffer(_callback_data *cache) {
	WaitForSingleObject((HANDLE **) cache->lock, INFINITE);
}
void snd_unlock_buffer(_callback_data *cache) {
	ReleaseSemaphore((HANDLE **) cache->lock, 1, NULL);
}

static void STDMETHODCALLTYPE OnVoiceProcessPassStart(THIS_ UINT32 b) {}
static void STDMETHODCALLTYPE OnVoiceProcessPassEnd(THIS) {}
static void STDMETHODCALLTYPE OnStreamEnd(THIS) {}
static void STDMETHODCALLTYPE OnBufferStart(THIS_ void *data) {}
static void STDMETHODCALLTYPE OnBufferEnd(THIS_ void *data) {
	_callback_data *cache = (_callback_data *) data;
	IXAudio2SourceVoice *source = cache->xa2source;
	XAUDIO2_BUFFER *buffer = cache->xa2buffer;
	WORD len = buffer->AudioBytes;

	if (info.no_rom) {
		return;
	}

	snd_lock_buffer(cache);

	if (!cache->filled) {
		snd.out_of_sync++;
	} else {
#ifndef RELEASE
		fprintf(stderr, "snd : %d %d %d %d %2d %d %f %f %4s\r",
				buffer->AudioBytes,
				snd.buffer.count,
				snd.brk,
				fps.total_frames_skipped,
				cache->filled,
				snd.out_of_sync,
				snd.frequency,
				machine.ms_frame,
				"");
#endif
		/*
	 	 * mi preparo per i prossimi frames da inviare, sempre
	 	 * che non abbia raggiunto la fine del buffer, nel
	 	 * qual caso devo puntare al suo inizio.
	 	 */
		if ((cache->read += len) >= cache->end) {
			cache->read = (SBYTE *) cache->start;
		}

		cache->filled--;
	}

	buffer->pAudioData = (const BYTE *) cache->read;

	if(IXAudio2SourceVoice_SubmitSourceBuffer(source, buffer, NULL) != S_OK) {
		fprintf(stderr, "Unable to submit source buffer\n");
	}

	snd_unlock_buffer(cache);
}
static void STDMETHODCALLTYPE OnLoopEnd(THIS_ void *data) {}
static void STDMETHODCALLTYPE OnVoiceError(THIS_ void* data, HRESULT Error) {}

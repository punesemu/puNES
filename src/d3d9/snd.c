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
} xaudio2;

BYTE snd_init(void) {
	cfg->channels = MONO;

	if (!cfg->audio) {
		return (EXIT_OK);
	}

	memset(&snd, 0x00, sizeof(snd));

	{
    	_callback_data *cache;

  		cache = malloc(sizeof(_callback_data));
		memset(cache, 0, sizeof(_callback_data));
		snd.cache = cache;
	}

	if (CoInitializeEx(NULL, COINIT_MULTITHREADED) != S_OK) {
		fprintf(stderr, "Unable to initialize COM interface\n");
		return (EXIT_ERROR);
	}

	if (XAudio2Create(&xaudio2.engine, 0, XAUDIO2_DEFAULT_PROCESSOR) != S_OK) {
		fprintf(stderr, "Unable to create XAudio2 object\n");
		return (EXIT_ERROR);
	}

	WORD factor = 8;

	switch (cfg->samplerate) {
		case S44100:
			snd.samplerate = 44100;
			snd.buffer.size = 512 * factor;
			break;
		case S22050:
			snd.samplerate = 22050;
			snd.buffer.size = 256 * factor;
			break;
		case S11025:
			snd.samplerate = 11025;
			snd.buffer.size = 128 * factor;
			break;
	}

	{
		WORD latency;
		long sample_latency;

		if (cfg->channels == STEREO) {
			latency = 200;
		} else {
			latency = 400;
		}

		sample_latency = latency * snd.samplerate * cfg->channels / 1000;
		snd.buffer.count = sample_latency / snd.buffer.size;
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

	snd.frequency = (fps.nominal * machine.cpu_cycles_frame) / snd.samplerate;
	//snd_frequency(snd_factor[apu.type][SND_FACTOR_NORMAL])

	{
    	_callback_data *cache = snd.cache;
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
		//cache->lock = SDL_CreateMutex();
		/* azzero completamente il buffer */
		memset(cache->start, 0, total_buffer_size);

		/* punto all'inizio del frame di scrittura */
		//snd.pos.current = snd.pos.last = 0;

		/* Submit the next filled buffer */
		memset(&xaudio2.buffer, 0x00, sizeof(xaudio2.buffer));

		xaudio2.buffer.AudioBytes = snd.buffer.size * sizeof(*cache->write);
		xaudio2.buffer.pAudioData = (const BYTE *) cache->read;
		xaudio2.buffer.PlayBegin = 0;
		xaudio2.buffer.PlayLength = snd.buffer.size / cfg->channels;
		xaudio2.buffer.LoopBegin = 0;
		xaudio2.buffer.LoopLength = 0;
		xaudio2.buffer.LoopCount = 0;
		xaudio2.buffer.pContext = snd.cache;

		cache->xa2buffer = &xaudio2.buffer;
		cache->xa2source = xaudio2.source;

		if(IXAudio2SourceVoice_SubmitSourceBuffer(xaudio2.source, cache->xa2buffer, NULL) != S_OK) {
			fprintf(stderr, "Unable to set sound engine\n");
			return (EXIT_ERROR);
		}

		cache->samples = xaudio2.buffer.PlayLength;
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
	_callback_data *cache = (_callback_data *) udata;
	IXAudio2SourceVoice *source = cache->xa2source;
	XAUDIO2_BUFFER *buffer = cache->xa2buffer;
	len = buffer->AudioBytes;

	if (info.no_rom) {
		return;
	}

	//SDL_mutexP(cache->lock);

	if (!cache->filled) {
	//	memset(stream, snd.last_sample, len);

		snd.out_of_sync++;
	} else {
//#ifndef RELEASE
		printf("5 %d %d %d\n", buffer->PlayLength, buffer->AudioBytes, cache->filled);

		//fprintf(stderr, "snd : %d %d %d %d %2d %d %f %f %4s\r", len, snd.buffer.count, snd.brk,
		//		fps.total_frames_skipped, cache->filled, snd.out_of_sync, snd.frequency,
		//		machine.ms_frame, "");
//#endif
		/* invio i samples richiesti alla scheda sonora */
		//memcpy(stream, cache->read, len);

		if (cache->read > (SBYTE *) cache->start) {
			/* salvo l'ultimo sample inviato */
			snd.last_sample = (SWORD) cache->read[len - 1];
		}

		/*
	 	 * mi preparo per i prossimi frames da inviare, sempre
	 	 * che non abbia raggiunto la fine del buffer, nel
	 	 * qual caso devo puntare al suo inizio.
	 	 */
		if ((cache->read += len) >= cache->end) {
			cache->read = (SBYTE *) cache->start;
		}

		//{
		//	SDL_AudioSpec *dev = snd.dev;

			/* decremento il numero dei frames pieni */
		//	cache->filled -= (((len / dev->channels) / sizeof(*cache->write)) / dev->samples);
		//}
	}

	//SDL_mutexV(cache->lock);

	buffer->pAudioData = (const BYTE *) cache->read;

	if(IXAudio2SourceVoice_SubmitSourceBuffer(source, buffer, NULL) != S_OK) {
		fprintf(stderr, "Unable to submit source buffer\n");
	}
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

		//if (cache->lock) {
			//SDL_DestroyMutex(cache->lock);
		//}

		free(snd.cache);
		snd.cache = NULL;
	}
}
void snd_quit(void) {
	snd_stop();
}

static void STDMETHODCALLTYPE OnVoiceProcessPassStart(THIS_ UINT32 b) {}
static void STDMETHODCALLTYPE OnVoiceProcessPassEnd(THIS) {}
static void STDMETHODCALLTYPE OnStreamEnd(THIS) {
	printf("3\n\n");
}
static void STDMETHODCALLTYPE OnBufferStart(THIS_ void *data) {
	printf("4\n");
}
static void STDMETHODCALLTYPE OnBufferEnd(THIS_ void *data) {
	snd_output(data, NULL, 0);
}
static void STDMETHODCALLTYPE OnLoopEnd(THIS_ void *data) {}
static void STDMETHODCALLTYPE OnVoiceError(THIS_ void* data, HRESULT Error) {}

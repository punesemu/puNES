/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include "libswresample/swresample.h"
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include "recording.h"
#include "thread_def.h"
#include "emu_thread.h"
#include "video/gfx_thread.h"
#include "video/gfx.h"
#include "clock.h"
#include "audio/snd.h"
#include "audio/channels.h"
#include "conf.h"
#include "settings.h"
#if defined (WITH_OPENGL)
#include "opengl.h"
#else
#endif
#include "gui.h"

typedef struct _ffmpeg_stream {
	int used;
	int encode;

	// video e audio
	AVCodecContext *avcc;
	AVCodec *avc;
	AVStream *avs;
	AVFrame *avf;
	int64_t pts;

	// video
	struct SwsContext *sws;

	// audio
	SwrContext *swr;
	WORD samples;
	SWORD *buffer;
	int64_t src_nb_samples;
} _ffmpeg_stream;

// Misc ---------------------------------------------------------------------------------------------------

INLINE static void ffmpeg_thread_lock(void);
INLINE static void ffmpeg_thread_unlock(void);

static void ffmpeg_fstream_set_default(_ffmpeg_stream *stream);
static void ffmpeg_fstream_close(_ffmpeg_stream *stream);
INLINE static BYTE ffmpeg_stream_write_frame(_ffmpeg_stream *fs);

static char *ffmpeg_av_make_error_string(int retcode);
static BYTE ffmpeg_context_setup(_recording_format_info *rfi, enum AVPixelFormat pixel_format);
static BYTE ffmpeg_stream_open(_ffmpeg_stream *fs, AVDictionary *opts, BYTE create_video_frame);

// Video --------------------------------------------------------------------------------------------------

static BYTE ffmpeg_video_add_stream(enum recording_format rf);
INLINE static BYTE ffmpeg_video_write_frame(int w, int h, int stride, uint8_t *rgb);

static AVFrame *ffmpeg_video_alloc_frame(enum AVPixelFormat pix_fmt, int width, int height);
static void ffmpeg_set_output_resolution(void);
static int ffmpeg_video_group_of_picture_size(void);
static void ffmpeg_video_mpeg_quality(_ffmpeg_stream *video);
static void ffmpeg_video_set_opt_vbr(AVDictionary *opts, const char *key, unsigned long vbr);
static void ffmpeg_video_set_opt_key(AVDictionary *opts, const char *key, unsigned long value);

static BYTE ffmpeg_video_add_stream_format_mpeg1(void);
static BYTE ffmpeg_video_add_stream_format_mpeg2(void);
static BYTE ffmpeg_video_add_stream_format_h264(void);
static BYTE ffmpeg_video_add_stream_format_mp4(void);
static BYTE ffmpeg_video_add_stream_format_hevc(void);
static BYTE ffmpeg_video_add_stream_format_webm(void);
static BYTE ffmpeg_video_add_stream_format_wmv(void);
static BYTE ffmpeg_video_add_stream_format_ffv(void);
static BYTE ffmpeg_video_add_stream_format_raw(void);
static BYTE ffmpeg_video_add_stream_format_audio(enum recording_format format);

// Audio --------------------------------------------------------------------------------------------------

static BYTE ffmpeg_audio_add_stream(void);
INLINE static BYTE ffmpeg_audio_write_frame(SWORD *data);

static AVFrame *ffmpeg_audio_alloc_frame(enum AVSampleFormat sample_fmt, uint64_t ch_layout, int samplerate, int nb_samples);
static enum AVSampleFormat ffmpeg_audio_select_sample_fmt(const AVCodec *codec);
static int ffmpeg_audio_select_samplerate(const AVCodec *codec);
static int ffmpeg_audio_select_channel_layout(const AVCodec *codec);

// --------------------------------------------------------------------------------------------------------

struct _ffmpeg {
	uTCHAR *filename;
	int format_type;
	AVFormatContext *format_ctx;

	thread_mutex_t lock;

	// video
	_ffmpeg_stream video;
	int w, h;
	int fps;

	// audio
	_ffmpeg_stream audio;
	BYTE disable_logo;
} ffmpeg;
_recording_format_info recording_format_info[REC_FORMAT_TOTAL] = {
	{ FALSE, "mpeg"    , { "mpg" , "mpeg", "end" } , REC_FORMAT_VIDEO, NULL, { "mpeg1video", "end" } },
	{ FALSE, "mpeg"    , { "mpg" , "mpeg", "end" } , REC_FORMAT_VIDEO, NULL, { "mpeg2video", "end" } },
	{ FALSE, "mp4"     , { "mp4" , "end" }         , REC_FORMAT_VIDEO, NULL, { "mpeg4", "msmpeg4", "libxvid", "end" } },
	{ FALSE, "mp4"     , { "mp4" , "end" }         , REC_FORMAT_VIDEO, NULL, { "h264_nvenc", "libx264", "h264_omx", "end" } },
	{ FALSE, "matroska", { "mkv" , "end" }         , REC_FORMAT_VIDEO, NULL, { "hevc_nvenc", "hevc_vaapi", "libx265", "end" } },
	{ FALSE, "webm"    , { "webm", "end" }         , REC_FORMAT_VIDEO, NULL, { "libvpx-vp9", "libvpx", "end" } },
	{ FALSE, "avi"     , { "wmv" , "end" }         , REC_FORMAT_VIDEO, NULL, { "wmv2", "wmv1", "end" } },
	{ FALSE, "avi"     , { "avi" , "end" }         , REC_FORMAT_VIDEO, NULL, { "ffv1", "end" } },
	{ FALSE, "avi"     , { "avi" , "end" }         , REC_FORMAT_VIDEO, NULL, { "rawvideo", "end" } },
	{ FALSE, "wav"     , { "wav" , "end" }         , REC_FORMAT_AUDIO, NULL, { "pcm_s16le", "end" } },
	{ FALSE, "mp3"     , { "mp3" , "end" }         , REC_FORMAT_AUDIO, NULL, { "libmp3lame", "end" } },
	{ FALSE, "adts"    , { "aac" , "end" }         , REC_FORMAT_AUDIO, NULL, { "aac", "end" } },
	{ FALSE, "flac"    , { "flac", "end" }         , REC_FORMAT_AUDIO, NULL, { "flac", "end" } },
	{ FALSE, "ogg"     , { "ogg" , "end" }         , REC_FORMAT_AUDIO, NULL, { "libvorbis", "end" } },
};

void recording_init(void) {
	int a;

	memset(&ffmpeg, 0x00, sizeof(ffmpeg));

	info.recording_on_air = FALSE;
	info.recording_is_a_video = FALSE;

	// controllo la presenze dei codec
	for (a = 0; a < REC_FORMAT_TOTAL; a++) {
		_recording_format_info *rfi = &recording_format_info[a];
		AVCodec *avc = NULL;
		int b = 0;

		rfi->codec = rfi->codecs_list[0];

		while (strcmp(rfi->codec, "end") != 0) {
			if ((avc = avcodec_find_encoder_by_name(rfi->codec)) != NULL) {
				break;
			}
			rfi->codec = rfi->codecs_list[++b];
		}

		if (avc == NULL) {
			rfi->present = FALSE;
			rfi->codec = NULL;
		} else {
			rfi->present = TRUE;
		}
	}

	if (thread_mutex_init_error(ffmpeg.lock)) {
		fprintf(stderr, "Unable to allocate the recording mutex\n");
		return;
	}
}
void recording_quit(void) {
	if (info.recording_on_air) {
		recording_finish(TRUE);
	}
	thread_mutex_destroy(ffmpeg.lock);
}

void recording_start(uTCHAR *filename, int format) {
	enum recording_format rf = (enum recording_format)format;
	int ret;

	emu_thread_pause();
	gfx_thread_pause();

	ffmpeg.filename = filename;
	ffmpeg.format_ctx = NULL;
	ffmpeg.fps = machine.fps;
	ffmpeg.disable_logo = TRUE;

	ffmpeg_set_output_resolution();

	ffmpeg_fstream_set_default(&ffmpeg.video);
	ffmpeg_fstream_set_default(&ffmpeg.audio);

	if (ffmpeg_video_add_stream(rf) == EXIT_ERROR) {
		ffmpeg_fstream_close(&ffmpeg.video);
	}
	if (ffmpeg_audio_add_stream() == EXIT_ERROR) {
		ffmpeg_fstream_close(&ffmpeg.audio);
	}

	if ((ffmpeg.video.used | ffmpeg.audio.used) == FALSE) {
		goto recording_start_end;
		return;
	}

	av_dump_format(ffmpeg.format_ctx, 0, ffmpeg.format_ctx->url, 1);

	if ((ret = avio_open(&ffmpeg.format_ctx->pb, ffmpeg.format_ctx->url, AVIO_FLAG_WRITE) < 0)) {
		fprintf(stderr, "cannot open file : %s.\n", ffmpeg_av_make_error_string(ret));
		goto recording_start_end;
		return;
	}

	if ((ret = avformat_write_header(ffmpeg.format_ctx, NULL) < 0)) {
		fprintf(stderr, "cannot write header file : %s.\n", ffmpeg_av_make_error_string(ret));
		goto recording_start_end;
		return;
	}

	info.recording_on_air = TRUE;
	ffmpeg.format_type = recording_format_info[rf].format_type;
	info.recording_is_a_video = (ffmpeg.format_type == REC_FORMAT_VIDEO);
	cfg->recording.last_type = ffmpeg.format_type;

	gui_update_recording_widgets();
	gui_update_apu_channels_widgets();

	settings_save_GUI();

	recording_start_end:
	gfx_thread_continue();
	emu_thread_continue();
}
void recording_finish(BYTE from_quit) {
	int ret = 0;

	if (from_quit == FALSE) {
		emu_thread_pause();
		gfx_thread_pause();
	}

	if (ffmpeg.video.encode) {
		ffmpeg_video_write_frame(0, 0, 0, NULL);
	}
	if (ffmpeg.audio.encode) {
		ffmpeg_audio_write_frame(NULL);
	}

	if ((ret = av_write_trailer(ffmpeg.format_ctx)) < 0) {
		fprintf(stderr, "error on write traile : %s\n", ffmpeg_av_make_error_string(ret));
	}

	if (ffmpeg.video.used) {
		ffmpeg_fstream_close(&ffmpeg.video);
	}
	if (ffmpeg.audio.used) {
		ffmpeg_fstream_close(&ffmpeg.audio);
	}

	if ((ret = avio_close(ffmpeg.format_ctx->pb)) < 0) {
		fprintf(stderr, "error on close file : %s\n", ffmpeg_av_make_error_string(ret));
	}

	if (ffmpeg.format_ctx) {
		if (ffmpeg.format_ctx->url) {
#if defined (_WIN32)
			free(ffmpeg.format_ctx->url);
#else
			av_free(ffmpeg.format_ctx->url);
#endif
		}
		ffmpeg.format_ctx->url = NULL;
		avformat_free_context(ffmpeg.format_ctx);
	}

	info.recording_on_air = FALSE;
	info.recording_is_a_video = FALSE;
	ffmpeg.format_type = REC_FORMAT_NONE;

	gui_update_recording_widgets();
	gui_update_apu_channels_widgets();

	if (from_quit == FALSE) {
		gfx_thread_continue();
		emu_thread_continue();
	}
}

void recording_video_frame(int w, int h, int stride, uint8_t *rgb) {
	if (ffmpeg.video.encode) {
		ffmpeg.video.encode = !ffmpeg_video_write_frame(w, h, stride, rgb);
	}
}

void recording_audio_tick(SWORD *data) {
	if (ffmpeg.audio.encode) {
		ffmpeg.audio.encode = !ffmpeg_audio_write_frame(data);
	}
}
void recording_audio_silenced_frame(void) {
	static const SWORD actual[2] = { 0, 0 };
	int i, samples = snd.samplerate / machine.fps;

	for (i = 0; i < samples; i++) {
		recording_audio_tick((SWORD *)&actual[0]);
	}
}

int recording_format_type(void) {
	return (ffmpeg.format_type);
}
void recording_decode_output_resolution(int *w, int *h) {
	switch (cfg->recording.output_resolution) {
		case REC_RES_CUSTOM:
			(*w) = 0;
			(*h) = 0;
			break;
		case REC_RES_256x240:
			(*w) = 256;
			(*h) = 240;
			break;
		case REC_RES_292x240:
			(*w) = 292;
			(*h) = 240;
			break;
		case REC_RES_320x240:
			(*w) = 320;
			(*h) = 240;
			break;
		case REC_RES_354x240:
			(*w) = 354;
			(*h) = 240;
			break;
		case REC_RES_512x480:
			(*w) = 512;
			(*h) = 480;
			break;
		case REC_RES_584x480:
			(*w) = 584;
			(*h) = 480;
			break;
		case REC_RES_640x480:
			(*w) = 640;
			(*h) = 480;
			break;
		case REC_RES_708x480:
			(*w) = 708;
			(*h) = 480;
			break;
		case REC_RES_768x720:
			(*w) = 768;
			(*h) = 720;
			break;
		case REC_RES_876x720:
			(*w) = 876;
			(*h) = 720;
			break;
		case REC_RES_960x720:
			(*w) = 960;
			(*h) = 720;
			break;
		case REC_RES_1064x720:
			(*w) = 1064;
			(*h) = 720;
			break;
		case REC_RES_1024x960:
			(*w) = 1024;
			(*h) = 960;
			break;
		case REC_RES_1170x960:
			(*w) = 1170;
			(*h) = 960;
			break;
		case REC_RES_1280x960:
			(*w) = 1280;
			(*h) = 960;
			break;
		case REC_RES_1418x960:
			(*w) = 1418;
			(*h) = 960;
			break;
		case REC_RES_1280x720:
			(*w) = 1280;
			(*h) = 720;
			break;
		case REC_RES_1920x1080:
			(*w) = 1920;
			(*h) = 1080;
			break;
	}
}

// Misc ---------------------------------------------------------------------------------------------------

INLINE static void ffmpeg_thread_lock(void) {
	thread_mutex_lock(ffmpeg.lock);
}
INLINE static void ffmpeg_thread_unlock(void) {
	thread_mutex_unlock(ffmpeg.lock);
}

static void ffmpeg_fstream_set_default(_ffmpeg_stream *stream) {
	stream->used = FALSE;
	stream->encode = FALSE;

	// video e audio
	stream->avcc = NULL;
	stream->avc = NULL;
	stream->avs = NULL;
	stream->avf = NULL;
	stream->pts = 0;

	// video
	stream->sws = NULL;

	// audio
	stream->samples = 0;
	stream->buffer = NULL;
	stream->src_nb_samples = 0;
	stream->swr = NULL;
}
static void ffmpeg_fstream_close(_ffmpeg_stream *fs) {
	fs->used = FALSE;
	fs->encode = FALSE;
	fs->avc = NULL;
	if (fs->avcc) {
		avcodec_close(fs->avcc);
		avcodec_free_context(&fs->avcc);
	}
	if (fs->avf) {
		av_frame_unref(fs->avf);
		av_frame_free(&fs->avf);
	}
	if (fs->sws) {
		sws_freeContext(fs->sws);
	}
	if (fs->swr) {
		swr_free(&fs->swr);

		if (fs->buffer) {
			free(fs->buffer);
			fs->buffer = NULL;
		}
	}
}
INLINE static BYTE ffmpeg_stream_write_frame(_ffmpeg_stream *fs) {
	AVPacket *pkt = av_packet_alloc();
	BYTE rc = EXIT_OK;
	int ret = 0;

	if (pkt == NULL) {
		return (EXIT_ERROR);
	}

	while (ret >= 0) {
		ret = avcodec_receive_packet(fs->avcc, pkt);

		if (ret == AVERROR(EAGAIN)) {
			continue;
		}
		if (ret == AVERROR_EOF) {
			rc = EXIT_ERROR;
			break;
		}
		if (ret < 0) {
			if (fs->avc->type == AVMEDIA_TYPE_VIDEO) {
				fprintf(stderr, "Error encoding video frame : %s.\n", ffmpeg_av_make_error_string(ret));
			} else {
				fprintf(stderr, "Error encoding audio frame : %s.\n", ffmpeg_av_make_error_string(ret));
			}
			rc = EXIT_ERROR;
			break;
		}

		av_packet_rescale_ts(pkt, fs->avcc->time_base, fs->avs->time_base);
		pkt->stream_index = fs->avs->index;

		ffmpeg_thread_lock();
		ret = av_interleaved_write_frame(ffmpeg.format_ctx, pkt);
		ffmpeg_thread_unlock();

		if (ret < 0) {
			if (fs->avc->type == AVMEDIA_TYPE_VIDEO) {
				fprintf(stderr, "Error while writing video frame : %s.\n", ffmpeg_av_make_error_string(ret));
			} else {
				fprintf(stderr, "Error while writing audio frame : %s.\n", ffmpeg_av_make_error_string(ret));
			}
			rc = EXIT_ERROR;
			break;
		}
	}
	av_packet_free(&pkt);
	return (rc);
}

static char *ffmpeg_av_make_error_string(int retcode) {
	static char buffer[256];

	return (av_make_error_string(buffer, sizeof(buffer), retcode));
}
static BYTE ffmpeg_context_setup(_recording_format_info *rfi, enum AVPixelFormat pixel_format) {
	_ffmpeg_stream *video = &ffmpeg.video;

	// alloco il contesto del formato
	if (!(ffmpeg.format_ctx = avformat_alloc_context())) {
		fprintf(stderr, "cannot allocate format context.\n");
		return (EXIT_ERROR);
	}

	// memorizzo il nome del file di output
#if defined (_WIN32)
	ffmpeg.format_ctx->url = gui_dup_wchar_to_utf8(ffmpeg.filename);
#else
	{
		size_t size = LENGTH_FILE_NAME_LONG * sizeof(uTCHAR);

		ffmpeg.format_ctx->url = av_malloc(size);
		memset(ffmpeg.format_ctx->url, 0x00, size);
		ffmpeg.format_ctx->url = av_strdup(ffmpeg.filename);
	}
#endif

	// configuro il contesto
	if (!(ffmpeg.format_ctx->oformat = av_guess_format(rfi->format, NULL, NULL))) {
		fprintf(stderr, "format not found.\n");
		return (EXIT_ERROR);
	}

	// controllo se il formato non supporta un codec video
	if (ffmpeg.format_ctx->oformat->video_codec == AV_CODEC_ID_NONE) {
		fprintf(stderr, "video codec unavailable.\n");
		return (EXIT_OK);
	}

	if (rfi->format_type == REC_FORMAT_AUDIO) {
		if (ffmpeg.disable_logo == TRUE) {
			return (EXIT_ERROR);
		}
		if (ffmpeg.format_ctx->oformat->video_codec == AV_CODEC_ID_THEORA) {
			return (EXIT_ERROR);
		}
		video->avc = avcodec_find_encoder(ffmpeg.format_ctx->oformat->video_codec);
	} else {
		video->avc = avcodec_find_encoder_by_name(rfi->codec);
	}

	// controllo se e' stato trovato un codec video
	if (video->avc == NULL) {
		fprintf(stderr, "video codec not found.\n");
		return (EXIT_ERROR);
	}

	// creo lo stream video
	if (!(video->avs = avformat_new_stream(ffmpeg.format_ctx, video->avc))) {
		fprintf(stderr, "cannot allocate video stream.\n");
		return (EXIT_ERROR);
	}

	video->avs->id = ffmpeg.format_ctx->nb_streams - 1;
	video->avs->time_base = av_make_q(1, ffmpeg.fps);

	// alloco il contesto del codec video
	if (!(video->avcc = avcodec_alloc_context3(video->avc))) {
		fprintf(stderr, "cannot allocate video codec context.\n");
		return (EXIT_ERROR);
	}

	// configuro il codec
	video->avcc->codec_type = AVMEDIA_TYPE_VIDEO;
	video->avcc->codec_id = video->avc->id;
	video->avcc->pix_fmt = pixel_format;
	video->avcc->width = ffmpeg.w;
	video->avcc->height = ffmpeg.h;
	video->avcc->framerate.num = ffmpeg.fps;
	video->avcc->framerate.den = 1;
	video->avcc->time_base.num = 1;
	video->avcc->time_base.den = ffmpeg.fps;
	video->avcc->gop_size = ffmpeg_video_group_of_picture_size();

	if (ffmpeg.format_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
		video->avcc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	return (EXIT_OK);
}
static BYTE ffmpeg_stream_open(_ffmpeg_stream *fs, AVDictionary *opts, BYTE create_video_frame) {
	int ret;

	if (fs->avc == NULL) {
		return (EXIT_ERROR);
	}

	// apro il codec
	if ((ret = avcodec_open2(fs->avcc, fs->avc, &opts) < 0)) {
		fprintf(stderr, "cannot open codec %s.\n", ffmpeg_av_make_error_string(ret));
		return (EXIT_ERROR);
	}

	// libero la memoria occupata dalle opzioni
	av_dict_free(&opts);
	opts = NULL;

	if ((ret = avcodec_parameters_from_context(fs->avs->codecpar, fs->avcc) < 0)) {
		fprintf(stderr, "%s.\n", ffmpeg_av_make_error_string(ret));
		return (EXIT_ERROR);
	}

	// solo in caso di video
	if (create_video_frame == TRUE) {
		_ffmpeg_stream *video = &ffmpeg.video;

		if (!(video->avf = ffmpeg_video_alloc_frame(video->avcc->pix_fmt, video->avcc->width, video->avcc->height))) {
			fprintf(stderr, "Could not allocate picture\n");
			return (EXIT_ERROR);
		}
	}

	return (EXIT_OK);
}

// Video --------------------------------------------------------------------------------------------------

static BYTE ffmpeg_video_add_stream(enum recording_format rf) {
	_ffmpeg_stream *video = &ffmpeg.video;
	BYTE ret = EXIT_OK;

	switch (rf) {
		case REC_FORMAT_VIDEO_MPG_MPEG1:
			ret = ffmpeg_video_add_stream_format_mpeg1();
			break;
		case REC_FORMAT_VIDEO_MPG_MPEG2:
			ret = ffmpeg_video_add_stream_format_mpeg2();
			break;
		case REC_FORMAT_VIDEO_MP4_MPEG4:
			ret = ffmpeg_video_add_stream_format_mp4();
			break;
		case REC_FORMAT_VIDEO_MP4_H264:
			ret = ffmpeg_video_add_stream_format_h264();
			break;
		case REC_FORMAT_VIDEO_MKV_HEVC:
			ret = ffmpeg_video_add_stream_format_hevc();
			break;
		case REC_FORMAT_VIDEO_WEB_WEBM:
			ret = ffmpeg_video_add_stream_format_webm();
			break;
		case REC_FORMAT_VIDEO_AVI_WMV:
			ret = ffmpeg_video_add_stream_format_wmv();
			break;
		case REC_FORMAT_VIDEO_AVI_FFV:
			ret = ffmpeg_video_add_stream_format_ffv();
			break;
		case REC_FORMAT_VIDEO_AVI_RAW:
			ret = ffmpeg_video_add_stream_format_raw();
			break;
		case REC_FORMAT_AUDIO_WAV:
		case REC_FORMAT_AUDIO_MP3:
		case REC_FORMAT_AUDIO_AAC:
		case REC_FORMAT_AUDIO_FLAC:
		case REC_FORMAT_AUDIO_OGG:
			ret = ffmpeg_video_add_stream_format_audio(rf);
			break;
		default:
			fprintf(stderr, "Encoding format not supported.\n");
			ret = EXIT_ERROR;
			break;
	}

	if (ret == EXIT_OK) {
		video->used = TRUE;
		video->encode = TRUE;
	}

	return (ret);
}
INLINE static BYTE ffmpeg_video_write_frame(int w, int h, int stride, uint8_t *rgb) {
	_ffmpeg_stream *video = &ffmpeg.video;
	AVFrame *frame = NULL;
	int ret = 0;

	if (rgb) {
#if defined (WITH_OPENGL)
		uint8_t *in_data[4] = { rgb + ((h - 1) * stride), 0, 0, 0 };
		int in_linesize[4] = { -stride, 0, 0, 0 };
#else
		uint8_t *in_data[4] = { rgb, 0, 0, 0 };
		int in_linesize[4] = { stride, 0, 0, 0 };
#endif

		video->sws = sws_getCachedContext(video->sws,
			w, h, AV_PIX_FMT_BGRA,
			video->avcc->width, video->avcc->height, video->avcc->pix_fmt,
			SWS_BICUBIC, NULL, NULL, NULL);

		sws_scale(video->sws, (const uint8_t *const *)in_data, in_linesize, 0, h, video->avf->data, video->avf->linesize);

		if (video->pts == 0) {
			video->pts = gfx.frame.in_draw;
		}
		video->avf->pts = gfx.frame.in_draw - video->pts;
		frame = video->avf;
	}

	if ((ret = avcodec_send_frame(video->avcc, frame)) < 0) {
		fprintf(stderr, "Error submitting a video frame for encoding: %s.\n", ffmpeg_av_make_error_string(ret));
		return (EXIT_ERROR);
	}

	return (ffmpeg_stream_write_frame(video));
}

static AVFrame *ffmpeg_video_alloc_frame(enum AVPixelFormat pix_fmt, int width, int height) {
	AVFrame *avframe;

	if (!(avframe = av_frame_alloc())) {
		return (NULL);
	}

	avframe->format = pix_fmt;
	avframe->width = width;
	avframe->height = height;

	if (av_frame_get_buffer(avframe, 32) < 0) {
		fprintf(stderr, "Could not allocate frame data.\n");
		return (NULL);
	}

	return (avframe);
}
static void ffmpeg_set_output_resolution(void) {
	if (cfg->recording.use_emu_resolution == TRUE) {
		ffmpeg.w = gfx.w[VIDEO_MODE];
		ffmpeg.h = gfx.h[VIDEO_MODE];
	} else if (cfg->recording.output_resolution == REC_RES_CUSTOM) {
		ffmpeg.w = cfg->recording.output_custom_w;
		ffmpeg.h = cfg->recording.output_custom_h;
	} else {
		recording_decode_output_resolution(&ffmpeg.w, &ffmpeg.h);
	}
	if (cfg->recording.follow_rotation == TRUE) {
		if (!cfg->fullscreen && ((cfg->screen_rotation == ROTATE_90) || (cfg->screen_rotation == ROTATE_270))) {
			int tmp = ffmpeg.w;

			ffmpeg.w = ffmpeg.h;
			ffmpeg.h = tmp;
		}
	}
	ffmpeg.w = (ffmpeg.w / 2) * 2;
	ffmpeg.h = (ffmpeg.h / 2) * 2;

}
static int ffmpeg_video_group_of_picture_size(void) {
	// http://www2.acti.com/download_file/Product/support/DesignSpec_Note_GOP_20091120.pdf
	if (ffmpeg.fps < 5) {
		return (24);
	} else if (ffmpeg.fps < 15) {
		return (60);
	} else {
		return (ffmpeg.fps * 4);
	}
}
static void ffmpeg_video_mpeg_quality(_ffmpeg_stream *video) {
	switch (cfg->recording.quality) {
		case REC_QUALITY_LOW:
			video->avcc->qmin = 28;
			video->avcc->qmax = 31;
			break;
		case REC_QUALITY_MEDIUM:
			video->avcc->qmin = 18;
			video->avcc->qmax = 21;
			break;
		case REC_QUALITY_HIGH:
			video->avcc->qmin = 1;
			video->avcc->qmax = 4;
			break;
	}
	video->avcc->qcompress = 0.8f;
}
static void ffmpeg_video_set_opt_vbr(AVDictionary *opts, const char *key, unsigned long vbr) {
	vbr = (int)(((100 - vbr) * 51) / 100);
	ffmpeg_video_set_opt_key(opts, key, vbr);
}
static void ffmpeg_video_set_opt_key(AVDictionary *opts, const char *key, unsigned long value) {
	char buffer[10];

	snprintf(buffer, sizeof(buffer), "%d", (int)value);
	av_dict_set(&opts, key, buffer, 0);
}

static BYTE ffmpeg_video_add_stream_format_mpeg1(void) {
	_ffmpeg_stream *video = &ffmpeg.video;
	_recording_format_info *rfi = &recording_format_info[REC_FORMAT_VIDEO_MPG_MPEG1];
	enum AVPixelFormat dst_pixel_format = AV_PIX_FMT_YUV420P;

	// configuro il contesto
	if (ffmpeg_context_setup(rfi, dst_pixel_format) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	ffmpeg_video_mpeg_quality(video);

	// setto il VBV buffer
	{
		AVCPBProperties *props;

		props = (AVCPBProperties *)av_stream_new_side_data(video->avs, AV_PKT_DATA_CPB_PROPERTIES, sizeof(*props));
		props->buffer_size = 7360 * 1024;
		props->max_bitrate = 0;
		props->min_bitrate = 0;
		props->avg_bitrate = 0;
		props->vbv_delay = UINT64_MAX;
	}

	video->avcc->max_b_frames = 1;
	video->avcc->mb_decision = 2;
	video->avcc->thread_count = FFMIN(8, gui_hardware_concurrency());

	if (ffmpeg_stream_open(video, NULL, TRUE) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
static BYTE ffmpeg_video_add_stream_format_mpeg2(void) {
	_ffmpeg_stream *video = &ffmpeg.video;
	_recording_format_info *rfi = &recording_format_info[REC_FORMAT_VIDEO_MPG_MPEG2];
	enum AVPixelFormat dst_pixel_format = AV_PIX_FMT_YUV420P;

	// configuro il contesto
	if (ffmpeg_context_setup(rfi, dst_pixel_format) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	ffmpeg_video_mpeg_quality(video);

	// setto il VBV buffer
	{
		AVCPBProperties *props;

		props = (AVCPBProperties *)av_stream_new_side_data(video->avs, AV_PKT_DATA_CPB_PROPERTIES, sizeof(*props));
		props->buffer_size = 7360 * 1024;
		props->max_bitrate = 0;
		props->min_bitrate = 0;
		props->avg_bitrate = 0;
		props->vbv_delay = UINT64_MAX;
	}

	video->avcc->max_b_frames = 2;
	video->avcc->thread_count = FFMIN(8, gui_hardware_concurrency());

	if (ffmpeg_stream_open(video, NULL, TRUE) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
static BYTE ffmpeg_video_add_stream_format_mp4(void) {
	_ffmpeg_stream *video = &ffmpeg.video;
	_recording_format_info *rfi = &recording_format_info[REC_FORMAT_VIDEO_MP4_MPEG4];
	enum AVPixelFormat dst_pixel_format = AV_PIX_FMT_YUV420P;

	// configuro il contesto
	if (ffmpeg_context_setup(rfi, dst_pixel_format) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	ffmpeg_video_mpeg_quality(video);

	video->avcc->thread_count = FFMIN(8, gui_hardware_concurrency());

	if (ffmpeg_stream_open(video, NULL, TRUE) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
static BYTE ffmpeg_video_add_stream_format_h264(void) {
	_ffmpeg_stream *video = &ffmpeg.video;
	_recording_format_info *rfi = &recording_format_info[REC_FORMAT_VIDEO_MP4_H264];
	enum AVPixelFormat dst_pixel_format = AV_PIX_FMT_YUV420P;
	AVDictionary *opts = NULL;
	unsigned long vbr = 54;

	switch (cfg->recording.quality) {
		case REC_QUALITY_LOW:
			vbr = 40;
			break;
		case REC_QUALITY_MEDIUM:
			vbr = 64;
			break;
		case REC_QUALITY_HIGH:
			vbr = 90;
			dst_pixel_format = AV_PIX_FMT_YUV444P;
			break;
	}

	// configuro il contesto
	if (ffmpeg_context_setup(rfi, dst_pixel_format) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	if ((strcmp(video->avc->name, "libx264") == 0)) {
		// htps://trac.ffmpeg.org/wiki/Encode/H.264
		av_dict_set(&opts, "preset", "ultrafast", 0);
		av_dict_set(&opts, "tune", "zerolatency", 0);
		if (cfg->recording.quality == REC_QUALITY_HIGH) {
			av_dict_set(&opts, "profile", "high444", 0);
		}
		ffmpeg_video_set_opt_vbr(opts, "crf", vbr);
	} else if ((strcmp(video->avc->name, "h264_nvenc") == 0)) {
		// https://superuser.com/questions/1296374/best-settings-for-ffmpeg-with-nvenc
		av_dict_set(&opts, "preset", "3", 0);
		av_dict_set(&opts, "zerolatency", "1", 0);
		if (cfg->recording.quality == REC_QUALITY_HIGH) {
			av_dict_set(&opts, "profile", "3", 0);
		}
		av_dict_set(&opts, "rc", "0", 0);
		ffmpeg_video_set_opt_vbr(opts, "qp", vbr);
	} else if ((strcmp(video->avc->name, "h264_omx") == 0)) {
		vbr = (ffmpeg.w * ffmpeg.h * ffmpeg.fps * vbr) >> 7;
		if (vbr < 4000) {
			vbr = 4000;
		}
		video->avcc->profile = FF_PROFILE_H264_HIGH;
		video->avcc->bit_rate = vbr;
	}

	video->avcc->thread_count = FFMIN(8, gui_hardware_concurrency());

	if (ffmpeg_stream_open(video, opts, TRUE) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
static BYTE ffmpeg_video_add_stream_format_hevc(void) {
	_ffmpeg_stream *video = &ffmpeg.video;
	_recording_format_info *rfi = &recording_format_info[REC_FORMAT_VIDEO_MKV_HEVC];
	enum AVPixelFormat dst_pixel_format = AV_PIX_FMT_YUV420P;
	AVDictionary *opts = NULL;
	unsigned long vbr = 54;

	switch (cfg->recording.quality) {
		case REC_QUALITY_LOW:
			vbr = 40;
			break;
		case REC_QUALITY_MEDIUM:
			vbr = 64;
		break;
		case REC_QUALITY_HIGH:
			vbr = 90;
			av_dict_set(&opts, "profile", "2", 0);
			break;
	}

	// configuro il contesto
	if (ffmpeg_context_setup(rfi, dst_pixel_format) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	if ((strcmp(video->avc->name, "libx265") == 0)) {
		// https://trac.ffmpeg.org/wiki/Encode/H.265
		av_dict_set(&opts, "preset", "ultrafast", 0);
		av_dict_set(&opts, "tune", "zerolatency", 0);
		// configuro il constant rate factor
		ffmpeg_video_set_opt_vbr(opts, "crf", vbr);
	} else if ((strcmp(video->avc->name, "hevc_nvenc") == 0)) {
		av_dict_set(&opts, "preset", "3", 0);
		av_dict_set(&opts, "zerolatency", "1", 0);
		av_dict_set(&opts, "rc", "0", 0);
		ffmpeg_video_set_opt_vbr(opts, "qp", vbr);
	}
	video->avcc->thread_count = FFMIN(8, gui_hardware_concurrency());

	if (ffmpeg_stream_open(video, opts, TRUE) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
static BYTE ffmpeg_video_add_stream_format_webm(void) {
	_ffmpeg_stream *video = &ffmpeg.video;
	_recording_format_info *rfi = &recording_format_info[REC_FORMAT_VIDEO_WEB_WEBM];
	enum AVPixelFormat dst_pixel_format = AV_PIX_FMT_YUV420P;
	AVDictionary *opts = NULL;
	// seleziono in constant quality (CQ) (https://developers.google.com/media/vp9/settings/vod/)
	int crf = 31;

	switch (cfg->recording.quality) {
		case REC_QUALITY_LOW:
			crf = 35;
			break;
		case REC_QUALITY_MEDIUM:
			crf = 25;
			break;
		case REC_QUALITY_HIGH:
			crf = 18;
			dst_pixel_format = AV_PIX_FMT_YUV444P;
			break;
	}

	// configuro il contesto
	if (ffmpeg_context_setup(rfi, dst_pixel_format) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	// https://trac.ffmpeg.org/wiki/Encode/VP9
	av_dict_set(&opts, "deadline", "realtime", 0);
	av_dict_set(&opts, "cpu-used", "8", 0);

	ffmpeg_video_set_opt_key(opts, "crf", crf);

	video->avcc->bit_rate = 0;
	video->avcc->thread_count = FFMIN(8, gui_hardware_concurrency());

	if (ffmpeg_stream_open(video, opts, TRUE) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
static BYTE ffmpeg_video_add_stream_format_wmv(void) {
	_ffmpeg_stream *video = &ffmpeg.video;
	_recording_format_info *rfi = &recording_format_info[REC_FORMAT_VIDEO_AVI_WMV];
	enum AVPixelFormat dst_pixel_format = AV_PIX_FMT_YUV420P;
	AVDictionary *opts = NULL;

	// configuro il contesto
	if (ffmpeg_context_setup(rfi, dst_pixel_format) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	switch (cfg->recording.quality) {
		case REC_QUALITY_LOW:
			video->avcc->bit_rate = 2500000;
			break;
		case REC_QUALITY_MEDIUM:
			video->avcc->bit_rate = 9800000;
			break;
		case REC_QUALITY_HIGH:
			video->avcc->bit_rate = 25000000;
			break;
	}

	video->avcc->mb_decision = FF_MB_DECISION_RD;
	video->avcc->thread_count = FFMIN(8, gui_hardware_concurrency());

	if (ffmpeg_stream_open(video, opts, TRUE) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
static BYTE ffmpeg_video_add_stream_format_ffv(void) {
	_ffmpeg_stream *video = &ffmpeg.video;
	_recording_format_info *rfi = &recording_format_info[REC_FORMAT_VIDEO_AVI_FFV];
	enum AVPixelFormat dst_pixel_format = AV_PIX_FMT_YUV444P;
	AVDictionary *opts = NULL;

	// configuro il contesto
	if (ffmpeg_context_setup(rfi, dst_pixel_format) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	// https://trac.ffmpeg.org/wiki/Encode/FFV1
	av_dict_set(&opts, "level", "3", 0);
	av_dict_set(&opts, "coder", "0", 0);
	av_dict_set(&opts, "slices", "24", 0);
	av_dict_set(&opts, "slicecrc", "1", 0);
	av_dict_set(&opts, "threads", "6", 0);
	av_dict_set(&opts, "g", "6", 0);

	if (ffmpeg_stream_open(video, opts, TRUE) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
static BYTE ffmpeg_video_add_stream_format_raw(void) {
	_ffmpeg_stream *video = &ffmpeg.video;
	_recording_format_info *rfi = &recording_format_info[REC_FORMAT_VIDEO_AVI_RAW];
	enum AVPixelFormat dst_pixel_format = AV_PIX_FMT_BGR24;

	// configuro il contesto
	if (ffmpeg_context_setup(rfi, dst_pixel_format) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	if (ffmpeg_stream_open(video, NULL, TRUE) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
static BYTE ffmpeg_video_add_stream_format_audio(enum recording_format format) {
	_ffmpeg_stream *video = &ffmpeg.video;
	_recording_format_info *rfi = &recording_format_info[format];
	enum AVPixelFormat dst_pixel_format = AV_PIX_FMT_RGB24;

	// configuro il contesto
	if (ffmpeg_context_setup(rfi, dst_pixel_format) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	// nel caso sia possibile memorizzare un logo apro lo streaming video
	if (ffmpeg_stream_open(video, NULL, TRUE) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}

// Audio --------------------------------------------------------------------------------------------------

static BYTE ffmpeg_audio_add_stream(void) {
	_ffmpeg_stream *audio = &ffmpeg.audio;
	enum AVSampleFormat src_sample_fmt = AV_SAMPLE_FMT_S16;
	int src_sample_rate = snd.samplerate;
	uint64_t src_channel_layout = cfg->channels_mode == CH_MONO ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;
	uint64_t dst_nb_samples;

	if (ffmpeg.format_ctx->oformat->audio_codec == AV_CODEC_ID_NONE) {
		fprintf(stderr, "audio codec unavailable.\n");
		return (EXIT_OK);
	}

	if (!(audio->avc = avcodec_find_encoder(ffmpeg.format_ctx->oformat->audio_codec))) {
		fprintf(stderr, "audio codec not found.\n");
		return (EXIT_ERROR);
	}

	if (!(audio->avs = avformat_new_stream(ffmpeg.format_ctx, NULL))) {
		fprintf(stderr, "could not alloc audio stream.\n");
		return (EXIT_ERROR);
	}

	if (!(audio->avcc = avcodec_alloc_context3(audio->avc))) {
		fprintf(stderr, "could not alloc an encoding context.\n");
		return (EXIT_ERROR);
	}

	audio->avcc->bit_rate = 128000;
	audio->avcc->sample_fmt = ffmpeg_audio_select_sample_fmt(audio->avc);
	audio->avcc->sample_rate = ffmpeg_audio_select_samplerate(audio->avc);
	audio->avcc->channel_layout = ffmpeg_audio_select_channel_layout(audio->avc);
	audio->avcc->channels = av_get_channel_layout_nb_channels(audio->avcc->channel_layout);

	audio->avs->id = ffmpeg.format_ctx->nb_streams - 1;
	audio->avs->time_base = (AVRational){ 1, audio->avcc->sample_rate };

	if (ffmpeg.format_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
		audio->avcc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	if (ffmpeg_stream_open(audio, NULL, FALSE) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	if (audio->avcc->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE) {
		dst_nb_samples = 2048;
	} else {
		dst_nb_samples = audio->avcc->frame_size;
	}
	audio->avf = ffmpeg_audio_alloc_frame(audio->avcc->sample_fmt, audio->avcc->channel_layout,
		audio->avcc->sample_rate, dst_nb_samples);
	audio->src_nb_samples = av_rescale_rnd(dst_nb_samples, src_sample_rate, audio->avcc->sample_rate, AV_ROUND_DOWN);

	if ((src_sample_fmt != audio->avcc->sample_fmt) ||
		(src_sample_rate != audio->avcc->sample_rate) ||
		(src_channel_layout != audio->avcc->channel_layout)) {
		if (!(audio->swr = swr_alloc_set_opts(NULL,
			audio->avcc->channel_layout, audio->avcc->sample_fmt, audio->avcc->sample_rate,
			src_channel_layout, src_sample_fmt, src_sample_rate,
			0, NULL))) {
			fprintf(stderr, "error allocating the resampling context.\n");
			return (EXIT_ERROR);
		}

		if (swr_init(audio->swr) < 0) {
			fprintf(stderr, "error opening the resampling context.\n");
			return (EXIT_ERROR);
		}

		if (!(audio->buffer = (SWORD *)malloc(audio->src_nb_samples * snd.channels * sizeof(*audio->buffer)))) {
			fprintf(stderr, "unable to allocate audio buffers.\n");
			return (EXIT_ERROR);
		}
	} else {
		audio->buffer = (SWORD *)audio->avf->extended_data[0];
	}

	audio->samples = 0;
	audio->pts = 0;

	audio->used = TRUE;
	audio->encode = TRUE;

	return (EXIT_OK);
}
INLINE static BYTE ffmpeg_audio_write_frame(SWORD *data) {
	_ffmpeg_stream *audio = &ffmpeg.audio;
	AVFrame *frame = NULL;
	BYTE convert = TRUE;
	int ret;

	if (data) {
		SWORD *q;

		if (cfg->channels_mode != CH_MONO) {
			q = audio->buffer + (audio->samples * 2);
			(*q) = (*data);
			(*++q) = (*++data);
		} else {
			q = audio->buffer + audio->samples;
			(*q) = (*data);
		}

		if (++audio->samples < audio->src_nb_samples) {
			return (EXIT_OK);
		}

		audio->samples = 0;
		frame = audio->avf;

		if (audio->swr) {
			if (swr_convert(audio->swr, NULL, 0, (const uint8_t **)&audio->buffer, audio->src_nb_samples) < 0) {
				fprintf(stderr, "Error feeding audio data to the resampler\n");
				return (EXIT_ERROR);
			}
		}
	}

	while (convert) {
		uint8_t **out = (uint8_t **)&audio->avf->extended_data[0];
		int out_count = audio->avf->nb_samples;

		if (audio->swr) {
			ret = swr_get_out_samples(audio->swr, 0);

			if (frame) {
				if (ret < (out_count * audio->avcc->channels)) {
					break;
				}
			} else if (ret <= 0) {
				convert = FALSE;
				out = NULL;
				out_count = 0;
			}

			if ((ret = swr_convert(audio->swr, out, out_count, NULL, 0)) < 0) {
				fprintf(stderr, "Error reading audio data from the resampler\n");
				return (EXIT_ERROR);
			}
		} else {
			convert = FALSE;
			ret = out_count;
		}

		audio->avf->nb_samples = ret;
		audio->avf->pts = audio->pts;
		audio->pts += audio->avf->nb_samples;

		if ((ret = avcodec_send_frame(audio->avcc, ret ? audio->avf : NULL)) < 0) {
			fprintf(stderr, "Error submitting a audio frame for encoding: %s.\n", ffmpeg_av_make_error_string(ret));
			return (EXIT_ERROR);
		}

		if ((ret = ffmpeg_stream_write_frame(audio)) == EXIT_ERROR) {
			return (ret);
		}
	}

	return (EXIT_OK);
}

static AVFrame *ffmpeg_audio_alloc_frame(enum AVSampleFormat sample_fmt, uint64_t ch_layout, int samplerate, int nb_samples) {
	AVFrame *avframe;

	if (!(avframe = av_frame_alloc())) {
		fprintf(stderr, "Error allocating an audio frame\n");
		return (NULL);
	}

	avframe->nb_samples = nb_samples;
	avframe->format = sample_fmt;
	avframe->channel_layout = ch_layout;
	avframe->sample_rate = samplerate;

	if (nb_samples && (av_frame_get_buffer(avframe, 0) < 0)) {
		fprintf(stderr, "Error allocating an audio buffer\n");
		return (NULL);
	}

	return (avframe);
}
static enum AVSampleFormat ffmpeg_audio_select_sample_fmt(const AVCodec *codec) {
	const enum AVSampleFormat *p = codec->sample_fmts;

	if (!p) {
		return AV_SAMPLE_FMT_S16;
	}

	while ((*p) != AV_SAMPLE_FMT_NONE) {
		if ((*p) == AV_SAMPLE_FMT_S16) {
			return AV_SAMPLE_FMT_S16;
		}
		p++;
	}
	return (codec->sample_fmts[0]);
}
static int ffmpeg_audio_select_samplerate(const AVCodec *codec) {
	int best_samplerate = 0;
	const int *p;

	if (!codec->supported_samplerates) {
		return 44100;
	}

	p = codec->supported_samplerates;
	while (*p) {
		best_samplerate = FFMAX(*p, best_samplerate);
		p++;
	}

	p = codec->supported_samplerates;
	while (*p) {
		if ((*p) == snd.samplerate) {
			best_samplerate = snd.samplerate;
			break;
		}
		p++;
	}

	return (best_samplerate);
}
static int ffmpeg_audio_select_channel_layout(const AVCodec *codec) {
	const uint64_t *p;
	uint64_t best_ch_layout = 0;
	int best_nb_channels = 0;

	if (!codec->channel_layouts) {
		return AV_CH_LAYOUT_STEREO;
	}

	p = codec->channel_layouts;
	while (*p) {
		int nb_channels = av_get_channel_layout_nb_channels(*p);

		if (nb_channels > best_nb_channels) {
			best_ch_layout = (*p);
			best_nb_channels = nb_channels;
		}
		p++;
	}

	p = codec->channel_layouts;
	while (*p) {
		int nb_channels = av_get_channel_layout_nb_channels(*p);

		if (nb_channels == snd.channels) {
			best_ch_layout = (*p);
			best_nb_channels = nb_channels;
			break;
		}
		p++;
	}
	return (best_ch_layout);
}

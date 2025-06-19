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

#ifndef RECORDING_H
#define RECORDING_H

#include "common.h"
#include "info.h"

enum recording_format {
	REC_FORMAT_VIDEO_MPG_MPEG1,
	REC_FORMAT_VIDEO_MPG_MPEG2,
	REC_FORMAT_VIDEO_MP4_MPEG4,
	REC_FORMAT_VIDEO_MP4_H264,
	REC_FORMAT_VIDEO_MKV_HEVC,
	REC_FORMAT_VIDEO_WEB_WEBM,
	REC_FORMAT_VIDEO_AVI_WMV,
	REC_FORMAT_VIDEO_AVI_FFV,
	REC_FORMAT_VIDEO_AVI_RAW,
	REC_FORMAT_VIDEO_TOTAL,

	REC_FORMAT_AUDIO_WAV = REC_FORMAT_VIDEO_TOTAL,
	REC_FORMAT_AUDIO_MP3,
	REC_FORMAT_AUDIO_AAC,
	REC_FORMAT_AUDIO_FLAC,
	REC_FORMAT_AUDIO_OGG,
	REC_FORMAT_AUDIO_OPUS,
	REC_FORMAT_AUDIO_TOTAL,
	REC_FORMAT_TOTAL = REC_FORMAT_AUDIO_TOTAL
};
enum recording_quality {
	REC_QUALITY_LOW,
	REC_QUALITY_MEDIUM,
	REC_QUALITY_HIGH
};
enum recording_format_type {
	REC_FORMAT_NONE,
	REC_FORMAT_VIDEO,
	REC_FORMAT_AUDIO
};
enum recording_output_resolution {
	REC_RES_CUSTOM,
	REC_RES_256x240,
	REC_RES_292x240,
	REC_RES_320x240,
	REC_RES_354x240,
	REC_RES_512x480,
	REC_RES_584x480,
	REC_RES_640x480,
	REC_RES_708x480,
	REC_RES_768x720,
	REC_RES_876x720,
	REC_RES_960x720,
	REC_RES_1064x720,
	REC_RES_1024x960,
	REC_RES_1170x960,
	REC_RES_1280x960,
	REC_RES_1418x960,
	REC_RES_1280x720,
	REC_RES_1920x1080
};

typedef struct _recording_format_info {
	BYTE present;
	char format[10];
	char *suffix_list[3];
	int format_type;
	int recording_format;
	char *codec;
	char *codecs_list[5];
} _recording_format_info;

extern _recording_format_info recording_format_info[];

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void recording_init(void);
EXTERNC void recording_quit(void);

EXTERNC void recording_start(uTCHAR *filename, int format);
EXTERNC void recording_finish(BYTE from_quit);

EXTERNC void recording_video_frame(int w, int h, int stride, uint8_t *rgb);

EXTERNC void recording_audio_tick(SWORD *data);
EXTERNC void recording_audio_silenced_frame(void);

EXTERNC int recording_format_type(void);
EXTERNC void recording_decode_output_resolution(int *w, int *h);

#undef EXTERNC

#endif /* RECORDING_H */

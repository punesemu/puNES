/*
 * param.h
 *
 *  Created on: 01/ago/2011
 *      Author: fhorse
 */

#ifndef PARAM_H_
#define PARAM_H_

#ifdef __CMDLINE__
static const char *opt_short = "m:f:k:s:o:i:n:p:r:v:u:t:a:l:c:q:g:Vh?";

static const struct option opt_long[] = {
	{ "mode",               required_argument, NULL, 'm'},
	{ "fps",                required_argument, NULL, 'f'},
	{ "frameskip",          required_argument, NULL, 'k'},
	{ "size",               required_argument, NULL, 's'},
	{ "overscan",           required_argument, NULL, 'o'},
	{ "filter",             required_argument, NULL, 'i'},
	{ "ntsc-format",        required_argument, NULL, 'n'},
	{ "palette",            required_argument, NULL, 'p'},
	{ "rendering",          required_argument, NULL, 'r'},
	{ "vsync",              required_argument, NULL, 'v'},
	{ "fullscreen",         required_argument, NULL, 'u'},
	{ "stretch-fullscreen", required_argument, NULL, 't'},
	{ "audio",              required_argument, NULL, 'a'},
	{ "samplerate",         required_argument, NULL, 'l'},
	{ "channels",           required_argument, NULL, 'c'},
	{ "audio-quality",      required_argument, NULL, 'q'},
	{ "swap-duty",          required_argument, NULL,  0 },
	{ "gamegenie",          required_argument, NULL, 'g'},
	{ "help",               no_argument,       NULL, 'h'},
	{ "version",            no_argument,       NULL, 'V'},
	{ 0,                    0,                 0,     0 }
};
#endif

#endif /* PARAM_H_ */

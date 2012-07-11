/*
 * cfgfile.h
 *
 *  Created on: 31/lug/2011
 *      Author: fhorse
 */

#ifndef CFGFILE_H_
#define CFGFILE_H_

#include "common.h"

typedef struct {
	BYTE saveOnExit;
	BYTE mode;
	BYTE audio;
	BYTE samplerate;
	BYTE channels;
	BYTE audio_filter;
	BYTE fps;
	BYTE frameskip;
} _config;

_config *cfg;
_config cfg_from_file;

void cfgfileInit(void);
void cfgfileParse(void);
void cfgfileSave(void);
void cfgfilePgsParse(void);
void cfgfilePgsSave(void);
void cfgfileInputParse(void);
void cfgfileInputSave(void);

#endif /* CFGFILE_H_ */

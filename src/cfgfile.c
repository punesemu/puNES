/*
 * cfgfile.c
 *
 *  Created on: 31/lug/2011
 *      Author: fhorse
 */

#include <libgen.h>
#include <stdlib.h>
#include "cfgfile.h"
#include "gui.h"
#include "version.h"
#include "clock.h"
#include "sdlgfx.h"
#include "sdlsnd.h"
#include "sdltext.h"
#include "fps.h"
#include "param.h"
#include "savestate.h"
#include "input.h"
#include "gamegenie.h"
#include "audio_filter.h"
#ifdef OPENGL
#include "opengl.h"
#endif

#define INIFILE NAME  ".cfg"
#define INPUTFILE     "input.cfg"
#define MAXLEN         512
#define cfgEvaluate(src, dst, chr)\
{\
	char *buf = 0;\
	buf = strtok(src, chr);\
	if (!buf) {\
		continue;\
	}\
	strcpy(dst, buf);\
}
#define cfgSearch(structp, prm, start, desc, cmd)\
{\
	char buf[MAXLEN];\
	strcpy(buf,  structp[prm].lname);\
	trimSpace(buf);\
	if (strcmp(key, buf) == 0) {\
		paramSearch(start, value, desc, cmd);\
		continue;\
	}\
}
#define cfgInputSearch(param, port, type)\
{\
	BYTE index, found = FALSE;\
	for(index = 0; index < LENGTH(param); index++) {\
		char buf[512];\
		strcpy(buf, param[index].lname);\
		trimSpace(buf);\
		if (strcmp(key, buf) == 0) {\
			if (type == JOYSTICK) {\
				if (index == (LENGTH(param) - 1)) {\
					port.joyID = nameToJsn(value);\
				} else {\
					port.input[JOYSTICK][index] = nameToJsv(value);\
				}\
			} else {\
				port.input[KEYBOARD][index] = keyvalFromName(value);\
			}\
			found = TRUE;\
			break;\
		}\
	}\
	if (found) {\
		continue;\
	}\
}
void setDefault(void);
void setDefaultPgs(void);
void trimSpace(char *src);
void writeParam(_param *prmtr, FILE *fp, BYTE prm, char *value);
void writeInputParam(_param *prmtr, FILE *fp, BYTE end, _port port, BYTE numport, BYTE type);
BYTE namePgsFile(char *file);

void cfgfileInit(void) {
	cfg = &cfg_from_file;
}
void cfgfileParse(void) {
#ifdef OPENGL
	BYTE render = 0;
#endif
	FILE *fp;
	char tmp[MAXLEN], line[MAXLEN];

	/* attivo la modalita' configurazione */
	gfx.onCfg = TRUE;
	/* default */
	setDefault();
	/* leggo la configurazione input */
	cfgfileInputParse();
	/* apro il file di configurazione */
	sprintf(tmp, "%s/%s", info.baseFolder, INIFILE);
	/* se non esiste allora lo creo */
	if ((fp = fopen(tmp, "r")) == NULL) {
		textAddLineInfo(1, "configuration [yellow]not found, [green]created");
		cfgfileSave();
		return;
	}
	/* leggo il file di configurazione */
	while (fgets(line, sizeof(line), fp)) {
		char key[MAXLEN], value[MAXLEN];
		/* elimino gli spazi */
		trimSpace(line);
		/* se la linea non inizia con # (commento) o '\n' allora la esamino */
		if ((line[0] != '#') && (line[0] != '\n')) {
			/* separo quello che viene prima dell'uguale... */
			cfgEvaluate(line, key, "=");
			/* ...da quello che c'e' dopo */
			cfgEvaluate(NULL, value, "\n");
			/* mode */
			cfgSearch(param, P_MODE, 0, pMode, cfg_from_file.mode = index);
			/* fps */
			cfgSearch(param, P_FPS, 0, pFps, cfg_from_file.fps = index);
			/* frame skip */
			cfgSearch(param, P_FSK, 0, pFsk, cfg_from_file.frameskip = index);
			/* size */
			cfgSearch(param, P_SIZE, 1, pSize, gfx.scale = gfx.scaleBeforeFullscreen = index);
			/* overscan default */
			cfgSearch(param, P_OVERSCAN, 0, pOverscan, gfx.overscanDefault = index);
			/* filter */
			cfgSearch(param, P_FILTER, 0, pFilter, gfx.filter = index);
			/* ntsc format */
			cfgSearch(param, P_NTSCFORMAT, 0, pNtsc, gfx.ntscFormat = index);
			/* palette */
			cfgSearch(param, P_PALETTE, 0, pPalette, gfx.palette = index);
#ifdef OPENGL
			/* rendering */
			cfgSearch(param, P_RENDER, 0, pRendering, render = index);
			/* vsync */
			cfgSearch(param, P_VSYNC, 0, pOffOn, gfx.vsync = index);
			/* fullscreen */
			cfgSearch(param, P_FSCREEN, 0, pNoYes, gfx.fullscreen = index);
			/* stretch in fullscreen */
			cfgSearch(param, P_STRETCH, 0, pNoYes, opengl.aspectRatio = !index);
#endif
			/* audio */
			cfgSearch(param, P_AUDIO, 0, pOffOn, cfg_from_file.audio = index);
			/* sample rate */
			cfgSearch(param, P_SAMPLERATE, 0, pSamplerate, cfg_from_file.samplerate = index);
			/* channels */
			cfgSearch(param, P_CHANNELS, 0, pChannels, cfg_from_file.channels = index);
			/* audio filter */
			cfgSearch(param, P_CHANNELS, 0, pAudioFilter, cfg_from_file.audio_filter = index);
			/* game genie */
			cfgSearch(param, P_GAMEGENIE, 0, pNoYes, gamegenie.enabled = index);
			/* save on exit */
			cfgSearch(param, P_SAVEONEXIT, 0, pNoYes, cfg_from_file.saveOnExit = index);
		}
	}

	textAddLineInfo(1, "configuration [green]loaded");

#ifdef OPENGL
	switch (render) {
		case 0:
			gfx.opengl = FALSE;
			opengl.glsl.enabled = FALSE;
			break;
		case 1:
			gfx.opengl = TRUE;
			opengl.glsl.enabled = FALSE;
			break;
		case 2:
			gfx.opengl = TRUE;
			opengl.glsl.enabled = TRUE;
			break;
	}
#endif

	if (gamegenie.enabled) {
		gamegenie_check_rom_present(TRUE);
	}

	/* the end */
	fclose(fp);
}
void cfgfileSave(void) {
	FILE *fp;
	char tmp[MAXLEN];

#ifdef OPENGL
	BYTE render;

	if (!gfx.opengl) {
		render = 0;
	} else {
		if (!opengl.glsl.compliant) {
			render = 1;
		} else if (!opengl.glsl.enabled) {
			render = 1;
		} else {
			render = 2;
		}
	}
#endif

	/* apro il file */
	sprintf(tmp, "%s/%s", info.baseFolder, INIFILE);
	if ((fp = fopen(tmp, "w")) == NULL) {
		fprintf(stderr, "ERROR: File not found : %s", INIFILE);
		return;
	}
	/* mode */
	writeParam((_param *) param, fp, P_MODE, pMode[cfg_from_file.mode].sname);
	/* fps */
	writeParam((_param *) param, fp, P_FPS, pFps[cfg_from_file.fps].sname);
	/* fps */
	writeParam((_param *) param, fp, P_FSK, pFsk[cfg_from_file.frameskip].sname);
	/* size */
	writeParam((_param *) param, fp, P_SIZE,
			(gfx.fullscreen ? pSize[gfx.scaleBeforeFullscreen].sname : pSize[gfx.scale].sname));
	/* overscan default */
	writeParam((_param *) param, fp, P_OVERSCAN, pOverscan[gfx.overscanDefault].sname);
	/* filter */
	writeParam((_param *) param, fp, P_FILTER, pFilter[gfx.filter].sname);
	/* ntsc format */
	writeParam((_param *) param, fp, P_NTSCFORMAT, pNtsc[gfx.ntscFormat].sname);
	/* palette */
	writeParam((_param *) param, fp, P_PALETTE, pPalette[gfx.palette].sname);
#ifdef OPENGL
	/* rendering */
	writeParam((_param *) param, fp, P_RENDER, pRendering[render].sname);
	/* vsync */
	writeParam((_param *) param, fp, P_VSYNC, pOffOn[gfx.vsync].sname);
	/* fullscreen */
	writeParam((_param *) param, fp, P_FSCREEN, pNoYes[gfx.fullscreen].sname);
	/* stretch in fullscreen */
	writeParam((_param *) param, fp, P_STRETCH, pNoYes[!opengl.aspectRatio].sname);
#endif
	/* audio */
	writeParam((_param *) param, fp, P_AUDIO, pOffOn[cfg_from_file.audio].sname);
	/* sample rate */
	writeParam((_param *) param, fp, P_SAMPLERATE, pSamplerate[cfg_from_file.samplerate].sname);
	/* channels */
	writeParam((_param *) param, fp, P_CHANNELS, pChannels[cfg_from_file.channels].sname);
	/* audio filter */
	writeParam((_param *) param, fp, P_AUDIO_FILTER, pChannels[cfg_from_file.audio_filter].sname);
	/* game genie */
	writeParam((_param *) param, fp, P_GAMEGENIE, pNoYes[gamegenie.enabled].sname);
	/* save settings on exit */
	writeParam((_param *) param, fp, P_SAVEONEXIT, pNoYes[cfg_from_file.saveOnExit].sname);
	/* the end */
	fclose(fp);

	cfgfileInputSave();

}
void cfgfilePgsParse(void) {
	FILE *fp;
	char tmp[MAXLEN], line[MAXLEN];

	/* default */
	setDefaultPgs();

	if (namePgsFile(tmp)) {
		return;
	}

	if ((fp = fopen(tmp, "r")) == NULL) {
		if (!gamegenie.print) {
			textAddLineInfo(1, "rom configuration [yellow]not found");
		}
		return;
	}

	/* leggo il file di configurazione */
	while (fgets(line, sizeof(line), fp)) {
		char key[MAXLEN], value[MAXLEN];
		/* elimino gli spazi */
		trimSpace(line);
		/* se la linea non inizia con # (commento) o '\n' allora la esamino */
		if ((line[0] != '#') && (line[0] != '\n')) {
			/* separo quello che viene prima dell'uguale... */
			cfgEvaluate(line, key, "=");
			/* ...da quello che c'e' dopo */
			cfgEvaluate(NULL, value, "\n");
			/* last save slot */
			cfgSearch(paramPgs, PGS_SLOT, 0, pSlot, savestate.slot = index);
			/* overscan */
			cfgSearch(paramPgs, PGS_OVERSCAN, 0, pOverscan, gfx.overscan = index);
		}
	}

	if (!gamegenie.print) {
		textAddLineInfo(1, "rom configuration [green]loaded");
	}

	fclose(fp);
}
void cfgfilePgsSave(void) {
	FILE *fp;
	char tmp[MAXLEN];

	if (namePgsFile(tmp)) {
		return;
	}
	if ((fp = fopen(tmp, "w")) == NULL) {
		return;
	}
	/* last save slot */
	writeParam((_param *) paramPgs, fp, PGS_SLOT, pSlot[savestate.slot].sname);
	/* overscan */
	writeParam((_param *) paramPgs, fp, PGS_OVERSCAN, pOverscan[gfx.overscan].sname);

	fclose(fp);
}
void cfgfileInputParse(void) {
	FILE *fp;
	char tmp[MAXLEN], line[MAXLEN];

	/* apro il file di configurazione */
	sprintf(tmp, "%s/%s", info.baseFolder, INPUTFILE);
	/* se non esiste lo creo */
	if ((fp = fopen(tmp, "r")) == NULL) {
		cfgfileInputSave();
		return;
	}
	/* leggo il file di configurazione */
	while (fgets(line, sizeof(line), fp)) {
		char key[MAXLEN], value[MAXLEN];
		/* elimino gli spazi */
		trimSpace(line);
		/* se la linea non inizia con # (commento) o '\n' allora la esamino */
		if ((line[0] != '#') && (line[0] != '\n')) {
			/* separo quello che viene prima dell'uguale... */
			cfgEvaluate(line, key, "=");
			/* ...da quello che c'e' dopo */
			cfgEvaluate(NULL, value, "\n");

			cfgSearch(paramInputCtrl, 0, 0, pController, port1.type = index);
			cfgSearch(paramInputCtrl, 1, 0, pController, port2.type = index);

			cfgInputSearch(paramInputP1K, port1, KEYBOARD);
			cfgInputSearch(paramInputP1J, port1, JOYSTICK);

			cfgInputSearch(paramInputP2K, port2, KEYBOARD);
			cfgInputSearch(paramInputP2J, port2, JOYSTICK);
		}
	}

	textAddLineInfo(1, "input configuration [green]loaded");

	fclose(fp);
}
void cfgfileInputSave(void) {
	FILE *fp;
	char tmp[MAXLEN];

	/* apro il file */
	sprintf(tmp, "%s/%s", info.baseFolder, INPUTFILE);
	if ((fp = fopen(tmp, "w")) == NULL) {
		return;
	}

	fprintf(fp, "# input configuration\n\n");

	writeParam((_param *) paramInputCtrl, fp, 0, pController[port1.type].sname);
	writeParam((_param *) paramInputCtrl, fp, 1, pController[port2.type].sname);

	writeInputParam((_param *) paramInputP1K, fp, LENGTH(paramInputP1K), port1, 1, KEYBOARD);
	writeInputParam((_param *) paramInputP1J, fp, LENGTH(paramInputP1J), port1, 1, JOYSTICK);

	writeInputParam((_param *) paramInputP2K, fp, LENGTH(paramInputP2K), port2, 2, KEYBOARD);
	writeInputParam((_param *) paramInputP2J, fp, LENGTH(paramInputP2J), port2, 2, JOYSTICK);

	fclose(fp);
}

void setDefault(void) {

#define _portKbDefault(port, button, name)\
		port.input[KEYBOARD][button] = keyvalFromName(name);
#define _portJsDefault(port, button, name)\
		port.input[JOYSTICK][button] = nameToJsv(name)
#define portKbDefault(port, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10)\
	_portKbDefault(port, BUT_A,  s1);\
	_portKbDefault(port, BUT_B,  s2);\
	_portKbDefault(port, SELECT, s3);\
	_portKbDefault(port, START,  s4);\
	_portKbDefault(port, UP,     s5);\
	_portKbDefault(port, DOWN,   s6);\
	_portKbDefault(port, LEFT,   s7);\
	_portKbDefault(port, RIGHT,  s8);\
	_portKbDefault(port, TRBA,   s9);\
	_portKbDefault(port, TRBB,   s10)
#define portJsDefault(port, id, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10)\
	port.joyID = nameToJsn(id);\
	_portJsDefault(port, BUT_A,  s1);\
	_portJsDefault(port, BUT_B,  s2);\
	_portJsDefault(port, SELECT, s3);\
	_portJsDefault(port, START,  s4);\
	_portJsDefault(port, UP,     s5);\
	_portJsDefault(port, DOWN,   s6);\
	_portJsDefault(port, LEFT,   s7);\
	_portJsDefault(port, RIGHT,  s8);\
	_portJsDefault(port, TRBA,   s9);\
	_portJsDefault(port, TRBB,   s10)

	/* default */
	cfg_from_file.mode = AUTO;
	machine = machinedb[NTSC - 1];
	cfg_from_file.fps = FPSDEFAULT;
	cfg_from_file.frameskip = 0;
	gfx.scale = X2;
	gfx.scaleBeforeFullscreen = gfx.scale;
	gfx.overscanDefault = OSCANOFF;
	gfx.filter = RGBNTSC;
	gfx.palette = PALETTENTSC;
	gfx.ntscFormat = COMPOSITE;
#ifdef OPENGL
	gfx.opengl = TRUE;
	opengl.glsl.enabled = FALSE;
	gfx.vsync = TRUE;
	gfx.fullscreen = NOFULLSCR;
	opengl.aspectRatio = FALSE;
	cfg_from_file.saveOnExit = FALSE;
#endif
	cfg_from_file.audio = TRUE;
	cfg_from_file.samplerate = S44100;
	cfg_from_file.channels = STEREO;
	cfg_from_file.channels = AF_NONE;
	gamegenie.enabled = FALSE;
	port1.type = STDCTRL;
	portKbDefault(port1, "S", "A", "Z", "X", "Up", "Down", "Left", "Right", "W", "Q");
	portJsDefault(port1, "JOYSTICKID1", "JB1", "JB0", "JB8", "JB9", "JA1MIN", "JA1PLS", "JA0MIN",
			"JA0PLS", "JB2", "JB3");
	port2.type = FALSE;
	port2.joyID = nameToJsn("JOYSTICKID2");

}
void setDefaultPgs(void) {
	gfx.overscan = OSCANDEF;
}
void trimSpace(char *src) {
	const char *current = src;
	char out[MAXLEN];
	unsigned int i = 0, size = strlen(src);

	strcpy(out, src);
	while (current != '\0' && i < size) {
		if (*current != ' ' && *current != '\t') {
			out[i++] = *current;
		}
		++current;
	}
	out[i] = '\0';
	strcpy(src, out);
}
void writeParam(_param *prmtr, FILE *fp, BYTE prm, char *value) {
	if (prmtr[prm].comment1 != NULL) {
		fprintf(fp, "%s\n", prmtr[prm].comment1);
	}
	if (prmtr[prm].comment2 != NULL) {
		fprintf(fp, "%s\n", prmtr[prm].comment2);
	}
	fprintf(fp, "%s = %s\n\n", prmtr[prm].lname, value);
}
void writeInputParam(_param *prmtr, FILE *fp, BYTE end, _port port, BYTE numport, BYTE type) {
	BYTE index;

	for (index = 0; index < end; index++) {
		if (index == 0) {
			if (type == JOYSTICK) {
				fprintf(fp, "# player %d joystick\n", numport);
				fprintf(fp, "%s = %s\n", prmtr[end - 1].lname, jsnToName(port.joyID));
				end--;
			} else {
				fprintf(fp, "# player %d keyboard\n", numport);
			}
		}
		if (type == JOYSTICK) {
			fprintf(fp, "%s = %s\n", prmtr[index].lname, jsvToName(port.input[JOYSTICK][index]));
		} else {
			fprintf(fp, "%s = %s\n", prmtr[index].lname, keyvalToName(port.input[KEYBOARD][index]));
		}
	}
}
BYTE namePgsFile(char *file) {
	char ext[10], *lastDot;

	/* game genie */
	if (info.mapper == GAMEGENIE_MAPPER) {
		return (EXIT_ERROR);
	}

	if (!info.romFile[0]) {
		return (EXIT_ERROR);
	}

	sprintf(file, "%s" PERGAMEFOLDER "/%s", info.baseFolder, basename(info.romFile));
	sprintf(ext, ".pgs");

	/* rintraccio l'ultimo '.' nel nome */
	lastDot = strrchr(file, '.');
	/* elimino l'estensione */
	*lastDot = 0x00;
	/* aggiungo l'estensione */
	strcat(file, ext);

	return (EXIT_OK);
}

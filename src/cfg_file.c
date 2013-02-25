/*
 * cfg_file.c
 *
 *  Created on: 31/lug/2011
 *      Author: fhorse
 */

#include <libgen.h>
#include <stdlib.h>
#include "cfg_file.h"
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
#include "audio_quality.h"
#include "opengl.h"

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
		param_search(start, value, desc, cmd);\
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
					port.joy_id = nameToJsn(value);\
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

void cfg_file_init(void) {
	cfg = &cfg_from_file;
}
void cfg_file_parse(void) {
	FILE *fp;
	char tmp[MAXLEN], line[MAXLEN];

	/* attivo la modalita' configurazione */
	info.on_cfg = TRUE;

	/* default */
	setDefault();
	/* leggo la configurazione input */
	cfg_file_input_parse();
	/* apro il file di configurazione */
	sprintf(tmp, "%s/%s", info.base_folder, INIFILE);
	/* se non esiste allora lo creo */
	if ((fp = fopen(tmp, "r")) == NULL) {
		textAddLineInfo(1, "configuration [yellow]not found, [green]created");
		cfg_file_save();
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
			cfgSearch(param, P_MODE, 0, param_mode, cfg_from_file.mode = index);
			/* fps */
			cfgSearch(param, P_FPS, 0, param_fps, cfg_from_file.fps = index);
			/* frame skip */
			cfgSearch(param, P_FSK, 0, param_fsk, cfg_from_file.frameskip = index);
			/* size */
			cfgSearch(param, P_SIZE, 1, param_size, cfg_from_file.scale = index);
			/* overscan default */
			cfgSearch(param, P_OVERSCAN, 0, param_oscan, cfg_from_file.oscan_default = index);
			/* filter */
			cfgSearch(param, P_FILTER, 0, param_filter, cfg_from_file.filter = index);
			/* ntsc format */
			cfgSearch(param, P_NTSCFORMAT, 0, param_ntsc, cfg_from_file.ntsc_format = index);
			/* palette */
			cfgSearch(param, P_PALETTE, 0, param_palette, cfg_from_file.palette = index);
			/* rendering */
			cfgSearch(param, P_RENDER, 0, param_render, cfg_from_file.render = index);
			/* vsync */
			cfgSearch(param, P_VSYNC, 0, param_off_on, cfg_from_file.vsync = index);
			/* fullscreen */
			cfgSearch(param, P_FSCREEN, 0, param_no_yes, cfg_from_file.fullscreen = index);
			/* stretch in fullscreen */
			cfgSearch(param, P_STRETCH, 0, param_no_yes, cfg_from_file.aspect_ratio = !index);
			/* audio */
			cfgSearch(param, P_AUDIO, 0, param_off_on, cfg_from_file.audio = index);
			/* sample rate */
			cfgSearch(param, P_SAMPLERATE, 0, param_samplerate, cfg_from_file.samplerate = index);
			/* channels */
			cfgSearch(param, P_CHANNELS, 0, param_channels, cfg_from_file.channels = index);
			/* audio quality */
			cfgSearch(param, P_AUDIO_QUALITY, 0, param_audio_quality,
			        cfg_from_file.audio_quality = index);
			/* swap duty cycles */
			cfgSearch(param, P_SWAP_DUTY, 0, param_no_yes, cfg_from_file.swap_duty = index);
			/* game genie */
			cfgSearch(param, P_GAMEGENIE, 0, param_no_yes, cfg_from_file.gamegenie = index);
			/* save on exit */
			cfgSearch(param, P_SAVEONEXIT, 0, param_no_yes, cfg_from_file.save_on_exit = index);
		}
	}

	textAddLineInfo(1, "configuration [green]loaded");

	gfx.scale_before_fscreen = cfg_from_file.scale;

	gfxSetRender(cfg_from_file.render);

	if (cfg_from_file.gamegenie) {
		gamegenie_check_rom_present(TRUE);
	}

	/* the end */
	fclose(fp);
}
void cfg_file_save(void) {
	FILE *fp;
	char tmp[MAXLEN];

	/* apro il file */
	sprintf(tmp, "%s/%s", info.base_folder, INIFILE);
	if ((fp = fopen(tmp, "w")) == NULL) {
		fprintf(stderr, "ERROR: File not found : %s", INIFILE);
		return;
	}
	/* mode */
	writeParam((_param *) param, fp, P_MODE, param_mode[cfg_from_file.mode].sname);
	/* fps */
	writeParam((_param *) param, fp, P_FPS, param_fps[cfg_from_file.fps].sname);
	/* fps */
	writeParam((_param *) param, fp, P_FSK, param_fsk[cfg_from_file.frameskip].sname);
	/* size */
	{
		BYTE scale = (cfg_from_file.fullscreen ? gfx.scale_before_fscreen : cfg_from_file.scale);

		writeParam((_param *) param, fp, P_SIZE, param_size[scale].sname);
	}
	/* overscan default */
	writeParam((_param *) param, fp, P_OVERSCAN, param_oscan[cfg_from_file.oscan_default].sname);
	/* filter */
	writeParam((_param *) param, fp, P_FILTER, param_filter[cfg_from_file.filter].sname);
	/* ntsc format */
	writeParam((_param *) param, fp, P_NTSCFORMAT, param_ntsc[cfg_from_file.ntsc_format].sname);
	/* palette */
	writeParam((_param *) param, fp, P_PALETTE, param_palette[cfg_from_file.palette].sname);
	/* rendering */
	writeParam((_param *) param, fp, P_RENDER, param_render[cfg_from_file.render].sname);
	/* vsync */
	writeParam((_param *) param, fp, P_VSYNC, param_off_on[cfg_from_file.vsync].sname);
	/* fullscreen */
	writeParam((_param *) param, fp, P_FSCREEN, param_no_yes[cfg_from_file.fullscreen].sname);
	/* stretch in fullscreen */
	writeParam((_param *) param, fp, P_STRETCH, param_no_yes[!cfg_from_file.aspect_ratio].sname);
	/* audio */
	writeParam((_param *) param, fp, P_AUDIO, param_off_on[cfg_from_file.audio].sname);
	/* sample rate */
	writeParam((_param *) param, fp, P_SAMPLERATE,
	        param_samplerate[cfg_from_file.samplerate].sname);
	/* channels */
	writeParam((_param *) param, fp, P_CHANNELS, param_channels[cfg_from_file.channels].sname);
	/* audio quality */
	writeParam((_param *) param, fp, P_AUDIO_QUALITY,
			param_audio_quality[cfg_from_file.audio_quality].sname);
	/* swap duty cycles */
	writeParam((_param *) param, fp, P_SWAP_DUTY, param_no_yes[cfg_from_file.swap_duty].sname);
	/* game genie */
	writeParam((_param *) param, fp, P_GAMEGENIE, param_no_yes[cfg_from_file.gamegenie].sname);
	/* save settings on exit */
	writeParam((_param *) param, fp, P_SAVEONEXIT, param_no_yes[cfg_from_file.save_on_exit].sname);
	/* the end */
	fclose(fp);

	cfg_file_input_save();

}
void cfg_file_pgs_parse(void) {
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
			cfgSearch(param_pgs, PGS_SLOT, 0, param_slot, savestate.slot = index);
			/* overscan */
			cfgSearch(param_pgs, PGS_OVERSCAN, 0, param_oscan, cfg_from_file.oscan = index);
		}
	}

	if (!gamegenie.print) {
		textAddLineInfo(1, "rom configuration [green]loaded");
	}

	fclose(fp);
}
void cfg_file_pgs_save(void) {
	FILE *fp;
	char tmp[MAXLEN];

	if (namePgsFile(tmp)) {
		return;
	}
	if ((fp = fopen(tmp, "w")) == NULL) {
		return;
	}
	/* last save slot */
	writeParam((_param *) param_pgs, fp, PGS_SLOT, param_slot[savestate.slot].sname);
	/* overscan */
	writeParam((_param *) param_pgs, fp, PGS_OVERSCAN, param_oscan[cfg_from_file.oscan].sname);

	fclose(fp);
}
void cfg_file_input_parse(void) {
	FILE *fp;
	char tmp[MAXLEN], line[MAXLEN];

	/* apro il file di configurazione */
	sprintf(tmp, "%s/%s", info.base_folder, INPUTFILE);
	/* se non esiste lo creo */
	if ((fp = fopen(tmp, "r")) == NULL) {
		cfg_file_input_save();
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

			cfgSearch(param_input_ctrl, 0, 0, param_controller, port1.type = index);
			cfgSearch(param_input_ctrl, 1, 0, param_controller, port2.type = index);

			cfgInputSearch(param_input_p1k, port1, KEYBOARD);
			cfgInputSearch(param_input_p1j, port1, JOYSTICK);

			cfgInputSearch(param_input_p2k, port2, KEYBOARD);
			cfgInputSearch(param_input_p2j, port2, JOYSTICK);
		}
	}

	textAddLineInfo(1, "input configuration [green]loaded");

	fclose(fp);
}
void cfg_file_input_save(void) {
	FILE *fp;
	char tmp[MAXLEN];

	/* apro il file */
	sprintf(tmp, "%s/%s", info.base_folder, INPUTFILE);
	if ((fp = fopen(tmp, "w")) == NULL) {
		return;
	}

	fprintf(fp, "# input configuration\n\n");

	writeParam((_param *) param_input_ctrl, fp, 0, param_controller[port1.type].sname);
	writeParam((_param *) param_input_ctrl, fp, 1, param_controller[port2.type].sname);

	writeInputParam((_param *) param_input_p1k, fp, LENGTH(param_input_p1k), port1, 1, KEYBOARD);
	writeInputParam((_param *) param_input_p1j, fp, LENGTH(param_input_p1j), port1, 1, JOYSTICK);

	writeInputParam((_param *) param_input_p2k, fp, LENGTH(param_input_p2k), port2, 2, KEYBOARD);
	writeInputParam((_param *) param_input_p2j, fp, LENGTH(param_input_p2j), port2, 2, JOYSTICK);

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
	port.joy_id = nameToJsn(id);\
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
	cfg_from_file.scale = gfx.scale_before_fscreen = X2;
	cfg_from_file.oscan_default = OSCAN_OFF;
	cfg_from_file.filter = RGBNTSC;
	cfg_from_file.ntsc_format = COMPOSITE;
	cfg_from_file.palette = PALETTENTSC;

	cfg_from_file.render = RENDER_OPENGL;
	gfxSetRender(cfg_from_file.render);

	cfg_from_file.vsync = TRUE;
	cfg_from_file.fullscreen = NOFULLSCR;
	cfg_from_file.aspect_ratio = FALSE;
	cfg_from_file.save_on_exit = FALSE;

	cfg_from_file.audio = TRUE;
	cfg_from_file.samplerate = S44100;
	cfg_from_file.channels = STEREO;
	cfg_from_file.audio_quality = AQ_HIGH;
	cfg_from_file.swap_duty = 0;
	cfg_from_file.gamegenie = FALSE;

	port1.type = STDCTRL;
	portKbDefault(port1, "S", "A", "Z", "X", "Up", "Down", "Left", "Right", "W", "Q");
	portJsDefault(port1, "JOYSTICKID1", "JB1", "JB0", "JB8", "JB9", "JA1MIN", "JA1PLS", "JA0MIN",
			"JA0PLS", "JB2", "JB3");

	port2.type = FALSE;
	port2.joy_id = nameToJsn("JOYSTICKID2");

}
void setDefaultPgs(void) {
	cfg_from_file.oscan = OSCAN_DEFAULT;
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
				fprintf(fp, "%s = %s\n", prmtr[end - 1].lname, jsnToName(port.joy_id));
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

	if (!info.rom_file[0]) {
		return (EXIT_ERROR);
	}

	sprintf(file, "%s" PERGAME_FOLDER "/%s", info.base_folder, basename(info.rom_file));
	sprintf(ext, ".pgs");

	/* rintraccio l'ultimo '.' nel nome */
	lastDot = strrchr(file, '.');
	/* elimino l'estensione */
	*lastDot = 0x00;
	/* aggiungo l'estensione */
	strcat(file, ext);

	return (EXIT_OK);
}

/*
 * cfg_file.c
 *
 *  Created on: 31/lug/2011
 *      Author: fhorse
 */

#include "cfg_file.h"
#include "gfx.h"
#include "clock.h"
#include "fps.h"

void set_default(void);

void cfg_file_init(void) {
	cfg = &cfg_from_file;
}
void cfg_file_parse(void) {
	/* attivo la modalita' configurazione */
	info.on_cfg = TRUE;

	/* default */
	set_default();

	return;
}
void cfg_file_save(void) {
	return;
}
void cfg_file_pgs_parse(void) {
	return;
}
void cfg_file_pgs_save(void) {
	return;
}
void cfg_file_input_parse(void) {
	return;
}
void cfg_file_input_save(void) {
	return;
}

void set_default(void) {
	cfg_from_file.mode = AUTO;
	machine = machinedb[NTSC - 1];

	cfg_from_file.fps = FPS_DEFAULT;
	cfg_from_file.frameskip = 0;
	cfg_from_file.scale = gfx.scale_before_fscreen = X2;
	cfg_from_file.oscan_default = OSCAN_OFF;
	cfg_from_file.filter = NTSC_FILTER;
	cfg_from_file.ntsc_format = COMPOSITE;
	cfg_from_file.palette = PALETTE_NTSC;

	//cfg_from_file.render = RENDER_OPENGL;
	//gfx_set_render(cfg_from_file.render);
}

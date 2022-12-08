/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

#if !defined(_WIN32)
#include <stdio.h>
#include <string.h>
#endif
#include <stdlib.h>
#include <math.h>
#include "gfx_monitor.h"
#include "clock.h"
#include "pnp_vendor.h"
#include "gfx.h"

/*
 * Makefile (thx to https://github.com/vcrhonek/hwdata)
pnp.ids.xlsx:
	@curl -o $@ \
	    https://uefi.org/uefi-pnp-export

pnp.ids.orig: pnp.ids.xlsx
	grep "class" $? | \
	    tr ' ' ' ' | \
	    sed -n \
	        -e 's/\s\{2,\}/ /g' \
	        -e 's/\&amp;/\&/g' \
	        -e "s/\&#039;/'/g" \
	        -e "s/“/'/g" \
	        -e "s/”/'/g" \
	        -e 's:^.*<tr class=".*"><td>\(.*\)</td><td>\([a-zA-Z@]\{3\}\).*</td><td>.*$$:\2\t\1:p' | \
	    sed 's/\s*$$//' | sort -u >$@

pnp.ids: pnp.ids.orig pnp.ids.patch
	patch -p1 -o $@ pnp.ids.orig pnp.ids.patch

pnp_vendor.h: pnp.ids.orig
	echo -e "typedef struct _pnp_vendor {\n\tconst uTCHAR id[4];\n\tconst uTCHAR name[80];\n} _pnp_vendor;\n\nstatic const _pnp_vendor vendors[] = {" >$@
	cat $? | \
		sed -n \
		-e 's:^\(.*\)\t\(.*\)$$:\t{uL("\1"), uL("\2")},:p' >>$@
	echo "};" >>$@
*/
/*
 * sh
 curl -o pnp.ids.xlsx https://uefi.org/uefi-pnp-export
 grep "class" pnp.ids.xlsx | \
   tr ' ' ' ' | \
   sed -n \
   -e 's/\s\{2,\}/ /g' \
   -e 's/\&amp;/\&/g' \
   -e "s/\&#039;/'/g" \
   -e "s/“/'/g" \
   -e "s/”/'/g" \
   -e 's:^.*<tr class=".*"><td>\(.*\)</td><td>\([a-zA-Z@]\{3\}\).*</td><td>.*$:\2\t\1:p' | \
   sed 's/\s*$//' | sort -u >pnp.ids.orig
 echo -e "typedef struct _pnp_vendor {\n\tconst uTCHAR id[4];\n\tconst uTCHAR name[80];\n} _pnp_vendor;\n\nstatic const _pnp_vendor vendors[] = {" >pnp_vendor.h
 cat pnp.ids.orig | sed -n -e 's:^\(.*\)\t\(.*\)$:\t{uL("\1"), uL("\2")},:p' >>pnp_vendor.h
 echo "};" >>pnp_vendor.h
*/

#define edid_vendor_byte1(edid) ((edid[0x08] & 0x7C) >> 2) + '@'
#define edid_vendor_byte2(edid) (((edid[0x08] & 0x03) << 3) | ((edid[0x09] & 0xE0) >> 5)) + '@'
#define edid_vendor_byte3(edid) ((edid[0x09] & 0x1F) >> 0) + '@'

static void enum_resolutions(void);
static void free_resolutions(void);
static void edid_decode_lf_string(const uint8_t *s, int nchars, uTCHAR *result);
static void edid_decode_check_sum(const uint8_t *edid, _monitor_edid *me);
static BYTE edid_decode_header(const uint8_t *edid);
static BYTE edid_decode_vendor_and_product_identification(const uint8_t *edid, _monitor_edid *me);
static BYTE edid_decode_descriptors(const uint8_t *edid, _monitor_edid *me);
static void edid_decode_display_descriptor(const uint8_t *desc, _monitor_edid *me);
static void edid_decode_detailed_timing(const uint8_t *timing, _monitor_edid_detailed_timing *dt);
static const uTCHAR *pnp_find_vendor(const uTCHAR *code);
static uTCHAR *make_display_name(const _monitor_edid *me);
static int search_rrate(_monitor_mode_info *list, int nmodes, int rrate);
#if !defined (RELEASE)
static void print_info(void);
#endif

_monitor monitor;

void gfx_monitor_init(void) {
	memset(&monitor, 0x00, sizeof(monitor));
}
void gfx_monitor_quit(void) {
	int i;

	monitor.enabled = FALSE;
	monitor.active = -1;

	for (i = 0; i < monitor.nmonitor; i++) {
		_monitor_info *mi = &monitor.monitors[i];

		mi->mode_org = -1;
		mi->mode_new = -1;

		mi->nmodes = 0;
		if (mi->modes) {
			free(mi->modes);
			mi->modes = NULL;
		}
		if (mi->edid) {
			free(mi->edid);
			mi->edid = NULL;
		}
	}
	monitor.nmonitor = 0;
	if (monitor.monitors) {
		free(monitor.monitors);
		monitor.monitors = NULL;
	}
	free_resolutions();
}
void gfx_monitor_enum_monitors(void) {
	int a, emu_x = 0, emu_y = 0;

	gfx_monitor_quit();

	if (gfx.is_wayland) {
		return;
	}

	if (gui_monitor_enum_monitors() == EXIT_ERROR) {
		gfx_monitor_quit();
		return;
	}

	for (a = 0; (a < 5) && (monitor.active == -1); a++) {
		int b;

		gui_mainwindow_coords(&emu_x, &emu_y, a);

		for (b = 0; b < monitor.nmonitor; b++) {
			_monitor_info *mi = &monitor.monitors[b];

			if ((emu_x >= mi->x) && (emu_x < (mi->x + mi->w)) && (emu_y >= mi->y) && (emu_y < (mi->y + mi->h))) {
				mi->in_use = TRUE;
				monitor.active = b;
				break;
			}
		}
	}

	if ((monitor.active == -1) || (monitor.monitors[monitor.active].mode_org == -1)) {
		gfx_monitor_quit();
		return;
	}

	enum_resolutions();

#if !defined (RELEASE)
	print_info();
#endif
}
BYTE gfx_monitor_set_res(int w, int h, BYTE adaptive_rrate, BYTE change_rom_mode) {
	_monitor_mode_info *mode_info_org;
	_monitor_info *mi;
	double rrate;
	int mode_new;

	if (!change_rom_mode) {
		gfx_monitor_enum_monitors();
	}

	if (gfx.is_wayland || !monitor.enabled || (monitor.active == -1)) {
		return (FALSE);
	}

	mi = &monitor.monitors[monitor.active];

	if (mi->mode_org == -1) {
		return (FALSE);
	}

	mode_info_org = &mi->modes[mi->mode_org];

	if ((w == -1) || (h == -1)) {
		w = mode_info_org->w;
		h = mode_info_org->h;
	}

	mode_new = -1;

	// inizio la ricerca della risoluzione adatta
	{
		_monitor_mode_info *list = NULL;
		int mode, nmodes = 0;

		for (mode = 0; mode < mi->nmodes; mode++) {
			_monitor_mode_info *dst = NULL, *src = &mi->modes[mode];

			if ((src->w != w) || (src->h != h)) {
				continue;
			}
			list = (_monitor_mode_info *)realloc(list, (nmodes + 1) * sizeof(_monitor_mode_info));
			dst = &list[nmodes];
			memcpy(dst, src, sizeof(_monitor_mode_info));
			nmodes++;
		}

		if (list) {
			if (adaptive_rrate) {
				if (machine.type == NTSC) {
					rrate = 60.0f;
				} else {
					rrate = 50.0f;
				}
				// cerco un refresh rate compatibile con la regione della rom
				if ((mode_new = search_rrate(list, nmodes, (int)rrate)) == -1) {
					mode_new = search_rrate(list, nmodes, (int)rrate * 2);
				}
			}
			// se la ricerca precedente non e' andata a buon fine
			// cerco una risoluzione con il refresh rate attuale
			if (mode_new == -1) {
				mode_new = search_rrate(list, nmodes, (int)mode_info_org->rounded_rrate);
			}
			// altrimenti prendo la prima risoluzione con un
			// refresh rate superiore a 50
			if (mode_new == -1) {
				mode_new = search_rrate(list, nmodes, -1);
			}
			// recupero l'indice della nuova modalita'
			if (mode_new >= 0) {
				_monitor_mode_info *mode_info_new = &list[mode_new];

				for (mode = 0; mode < mi->nmodes; mode++) {
					_monitor_mode_info *mm = &mi->modes[mode];

					if ((mm->w != mode_info_new->w) || (mm->h != mode_info_new->h) || (mm->rrate != mode_info_new->rrate)) {
						continue;
					}
					if (mode == mi->mode_in_use) {
						mode_new = -1;
						break;
					}
					mode_new = mode;
					break;
				}
			}
			free(list);
		}
	}

	if (mode_new != -1) {
		_monitor_mode_info *mode_info_new = &mi->modes[mode_new];

		ufprintf(stderr, uL("gfx_monitor: " uPs("") " switch to %dx%d, %fHz\n"),
			mi->desc, mode_info_new->w, mode_info_new->h, mode_info_new->rrate);

		if (!change_rom_mode) {
			gui_mainwindow_before_set_res();
		}
		gui_monitor_set_res((void *)mi, (void *)mode_info_new);

		mi->mode_new = mode_new;
		mi->mode_in_use = mi->mode_new;

		gui_sleep(250);

		gui_overlay_info_append_msg_precompiled(28, NULL);

		return (TRUE);
	}

	return (FALSE);
}
BYTE gfx_monitor_restore_res(void) {
	_monitor_mode_info *mode_info_org;
	_monitor_info *mi;

	if (gfx.is_wayland || !monitor.enabled || (monitor.active == -1)) {
		return (FALSE);
	}

	mi = &monitor.monitors[monitor.active];

	if ((mi->mode_org == -1) || (mi->mode_new == -1)) {
		return (FALSE);
	}

	mode_info_org = &mi->modes[mi->mode_org];

	ufprintf(stderr, uL("gfx_monitor: " uPs("") " restore to %dx%d, %fHz\n"),
		mi->desc, mode_info_org->w, mode_info_org->h, mode_info_org->rrate);

	gui_monitor_set_res((void *)mi, (void *)mode_info_org);

	mi->mode_new = -1;
	mi->mode_in_use = mi->mode_org;

	gui_sleep(250);

	gui_overlay_info_append_msg_precompiled(28, NULL);

	return (TRUE);
}
BYTE gfx_monitor_mode_in_use_info(int *x, int *y, int *w, int *h, int *rrate) {
	_monitor_mode_info *mode_info;
	_monitor_info *mi;

	if (gfx.is_wayland || !monitor.enabled || (monitor.active == -1)) {
		return (EXIT_ERROR);
	}

	mi = &monitor.monitors[monitor.active];
	mode_info = &mi->modes[mi->mode_in_use];

	if (x && y) {
		gui_monitor_get_current_x_y(mi, x, y);
	}
	if (w) {
		(*w) = mode_info->w;
	}
	if (h) {
		(*h) = mode_info->h;
	}
	if (rrate) {
		(*rrate) = (int)mode_info->rounded_rrate;
	}
	return (EXIT_OK);
}
void gfx_monitor_edid_parse(const uint8_t *edid, _monitor_info *mi) {
	mi->edid = NULL;
	memset(mi->desc, 0x00, sizeof(mi->desc));

	if (edid != NULL) {
		_monitor_edid *me = (_monitor_edid *)malloc(sizeof(_monitor_edid));

		memset(me, 0x00, sizeof(_monitor_edid));

		edid_decode_check_sum(edid, me);

		if (edid_decode_header(edid)
			&& edid_decode_vendor_and_product_identification(edid, me)
			&& edid_decode_descriptors(edid, me)) {
			mi->edid = me;
		} else {
			free(me);
			me = NULL;
		}

		ustrncpy(mi->desc, make_display_name(me), usizeof(mi->desc));
	} else {
		ustrncpy(mi->desc, uL("Unknown Monitor"), usizeof(mi->desc));
	}
}
uTCHAR *gfx_monitor_edid_decode_model(const uint8_t *edid) {
	static uTCHAR model[9];

	memset(model, 0x00, sizeof(model));

	// Manufacturer Code
	model[0] = edid_vendor_byte1(edid);
	model[1] = edid_vendor_byte2(edid);
	model[2] = edid_vendor_byte3(edid);

	usnprintf(model + 3, usizeof(model) - 3, uL("%X%X%X%X"),
		(edid[0x0B] & 0xF0) >> 4, edid[0x0B] & 0x0F, (edid[0x0A] & 0xF0) >> 4, edid[0x0A] & 0x0F);

	return (model);
}

static void enum_resolutions(void) {
	_monitor_resolution *lmr = NULL;
	int a, b, c, nres = 0;

	free_resolutions();

	// primo passaggio : identifico le risoluzioni univoche
	for (a = 0; a < monitor.nmonitor; a++) {
		_monitor_info *m = &monitor.monitors[a];

		for (b = 0; b < m->nmodes; b++) {
			_monitor_mode_info *mi = &m->modes[b];
			BYTE finded = FALSE;

			for (c = 0; c < nres; c++) {
				_monitor_resolution *mr = &lmr[c];

				if ((mr->w == mi->w) && (mr->h == mi->h)) {
					finded = TRUE;
					break;
				}
			}
			if (!finded) {
				_monitor_resolution *mr = NULL, *list = NULL;

				list = (_monitor_resolution *)realloc(lmr, (nres + 1) * sizeof(_monitor_resolution));
				if (!list) {
					free(lmr);
					return;
				}
				lmr = list;
				mr = &lmr[nres];
				memset(mr, 0x00, sizeof(_monitor_resolution));
				nres++;

				mr->w = mi->w;
				mr->h = mi->h;
			}
		}
	}

	// secondo passaggio : identifico le risoluzioni comuni a tutti i monitors
	for (a = 0; a < nres; a++) {
		_monitor_resolution *res = &lmr[a];
		int nmonitor = 0;

		for (b = 0; b < monitor.nmonitor; b++) {
			_monitor_info *m = &monitor.monitors[b];

			for (c = 0; c < m->nmodes; c++) {
				_monitor_mode_info *mi = &m->modes[c];

				if ((res->w == mi->w) && (res->h == mi->h)) {
					nmonitor++;
					break;
				}
			}
		}
		if (nmonitor == monitor.nmonitor) {
			_monitor_resolution *mr = NULL, *list = NULL;

			list = (_monitor_resolution *)realloc(monitor.resolutions, (monitor.nres + 1) * sizeof(_monitor_resolution));
			if (!list) {
				free(lmr);
				free_resolutions();
				return;
			}
			monitor.resolutions = list;
			mr = &monitor.resolutions[monitor.nres];
			memset(mr, 0x00, sizeof(_monitor_resolution));
			monitor.nres++;

			mr->w = res->w;
			mr->h = res->h;
		}
	}

	free(lmr);

	{
		int index;

		// ordino le risoluzioni dalle più alte alle più basse
		// (windows me le da in ordine ascendente, linux in ordine discendente, openbsd in ordine sparso)
		for (a = 0; a < monitor.nres - 1; a++) {
			index = a;

			// trovo la risoluzione piu' alta nella lista
			for (b = a + 1; b < monitor.nres; b++) {
				if (monitor.resolutions[b].w > monitor.resolutions[index].w) {
					index = b;
				} else if (monitor.resolutions[b].w == monitor.resolutions[index].w) {
					if (monitor.resolutions[b].h > monitor.resolutions[index].h) {
						index = b;
					}
				}
			}

			// cambio la risoluzione più elevata con il primo slot disponibile
			{
				_monitor_resolution *mi1 = &monitor.resolutions[index];
				_monitor_resolution *mi2 = &monitor.resolutions[a];
				_monitor_resolution tmp;

				tmp.w = mi1->w;
				tmp.h = mi1->h;

				mi1->w = mi2->w;
				mi1->h = mi2->h;
				mi2->w = tmp.w;
				mi2->h = tmp.h;
			}
		}
	}
}
static void free_resolutions(void) {
	monitor.nres = 0;
	if (monitor.resolutions) {
		free(monitor.resolutions);
		monitor.resolutions = NULL;
	}
}
static void edid_decode_check_sum(const uint8_t *edid, _monitor_edid *me) {
	uint8_t check = 0;
	int i;

	for (i = 0; i < 128; ++i) {
		check += edid[i];
	}
	me->checksum = check;
}
static BYTE edid_decode_header(const uint8_t *edid) {
	if (memcmp(edid, "\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00", 8) == 0) {
		return (TRUE);
	}
	return (FALSE);
}
static BYTE edid_decode_vendor_and_product_identification(const uint8_t *edid, _monitor_edid *me) {
	// Manufacturer Code
	me->manufacturer_code[0] = edid_vendor_byte1(edid);
	me->manufacturer_code[1] = edid_vendor_byte2(edid);
	me->manufacturer_code[2] = edid_vendor_byte3(edid);
	me->manufacturer_code[3] = '\0';

	// Product Code
	me->product_code = (edid[0x0B] << 8) | edid[0x0A];

	// Serial Number
	me->serial_number = edid[0x0C] | (edid[0x0D] << 8) | (edid[0x0E] << 16) | (edid[0x0F] << 24);

	// Screen Size / Aspect Ratio
	if ((edid[0x15] == 0) && (edid[0x16] == 0)) {
		me->w_mm = -1;
		me->h_mm = -1;
		me->aspect_ratio = -1.0;
	} else if (edid[0x16] == 0) {
		me->w_mm = -1;
		me->h_mm = -1;
		me->aspect_ratio = 100.0 / (edid[0x15] + 99);
	} else if (edid[0x15] == 0) {
		me->w_mm = -1;
		me->h_mm = -1;
		me->aspect_ratio = 100.0 / (edid[0x16] + 99);
		me->aspect_ratio = 1 / me->aspect_ratio;
	} else {
		me->w_mm = 10 * edid[0x15];
		me->h_mm = 10 * edid[0x16];
	}

	return (TRUE);
}
static BYTE edid_decode_descriptors(const uint8_t *edid, _monitor_edid *me) {
	int i;
	int timing_idx;

	timing_idx = 0;

	for (i = 0; i < 4; ++i) {
		int index = 0x36 + i * 18;

		if ((edid[index + 0] == 0) && (edid[index + 1] == 0)) {
			edid_decode_display_descriptor(edid + index, me);
		} else {
			edid_decode_detailed_timing(edid + index, &me->dtimings[timing_idx++]);
		}
	}

	me->ndtimings = timing_idx;

	return (TRUE);
}
static void edid_decode_lf_string(const uint8_t *s, int nchars, uTCHAR *result) {
	int i;

	for (i = 0; i < nchars; ++i) {
		if (s[i] == 0x0A) {
			*result++ = '\0';
			break;
		} else if (s[i] == 0x00) {
			// Convert embedded 0's to spaces
			*result++ = ' ';
		} else {
			*result++ = s[i];
		}
	}
}
static void edid_decode_display_descriptor(const uint8_t *desc, _monitor_edid *me) {
	switch (desc[0x03]) {
		case 0xFC:
			edid_decode_lf_string(desc + 5, 13, me->dsc_product_name);
			break;
		case 0xFF:
			edid_decode_lf_string(desc + 5, 13, me->dsc_serial_number);
			break;
		case 0xFE:
			edid_decode_lf_string(desc + 5, 13, me->dsc_string);
			break;
		case 0xFD: // Range Limits
		case 0xFB: // Color Point
		case 0xFA: // Timing Identifications
		case 0xF9: // Color Management
		case 0xF8: // Timing Codes
		case 0xF7: // Established Timings
		case 0x10:
			break;
	}
}
static void edid_decode_detailed_timing(const uint8_t *timing, _monitor_edid_detailed_timing *dt) {
	dt->w_mm = timing[0x0c] | ((timing[0x0e] & 0xF0) << 4);
	dt->h_mm = timing[0x0d] | ((timing[0x0e] & 0x0F) << 8);
}
static const uTCHAR *pnp_find_vendor(const uTCHAR *code) {
	unsigned int i;

	for (i = 0; i < LENGTH(vendors); i++) {
		const _pnp_vendor *v = &vendors[i];

		if (ustrcmp(v->id, code) == 0)
			return (v->name);
	}

	return (NULL);
}
static uTCHAR *make_display_name(const _monitor_edid *me) {
	int w_mm, h_mm, inches;
	static uTCHAR buff[64];
	const uTCHAR *vendor = NULL;

	memset(buff, 0x00, sizeof(buff));

	if (!me) {
		return (buff);
	}

	vendor = pnp_find_vendor(me->manufacturer_code);

	if (!vendor) {
		BYTE is_good = TRUE;
		unsigned int i;

		for (i = 0; i < (usizeof(me->manufacturer_code) - 1); i++) {
			if (((me->manufacturer_code[i] < 'A') || (me->manufacturer_code[i] > 'Z')) &&
				((me->manufacturer_code[i] < 'a') || (me->manufacturer_code[i] > 'z'))) {
				is_good = FALSE;
				break;
			}
		}
		if (is_good) {
			vendor = &me->manufacturer_code[0];
		}
	}

	if (vendor) {
		w_mm = -1;
		h_mm = -1;

		if ((me->w_mm != -1) && me->h_mm) {
			w_mm = me->w_mm;
			h_mm = me->h_mm;
		} else if (me->ndtimings) {
			w_mm = me->dtimings[0].w_mm;
			h_mm = me->dtimings[0].h_mm;
		}

		if ((w_mm != -1) && (h_mm != -1)) {
			double d = sqrt(w_mm * w_mm + h_mm * h_mm);

			inches = (int)(d / 25.4f + 0.5f);
		} else {
			inches = -1;
		}

		if (inches > 0) {
			usnprintf(buff, usizeof(buff), uL("" uPs("") " %d\""), vendor, inches);
		} else {
			usnprintf(buff, usizeof(buff), uL("" uPs("")), vendor);
		}
	}
	return (buff);
}
static int search_rrate(_monitor_mode_info *list, int nmodes, int rrate) {
	int mode;

	if (!list || (nmodes == 0)) {
		return (-1);
	}

	for (mode = 0; mode < nmodes; mode++) {
		_monitor_mode_info *mm = &list[mode];

		if (rrate == -1) {
			if (mm->rounded_rrate < 50) {
				continue;
			}
			return (mode);
		}
		if (((int)mm->rounded_rrate != rrate)) {
			continue;
		}
		return (mode);
	}
	return (-1);
}
#if !defined (RELEASE)
static void print_info(void) {
	int a, b;

	for (a = 0; a < monitor.nmonitor; a++) {
		_monitor_info *mi = &monitor.monitors[a];

		ufprintf(stderr, uL("gfx_monitor : %d - " uPs("") " - %dx%d - %dx%d " uPs("") "\n"), a, !mi->desc[0] ? mi->name :  mi->desc,
			mi->w, mi->h, mi->x, mi->y, mi->in_use ? uL("(puNES)") : uL(""));

		for (b = 0; b < mi->nmodes; b++) {
			_monitor_mode_info *mm = &mi->modes[b];

			ufprintf(stderr, uL("\t%2d : %4dx%-4d    %10f %10f" uPs("")"\n"), b,
				mm->w, mm->h,
				mm->rrate, mm->rounded_rrate,
				mi->modes[mi->mode_org].id == mm->id ? uL(" *") : uL(""));
		}
	}

	if (monitor.nres) {
		ufprintf(stderr, uL("gfx_monitor : common resolutions :\n"));

		for (a = 0; a < monitor.nres; a++) {
			_monitor_resolution *mr = &monitor.resolutions[a];

			ufprintf(stderr, uL("\t%2d : %4dx%-4d\n"), a, mr->w, mr->h);
		}
	} else {
		ufprintf(stderr, uL("gfx_monitor : no valid resolution found\n"));
	}
}
#endif

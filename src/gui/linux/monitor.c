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

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <X11/Xatom.h>
#include "video/gfx_monitor.h"

static void free_resources_on_err(XRRScreenResources *sr, XRRCrtcInfo *ci, XRROutputInfo *oi, Display *dpy);

BYTE gui_monitor_enum_monitors(void) {
	Display *display = XOpenDisplay(NULL);
	XRRScreenResources *sr;
	Window root;
	int crtc;

	{
		int eventb, errorb, major, minor;

		monitor.enabled = FALSE;
		if (XRRQueryExtension(display, &eventb, &errorb) && XRRQueryVersion(display, &major, &minor)) {
			monitor.enabled = (major > 1 || minor >= 3);
		}
	}

	if (!monitor.enabled) {
		XCloseDisplay(display);
		return (EXIT_ERROR);
	}

	root = RootWindow(display, 0);
	sr = XRRGetScreenResourcesCurrent(display, root);

	for (crtc = 0; crtc < sr->noutput; crtc++) {
		XRRCrtcInfo *ci = XRRGetCrtcInfo(display, sr, sr->crtcs[crtc]);
		int output;

		if (ci == NULL) {
			continue;
		}

		for (output = 0; output < ci->noutput; ++output) {
			XRROutputInfo *oi = XRRGetOutputInfo(display, sr, ci->outputs[output]);

			if (oi->connection == RR_Connected) {
				_monitor_info *mi = NULL, *list = NULL;
				int omode;

				// memorizzo le informazioni necessarie
				list = (_monitor_info *)realloc(monitor.monitors, (monitor.nmonitor + 1) * sizeof(_monitor_info));
				if (!list) {
					free_resources_on_err(sr, ci, oi, display);
					return (EXIT_ERROR);
				}
				monitor.monitors = list;
				mi = &monitor.monitors[monitor.nmonitor];
				memset(mi, 0x00, sizeof(_monitor_info));

				ustrncpy(mi->name, oi->name, sizeof(mi->name) - 1);

				mi->in_use = FALSE;
				mi->crtc = sr->crtcs[crtc];
				mi->rotation = ci->rotation;
				mi->x = ci->x;
				mi->y = ci->y;
				mi->w = (int)ci->width;
				mi->h = (int)ci->height;
				mi->output = ci->outputs[output];
				mi->mode_org = -1;
				mi->mode_new = -1;
				mi->mode_in_use = -1;

				monitor.nmonitor++;

				{
					int nprops, property;
					Atom *lp = XRRListOutputProperties(display, mi->output, &nprops);

					for (property = 0; property < nprops; property++) {
						if (strcmp(XGetAtomName(display, lp[property]), "EDID") == 0) {
							unsigned long nitems, bytes_after;
							unsigned char *value;
							int actual_format;
							Atom actual_type;
							uint8_t *edid = NULL;

							XRRGetOutputProperty(display, mi->output, lp[property],
								0, 128, FALSE, FALSE, AnyPropertyType,
								&actual_type, &actual_format, &nitems, &bytes_after, &value);

							if ((actual_type == XA_INTEGER) && (actual_format == 8) && (nitems >= 128) && value) {
								edid = (uint8_t *)value;
							}
							gfx_monitor_edid_parse(edid, mi);
							XFree(value);
							break;
						}
					}
				}

				for (omode = 0; omode < oi->nmode; ++omode) {
					RRMode output_mode = oi->modes[omode];
					XRRModeInfo *mode_info = NULL;
					int smode;

					for (smode = 0; smode < sr->nmode; ++smode) {
						mode_info = &sr->modes[smode];

						if (mode_info->id == output_mode) {
							break;
						}
					}

					if (mode_info == NULL) {
						free_resources_on_err(sr, ci, oi, display);
						return (EXIT_ERROR);
					}

					// con la macchina virtuale dell'OpenBSD alcune risoluzioni hanno una frequenza pari a 0
					if (!mode_info->dotClock || !mode_info->hTotal || !mode_info->vTotal) {
						continue;
					}

					//if ((mode_info.modeFlags & RR_Interlace) == 0) {
					{
						_monitor_mode_info *mm = NULL, *lst = NULL;

						lst = (_monitor_mode_info *)realloc(mi->modes, (mi->nmodes + 1) * sizeof(_monitor_mode_info));
						if (!lst) {
							free_resources_on_err(sr, ci, oi, display);
							return (EXIT_ERROR);
						}
						mi->modes = lst;
						mm = &mi->modes[mi->nmodes];
						memset(mm, 0x00, sizeof(_monitor_mode_info));

						mm->id = mode_info->id;
						mm->flags = mode_info->modeFlags;
						mm->w = (int)mode_info->width;
						mm->h = (int)mode_info->height;
						if (mode_info->hTotal && mode_info->vTotal) {
							mm->rrate = ((double)mode_info->dotClock / ((double)mode_info->hTotal * (double)mode_info->vTotal));
						} else {
							mm->rrate = -1;
						}
						mm->rounded_rrate = round(mm->rrate);
						if (ci->mode == mm->id) {
							mi->mode_org = mi->mode_in_use = mi->nmodes;
						}
						mi->nmodes++;
					}
				}
			}
			XRRFreeOutputInfo(oi);
		}
		XRRFreeCrtcInfo(ci);
	}
	XRRFreeScreenResources(sr);
	XCloseDisplay(display);

	return (EXIT_OK);
}
void gui_monitor_set_res(void *monitor_info, void *mode_info) {
	_monitor_info *mi = (_monitor_info *)monitor_info;
	_monitor_mode_info *mmi = (_monitor_mode_info *)mode_info;
	Display *display = XOpenDisplay(NULL);
	Window root = RootWindow(display, 0);
	XRRScreenResources *sr = XRRGetScreenResources(display, root);
	XRRCrtcInfo *ci = XRRGetCrtcInfo(display, sr, mi->crtc);

	XRRSetCrtcConfig(display, sr, mi->crtc, CurrentTime, ci->x, ci->y, mmi->id, ci->rotation, ci->outputs, ci->noutput);

	XRRFreeCrtcInfo(ci);
	XRRFreeScreenResources(sr);
	XCloseDisplay(display);
}
void gui_monitor_get_current_x_y(void *monitor_info, int *x, int *y) {
	_monitor_info *mi = (_monitor_info *)monitor_info;
	Display *display = XOpenDisplay(NULL);
	Window root = RootWindow(display, 0);
	XRRScreenResources *sr = XRRGetScreenResourcesCurrent(display, root);
	XRRCrtcInfo *ci = XRRGetCrtcInfo(display, sr, mi->crtc);

	(*x) = ci->x;
	(*y) = ci->y;

	XRRFreeCrtcInfo(ci);
	XRRFreeScreenResources(sr);
	XCloseDisplay(display);
}

static void free_resources_on_err(XRRScreenResources *sr, XRRCrtcInfo *ci, XRROutputInfo *oi, Display *dpy) {
	if (sr) {
		XRRFreeScreenResources(sr);
	}
	if (ci) {
		XRRFreeCrtcInfo(ci);
	}
	if (oi) {
		XRRFreeOutputInfo(oi);
	}
	if (dpy) {
		XCloseDisplay(dpy);
	}
}

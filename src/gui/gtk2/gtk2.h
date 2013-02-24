/*
 * gtk2.h
 *
 *  Created on: 21/lug/2011
 *      Author: fhorse
 */

#ifndef GTK2_H_
#define GTK2_H_

#include <gtk/gtk.h>
#include "common.h"
#include "emu.h"
#include "keyboard.h"
#include "joystick.h"
#include "cfginput.h"
#ifdef __NETPLAY__
#include <arpa/inet.h>
#include "netplay.h"
#endif

#define g_timeout_redraw_start()\
		redraw = TRUE;\
		g_timeout_add(25, (GSourceFunc) time_handler_redraw, NULL)
#define g_timeout_redraw_stop()\
		redraw = FALSE
#define exit_thread(value) g_thread_exit(value)
#define icon_inline(wid, icn)\
{\
	GdkPixbuf *pixbuf;\
	pixbuf = gdk_pixbuf_new_from_inline(-1, icn, FALSE, NULL);\
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(wid), gtk_image_new_from_pixbuf(pixbuf));\
	g_object_unref(pixbuf), pixbuf = NULL;\
}

enum { IN = 1, OUT = 2, SPACING = 4 };

typedef struct {
	void *arg[2];
} _guievent;
typedef struct {
	const gchar *home;
	const gchar *last_state_path;
	struct timeval counterStart;
	int8_t cpu_cores;
	uint8_t start;
	int x;
	int y;
	uint8_t left_button;
	uint8_t right_button;
} _gui;

_gui gui;

GtkWidget *mainWin, *hboxss, *saveslot, *sssave, *ssload;
gboolean guiupdate, redraw;

void guiInit(int argc, char **argv);
BYTE guiCreate(void);
void guiSetVideoMode(void);
void guiStart(void);
void guiEvent(void);
GdkNativeWindow guiWindowID(void);
void guiUpdate(void);
void guiFullscreen(void);
void guiTimeline(void);
void guiSavestate(BYTE slot);
double (*guiGetMs)(void);
int guiSleep(double ms);
void guiAddEvent(void *funct, void *args);
void guiFlush(void);
void guiSetThreadAffinity(uint8_t core);

gboolean time_handler_redraw(void);
void make_reset(int type);
void file_open(void);
void mainWin_destroy(void);
void help_about(void);

#endif /* GTK2_H_ */

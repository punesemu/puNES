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
#include "cfg_input.h"
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

GtkWidget *main_win, *hboxss, *saveslot, *sssave, *ssload;
gboolean gui_in_update, redraw;

void gui_init(int argc, char **argv);
void gui_quit(void);
BYTE gui_create(void);
void gui_set_video_mode(void);
void gui_start(void);
void gui_event(void);
GdkNativeWindow gui_emu_frame_id(void);
void gui_update(void);
void gui_fullscreen(void);
void gui_timeline(void);
void gui_save_slot(BYTE slot);
double (*gui_get_ms)(void);
void gui_sleep(double ms);
void gui_add_event(void *funct, void *args);
void gui_flush(void);
void gui_set_thread_affinity(uint8_t core);

gboolean time_handler_redraw(void);
void make_reset(int type);
void file_open(void);
void main_win_destroy(void);
void help_about(void);

#endif /* GTK2_H_ */

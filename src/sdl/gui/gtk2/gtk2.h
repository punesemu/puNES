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
#include "gtk_wrap.h"
#include "emu.h"
#include "keyboard.h"
#include "joystick.h"
#if defined (__NETPLAY__)
#include <arpa/inet.h>
#include "netplay.h"
#endif

#define g_timeout_redraw_start()\
	redraw = TRUE;\
	g_timeout_add(25, (GSourceFunc) time_handler_redraw, NULL)
#define g_timeout_redraw_stop()\
	redraw = FALSE
#define exit_thread(value) g_thread_exit(value)

enum { IN = 1, OUT = 2, SPACING = 4 };

typedef struct {
	void *arg[2];
} _guievent;
typedef struct {
	const gchar *home;
	const gchar *last_state_path;
	uint8_t freed_last_state_path;
	struct timeval counterStart;
	int8_t cpu_cores;
	uint8_t start;
	int x;
	int y;
	uint8_t left_button;
	uint8_t right_button;
	/* lost focus pause */
	uint8_t main_win_lfp;
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
void gui_print_usage(char *usage);
void gui_reset_video(void);
int gui_uncompress_selection_dialog();

gboolean time_handler_redraw(void);
void make_reset(int type);
void file_open(void);
gboolean main_win_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data);
void file_open_filter_add(GtkWidget *filechooser, const gchar *title, const gchar *pattern);
void help_about(void);
void change_rom(char *rom);

#endif /* GTK2_H_ */

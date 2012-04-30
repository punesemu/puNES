/*
 * gtk2.c
 *
 *  Created on: 21/lug/2011
 *      Author: fhorse
 */

#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <string.h>
#include "gtk2.h"
#include "icon.h"
#include "overscan.h"
#include "sdlgfx.h"
#include "sdlsnd.h"
#include "clock.h"
#define _INPUTINLINE_
#include "input.h"
#undef  _INPUTINLINE_
#include "palette.h"
#include "version.h"
#include "cfgfile.h"
#include "timeline.h"
#include "ppu.h"
#include "fps.h"
#include "savestate.h"
#include "tas.h"
#include "gamegenie.h"
#ifdef OPENGL
#include "opengl.h"
#endif
#include "menu/menu.h"

#define tlPressed(type)\
	emuPause(TRUE, SNDNOSYNC);\
	type = TRUE;\
	if (tl.snapsFill) {\
		/* faccio lo screenshot dello screen attuale */\
		memcpy(tl.snaps[TLSNAPFREE] + tl.preview, screen.data, screenSize());\
	}
#define tlRelease(type)\
	if (tl.snapsFill) {\
		BYTE snap = gtk_range_get_value(GTK_RANGE(timeline));\
		if (snap != (tl.snapsFill - 1)) {\
			timelineBack(TLNORMAL, snap);\
		}\
	}\
	gtk_widget_grab_focus(GTK_WIDGET(sock));\
	type = FALSE;\
	emuPause(FALSE, 2000)

enum {
	COLUMN_STRING, COLUMN_INT, N_COLUMNS
};

typedef struct {
	GdkWindow *window;
	gint width;
	gint height;
	gint x;
	gint y;
	gint save_y;
	gint save_index;
	gint counter;
	gboolean popup;
	gint mode;
	gboolean keyboard;
	gint key;
	guint last_key;
	gint selected;
	gulong hook_id;
}  _trcb;

double highResolutionMs(void);
gboolean mainWin_configure_event(void);
gboolean sock_key_press_event(GtkWidget *widget, GdkEventKey *event);
gboolean sock_key_release_event(GtkWidget *widget, GdkEventKey *event);
gboolean mouse_button_press_release_event(GtkWidget *widget, GdkEventButton *event);
gboolean mouse_motion_notify_event(GtkWidget *widget, GdkEventMotion *event);

void file_open_filter_add(GtkWidget *filechooser, const gchar *title, const gchar *pattern);
gboolean timeline_value_changed(GtkRange *range);
gboolean timeline_button_press_release_event(GtkWidget *widget, GdkEventButton *event);
gchar *timeline_format_value(GtkScale *scale, gdouble value);
void saveslot_changed(GtkComboBox *widget);
void saveslot_control(GtkCellLayout *cell_layout, GtkCellRenderer *cell, GtkTreeModel *tree_model,
		GtkTreeIter *iter, gpointer data);
void saveslot_notify_popup_shown(GtkComboBox *widget);
void saveslot_preview(void);
gboolean saveslot_key_press_event(GSignalInvocationHint *ihint, guint n_param_values,
		const GValue *param_values, gpointer data);
int __nsleep(const struct timespec *req, struct timespec *rem);

_trcb trcb;

GtkWidget *sock, *hbox;
GtkWidget *hboxtl, *timeline;
GtkAllocation alSeparator;
GdkRectangle monitor;
gint guiPosX, guiPosY;

void guiInit(int argc, char **argv) {
	memset(&gui, 0, sizeof(_gui));
	guiPosX = guiPosY = 100;
	info.gui = TRUE;
	guiupdate = FALSE;

	g_thread_init(NULL);
	gdk_threads_init();
	gtk_init(&argc, &argv);

	/* cerco il numero dei cores */
	{
		long nprocs = -1;

		nprocs = sysconf(_SC_NPROCESSORS_ONLN);
		gui.cpu_cores = nprocs;
	}

	/* cerco la HOME e imposto la directory base */
	{
		gui.home = g_getenv("HOME");

		if (!gui.home) {
			gui.home = g_get_home_dir();
		}

		sprintf(info.baseFolder, "%s/.%s", gui.home, NAME);
	}

	gettimeofday(&gui.counterStart, NULL);
	guiGetMs = highResolutionMs;
}
BYTE guiCreate(void) {
	GtkWidget *vbox, *separator;

	/* main window */
	mainWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_icon(GTK_WINDOW(mainWin),
			gdk_pixbuf_new_from_inline(-1, icon_inline, FALSE, NULL));

	/* destroy event */
	g_signal_connect(G_OBJECT(mainWin), "destroy", G_CALLBACK(mainWin_destroy), NULL);

	/* configure event */
	g_signal_connect(G_OBJECT(mainWin), "configure-event", G_CALLBACK(mainWin_configure_event),
			NULL);

	/* la finestra non e' ridimensionabile */
	gtk_window_set_resizable(GTK_WINDOW(mainWin), FALSE);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(mainWin), vbox);

	/* menu */
	menu_create(mainWin, vbox);

	/* sdl window */
	{
		GtkWidget *event_box;

		event_box = gtk_event_box_new();
		gtk_box_pack_start(GTK_BOX(vbox), event_box, TRUE, FALSE, 0);

		gtk_widget_add_events(event_box, GDK_BUTTON_PRESS_MASK);
		gtk_widget_add_events(event_box, GDK_POINTER_MOTION_MASK);
		gtk_widget_add_events(event_box, GDK_BUTTON_RELEASE_MASK);

		/* mouse event */
		g_signal_connect(G_OBJECT(event_box), "button-press-event",
				G_CALLBACK(mouse_button_press_release_event), NULL);
		g_signal_connect(G_OBJECT(event_box), "motion-notify-event",
				G_CALLBACK(mouse_motion_notify_event), NULL);
		g_signal_connect(G_OBJECT(event_box), "button-release-event",
				G_CALLBACK(mouse_button_press_release_event), NULL);

		sock = gtk_socket_new();
		gtk_container_add(GTK_CONTAINER(event_box), sock);

		/* indico che l'sdl puo' avere il focus */
		gtk_widget_set_can_focus(GTK_WIDGET(sock), TRUE);

		/* keyboard event */
		g_signal_connect(G_OBJECT(sock), "key-press-event", G_CALLBACK(sock_key_press_event), NULL);
		g_signal_connect(G_OBJECT(sock), "key-release-event", G_CALLBACK(sock_key_release_event),
		        NULL);
	}

	/* Toolbar */
	hbox = gtk_hbox_new(FALSE, SPACING);

	separator = gtk_vseparator_new();
	gtk_widget_get_allocation(GTK_WIDGET(separator), &alSeparator);
	alSeparator = GTK_WIDGET(separator)->allocation;

	/* Timeline */
	{
		PangoFontDescription *font_desc = pango_font_description_new();

		hboxtl = gtk_hbox_new(FALSE, SPACING);

		gtk_widget_set_size_request(GTK_WIDGET(hboxtl), SCRROWS, -1);

		gtk_box_pack_end(GTK_BOX(hbox), hboxtl, FALSE, FALSE, 0);

		timeline = gtk_hscale_new_with_range(0, TLSNAPS - 1, 1);

		/* il timeline non puo' avere il focus */
		gtk_widget_set_can_focus(GTK_WIDGET(timeline), FALSE);

		gtk_widget_set_size_request(GTK_WIDGET(timeline),
				SCRROWS - (alSeparator.width * 2) - (SPACING * 2), -1);

		gtk_scale_set_digits(GTK_SCALE(timeline), 0);
		gtk_scale_set_value_pos(GTK_SCALE(timeline), GTK_POS_LEFT);
		gtk_scale_set_draw_value(GTK_SCALE(timeline), TRUE);

		gtk_range_set_increments(GTK_RANGE(timeline), 1, 0);

		pango_font_description_set_family_static(font_desc, "monospace");
		gtk_widget_modify_font(timeline, font_desc);
		pango_font_description_free(font_desc);

		{
			BYTE i;

			for (i = 0; i < TLSNAPS; i++) {
				gtk_scale_add_mark(GTK_SCALE(timeline), i, GTK_POS_RIGHT, NULL);
			}
		}

		gtk_box_pack_end(GTK_BOX(hboxtl), gtk_vseparator_new(), FALSE, FALSE, 0);
		gtk_box_pack_end(GTK_BOX(hboxtl), timeline, FALSE, FALSE, 0);
		gtk_box_pack_end(GTK_BOX(hboxtl), gtk_vseparator_new(), FALSE, FALSE, 0);

		g_signal_connect(G_OBJECT(timeline), "value-changed", G_CALLBACK(timeline_value_changed),
				NULL);
		g_signal_connect(G_OBJECT(timeline), "format-value", G_CALLBACK(timeline_format_value),
				NULL);

		gtk_widget_add_events(GTK_WIDGET(timeline), GDK_BUTTON_PRESS_MASK);
		gtk_widget_add_events(GTK_WIDGET(timeline), GDK_BUTTON_RELEASE_MASK);

		/* mouse event */
		g_signal_connect(G_OBJECT(timeline), "button-press-event",
				G_CALLBACK(timeline_button_press_release_event), NULL);
		g_signal_connect(G_OBJECT(timeline), "button-release-event",
				G_CALLBACK(timeline_button_press_release_event), NULL);

		gtk_widget_show(hboxtl);
	}

	/* SaveSlot */
	{
		GtkListStore *store;
		GtkCellRenderer *renderer;

		hboxss = gtk_hbox_new(FALSE, SPACING);

		gtk_box_pack_end(GTK_BOX(hbox), hboxss, FALSE, FALSE, 0);

		sssave = gtk_button_new_with_label("save");
		saveslot = gtk_combo_box_new();
		ssload = gtk_button_new_with_label("load");

		gtk_box_pack_start(GTK_BOX(hboxss), gtk_vseparator_new(), FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hboxss), sssave, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hboxss), saveslot, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hboxss), ssload, FALSE, FALSE, 0);

		store = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_INT);

		{
			BYTE i;

			for (i = 0; i < SSAVAILABLE; i++) {
				char item[10];
				GtkTreeIter iter;

				gtk_list_store_append(store, &iter);
				sprintf(item, "Slot %d", i);
				gtk_list_store_set(store, &iter, COLUMN_STRING, item, COLUMN_INT, i, -1);
			}
		}

		gtk_combo_box_set_model(GTK_COMBO_BOX(saveslot), GTK_TREE_MODEL(store));

		g_object_unref(store);

		gtk_combo_box_set_active(GTK_COMBO_BOX(saveslot), 0);

		renderer = gtk_cell_renderer_text_new();
		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(saveslot), renderer, TRUE);
		gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(saveslot), renderer, "text", COLUMN_STRING);
		gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(saveslot), renderer, saveslot_control,
				NULL, NULL);

		g_signal_connect(G_OBJECT(saveslot), "changed", G_CALLBACK(saveslot_changed), NULL);
		g_signal_connect(G_OBJECT(saveslot), "notify::popup-shown",
				G_CALLBACK(saveslot_notify_popup_shown), NULL);
		g_signal_connect_swapped(G_OBJECT(sssave), "clicked",
		        G_CALLBACK(menu_state_saveslot_action), GINT_TO_POINTER(SAVE));
		g_signal_connect_swapped(G_OBJECT(ssload), "clicked",
		        G_CALLBACK(menu_state_saveslot_action),
				GINT_TO_POINTER(LOAD));

		/* il saveslot non puo' avere il focus */
		gtk_combo_box_set_focus_on_click(GTK_COMBO_BOX(saveslot), FALSE);
		gtk_button_set_focus_on_click(GTK_BUTTON(sssave), FALSE);
		gtk_button_set_focus_on_click(GTK_BUTTON(ssload), FALSE);

		gtk_widget_show(hboxss);
	}

	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	gtk_widget_show_all(hbox);

	gtk_widget_show_all(vbox);

	gtk_widget_show_all(mainWin);

	g_object_ref_sink(separator);

	return (EXIT_OK);
}
void guiSetVideoMode(void) {
	WORD rows = SCRROWS;

	if (gfx.scale == X1) {
		gtk_widget_hide(hboxss);

		if (overscan.enabled) {
			menu_help_hide();
			rows = gfx.rows;
		} else {
			menu_help_show();
		}
	} else {
		gtk_widget_show(hboxss);
		menu_help_show();
	}
	gtk_widget_set_size_request(GTK_WIDGET(hboxtl), rows, -1);
	gtk_widget_set_size_request(GTK_WIDGET(timeline),
			rows - (alSeparator.width * 2) - (SPACING * 2), -1);
	gtk_widget_set_size_request(sock, gfx.w[VIDEOMODE], gfx.h[VIDEOMODE]);
}
void guiStart(void) {
	gtk_idle_add((GtkFunction) emuLoop, mainWin);
	gui.start = TRUE;
	gtk_main();
}
void guiEvent(void) {
	while (gtk_events_pending()) {
		gtk_main_iteration();
	}

	if (info.no_rom | info.pause) {
		return;
	}

	if (tas.type) {
		tasFrame();
		return;
	}

	jsControl(&js1, &port1);
	/* i due joystick non possono essere gli stessi */
	if (port2.joyID != port1.joyID) {
		jsControl(&js2, &port2);
	}
	inputTurboButtonsControl(&port1);
	inputTurboButtonsControl(&port2);
}
GdkNativeWindow guiWindowID(void) {
	return (gtk_socket_get_id(GTK_SOCKET(sock)));
}
void guiUpdate(void) {
	char title[255];

	guiupdate = TRUE;

	/* aggiorno il titolo */
	emuSetTitle(title);
	gtk_window_set_title(GTK_WINDOW(mainWin), title);

	menu_nes_check();
	menu_settings_check();
	menu_state_check();

	guiupdate = FALSE;
	guiFlush();
}
#ifdef OPENGL
void guiFullscreen(void) {
	if (guiupdate) {
		return;
	}

	guiFlush();

	/* Fullscreen */
	if (gfx.fullscreen == NOFULLSCR || gfx.fullscreen == NOCHANGE) {
		/* salvo il valore scale prima del fullscreen */
		gfx.scaleBeforeFullscreen = gfx.scale;
		/* trovo la risoluzione del monitor in uso */
		{
			GdkScreen *screen = gtk_window_get_screen(GTK_WINDOW(mainWin));
			gdk_screen_get_monitor_geometry(screen,
			        gdk_screen_get_monitor_at_window(screen, GTK_WIDGET(mainWin)->window),
			        &monitor);
			gfx.w[MONITOR] = monitor.width;
			gfx.h[MONITOR] = monitor.height;
		}
		/* salvo la posizione */
		gtk_window_get_position(GTK_WINDOW(mainWin), &guiPosX, &guiPosY);
		/* nascondo la finestra */
		if (gui.start) {
			gtk_widget_hide(mainWin);
		}
		/* nascondo il menu */
		menu_hide();
		/* nascondo la toolbar */
		gtk_widget_hide(hbox);
		/* abilito il ridimensionamento della finestra */
		gtk_window_set_resizable(GTK_WINDOW(mainWin), TRUE);
		/* disabilito le decorazioni */
		gtk_window_set_decorated(GTK_WINDOW(mainWin), FALSE);
		/* abilito il fullscreen */
		gfxSetScreen(NOCHANGE, NOCHANGE, FULLSCR, NOCHANGE, FALSE);
		/* nascondo il cursore, se serve */
		if (!opengl.rotation && (port1.type != ZAPPER) && (port2.type != ZAPPER)) {
			SDL_ShowCursor(SDL_DISABLE);
		}
		/* indico alle gtk che sono in fullscreen */
		gtk_window_fullscreen(GTK_WINDOW(mainWin));
		/* muovo la finestra nell'angolo alto a sinistra del monitor */
		gtk_window_move(GTK_WINDOW(mainWin), monitor.x, monitor.y);
	} else {
		/* nascondo la finestra */
		gtk_widget_hide(mainWin);
		/* riabilito le decorazioni */
		gtk_window_set_decorated(GTK_WINDOW(mainWin), TRUE);
		/* ribilito la toolbar */
		gtk_widget_show(hbox);
		/* ribilito il menu */
		menu_show();
		/* esco dal fullscreen */
		gtk_window_unfullscreen(GTK_WINDOW(mainWin));
		/* ripristino i valori di scale ed esco dal fullscreen */
		gfxSetScreen(gfx.scaleBeforeFullscreen, NOCHANGE, NOFULLSCR, NOCHANGE, FALSE);
		/* riabilito la visualizzazione del puntatore */
		SDL_ShowCursor(SDL_ENABLE);
		/* blocco il ridimensionamento */
		gtk_window_set_resizable(GTK_WINDOW(mainWin), FALSE);
		/* riposiziono la finestra nella posizione prima del fullscreen */
		gtk_window_move(GTK_WINDOW(mainWin), guiPosX, guiPosY);
	}

	/* visualizzo la finestra*/
	gtk_widget_show(mainWin);
	/* setto il focus */
	gtk_widget_grab_focus(GTK_WIDGET(sock));

	guiFlush();
}
#endif
void guiTimeline(void) {
	tl.update = TRUE;
	gtk_range_set_value(GTK_RANGE(timeline), tl.snapsFill - 1);
	tl.update = FALSE;
}
void guiSavestate(BYTE slot) {
	if (slot >= SSAVAILABLE) {
		slot = SSAVAILABLE - 1;
	}
	menu_state_saveslot_set(slot);
}
int guiSleep(double ms) {
	struct timespec req = { 0 }, rem = { 0 };
	time_t sec;

	if (ms <= 0) {
		return (EXIT_OK);
	}

	sec = (int) (ms / 1000);
	ms = ms - (sec * 1000);
	req.tv_sec = sec;
	req.tv_nsec = ms * 1000000L;
	__nsleep(&req, &rem);
	return (EXIT_OK);
}
int __nsleep(const struct timespec *req, struct timespec *rem) {
	struct timespec temp_rem;

	if (nanosleep(req, rem) == -1) {
		__nsleep(rem, &temp_rem);
	} else {
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}
void guiAddEvent(void *funct, void *args) {
	g_idle_add((GSourceFunc) funct, (gpointer) args);
}
void guiFlush(void) {
    while (gtk_events_pending()) {
    	gtk_main_iteration();
	}
}
void guiSetThreadAffinity(uint8_t core) {
	cpu_set_t cpuset;
	pthread_t thread;

	thread = pthread_self();
	CPU_ZERO(&cpuset);
	CPU_SET(core, &cpuset);
	pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
}

double highResolutionMs(void) {
	struct timeval time;

	double elapsed_seconds; /* diff between seconds counter */
	double elapsed_useconds; /* diff between microseconds counter */

	gettimeofday(&time, NULL);

    elapsed_seconds  = time.tv_sec  - gui.counterStart.tv_sec;
    elapsed_useconds = time.tv_usec - gui.counterStart.tv_usec;

    //return ((elapsed_seconds * 1000) + (elapsed_useconds / 1000.0f) + 0.5f);
    return ((elapsed_seconds * 1000) + (elapsed_useconds / 1000.0f));
}
/* mainWin */
void mainWin_destroy(void) {
	if (cfg_controllers_toplevel != NULL) {
		gtk_widget_destroy(cfg_controllers_toplevel);
	}

	gtk_main_quit();

	info.stop = TRUE;
}
gboolean mainWin_configure_event(void) {
	sndWmEvent(2000);
	/*
	 * devo fare un return FALSE se voglio che l'evento
	 * sia propagato all'uscita della routine (e deve
	 * esserlo necessariamente).
	 */
	return (FALSE);
}
gboolean sock_key_press_event(GtkWidget *widget, GdkEventKey *event) {
	guint keyval = gdk_keyval_to_lower(event->keyval);

	switch (keyval) {
		case GDK_KEY_Control_L:
			if (!tl.key) {
				tlPressed(tl.key);
			}
			return (TRUE);
		case GDK_KEY_Shift_L:
			fpsFastForward();
			return (TRUE);
		case GDK_Left:
			if (tl.key) {
				BYTE snap = gtk_range_get_value(GTK_RANGE(timeline));
				gtk_range_set_value(GTK_RANGE(timeline), snap - 1);
				return (TRUE);
			}
			break;
		case GDK_Right:
			if (tl.key) {
				BYTE snap = gtk_range_get_value(GTK_RANGE(timeline));
				gtk_range_set_value(GTK_RANGE(timeline), snap + 1);
				return (TRUE);
			}
			break;
	}
#ifdef OPENGL
	if (gfx.fullscreen == FULLSCR) {
		switch (keyval) {
			case GDK_a:
				if ((event->state & 0x1F0D) == GDK_CONTROL_MASK) {
					menu_audio_set_audio_enable();
					return (TRUE);
				}
				break;
			case GDK_q:
				if ((event->state & 0x1F0D) == GDK_CONTROL_MASK) {
					guiFullscreen();
					mainWin_destroy();
					return (TRUE);
				}
				break;
			case GDK_r:
				menu_video_effect_set();
				return (TRUE);
			case GDK_w:
				if ((event->state & 0x1F0D) == GDK_CONTROL_MASK) {
					cfgfileSave();
					return (TRUE);
				}
				break;
			case GDK_s:
				if ((event->state & 0x1F0D) == GDK_MOD1_MASK) {
					menu_nes_fds_select_side(0xFFF);
					return (TRUE);
				}
				break;
			case GDK_e:
				if ((event->state & 0x1F0D) == GDK_MOD1_MASK) {
					menu_nes_fds_eject_insert_disk();
					return (TRUE);
				}
				break;
			case GDK_p:
				menu_video_fullscreen_switch_stretch();
				return (TRUE);
			case GDK_F1:
				menu_state_saveslot_action(SAVE);
				return (TRUE);
			case GDK_F2:
				menu_state_saveslot_incdec(DEC);
				return (TRUE);
			case GDK_F3:
				menu_state_saveslot_incdec(INC);
				return (TRUE);
			case GDK_F4:
				menu_state_saveslot_action(LOAD);
				return (TRUE);
			case GDK_F6:
				menu_mode_set_mode(PAL);
				return (TRUE);
			case GDK_F7:
				menu_mode_set_mode(NTSC);
				return (TRUE);
			case GDK_F8:
				menu_mode_set_mode(DENDY);
				return (TRUE);
			case GDK_F9:
				menu_mode_set_mode(AUTO);
				return (TRUE);
			case GDK_Escape:
				guiFullscreen();
				return (TRUE);
			case GDK_Return:
				if ((event->state & 0x1F0D) == GDK_MOD1_MASK) {
					guiFullscreen();
					return (TRUE);
				}
				break;
			case GDK_F11:
				make_reset(HARD);
				return (TRUE);
			case GDK_F12:
				make_reset(RESET);
				return (TRUE);
		}
	}
#endif
	if (inputPort1 && !inputPort1(PRESSED, keyval, KEYBOARD, &port1)) {
		return (TRUE);
	}
	if (inputPort2 && !inputPort2(PRESSED, keyval, KEYBOARD, &port2)) {
		return (TRUE);
	}
	return (FALSE);
}
gboolean sock_key_release_event(GtkWidget *widget, GdkEventKey *event) {
	guint keyval = gdk_keyval_to_lower(event->keyval);

	switch (keyval) {
		case GDK_KEY_Control_L:
			if (tl.key) {
				tlRelease(tl.key);
			}
			return (TRUE);
		case GDK_KEY_Shift_L:
			fpsNormalize();
			return (TRUE);
	}
	if (inputPort1 && !inputPort1(RELEASED, keyval, KEYBOARD, &port1)) {
		return (TRUE);
	}
	if (inputPort2 && !inputPort2(RELEASED, keyval, KEYBOARD, &port2)) {
		return (TRUE);
	}
	return (FALSE);
}
gboolean mouse_button_press_release_event(GtkWidget *widget, GdkEventButton *event) {
	switch (event->type) {
		case GDK_BUTTON_PRESS:
			if (event->button == 1) {
				gui.left_button = TRUE;
#ifdef OPENGL
				opengl.xDiff = event->x - (opengl.yRotate * slowFactor);
				opengl.yDiff = -event->y + (opengl.xRotate * slowFactor);
#endif
			}
			if (event->button == 3) {
				gui.right_button = TRUE;
			}
			break;
		case GDK_BUTTON_RELEASE:
			if (event->button == 1) {
				gui.left_button = FALSE;
			}
			if (event->button == 3) {
				gui.right_button = FALSE;
			}
			break;
		default:
			break;
	}
	return (FALSE);
}
gboolean mouse_motion_notify_event(GtkWidget *widget, GdkEventMotion *event) {
	gui.x = event->x;
	gui.y = event->y;

#ifdef OPENGL
	if (gui.left_button && opengl.rotation) {
		opengl.xRotate = (event->y + opengl.yDiff) / slowFactor;
		opengl.yRotate = (event->x - opengl.xDiff) / slowFactor;
	}
#endif
	return (FALSE);
}

/* open */
void file_open(void) {
	GtkWidget *dialog;

	emuPause(TRUE, SNDNOSYNC);

	/* potrei essere entrato con il CTRL+O */
	tl.key = FALSE;

	dialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(mainWin),
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);

	file_open_filter_add(dialog, "All supported formats", "*.nes;*.NES;*.fds;*.FDS;*.fm2;*.FM2");
	file_open_filter_add(dialog, "Nes rom files", "*.nes;*.NES");
	file_open_filter_add(dialog, "FDS image files", "*.fds;*.FDS");
	file_open_filter_add(dialog, "TAS movie files", "*.fm2;*.FM2");
	file_open_filter_add(dialog, "All files", "*.*;");

	if (info.romFile[0]) {
		gui.last_state_path = g_path_get_dirname(info.romFile);
	} else {
		gui.last_state_path = gui.home;
	}

	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), gui.last_state_path);

	/* ridisegno lo screen sdl ogni tot millisecondi */
	g_timeout_redraw_start();

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		strcpy(info.loadRomFile, gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
		gamegenie_reset(FALSE);
		make_reset(CHANGEROM);
		guiUpdate();
		gui.last_state_path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));
	}
	gtk_widget_destroy(dialog);

	g_timeout_redraw_stop();

	emuPause(FALSE, 2000);
}
void file_open_filter_add(GtkWidget *filechooser, const gchar *title, const gchar *pattern) {
	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, title);
	if (g_strrstr(pattern, ";")) {
		char *p, **patterns = g_strsplit(pattern, ";", 0);
		int n = 0;
		while ((p = patterns[n++])) {
			gtk_file_filter_add_pattern(filter, g_strdup(p));
		}
		g_strfreev(patterns);
	}
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(filechooser), filter);
}
/* about */
void help_about(void) {
	GdkPixbuf *pixbuf;
	GtkWidget *dialog;

	emuPause(TRUE, SNDNOSYNC);

	pixbuf = gdk_pixbuf_new_from_inline(-1, pin_inline, FALSE, NULL);
	dialog = gtk_about_dialog_new();
	gtk_window_set_icon(GTK_WINDOW(dialog),
			gdk_pixbuf_new_from_inline(-1, pin_inline, FALSE, NULL));
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), NAME);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), VERSION);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), AUTHOR);
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), COMMENT);
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), WEBSITE);
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), pixbuf);
	g_object_unref(pixbuf), pixbuf = NULL;

	/* ridisegno lo screen sdl ogni tot millisecondi */
	g_timeout_redraw_start();

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	g_timeout_redraw_stop();

	emuPause(FALSE, 2000);
}
/* reset */
void make_reset(int type) {
	if (type == HARD) {
		if (gamegenie.enabled && gamegenie.rom_present) {
			if (info.mapper != GAMEGENIE_MAPPER) {
				strcpy(info.loadRomFile, info.romFile);
			}
			gamegenie_reset(TRUE);
			type = CHANGEROM;
		} else {
			/*
			 * se e' stato disabilitato il game genie quando ormai
			 * e' gia' in esecuzione e si preme un reset, carico la rom.
			 */
			if (info.mapper == GAMEGENIE_MAPPER) {
				gamegenie_reset(TRUE);
				type = CHANGEROM;
			}
		}
	}

	if (emuReset(type)) {
		mainWin_destroy();
	}
}
/* timeline */
gboolean timeline_value_changed(GtkRange *range) {
	BYTE snap = gtk_range_get_value(GTK_RANGE(range));

	if (tl.update == TRUE) {
		return (FALSE);
	}

	/* se non ci sono snap esco */
	if (!tl.snapsFill) {
		gtk_range_set_value(GTK_RANGE(range), 0);
		return (FALSE);
	}
	/* snap non puo' essere mai maggiore del numero di snap effettuate */
	if (snap > (tl.snapsFill - 1)) {
		gtk_range_set_value(GTK_RANGE(range), tl.snapsFill - 1);
		snap = (tl.snapsFill - 1);
	}
	if (snap == (tl.snapsFill - 1)) {
		memcpy(screen.data, tl.snaps[TLSNAPFREE] + tl.preview, screenSize());
		gfxDrawScreen(TRUE);
		return (FALSE);
	}
	timelinePreview(snap);
	return (FALSE);
}
gboolean timeline_button_press_release_event(GtkWidget *widget, GdkEventButton *event) {
	switch (event->type) {
		case GDK_BUTTON_PRESS:
			if (event->button == 1) {
				tlPressed(tl.button)
			}
			break;
		case GDK_BUTTON_RELEASE:
			if (event->button == 1) {
				tlRelease(tl.button);
			}
			break;
		default:
			break;
	}
	return (FALSE);
}
gchar *timeline_format_value(GtkScale *scale, gdouble value) {
	if (tl.button) {
		BYTE dec = 0;

		if (tl.snapsFill) {
			dec = ((tl.snapsFill - 1) - value) * TLSNAPSEC;
		}

		if (!dec) {
			return (g_strdup_printf("  %d s", 0));
		} else {
			return (g_strdup_printf("% 2d s", -abs(((tl.snapsFill - 1) - value) * TLSNAPSEC)));
		}
	} else {
		return (g_strdup_printf("% 2d s", (int) value * TLSNAPSEC));
	}
}
/* misc */
gboolean time_handler_redraw(void) {
	gfxDrawScreen(TRUE);
	return (redraw);
}

void saveslot_changed(GtkComboBox *widget) {
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (guiupdate) {
		return;
	}

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));

	gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter);

	gtk_tree_model_get(model, &iter, COLUMN_INT, &savestate.slot, -1);

	guiUpdate();

	/* setto il focus */
	gtk_widget_grab_focus(GTK_WIDGET(sock));
}
void saveslot_control(GtkCellLayout *cell_layout, GtkCellRenderer *cell, GtkTreeModel *tree_model,
		GtkTreeIter *iter, gpointer data) {
	guint index;

	enum {
		FIRST_ITER, INC_POINTER, DEC_POINTER, RESET_MODE, OUT_OF_WIDTH
	};
	enum {
		NO_KEY, KEY_UP, KEY_DOWN
	};

	gtk_tree_model_get(tree_model, iter, COLUMN_INT, &index, -1);

	if (trcb.window) {
		gint x, y;

		if (trcb.popup) {
			if (index == trcb.counter) {
				if (++trcb.counter == SSAVAILABLE) {
					GtkTreeIter active_iter;
					GtkTreeModel *model;

					model = gtk_combo_box_get_model(GTK_COMBO_BOX(saveslot));
					gtk_combo_box_get_active_iter(GTK_COMBO_BOX(saveslot), &active_iter);
					gtk_tree_model_get(model, &active_iter, COLUMN_INT, &trcb.selected, -1);

					saveslot_preview();

					trcb.popup = FALSE;
					trcb.counter = 0;
				}
			} else {
				trcb.counter = 0;
			}
		} else {
			gdk_window_get_pointer(GDK_WINDOW(trcb.window), &x, &y, NULL);
			if (trcb.mode < OUT_OF_WIDTH) {
				if ((x >= trcb.width) || (x <= 0)) {
					trcb.mode = OUT_OF_WIDTH;
				} else if (y > trcb.y) {
					trcb.mode = INC_POINTER;
				} else if (y < trcb.y) {
					trcb.mode = DEC_POINTER;
				}
			}
			if (trcb.counter++ == 1) {
				trcb.counter = 0;
				/*
				 * qui tratto il caso in cui l'ultimo stato era stato
				 * dato dalla tastiera e quello attuale e' dato dal movimento
				 * del mouse.
				 */
				if (trcb.last_key == KEY_DOWN) {
					trcb.mode = DEC_POINTER;
					trcb.last_key = NO_KEY;
				} else if (trcb.last_key == KEY_UP) {
					trcb.mode = INC_POINTER;
					trcb.last_key = NO_KEY;
				}
				/*
				 * qui tratto il caso in cui lo stato attuale e'
				 * dato dalla tastiera.
				 */
				if (trcb.keyboard) {
					trcb.keyboard = FALSE;
					switch (trcb.key) {
						case GDK_Up:
						case GDK_KP_Up:
							trcb.last_key = KEY_UP;
							if (trcb.selected == 0) {
								trcb.mode = INC_POINTER;
							} else {
								trcb.mode = DEC_POINTER;
							}
							break;
						case GDK_Home:
						case GDK_Page_Up:
							trcb.mode = RESET_MODE;
							trcb.selected = 0;
							break;
						case GDK_Down:
						case GDK_KP_Down:
							trcb.last_key = KEY_DOWN;
							if (trcb.selected == (SSAVAILABLE - 1)) {
								trcb.mode = DEC_POINTER;
							} else {
								trcb.mode = INC_POINTER;
							}
							break;
						case GDK_End:
						case GDK_Page_Down:
							trcb.mode = RESET_MODE;
							trcb.selected = SSAVAILABLE - 1;
							break;
					}
				}
				switch (trcb.mode) {
					case OUT_OF_WIDTH:
						trcb.selected = index;
						trcb.mode = RESET_MODE;
						break;
					case INC_POINTER:
						trcb.selected = index;
						break;
					case DEC_POINTER:
						trcb.selected = trcb.save_index;
						break;
				}
				saveslot_preview();
			}
			trcb.save_index = index;
			trcb.y = y;
			trcb.x = x;
		}
	}

	if (!savestate.slotState[index]) {
		g_object_set(cell, "foreground", "Gray", "foreground-set", TRUE, NULL);
	} else {
		/* imposto il colore di default */
		g_object_set(cell, "foreground-set", FALSE, NULL);
	}
}
void saveslot_notify_popup_shown(GtkComboBox *widget) {
	gboolean mode;

	g_object_get(GTK_OBJECT(widget), "popup-shown", &mode, NULL);

	if (mode == 1) {
		emuPause(TRUE, SNDNOSYNC);
		/* intercetto il "key-press-event" del combobox */
		trcb.hook_id = g_signal_add_emission_hook(
				g_signal_lookup("key-press-event", GTK_TYPE_WINDOW), 0, saveslot_key_press_event,
				NULL, (GDestroyNotify) NULL);
		trcb.window = gdk_window_at_pointer(NULL, NULL);
		gdk_window_get_geometry(GDK_WINDOW(trcb.window), NULL, NULL, &trcb.width, &trcb.height,
				NULL);
		trcb.popup = TRUE;
		trcb.x = trcb.y = trcb.mode = trcb.counter = trcb.last_key = 0;
	} else {
		trcb.window = NULL;
		g_signal_remove_emission_hook(g_signal_lookup("key-press-event", GTK_TYPE_WINDOW),
				trcb.hook_id);
		savestate.previewStart = FALSE;
		emuPause(FALSE, 500);
	}
}
void saveslot_preview(void) {
	savestatePreview(trcb.selected);
}
gboolean saveslot_key_press_event(GSignalInvocationHint *ihint, guint n_param_values,
		const GValue *param_values, gpointer data) {
	if (trcb.window) {
		GdkEventKey *event = g_value_get_boxed(param_values + 1);
		if ((event->keyval == GDK_Up) || (event->keyval == GDK_KP_Up) || (event->keyval == GDK_Down)
				|| (event->keyval == GDK_KP_Down) || (event->keyval == GDK_Page_Up)
				|| (event->keyval == GDK_Page_Down) || (event->keyval == GDK_Home)
				|| (event->keyval == GDK_End))
			trcb.keyboard = TRUE;
		trcb.key = event->keyval;
	}
	return (TRUE);
}


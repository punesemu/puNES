/*
 * gtk2.c
 *
 *  Created on: 21/lug/2011
 *      Author: fhorse
 */

#if !defined (_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <string.h>
#include <libgen.h>
#include "info.h"
#include "gtk2.h"
#include "icon.h"
#include "overscan.h"
#include "gfx.h"
#include "snd.h"
#include "clock.h"
#include "input.h"
#include "palette.h"
#include "version.h"
#include "cfg_file.h"
#include "timeline.h"
#include "ppu.h"
#include "fps.h"
#include "save_slot.h"
#include "tas.h"
#include "gamegenie.h"
#include "opengl.h"
#include "menu/menu.h"
#include "cfg_input.h"
#include "l7zip/l7z.h"
#include "uncompress_selection.h"

#define tl_pressed(type)\
	emu_pause(TRUE);\
	type = TRUE;\
	if (tl.snaps_fill) {\
		/* faccio lo screenshot dello screen attuale */\
		memcpy(tl.snaps[TL_SNAP_FREE] + tl.preview, screen.data, screen_size());\
	}
#define tl_release(type)\
	if (tl.snaps_fill) {\
		BYTE snap = gtk_range_get_value(GTK_RANGE(timeline));\
		if (snap != (tl.snaps_fill - 1)) {\
			timeline_back(TL_NORMAL, snap);\
		}\
	}\
	gtk_widget_grab_focus(GTK_WIDGET(sock));\
	type = FALSE;\
	emu_pause(FALSE)

enum { COLUMN_STRING, COLUMN_INT, N_COLUMNS };

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

double high_resolution_ms(void);
gboolean main_win_configure_event(void);
gboolean main_win_focus_in_event(GtkWidget *widget, GdkEventFocus *event, gpointer data);
gboolean main_win_focus_out_event(GtkWidget *widget, GdkEventFocus *event, gpointer data);
gboolean sock_key_press_event(GtkWidget *widget, GdkEventKey *event);
gboolean sock_key_release_event(GtkWidget *widget, GdkEventKey *event);
gboolean mouse_button_press_release_event(GtkWidget *widget, GdkEventButton *event);
gboolean mouse_motion_notify_event(GtkWidget *widget, GdkEventMotion *event);

gboolean timeline_value_changed(GtkRange *range);
gboolean timeline_button_press_release_event(GtkWidget *widget, GdkEventButton *event);
gchar *timeline_format_value(GtkScale *scale, gdouble value);
void save_slot_changed(GtkComboBox *widget);
void save_slot_control(GtkCellLayout *cell_layout, GtkCellRenderer *cell, GtkTreeModel *tree_model,
		GtkTreeIter *iter, gpointer data);
void save_slot_notify_popup_shown(GtkComboBox *widget);
void save_slot_gui_preview(void);
gboolean save_slot_key_press_event(GSignalInvocationHint *ihint, guint n_param_values,
		const GValue *param_values, gpointer data);
void drag_data_received(GtkWidget *widget, GdkDragContext *context, gint x, gint y,
		GtkSelectionData *data, guint ttype, guint time, gpointer *NA);
int __nsleep(const struct timespec *req, struct timespec *rem);

_trcb trcb;

GtkWidget *sock, *hbox;
GtkWidget *hboxtl, *timeline;
GtkAllocation alSeparator;
GdkRectangle monitor;
gint gui_pos_x, gui_pos_y;

void gui_init(int argc, char **argv) {
	memset(&gui, 0, sizeof(_gui));
	gui_pos_x = gui_pos_y = 100;
	info.gui = TRUE;
	gui_in_update = FALSE;

	gui.main_win_lfp = TRUE;

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

		if (info.portable) {
			char path[sizeof(info.base_folder)], *dname;
			int length = readlink("/proc/self/exe", path, sizeof(path));

			if (length < 0) {
				fprintf(stderr, "INFO: Error resolving symlink /proc/self/exe.\n");
				info.portable = FALSE;
			} else if (length >= (signed int) sizeof(info.base_folder)) {
				fprintf(stderr, "INFO: Path too long. Truncated.\n");
				info.portable = FALSE;
			} else {
				/*
				 * I don't know why, but the string this readlink() function
				 * returns is appended with a '@'.
				 */
				if (path[length] == '@') {
					path[length] = 0;
				}

				dname = dirname(path);
				strcpy(info.base_folder, dname);
			}
		}

		if (!info.portable) {
			sprintf(info.base_folder, "%s/.%s", gui.home, NAME);
		}
 	}

	gettimeofday(&gui.counterStart, NULL);
	gui_get_ms = high_resolution_ms;
}
BYTE gui_create(void) {
	GtkWidget *vbox, *separator;

	/* main window */
	main_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_icon(GTK_WINDOW(main_win),
			gdk_pixbuf_new_from_inline(-1, icon_inline, FALSE, NULL));

	/* delete-event */
	g_signal_connect(G_OBJECT(main_win), "delete-event", G_CALLBACK(main_win_delete_event), NULL);

	/* configure event */
	//g_signal_connect(G_OBJECT(main_win), "configure-event", G_CALLBACK(main_win_configure_event),
	//		NULL);

	gtk_widget_set_events(main_win, GDK_FOCUS_CHANGE_MASK);
	g_signal_connect(G_OBJECT(main_win), "focus-in-event", G_CALLBACK(main_win_focus_in_event),
			NULL);
	g_signal_connect(G_OBJECT(main_win), "focus-out-event", G_CALLBACK(main_win_focus_out_event),
			NULL);

	/* la finestra non e' ridimensionabile */
	gtk_window_set_resizable(GTK_WINDOW(main_win), FALSE);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(main_win), vbox);

	/* menu */
	menu_create(main_win, vbox);

	/* sdl window */
	{
		GtkWidget *event_box;

		event_box = gtk_event_box_new();
		gtk_box_pack_start(GTK_BOX(vbox), event_box, TRUE, FALSE, 0);

		{
			const GtkTargetEntry target = { (gchar *) "text/uri-list", 0, 0 };

			gtk_drag_dest_set(event_box, GTK_DEST_DEFAULT_ALL, &target, 1, GDK_ACTION_COPY);

			/* drag'n drop events */
			g_signal_connect(event_box, "drag-data-received", G_CALLBACK(drag_data_received), NULL);
		}

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

		gtk_widget_set_size_request(GTK_WIDGET(hboxtl), SCR_ROWS, -1);

		gtk_box_pack_end(GTK_BOX(hbox), hboxtl, FALSE, FALSE, 0);

		timeline = gtk_hscale_new_with_range(0, TL_SNAPS - 1, 1);

		/* il timeline non puo' avere il focus */
		gtk_widget_set_can_focus(GTK_WIDGET(timeline), FALSE);

		gtk_widget_set_size_request(GTK_WIDGET(timeline),
				SCR_ROWS - (alSeparator.width * 2) - (SPACING * 2), -1);

		gtk_scale_set_digits(GTK_SCALE(timeline), 0);
		gtk_scale_set_value_pos(GTK_SCALE(timeline), GTK_POS_LEFT);
		gtk_scale_set_draw_value(GTK_SCALE(timeline), TRUE);

		gtk_range_set_increments(GTK_RANGE(timeline), 1, 0);

		pango_font_description_set_family_static(font_desc, "monospace");
		gtk_widget_modify_font(timeline, font_desc);
		pango_font_description_free(font_desc);

		{
			BYTE i;

			for (i = 0; i < TL_SNAPS; i++) {
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

			for (i = 0; i < SAVE_SLOTS; i++) {
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
		gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(saveslot), renderer, save_slot_control,
				NULL, NULL);

		g_signal_connect(G_OBJECT(saveslot), "changed", G_CALLBACK(save_slot_changed), NULL);
		g_signal_connect(G_OBJECT(saveslot), "notify::popup-shown",
				G_CALLBACK(save_slot_notify_popup_shown), NULL);
		g_signal_connect_swapped(G_OBJECT(sssave), "clicked",
		        G_CALLBACK(menu_state_save_slot_action), GINT_TO_POINTER(SAVE));
		g_signal_connect_swapped(G_OBJECT(ssload), "clicked",
		        G_CALLBACK(menu_state_save_slot_action), GINT_TO_POINTER(LOAD));

		/* il saveslot non puo' avere il focus */
		gtk_combo_box_set_focus_on_click(GTK_COMBO_BOX(saveslot), FALSE);
		gtk_button_set_focus_on_click(GTK_BUTTON(sssave), FALSE);
		gtk_button_set_focus_on_click(GTK_BUTTON(ssload), FALSE);

		gtk_widget_show(hboxss);
	}

	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	gtk_widget_show_all(hbox);

	gtk_widget_show_all(vbox);

	gtk_widget_show_all(main_win);

	g_object_ref_sink(separator);

	return (EXIT_OK);
}
void gui_quit(void) {
	if (gui.freed_last_state_path == TRUE) {
		g_free((gpointer) gui.last_state_path);
	}

	return;
}
void gui_set_video_mode(void) {
	WORD rows = SCR_ROWS;

	if (cfg->scale == X1) {
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
	gtk_widget_set_size_request(sock, gfx.w[VIDEO_MODE], gfx.h[VIDEO_MODE]);
}
void gui_start(void) {
	//gtk_idle_add((GtkFunction) emu_loop, main_win);
	g_idle_add((GtkFunction) emu_loop, main_win);
	gui.start = TRUE;

	gtk_main();
}
void gui_event(void) {
	while (gtk_events_pending()) {
		gtk_main_iteration();
	}

	if (info.no_rom | info.pause) {
		return;
	}

	if (tas.type) {
		tas_frame();
		return;
	}

	{
		BYTE i;

		for (i = PORT1; i < PORT_MAX; i++) {
			if (input_add_event[i]) {
				input_add_event[i](i);
			}
		}
	}
}
GdkNativeWindow gui_emu_frame_id(void) {
	return (gtk_socket_get_id(GTK_SOCKET(sock)));
}
void gui_update(void) {
	char title[255];

	gui_in_update = TRUE;

	/* aggiorno il titolo */
	emu_set_title(title);
	gtk_window_set_title(GTK_WINDOW(main_win), title);

	menu_file_check();
	menu_nes_check();
	menu_settings_check();
	menu_state_check();

	gui_in_update = FALSE;
	gui_flush();
}
void gui_fullscreen(void) {
	if (gui_in_update) {
		return;
	}

	gui_flush();

	/* Fullscreen */
	if ((cfg->fullscreen == NO_FULLSCR) || (cfg->fullscreen == NO_CHANGE)) {
		/* salvo il valore scale prima del fullscreen */
		gfx.scale_before_fscreen = cfg->scale;
		/* trovo la risoluzione del monitor in uso */
		{
			GdkScreen *screen = gtk_window_get_screen(GTK_WINDOW(main_win));
			gdk_screen_get_monitor_geometry(screen,
			        gdk_screen_get_monitor_at_window(screen, GTK_WIDGET(main_win)->window),
			        &monitor);
			gfx.w[MONITOR] = monitor.width;
			gfx.h[MONITOR] = monitor.height;
		}
		/* salvo la posizione */
		gtk_window_get_position(GTK_WINDOW(main_win), &gui_pos_x, &gui_pos_y);
		/* nascondo la finestra */
		if (gui.start) {
			gtk_widget_hide(main_win);
		}
		/* nascondo il menu */
		menu_hide();
		/* nascondo la toolbar */
		gtk_widget_hide(hbox);
		/* abilito il ridimensionamento della finestra */
		gtk_window_set_resizable(GTK_WINDOW(main_win), TRUE);
		/* disabilito le decorazioni */
		gtk_window_set_decorated(GTK_WINDOW(main_win), FALSE);
		/* abilito il fullscreen */
		gfx_set_screen(NO_CHANGE, NO_CHANGE, FULLSCR, NO_CHANGE, FALSE, FALSE);
		/* nascondo il cursore, se serve */
		if (!opengl.rotation && (input_zapper_is_connected((_port *) &port) == FALSE)) {
			SDL_ShowCursor(SDL_DISABLE);
		}
		/* indico alle gtk che sono in fullscreen */
		gtk_window_fullscreen(GTK_WINDOW(main_win));
		/* muovo la finestra nell'angolo alto a sinistra del monitor */
		gtk_window_move(GTK_WINDOW(main_win), monitor.x, monitor.y);
	} else {
		/* nascondo la finestra */
		gtk_widget_hide(main_win);
		/* riabilito le decorazioni */
		gtk_window_set_decorated(GTK_WINDOW(main_win), TRUE);
		/* ribilito la toolbar */
		gtk_widget_show(hbox);
		/* ribilito il menu */
		menu_show();
		/* esco dal fullscreen */
		gtk_window_unfullscreen(GTK_WINDOW(main_win));
		/* ripristino i valori di scale ed esco dal fullscreen */
		gfx_set_screen(gfx.scale_before_fscreen, NO_CHANGE, NO_FULLSCR, NO_CHANGE, FALSE, FALSE);
		/* riabilito la visualizzazione del puntatore */
		SDL_ShowCursor(SDL_ENABLE);
		/* blocco il ridimensionamento */
		gtk_window_set_resizable(GTK_WINDOW(main_win), FALSE);
		/* riposiziono la finestra nella posizione prima del fullscreen */
		gtk_window_move(GTK_WINDOW(main_win), gui_pos_x, gui_pos_y);
	}

	/* visualizzo la finestra*/
	gtk_widget_show(main_win);
	/* setto il focus */
	gtk_widget_grab_focus(GTK_WIDGET(sock));

	gui_flush();
}
void gui_timeline(void) {
	tl.update = TRUE;
	gtk_range_set_value(GTK_RANGE(timeline), tl.snaps_fill - 1);
	tl.update = FALSE;
}
void gui_save_slot(BYTE slot) {
	if (slot >= SAVE_SLOTS) {
		slot = SAVE_SLOTS - 1;
	}
	menu_state_save_slot_set(slot);
}
void gui_sleep(double ms) {
	struct timespec req = { 0 }, rem = { 0 };
	time_t sec;

	if (ms <= 0) {
		return;
	}

	sec = (time_t) (ms / 1000.0f);
	ms = ms - ((double) sec * 1000.0f);
	req.tv_sec = sec;
	req.tv_nsec = ms * 1000000L;
	__nsleep(&req, &rem);
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
void gui_add_event(void *funct, void *args) {
	g_idle_add((GSourceFunc) funct, (gpointer) args);
}
void gui_flush(void) {
	while (gtk_events_pending()) {
		gtk_main_iteration();
	}
}
void gui_set_thread_affinity(uint8_t core) {
	cpu_set_t cpuset;
	pthread_t thread;

	thread = pthread_self();
	CPU_ZERO(&cpuset);
	CPU_SET(core, &cpuset);
	pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
}
void gui_print_usage(char *usage) {
	printf("%s", usage);
}
void gui_reset_video(void) {
	return;
}
int gui_uncompress_selection_dialog(void) {
	return (uncompress_selection_dialog());
}

double high_resolution_ms(void) {
	struct timeval time;

	double elapsed_seconds; /* diff between seconds counter */
	double elapsed_useconds; /* diff between microseconds counter */

	gettimeofday(&time, NULL);

	elapsed_seconds  = time.tv_sec  - gui.counterStart.tv_sec;
	elapsed_useconds = time.tv_usec - gui.counterStart.tv_usec;

	//return ((elapsed_seconds * 1000) + (elapsed_useconds / 1000.0f) + 0.5f);
	return ((elapsed_seconds * 1000.0f) + (elapsed_useconds / 1000.0f));
}
/* main_win */
gboolean main_win_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
	if (cfg_input.father != NULL) {
		gtk_widget_destroy(cfg_input.father);
	}
	gtk_main_quit();
	info.stop = TRUE;
	return (TRUE);
}
gboolean main_win_configure_event(void) {
	/*
	 * devo fare un return FALSE se voglio che l'evento
	 * sia propagato all'uscita della routine (e deve
	 * esserlo necessariamente).
	 */
	return (FALSE);
}
gboolean main_win_focus_in_event(GtkWidget *widget, GdkEventFocus *event, gpointer data) {
	if ((cfg->bck_pause == FALSE) || (gui.main_win_lfp == FALSE)) {
		return (FALSE);
	}

	//g_timeout_redraw_stop();
	emu_pause(FALSE);

	return (TRUE);
}
gboolean main_win_focus_out_event(GtkWidget *widget, GdkEventFocus *event, gpointer data) {
	if ((cfg->bck_pause == FALSE) || (gui.main_win_lfp == FALSE)) {
		return (FALSE);
	}

	//g_timeout_redraw_start();
	emu_pause(TRUE);

	return (TRUE);
}
gboolean sock_key_press_event(GtkWidget *widget, GdkEventKey *event) {
	guint keyval = gdk_keyval_to_lower(event->keyval);

	switch (keyval) {
		case GDK_KEY_Control_L:
			if (!tl.key) {
				tl_pressed(tl.key);
			}
			return (TRUE);
		case GDK_KEY_Shift_L:
			fps_fast_forward();
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
	if (cfg->fullscreen == FULLSCR) {
		switch (keyval) {
			case GDK_a:
				if ((event->state & 0x1F0D) == GDK_CONTROL_MASK) {
					menu_audio_set_audio_enable();
					return (TRUE);
				}
				break;
			case GDK_q:
				if ((event->state & 0x1F0D) == GDK_CONTROL_MASK) {
					gui_fullscreen();
					main_win_delete_event(NULL, NULL, NULL);
					return (TRUE);
				}
				break;
			case GDK_r:
				menu_video_effect_set();
				return (TRUE);
			case GDK_w:
				if ((event->state & 0x1F0D) == GDK_CONTROL_MASK) {
					cfg_file_save();
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
			case GDK_0:
				menu_video_interpolation_set();
				return (TRUE);
			case GDK_p:
				menu_video_fullscreen_switch_stretch();
				return (TRUE);
			case GDK_F1:
				menu_state_save_slot_action(SAVE);
				return (TRUE);
			case GDK_F2:
				menu_state_save_slot_incdec(DEC);
				return (TRUE);
			case GDK_F3:
				menu_state_save_slot_incdec(INC);
				return (TRUE);
			case GDK_F4:
				menu_state_save_slot_action(LOAD);
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
				gui_fullscreen();
				return (TRUE);
			case GDK_Return:
				if ((event->state & 0x1F0D) == GDK_MOD1_MASK) {
					gui_fullscreen();
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

	{
		BYTE i, result = FALSE;

		for (i = PORT1; i < PORT_MAX; i++) {
			if (input_decode_event[i]
			        && (input_decode_event[i](PRESSED, keyval, KEYBOARD, &port[i]) == EXIT_OK)) {
				result = TRUE;
			}
		}

		return (result);
	}
}
gboolean sock_key_release_event(GtkWidget *widget, GdkEventKey *event) {
	guint keyval = gdk_keyval_to_lower(event->keyval);

	switch (keyval) {
		case GDK_KEY_Control_L:
			if (tl.key) {
				tl_release(tl.key);
			}
			return (TRUE);
		case GDK_KEY_Shift_L:
			fps_normalize();
			return (TRUE);
	}

	{
		BYTE i, result = FALSE;

		for (i = PORT1; i < PORT_MAX; i++) {
			if (input_decode_event[i]
			        && (input_decode_event[i](RELEASED, keyval, KEYBOARD, &port[i]) == EXIT_OK)) {
				result = TRUE;
			}
		}

		return (result);
	}
}
gboolean mouse_button_press_release_event(GtkWidget *widget, GdkEventButton *event) {
	switch (event->type) {
		case GDK_BUTTON_PRESS:
			if (event->button == 1) {
				gui.left_button = TRUE;
				opengl.x_diff = event->x - (opengl.y_rotate * slow_factor);
				opengl.y_diff = -event->y + (opengl.x_rotate * slow_factor);
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

	if (gui.left_button && opengl.rotation) {
		opengl.x_rotate = (event->y + opengl.y_diff) / slow_factor;
		opengl.y_rotate = (event->x - opengl.x_diff) / slow_factor;
	}

	return (FALSE);
}

/* open */
void file_open(void) {
	GtkWidget *dialog;

	emu_pause(TRUE);

	/* potrei essere entrato con il CTRL+O */
	tl.key = FALSE;

	dialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(main_win),
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);

	if (l7z_present() == TRUE) {
		if ((l7z_control_ext("rar") == EXIT_OK)) {
			file_open_filter_add(dialog, "All supported formats",
			        "*.zip;*.ZIP;*.7z;*.7Z;*.rar;*.RAR;*.nes;*.NES;*.fds;*.FDS;*.fm2;*.FM2");
			file_open_filter_add(dialog, "Compressed files", "*.zip;*.ZIP;*.7z;*.7Z;*.rar;*.RAR");
		} else {
			file_open_filter_add(dialog, "All supported formats",
			        "*.zip;*.ZIP;*.7z;*.7Z;*.nes;*.NES;*.fds;*.FDS;*.fm2;*.FM2");
			file_open_filter_add(dialog, "Compressed files", "*.zip;*.ZIP;*.7z;*.7Z");
		}
	} else {
		file_open_filter_add(dialog, "All supported formats",
		        "*.zip;*.ZIP;*.nes;*.NES;*.fds;*.FDS;*.fm2;*.FM2");
		file_open_filter_add(dialog, "Compressed files", "*.zip;*.ZIP");
	}
	file_open_filter_add(dialog, "Nes rom files", "*.nes;*.NES");
	file_open_filter_add(dialog, "FDS image files", "*.fds;*.FDS");
	file_open_filter_add(dialog, "TAS movie files", "*.fm2;*.FM2");
	file_open_filter_add(dialog, "All files", "*.*;");

	if (gui.freed_last_state_path == TRUE) {
		g_free((gpointer) gui.last_state_path);
		gui.freed_last_state_path = FALSE;
	}

	if (info.rom_file[0]) {
		gui.last_state_path = g_path_get_dirname(info.rom_file);
		gui.freed_last_state_path = TRUE;
	} else {
		gui.last_state_path = gui.home;
	}

	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), gui.last_state_path);

	/* ridisegno lo screen sdl ogni tot millisecondi */
	g_timeout_redraw_start();

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		gtk_widget_hide(dialog);
		change_rom(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
		gui.last_state_path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));
		gui.freed_last_state_path = TRUE;
	}

	gtk_widget_destroy(dialog);

	g_timeout_redraw_stop();

	emu_pause(FALSE);
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

	emu_pause(TRUE);

	pixbuf = gdk_pixbuf_new_from_inline(-1, pin_inline, FALSE, NULL);
	dialog = gtk_about_dialog_new();
	gtk_window_set_icon(GTK_WINDOW(dialog),
			gdk_pixbuf_new_from_inline(-1, pin_inline, FALSE, NULL));
	if (info.portable) {
		gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), NAME " Portable");
	} else {
		gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), NAME);
	}
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

	emu_pause(FALSE);
}
/* reset */
void make_reset(int type) {
	if (type == HARD) {
		if (cfg->gamegenie && gamegenie.rom_present) {
			if (info.mapper.id != GAMEGENIE_MAPPER) {
				strcpy(info.load_rom_file, info.rom_file);
			}
			gamegenie_reset();
			type = CHANGE_ROM;
		} else {
			/*
			 * se e' stato disabilitato il game genie quando ormai
			 * e' gia' in esecuzione e si preme un reset, carico la rom.
			 */
			if (info.mapper.id == GAMEGENIE_MAPPER) {
				gamegenie_reset();
				type = CHANGE_ROM;
			}
		}
	}

	if (emu_reset(type)) {
		main_win_delete_event(NULL, NULL, NULL);
	}
}
/* timeline */
gboolean timeline_value_changed(GtkRange *range) {
	BYTE snap = gtk_range_get_value(GTK_RANGE(range));

	if (tl.update == TRUE) {
		return (FALSE);
	}

	/* se non ci sono snap esco */
	if (!tl.snaps_fill) {
		gtk_range_set_value(GTK_RANGE(range), 0);
		return (FALSE);
	}
	/* snap non puo' essere mai maggiore del numero di snap effettuate */
	if (snap > (tl.snaps_fill - 1)) {
		gtk_range_set_value(GTK_RANGE(range), tl.snaps_fill - 1);
		snap = (tl.snaps_fill - 1);
	}
	if (snap == (tl.snaps_fill - 1)) {
		memcpy(screen.data, tl.snaps[TL_SNAP_FREE] + tl.preview, screen_size());
		gfx_draw_screen(TRUE);
		return (FALSE);
	}
	timeline_preview(snap);
	return (FALSE);
}
gboolean timeline_button_press_release_event(GtkWidget *widget, GdkEventButton *event) {
	switch (event->type) {
		case GDK_BUTTON_PRESS:
			if (event->button == 1) {
				tl_pressed(tl.button)
			}
			break;
		case GDK_BUTTON_RELEASE:
			if (event->button == 1) {
				tl_release(tl.button);
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

		if (tl.snaps_fill) {
			dec = ((tl.snaps_fill - 1) - value) * TL_SNAP_SEC;
		}

		if (!dec) {
			return (g_strdup_printf("  %d s", 0));
		} else {
			return (g_strdup_printf("% 2d s", -abs(((tl.snaps_fill - 1) - value) * TL_SNAP_SEC)));
		}
	} else {
		return (g_strdup_printf("% 2d s", (int) value * TL_SNAP_SEC));
	}
}
/* misc */
gboolean time_handler_redraw(void) {
	gfx_draw_screen(TRUE);
	return (redraw);
}

void save_slot_changed(GtkComboBox *widget) {
	GtkTreeIter iter;
	GtkTreeModel *model;
	guint slot;

	if (gui_in_update) {
		return;
	}

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));

	gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter);

	gtk_tree_model_get(model, &iter, COLUMN_INT, &slot, -1);
	save_slot.slot = slot;

	gui_update();

	/* setto il focus */
	gtk_widget_grab_focus(GTK_WIDGET(sock));
}
void save_slot_control(GtkCellLayout *cell_layout, GtkCellRenderer *cell, GtkTreeModel *tree_model,
		GtkTreeIter *iter, gpointer data) {
	guint index;

	enum { FIRST_ITER, INC_POINTER, DEC_POINTER, RESET_MODE, OUT_OF_WIDTH };
	enum { SL_NO_KEY, SL_KEY_UP, SL_KEY_DOWN };

	gtk_tree_model_get(tree_model, iter, COLUMN_INT, &index, -1);

	if (trcb.window) {
		gint x, y;

		if (trcb.popup) {
			if (index == (guint) trcb.counter) {
				if (++trcb.counter == SAVE_SLOTS) {
					GtkTreeIter active_iter;
					GtkTreeModel *model;

					model = gtk_combo_box_get_model(GTK_COMBO_BOX(saveslot));
					gtk_combo_box_get_active_iter(GTK_COMBO_BOX(saveslot), &active_iter);
					gtk_tree_model_get(model, &active_iter, COLUMN_INT, &trcb.selected, -1);

					save_slot_gui_preview();

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
				if (trcb.last_key == SL_KEY_DOWN) {
					trcb.mode = DEC_POINTER;
					trcb.last_key = SL_NO_KEY;
				} else if (trcb.last_key == SL_KEY_UP) {
					trcb.mode = INC_POINTER;
					trcb.last_key = SL_NO_KEY;
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
							trcb.last_key = SL_KEY_UP;
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
							trcb.last_key = SL_KEY_DOWN;
							if (trcb.selected == (SAVE_SLOTS - 1)) {
								trcb.mode = DEC_POINTER;
							} else {
								trcb.mode = INC_POINTER;
							}
							break;
						case GDK_End:
						case GDK_Page_Down:
							trcb.mode = RESET_MODE;
							trcb.selected = SAVE_SLOTS - 1;
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
				save_slot_gui_preview();
			}
			trcb.save_index = index;
			trcb.y = y;
			trcb.x = x;
		}
	}

	if (!save_slot.state[index]) {
		g_object_set(cell, "foreground", "Gray", "foreground-set", TRUE, NULL);
	} else {
		/* imposto il colore di default */
		g_object_set(cell, "foreground-set", FALSE, NULL);
	}
}
void save_slot_notify_popup_shown(GtkComboBox *widget) {
	gboolean mode;

	g_object_get(GTK_OBJECT(widget), "popup-shown", &mode, NULL);

	if (mode == 1) {
		emu_pause(TRUE);
		/* intercetto il "key-press-event" del combobox */
		trcb.hook_id = g_signal_add_emission_hook(
				g_signal_lookup("key-press-event", GTK_TYPE_WINDOW), 0, save_slot_key_press_event,
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
		save_slot.preview_start = FALSE;
		emu_pause(FALSE);
	}
}
void save_slot_gui_preview(void) {
	save_slot_preview(trcb.selected);
}
gboolean save_slot_key_press_event(GSignalInvocationHint *ihint, guint n_param_values,
		const GValue *param_values, gpointer data) {
	if (trcb.window) {
		GdkEventKey *event = (GdkEventKey *) g_value_get_boxed(param_values + 1);
		if ((event->keyval == GDK_Up) || (event->keyval == GDK_KP_Up) || (event->keyval == GDK_Down)
				|| (event->keyval == GDK_KP_Down) || (event->keyval == GDK_Page_Up)
				|| (event->keyval == GDK_Page_Down) || (event->keyval == GDK_Home)
				|| (event->keyval == GDK_End))
			trcb.keyboard = TRUE;
		trcb.key = event->keyval;
	}
	return (TRUE);
}
void change_rom(char *rom) {
	strncpy(info.load_rom_file, rom, sizeof(info.load_rom_file));
	gamegenie_reset();
	make_reset(CHANGE_ROM);
	gui_update();
}
void drag_data_received(GtkWidget *widget, GdkDragContext *context, gint x, gint y,
		GtkSelectionData *data, guint ttype, guint time, gpointer *NA) {
	gchar **uri_list;
	gboolean dnd_success = FALSE;

	if ((uri_list = gtk_selection_data_get_uris(data))) {
		gchar *path = NULL;
		gint len = 0;

		while (uri_list[len]) {
			path = g_filename_from_uri(uri_list[len], NULL, NULL);
			len++;
		}

		dnd_success = TRUE;

		emu_pause(TRUE);
		change_rom(path);
		emu_pause(FALSE);

		g_strfreev(uri_list);
		g_free(path);
	}

	gtk_drag_finish(context, dnd_success, FALSE, time);
}

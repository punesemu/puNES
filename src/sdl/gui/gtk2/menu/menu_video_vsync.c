/*
 * menu_video_vsync.c
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#include "menu_video_vsync.h"
#include "gfx.h"
#include "cfg_file.h"

enum {
	MVSYNC,
	NUMCHKS
};

void menu_video_vsync_set(int value);

static GtkWidget *check[NUMCHKS];

void menu_video_vsync(GtkWidget *video, GtkAccelGroup *accel_group) {
	check[MVSYNC] = gtk_check_menu_item_new_with_mnemonic("_VSync");

	gtk_menu_shell_append(GTK_MENU_SHELL(video), check[MVSYNC]);

	g_signal_connect_swapped(G_OBJECT(check[MVSYNC]), "activate", G_CALLBACK(menu_video_vsync_set),
	        NULL);
}
void menu_video_vsync_check(void) {
	if (gfx.opengl) {
		gtk_widget_set_sensitive(check[MVSYNC], TRUE);
	} else {
		gtk_widget_set_sensitive(check[MVSYNC], FALSE);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MVSYNC]), FALSE);
		return;
	}

	/* VSync */
	if (cfg->vsync == TRUE) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MVSYNC]), TRUE);
	} else {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MVSYNC]), FALSE);
	}
}
void menu_video_vsync_set(int value) {
	gint x, y;

	if (gui_in_update) {
		return;
	}

	/* salvo la posizione */
	gtk_window_get_position(GTK_WINDOW(main_win), &x, &y);

	/*
	 * se non nascondo la finestra, al momento del
	 * SDL_QuitSubSystem e del SDL_InitSubSystem
	 * l'applicazione crasha.
	 */
	gtk_widget_hide(main_win);

	/* switch vsync */
	cfg->vsync = !cfg->vsync;

	gfx_reset_video();
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);

	/* rispristino la posizione */
	gtk_window_move(GTK_WINDOW(main_win), x, y);

	gtk_widget_show(main_win);
}

/*
 * menu_video_txt_on_screen.c
 *
 *  Created on: 22/mar/2014
 *      Author: fhorse
 */

#include "menu_video_txt_on_screen.h"
#include "cfg_file.h"

enum {
	MTXTONSCREEN,
	NUMCHKS
};

static GtkWidget *check[NUMCHKS];

void menu_video_txt_on_screen(GtkWidget *video, GtkAccelGroup *accel_group) {
	check[MTXTONSCREEN] = gtk_check_menu_item_new_with_mnemonic("Te_xt on screen");

	gtk_menu_shell_append(GTK_MENU_SHELL(video), check[MTXTONSCREEN]);

	gtk_widget_add_accelerator(check[MTXTONSCREEN], "activate", accel_group, GDK_9,
	        (GdkModifierType) 0, GTK_ACCEL_VISIBLE);

	g_signal_connect_swapped(G_OBJECT(check[MTXTONSCREEN]), "activate",
	        G_CALLBACK(menu_video_txt_on_screen_set), NULL);
}
void menu_video_txt_on_screen_check(void) {
	if (cfg->txt_on_screen == TRUE) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MTXTONSCREEN]), TRUE);
	} else {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MTXTONSCREEN]), FALSE);
	}
}
void menu_video_txt_on_screen_set(void) {
	if (gui_in_update) {
		return;
	}

	cfg->txt_on_screen = !cfg->txt_on_screen;
}

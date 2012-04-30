/*
 * menu.c
 *
 *  Created on: 20/dic/2011
 *      Author: fhorse
 */

#include "menu.h"
#include "gui.h"

GtkWidget *menu;

void menu_create(GtkWidget *win, GtkWidget *mainbox) {
	GtkAccelGroup *accel_group = NULL;

	menu = gtk_menu_bar_new();

	accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(win), accel_group);

	/* File */
	menu_file(menu, accel_group);
	/* NES */
	menu_nes(menu, accel_group);
	/* Settings */
	menu_settings(menu, accel_group);
	/* State */
	menu_state(menu, accel_group);
	/* Help */
	menu_help(menu, accel_group);

	gtk_box_pack_start(GTK_BOX(mainbox), menu, FALSE, FALSE, 0);
}
void menu_hide(void) {
	gtk_widget_hide(menu);
}
void menu_show(void) {
	gtk_widget_show(menu);
}

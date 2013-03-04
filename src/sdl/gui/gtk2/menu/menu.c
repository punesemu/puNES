/*
 * menu.c
 *
 *  Created on: 20/dic/2011
 *      Author: fhorse
 */

#include "menu.h"
#include "gui.h"

#include "../configurations.h"

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

	{
		GtkWidget *submenu, *config;

		submenu = gtk_menu_new();
		config = gtk_menu_item_new_with_mnemonic("_Prova");

		/* append menu file to main menu */
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), config);
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(config), submenu);

		/* signals */
		g_signal_connect(G_OBJECT(config), "activate", G_CALLBACK(configurations_notebook), NULL);
	}

	gtk_box_pack_start(GTK_BOX(mainbox), menu, FALSE, FALSE, 0);
}
void menu_hide(void) {
	gtk_widget_hide(menu);
}
void menu_show(void) {
	gtk_widget_show(menu);
}

/*
 * menu_video_pixel_aspect_ratio.c
 *
 *  Created on: 16/mar/2014
 *      Author: fhorse
 */

#include "menu_video_pixel_aspect_ratio.h"
#include "cfg_file.h"
#include "opengl.h"
#include "gfx.h"

enum {
	MPAR,
	MPAR11,
	MPAR54,
	MPAR87,
	NUMCHKS
};

static GtkWidget *check[NUMCHKS];

void menu_video_pixel_aspect_ratio(GtkWidget *video, GtkAccelGroup *accel_group) {
	GtkWidget *menu;

	menu = gtk_menu_new();
	check[MPAR] = gtk_image_menu_item_new_with_mnemonic("Pixel Aspect Ra_tio");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(check[MPAR]), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(video), check[MPAR]);

	//gw_image_from_inline(palette, icon_inline);

	check[MPAR11] = gtk_check_menu_item_new_with_mnemonic("_1:1");
	check[MPAR54] = gtk_check_menu_item_new_with_mnemonic("_5:4");
	check[MPAR87] = gtk_check_menu_item_new_with_mnemonic("_8:7 (NTSC TV)");

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MPAR11]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MPAR54]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MPAR87]);

	g_signal_connect_swapped(G_OBJECT(check[MPAR11]), "activate",
	        G_CALLBACK(menu_video_pixel_aspect_ratio_set), GINT_TO_POINTER(PAR11));
	g_signal_connect_swapped(G_OBJECT(check[MPAR54]), "activate",
	        G_CALLBACK(menu_video_pixel_aspect_ratio_set), GINT_TO_POINTER(PAR54));
	g_signal_connect_swapped(G_OBJECT(check[MPAR87]), "activate",
	        G_CALLBACK(menu_video_pixel_aspect_ratio_set), GINT_TO_POINTER(PAR87));
}
void menu_video_pixel_aspect_ratio_check(void) {
	int index = 0;

	if (gfx.opengl) {
		gtk_widget_set_sensitive(check[MPAR], TRUE);
	} else {
		gtk_widget_set_sensitive(check[MPAR], FALSE);
	}

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MPAR11]), FALSE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MPAR54]), FALSE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MPAR87]), FALSE);

	switch (cfg->pixel_aspect_ratio) {
		case PAR11:
			index = MPAR11;
			break;
		case PAR54:
			index = MPAR54;
			break;
		case PAR87:
			index = MPAR87;
			break;
	}
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), TRUE);
}
void menu_video_pixel_aspect_ratio_set(int par) {
	if (gui_in_update) {
		return;
	}

	if (cfg->pixel_aspect_ratio == par) {
		return;
	}

	cfg->pixel_aspect_ratio = par;

	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE);
}

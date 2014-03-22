/*
 * menu_video_interpolation.c
 *
 *  Created on: 18/mar/2014
 *      Author: fhorse
 */

#include "menu_video_interpolation.h"
#include "cfg_file.h"
#include "opengl.h"

enum {
	MINTERPOLATION,
	NUMCHKS
};

static GtkWidget *check[NUMCHKS];

void menu_video_interpolation(GtkWidget *video, GtkAccelGroup *accel_group) {
	check[MINTERPOLATION] = gtk_check_menu_item_new_with_mnemonic("_Interpolation");

	gtk_menu_shell_append(GTK_MENU_SHELL(video), check[MINTERPOLATION]);

	gtk_widget_add_accelerator(check[MINTERPOLATION], "activate", accel_group, GDK_0,
	        (GdkModifierType) 0, GTK_ACCEL_VISIBLE);

	g_signal_connect_swapped(G_OBJECT(check[MINTERPOLATION]), "activate",
	        G_CALLBACK(menu_video_interpolation_set), NULL);
}
void menu_video_interpolation_check(void) {
	if (gfx.opengl) {
		gtk_widget_set_sensitive(check[MINTERPOLATION], TRUE);
	} else {
		gtk_widget_set_sensitive(check[MINTERPOLATION], FALSE);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MINTERPOLATION]), FALSE);
		return;
	}

	if (cfg->interpolation == TRUE) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MINTERPOLATION]), TRUE);
	} else {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MINTERPOLATION]), FALSE);
	}
}
void menu_video_interpolation_set(void) {
	if (gui_in_update) {
		return;
	}

	cfg->interpolation = !cfg->interpolation;

	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE);
}

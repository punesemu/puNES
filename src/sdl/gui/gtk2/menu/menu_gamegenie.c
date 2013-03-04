/*
 * menu_gamegenie.c
 *
 *  Created on: 17/apr/2012
 *      Author: fhorse
 */

#include "menu_gamegenie.h"
#include "gamegenie.h"
#include "cfg_file.h"

enum {
	MGAMEGENIE,
	NUMCHKS
};

static GtkWidget *check[NUMCHKS];

void menu_gamegenie(GtkWidget *mainmenu, GtkAccelGroup *accel_group) {
	check[MGAMEGENIE] = gtk_check_menu_item_new_with_mnemonic("_Game Genie");

	gtk_menu_shell_append(GTK_MENU_SHELL(mainmenu), check[MGAMEGENIE]);

	g_signal_connect_swapped(G_OBJECT(check[MGAMEGENIE]), "activate",
	        G_CALLBACK(menu_gamegenie_select), NULL);
}

void menu_gamegenie_check(void) {
	gtk_widget_set_sensitive(check[MGAMEGENIE], TRUE);

	if (cfg->gamegenie) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MGAMEGENIE]), TRUE);
	} else {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MGAMEGENIE]), FALSE);
	}
}
void menu_gamegenie_select(void) {
	if (guiupdate) {
		return;
	}

	cfg->gamegenie = !cfg->gamegenie;

	if (cfg->gamegenie) {
		gamegenie_check_rom_present(TRUE);
	}

	guiUpdate();
}

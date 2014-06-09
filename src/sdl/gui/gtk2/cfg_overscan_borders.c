/*
 * cfg_overscan_borders.c
 *
 *  Created on: 22/mar/2014
 *      Author: fhorse
 */

#include <string.h>
#include "common.h"
#define __GUI_BASE__
#include "gui.h"
#undef __GUI_BASE__
#include "cfg_overscan_borders.h"
#include "cfg_file.h"
#include "clock.h"
#include "gfx.h"

void cfg_overscan_borders_update_dialog(void);
void cfg_overscan_borders_combobox_init(void);
void cfg_overscan_borders_combobox_changed(GtkComboBox *combobox, gpointer user_data);
void cfg_overscan_borders_value_changed(GtkSpinButton *spinbutton, gint border);
void cfg_overscan_borders_preview_clicked(GtkWidget *widget, gpointer user_data);
void cfg_overscan_borders_default_clicked(GtkWidget *widget, gpointer user_data);
void cfg_overscan_borders_ok_clicked(GtkWidget *widget, gpointer user_data);
void cfg_overscan_borders_cancel_clicked(GtkWidget *widget, gpointer user_data);
void cfg_overscan_borders_window_destroy(GtkWidget *widget, gpointer user_data);

enum {
	OVSCAN_NAME,
	OVSCAN_MODE,
	OVSCAN_COLUMS
};

static const char ovscan_borders_button[4][15] = {
	"up", "down", "left", "right"
};

struct _cfg_overscan_borders {
	GtkBuilder *builder;
	GtkWidget *father;
	GtkWidget *child;

	BYTE save_overscan;
	BYTE force_set_mode;

	guint mode;

	_overscan_borders save_borders;
	_overscan_borders preview;
	_overscan_borders overscan_borders[2];
	_overscan_borders *borders;
} cfg_oscan;

void cfg_overscan_borders_dialog(void) {
	memset(&cfg_oscan, 0x00, sizeof(cfg_oscan));

	memcpy(&cfg_oscan.overscan_borders, &overscan_borders, sizeof(overscan_borders));

	/*
	 * salvo sia il parametro dell'overscan
	 * che il settaggio attuale dei bordi.
	 */
	cfg_oscan.save_overscan = cfg->oscan;
	cfg_oscan.save_borders = (*overscan.borders);

	cfg_oscan.borders = &cfg_oscan.overscan_borders[0];

	dg_create_gtkbuilder(&cfg_oscan.builder, OVERSCAN_BORDERS_DIALOG);

	cfg_oscan.father = GTK_WIDGET(
			gtk_builder_get_object(cfg_oscan.builder, "oscan_borders_dialog"));

	gtk_builder_connect_signals(cfg_oscan.builder, NULL);

	/*
	cfg_oscan.father = GTK_WIDGET(
			gtk_builder_get_object(cfg_oscan.builder, "overscan_borders_dialog"));
	*/

	gtk_builder_connect_signals(cfg_oscan.builder, NULL);

	cfg_overscan_borders_combobox_init();

	/* setto i valori massimi */
	{
		gint i;

		for (i = 0; i < sizeof(_overscan_borders); i++) {
			gtk_spin_button_set_range(
			        _gw_get_spinbutton(cfg_oscan.builder,
			                dg_obj_name("oscan_%s_spinbutton", ovscan_borders_button[i])),
			        OVERSCAN_BORDERS_MIN, OVERSCAN_BORDERS_MAX);
		}
	}

	cfg_overscan_borders_update_dialog();

	{
		gint i;

		for (i = 0; i < LENGTH(ovscan_borders_button); i++) {
			dg_signal_connect(cfg_oscan.builder,
					dg_obj_name("oscan_%s_spinbutton", ovscan_borders_button[i]), "value-changed",
					G_CALLBACK(cfg_overscan_borders_value_changed), GINT_TO_POINTER(i));
		}
	}

	dg_signal_connect(cfg_oscan.builder, "oscan_preview_button", "clicked",
			G_CALLBACK(cfg_overscan_borders_preview_clicked), NULL);
	dg_signal_connect(cfg_oscan.builder, "oscan_defaults_button", "clicked",
			G_CALLBACK(cfg_overscan_borders_default_clicked), NULL);

	dg_signal_connect(cfg_oscan.builder, "oscan_ok_button", "clicked",
			G_CALLBACK(cfg_overscan_borders_ok_clicked), NULL);
	dg_signal_connect(cfg_oscan.builder, "oscan_cancel_button", "clicked",
			G_CALLBACK(cfg_overscan_borders_cancel_clicked), NULL);

	g_signal_connect(G_OBJECT(cfg_oscan.father), "destroy",
	        G_CALLBACK(cfg_overscan_borders_window_destroy), NULL);

	/* disabilito la gestiore del docus della finestra principale */
	gui.main_win_lfp = FALSE;

	emu_pause(TRUE);

	/* ridisegno lo screen sdl ogni tot millisecondi */
	//g_timeout_redraw_start();

	gtk_widget_show(cfg_oscan.father);
}

void cfg_overscan_borders_update_dialog(void) {
	BYTE i, *src = (BYTE *)cfg_oscan.borders;

	for (i = 0; i < sizeof(_overscan_borders); i++) {
		char *obj_name;
		gdouble value = (*(src + i));

		obj_name = dg_obj_name("oscan_%s_spinbutton", ovscan_borders_button[i]);
		gtk_spin_button_set_value(_gw_get_spinbutton(cfg_oscan.builder, obj_name), value);
	}
}
void cfg_overscan_borders_combobox_init(void) {
	GtkComboBox *combobox;
	GtkListStore *liststore;
	static struct _ovscan_mode_list_element {
		char name[20];
	} ovscan_mode_list[] = {
		{ "NTSC"      },
		{ "PAL/Dendy" }
	};

	combobox = _gw_get_combobox(cfg_oscan.builder, "oscan_borders_mode_combobox");
	liststore = gtk_list_store_new(OVSCAN_COLUMS, G_TYPE_STRING, G_TYPE_INT);

	{
		BYTE i;

		for (i = 0; i < LENGTH(ovscan_mode_list); i++) {
			GtkTreeIter iter;

			gtk_list_store_append(liststore, &iter);
			gtk_list_store_set(liststore, &iter, OVSCAN_NAME, ovscan_mode_list[i].name, OVSCAN_MODE,
			        i, -1);
		}
	}

	gtk_combo_box_set_model(combobox, GTK_TREE_MODEL(liststore));
	g_object_unref(liststore);

	{
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();

		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), renderer, TRUE);
		gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(combobox), renderer, "text", 0);
	}

	g_signal_connect(G_OBJECT(combobox), "changed",
			G_CALLBACK(cfg_overscan_borders_combobox_changed), NULL);

	if (machine.type == NTSC) {
		cfg_oscan.mode = 0;
	} else {
		cfg_oscan.mode = 1;
	}

	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), cfg_oscan.mode);
	cfg_oscan.borders = &cfg_oscan.overscan_borders[cfg_oscan.mode];
}
void cfg_overscan_borders_combobox_changed(GtkComboBox *combobox, gpointer user_data) {
	GtkTreeIter iter;
	GtkTreeModel *model;

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));

	gtk_combo_box_get_active_iter(GTK_COMBO_BOX(combobox), &iter);

	gtk_tree_model_get(model, &iter, OVSCAN_MODE, &cfg_oscan.mode, -1);
	cfg_oscan.borders = &cfg_oscan.overscan_borders[cfg_oscan.mode];

	cfg_overscan_borders_update_dialog();
}
void cfg_overscan_borders_value_changed(GtkSpinButton *spinbutton, gint border) {
	gdouble value;
	BYTE *src = (BYTE *) cfg_oscan.borders;

	value = gtk_spin_button_get_value(spinbutton);
	(*(src + border)) = value;
}
void cfg_overscan_borders_preview_clicked(GtkWidget *widget, gpointer user_data) {
	cfg->oscan = OSCAN_ON;
	cfg_oscan.preview = (*cfg_oscan.borders);
	overscan.borders = &cfg_oscan.preview;
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
}
void cfg_overscan_borders_default_clicked(GtkWidget *widget, gpointer user_data) {
	cfg_file_set_overscan_default(cfg_oscan.borders, cfg_oscan.mode + NTSC);
	cfg_overscan_borders_update_dialog();
}
void cfg_overscan_borders_ok_clicked(GtkWidget *widget, gpointer user_data) {
	memcpy(&overscan_borders, &cfg_oscan.overscan_borders, sizeof(overscan_borders));

	gtk_widget_destroy(cfg_oscan.father);
}
void cfg_overscan_borders_cancel_clicked(GtkWidget *widget, gpointer user_data) {
	gtk_widget_destroy(cfg_oscan.father);
}
void cfg_overscan_borders_window_destroy(GtkWidget *widget, gpointer user_data) {
	cfg_oscan.father = NULL;

	if (cfg_oscan.child != NULL) {
		gtk_widget_destroy(cfg_oscan.child);
	}

	g_object_unref(G_OBJECT(cfg_oscan.builder));

	/* aggiorno l'attuale tabella */
	cfg_oscan.force_set_mode = overscan_set_mode(machine.type);

	/* ripristino il valore originario del parametro */
	if (cfg_oscan.save_overscan != cfg->oscan) {
		cfg_oscan.force_set_mode = TRUE;
		cfg->oscan = cfg_oscan.save_overscan;
	}

	/*
	 * se le dimensioni dei bordi sono cambiati rispetto ai
	 * valori di ingresso allora forzo il gfx_set_screen.
	 */
	{
		BYTE i, *src = (BYTE *) &cfg_oscan.save_borders, *dst = (BYTE *) overscan.borders;

		for (i = 0; i < sizeof(_overscan_borders); i++) {
			if ((*(src + i)) != (*(dst + i))) {
				cfg_oscan.force_set_mode = TRUE;
				break;
			}
		}
	}

	if (cfg_oscan.force_set_mode == TRUE) {
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	}

	//g_timeout_redraw_stop();
	emu_pause(FALSE);

	/* restituisco alla finestra principale la gestione del focus */
	gui.main_win_lfp = TRUE;
}

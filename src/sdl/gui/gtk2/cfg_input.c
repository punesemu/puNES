/*
 * cfg_input.c
 *
 *  Created on: 17/nov/2013
 *      Author: fhorse
 */

#include <string.h>
#include "cfg_input.h"
#include "cfg_std_pad.h"
#include "menu/menu_video_effect.h"
#include "opengl.h"
#include "cfg_file.h"
#include "param.h"

void cfg_input_update_dialog(void);

void cfg_input_controller_combobox_init(char *glade_combobox, _cfg_port *cfg_port);
void cfg_input_controller_combobox_changed(GtkComboBox *combobox, _cfg_port *cfg_port);
void cfg_input_controller_mode_combobox_init(char *glade_combobox);
void cfg_input_controller_mode_combobox_changed(GtkComboBox *combobox, gpointer user_data);
void cfg_input_setup_clicked(GtkWidget *widget, _cfg_port *cfg_port);
void cfg_input_permit_updown_leftright_checkbutton_toggled(GtkWidget *widget, gpointer user_data);
void cfg_input_default_clicked(GtkWidget *widget, gpointer user_data);
void cfg_input_ok_clicked(GtkWidget *widget, gpointer user_data);
void cfg_input_cancel_clicked(GtkWidget *widget, gpointer user_data);
void cfg_input_window_destroy(GtkWidget *widget, gpointer user_data);

enum {
	CTRL_NAME,
	CTRL_TYPE,
	CTRL_NUMBER,
	CTRL_COLUMNS
};

void cfg_input_dialog(void) {
	memset(&cfg_input, 0x00, sizeof(cfg_input));

	memcpy(&cfg_input.settings, &cfg->input, sizeof(_config_input));

	{
		BYTE i;

		for (i = PORT1; i < PORT_MAX; i++) {
			cfg_input.port[i].id = i + 1;
			memcpy(&cfg_input.port[i].port, &port[i], sizeof(_port));
		}
	}

	dg_create_gtkbuilder(&cfg_input.builder, INPUT_DIALOG);

	cfg_input.father = GTK_WIDGET(
	        gtk_builder_get_object(cfg_input.builder, "input_dialog"));

	gtk_builder_connect_signals(cfg_input.builder, NULL);

	{
		BYTE i;

		for (i = PORT1 ; i < PORT_MAX; i++) {
			cfg_input_controller_combobox_init(
			        dg_obj_name("input_ctrl%d_combobox", cfg_input.port[i].id), &cfg_input.port[i]);
		}
	}

	cfg_input_controller_mode_combobox_init(dg_obj_name("input_ctrl_mode_combobox"));

	cfg_input_update_dialog();

	dg_signal_connect_swapped(cfg_input.builder, "input_updown_leftright_checkbutton", "toggled",
	        cfg_input_permit_updown_leftright_checkbutton_toggled, NULL);

	dg_signal_connect(cfg_input.builder, "input_default_button", "clicked",
			cfg_input_default_clicked, NULL);
	dg_signal_connect(cfg_input.builder, "input_ok_button", "clicked",
			cfg_input_ok_clicked, NULL);
	dg_signal_connect(cfg_input.builder, "input_cancel_button", "clicked",
	        cfg_input_cancel_clicked, NULL);
	g_signal_connect(G_OBJECT(cfg_input.father), "destroy",
	        G_CALLBACK(cfg_input_window_destroy), NULL);

	emu_pause(TRUE);
	/* ridisegno lo screen sdl ogni tot millisecondi */
	g_timeout_redraw_start();

	gtk_widget_show(cfg_input.father);
}

void cfg_input_update_dialog(void) {
	BYTE i;
	_cfg_port *this;

	for (i = PORT1; i < PORT_MAX; i++) {
		char *obj_name;

		this = &cfg_input.port[i];

		obj_name = dg_obj_name("input_ctrl%d_combobox", this->id);
		gtk_combo_box_set_active(_gw_get_combobox(cfg_input.builder, obj_name), this->port.type);

		obj_name = dg_obj_name("input_ctrl%d_setup_button", this->id);
		dg_signal_disconnect(cfg_input.builder, obj_name, "clicked");
		switch (this->port.type) {
			case CTRL_DISABLED:
			case CTRL_ZAPPER:
				dg_set_sensitive(cfg_input.builder, obj_name, FALSE);
				break;
			case CTRL_STANDARD:
				dg_set_sensitive(cfg_input.builder, obj_name, TRUE);
				dg_signal_connect(cfg_input.builder, obj_name, "clicked",
						cfg_input_setup_clicked, this);
				break;
		}
	}

	{
		BYTE mode = TRUE;

		gtk_combo_box_set_active(
		        _gw_get_combobox(cfg_input.builder, dg_obj_name("input_ctrl_mode_combobox")),
		        cfg_input.settings.controller_mode);

		if (cfg_input.settings.controller_mode == CTRL_MODE_NES) {
			mode = FALSE;
		}

		for (i = PORT3; i <= PORT4; i++) {
			this = &cfg_input.port[i];

			dg_set_sensitive(cfg_input.builder, dg_obj_name("input_ctrl%d_label", this->id), mode);
			dg_set_sensitive(cfg_input.builder, dg_obj_name("input_ctrl%d_combobox", this->id),
			        mode);
			if (mode == FALSE) {
				dg_set_sensitive(cfg_input.builder,
				        dg_obj_name("input_ctrl%d_setup_button", this->id), mode);
			}
		}
	}

	gtk_toggle_button_set_active(
	        _gw_get_togglebutton(cfg_input.builder, "input_updown_leftright_checkbutton"),
	        cfg_input.settings.permit_updown_leftright);
}
void cfg_input_controller_combobox_init(char *glade_combobox, _cfg_port *cfg_port) {
	GtkComboBox *combobox;
	GtkListStore *liststore;
	static struct _ctrl_list_element {
		gint type;
		char name[20];
	} ctrl_list[] = {
		{ CTRL_DISABLED, "Disabled"     },
		{ CTRL_STANDARD, "Standard Pad" },
		{ CTRL_ZAPPER,   "Zapper"       }
	};

	combobox = _gw_get_combobox(cfg_input.builder, glade_combobox);
	liststore = gtk_list_store_new(CTRL_COLUMNS, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT);

	{
		BYTE i, length = LENGTH(ctrl_list) - ((cfg_port->id - 1) >> 1);

		for (i = 0; i < length; i++) {
			GtkTreeIter iter;

			gtk_list_store_append(liststore, &iter);
			gtk_list_store_set(liststore, &iter, CTRL_NAME, ctrl_list[i].name, CTRL_TYPE,
			        ctrl_list[i].type, CTRL_NUMBER, (cfg_port->id - 1), -1);
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
			G_CALLBACK(cfg_input_controller_combobox_changed), cfg_port);
}
void cfg_input_controller_combobox_changed(GtkComboBox *combobox, _cfg_port *cfg_port) {
	GtkTreeIter iter;
	GtkTreeModel *model;
	guint type;

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));

	gtk_combo_box_get_active_iter(GTK_COMBO_BOX(combobox), &iter);

	gtk_tree_model_get(model, &iter, CTRL_TYPE, &type, -1);
	cfg_port->port.type = type;

	cfg_input_update_dialog();
}
void cfg_input_controller_mode_combobox_init(char *glade_combobox) {
	GtkComboBox *combobox;
	GtkListStore *liststore;

	combobox = _gw_get_combobox(cfg_input.builder, glade_combobox);
	liststore = gtk_list_store_new(CTRL_COLUMNS, G_TYPE_STRING, G_TYPE_INT);

	{
		BYTE i;

		for (i = 0; i < LENGTH(param_controller_mode); i++) {
			GtkTreeIter iter;

			gtk_list_store_append(liststore, &iter);
			gtk_list_store_set(liststore, &iter, CTRL_NAME, param_controller_mode[i].lname,
			        CTRL_TYPE, i, -1);
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
			G_CALLBACK(cfg_input_controller_mode_combobox_changed), NULL);
}
void cfg_input_controller_mode_combobox_changed(GtkComboBox *combobox, gpointer user_data) {
	GtkTreeIter iter;
	GtkTreeModel *model;
	guint type;

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));

	gtk_combo_box_get_active_iter(GTK_COMBO_BOX(combobox), &iter);

	gtk_tree_model_get(model, &iter, CTRL_TYPE, &type, -1);
	cfg_input.settings.controller_mode = type;

	cfg_input_update_dialog();
}
void cfg_input_setup_clicked(GtkWidget *widget, _cfg_port *cfg_port) {
	switch (cfg_port->port.type) {
		case CTRL_DISABLED:
		case CTRL_ZAPPER:
			break;
		case CTRL_STANDARD:
			cfg_std_pad_dialog(cfg_port);
			break;
	}
}
void cfg_input_permit_updown_leftright_checkbutton_toggled(GtkWidget *widget, gpointer user_data) {
	cfg_input.settings.permit_updown_leftright = !cfg_input.settings.permit_updown_leftright;
}
void cfg_input_default_clicked(GtkWidget *widget, gpointer user_data) {
	_array_pointers_port array;
	BYTE i;

	for (i = PORT1; i < PORT_MAX; i++) {
		array.port[i] = &cfg_input.port[i].port;
	}

	cfg_file_set_all_input_default(&cfg_input.settings, &array);

	cfg_input_update_dialog();
}
void cfg_input_ok_clicked(GtkWidget *widget, gpointer user_data) {
	if (opengl.rotation && (input_zapper_is_connected((_port *) &cfg_input.port) == TRUE)) {
		menu_video_effect_set();
	}

	memcpy(&cfg->input, &cfg_input.settings, sizeof(_config_input));

	{
		BYTE i;

		for (i = PORT1; i < PORT_MAX; i++) {
			if (cfg_input.port[i].port.type != port[i].type) {
				BYTE a;

				for (a = TRB_A; a <= TRB_B; a++) {
					gint type = a - TRB_A;

					cfg_input.port[i].port.turbo[type].active = 0;
					cfg_input.port[i].port.turbo[type].counter = 0;
				}
			}
			memcpy(&port[i], &cfg_input.port[i].port, sizeof(_port));
		}
	}

	/* Faccio l'update del menu per i casi dello zapper e degli effetti */
	gui_update();

	cfg_file_input_save();

	input_init();

	js_quit();
	js_init();

	gtk_widget_destroy(cfg_input.father);
}
void cfg_input_cancel_clicked(GtkWidget *widget, gpointer user_data) {
	gtk_widget_destroy(cfg_input.father);
}
void cfg_input_window_destroy(GtkWidget *widget, gpointer user_data) {
	cfg_input.father = NULL;

	if (cfg_input.child != NULL) {
		gtk_widget_destroy(cfg_input.child);
	}

	g_object_unref(G_OBJECT(cfg_input.builder));

	g_timeout_redraw_stop();
	emu_pause(FALSE);
}

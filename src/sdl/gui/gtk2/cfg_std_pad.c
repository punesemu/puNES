/*
 * cfg_std_pad.c
 *
 *  Created on: 21/nov/2013
 *      Author: fhorse
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "cfg_std_pad.h"
#include "cfg_std_pad_image.h"
#include "cfg_file.h"
#include "param.h"

void cfg_std_pad_info_entry_print(int type, const char *fmt, ...);
void cfg_std_pad_enable_notebook_and_other(gint type, gint virtual_button, gint mode);
void cfg_std_pad_enable_joystick_notebook_buttons(gint mode);

gboolean cfg_std_pad_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
void cfg_std_pad_js_press_event(void);

void cfg_std_pad_joystick_device_combobox_init(_cfg_port *cfg_port);
void cfg_std_pad_joystick_device_combobox_changed(GtkComboBox *combobox, gpointer user_data);

void cfg_std_pad_input_clicked(GtkButton *button, gint virtual_button);
void cfg_std_pad_input_unset_clicked(GtkButton *button, gint virtual_button);
void cfg_std_pad_input_in_sequence_clicked(GtkButton *button, gint type);
void cfg_std_pad_input_unset_all_clicked(GtkButton *button, gint type);
void cfg_std_pad_input_default_all_clicked(GtkButton *button, gint type);

void cfg_std_pad_turbo_delay_value_changed(GtkRange *range, gint type);

void cfg_std_pad_ok_clicked(GtkWidget *widget, _cfg_port *cfg_port);
void cfg_std_pad_cancel_clicked(GtkWidget *widget, gpointer user_data);
void cfg_std_pad_window_destroy(GtkWidget *widget, gpointer user_data);

enum std_pad_combo_value {
	JOY_NAME,
	JOY_VALUE,
	JOY_COLUMNS,
	MAX_JOYSTICK = 16
};

static const char std_pad_input_type[2][10] = {
	"keyboard", "joystick"
};
static const char std_pad_button[10][2][15] = {
	{ "a",      "Button A" },
	{ "b",      "Button B" },
	{ "select", "Select"   },
	{ "start",  "Start"    },
	{ "up",     "Up"       },
	{ "down",   "Down"     },
	{ "left",   "Left"     },
	{ "right",  "Right"    },
	{ "ta",     "Turbo A"  },
	{ "tb",     "Turbo B"  }
};

struct _cfg_std_pad {
	GtkBuilder *builder;
	GtkButton *button_clicked;

	BYTE virtual_button;
	BYTE no_other_buttons;
	BYTE wait_js_input;
	BYTE in_sequence;
	BYTE force_exit_in_sequence;

	_cfg_port cfg;
} cfg_std_pad;

void cfg_std_pad_dialog(_cfg_port *cfg_port) {
	memset(&cfg_std_pad, 0x00, sizeof(cfg_std_pad));
	memcpy(&cfg_std_pad.cfg, cfg_port, sizeof(_cfg_port));

	dg_create_gtkbuilder(&cfg_std_pad.builder, INPUT_STD_PAD_DIALOG);

	cfg_input.child = GTK_WIDGET(
	        gtk_builder_get_object(cfg_std_pad.builder, "standard_pad_dialog"));

	gtk_builder_connect_signals(cfg_input.builder, NULL);

	{
		char label[50];

		snprintf(label, sizeof(label), "<b>Controller %d : Standard Pad</b>", cfg_port->id);

		gtk_label_set_markup_with_mnemonic(
				_gw_get_label(cfg_std_pad.builder, "standard_pad_frame_label"), label);
	}

	dg_image_from_inline(cfg_std_pad.builder, "standard_pad_image", standard_pad_image);

	cfg_std_pad_joystick_device_combobox_init(cfg_port);

	{
		gint a, b;

		for (a = KEYBOARD; a <= JOYSTICK; a++) {
			for (b = BUT_A; b < MAX_STD_PAD_BUTTONS; b++) {
				char text[30];
				gint virtual_button = b + (a * MAX_STD_PAD_BUTTONS);
				GtkWidget *widget;

				if (a == KEYBOARD) {
					snprintf(text, sizeof(text), "%s",
					        keyval_to_name(cfg_std_pad.cfg.port.input[a][b]));
				} else {
					snprintf(text, sizeof(text), "%s",
					        jsv_to_name(cfg_std_pad.cfg.port.input[a][b]));
				}

				widget = dg_widget_from_obj_name(cfg_std_pad.builder, "%s_%s_button",
				        std_pad_input_type[a], std_pad_button[b][0]);

				gtk_button_set_label(GTK_BUTTON(widget), text);

				g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(cfg_std_pad_input_clicked),
				        GINT_TO_POINTER(virtual_button));

				widget = dg_widget_from_obj_name(cfg_std_pad.builder, "%s_%s_unset_button",
				        std_pad_input_type[a], std_pad_button[b][0]);

				g_signal_connect(G_OBJECT(widget), "clicked",
				        G_CALLBACK(cfg_std_pad_input_unset_clicked),
				        GINT_TO_POINTER(virtual_button));
			}

			dg_signal_connect(cfg_std_pad.builder,
			        dg_obj_name("%s_in_sequence_button", std_pad_input_type[a]), "clicked",
			        cfg_std_pad_input_in_sequence_clicked, GINT_TO_POINTER(a));

			dg_signal_connect(cfg_std_pad.builder,
			        dg_obj_name("%s_unset_all_button", std_pad_input_type[a]), "clicked",
			        cfg_std_pad_input_unset_all_clicked, GINT_TO_POINTER(a));

			dg_signal_connect(cfg_std_pad.builder,
			        dg_obj_name("%s_default_all_button", std_pad_input_type[a]), "clicked",
			        cfg_std_pad_input_default_all_clicked, GINT_TO_POINTER(a));
		}
	}

	{
		gint a;

		for (a = TRB_A; a <= TRB_B; a++) {
			GtkWidget *widget;
			gint type = a - TRB_A;

			widget = dg_widget_from_obj_name(cfg_std_pad.builder, "%s_delay_hscale",
			        std_pad_button[a][0]);

			gtk_scale_add_mark(GTK_SCALE(widget), (double) TURBO_BUTTON_DELAY_DEFAULT, GTK_POS_TOP,
			        NULL);
			gtk_range_set_value(GTK_RANGE(widget), cfg_std_pad.cfg.port.turbo[type].frequency);

			g_signal_connect(G_OBJECT(widget), "value-changed",
			        G_CALLBACK(cfg_std_pad_turbo_delay_value_changed), GINT_TO_POINTER(type));
		}
	}

	dg_signal_connect(cfg_std_pad.builder, "standard_pad_ok_button", "clicked",
	        cfg_std_pad_ok_clicked, cfg_port);
	dg_signal_connect(cfg_std_pad.builder, "standard_pad_cancel_button", "clicked",
	        cfg_std_pad_cancel_clicked, NULL);
	g_signal_connect(G_OBJECT(cfg_input.child), "key_press_event",
			G_CALLBACK(cfg_std_pad_key_press_event), NULL);
	g_signal_connect(G_OBJECT(cfg_input.child), "destroy",
	        G_CALLBACK(cfg_std_pad_window_destroy), NULL);

	gtk_widget_hide(cfg_input.father);
	gtk_widget_show(cfg_input.child);
}

void cfg_std_pad_info_entry_print(int type, const char *fmt, ...) {
	char buffer[80];
	va_list ap;
	GtkWidget *widget;

	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	if (type == KEYBOARD) {
		widget = dg_widget_from_obj_name(cfg_std_pad.builder, "keyboard_info_entry");
	} else {
		widget = dg_widget_from_obj_name(cfg_std_pad.builder, "joystick_info_entry");
	}

	gtk_entry_set_text(GTK_ENTRY(widget), buffer);
}
void cfg_std_pad_enable_notebook_and_other(gint type, gint virtual_button, gint mode) {
	gint other_type = KEYBOARD;

	if ((mode == TRUE) && (cfg_std_pad.in_sequence == TRUE)) {
		;
	} else {
		if (type == KEYBOARD) {
			other_type = JOYSTICK;
		}

		/* notebook */
		gtk_widget_set_sensitive(
		        _gw_get_widget(cfg_std_pad.builder,
		                dg_obj_name("%s_notebook_alignment", std_pad_input_type[other_type])), mode);
		gtk_widget_set_sensitive(
		        _gw_get_widget(cfg_std_pad.builder,
		                dg_obj_name("%s_notebook_label", std_pad_input_type[other_type])), mode);

		/* device combobox */
		gtk_widget_set_sensitive(
		        _gw_get_widget(cfg_std_pad.builder,
		                dg_obj_name("%s_device_hbox", std_pad_input_type[type])), mode);

		/* notebook */
		{
			BYTE i;
			char *bt;

			/* button */
			for (i = BUT_A; i < MAX_STD_PAD_BUTTONS; i++) {
				bt = dg_obj_name("%s_%s_button", std_pad_input_type[type], std_pad_button[i][0]);
				gtk_widget_set_sensitive(_gw_get_widget(cfg_std_pad.builder, bt), mode);

				bt = dg_obj_name("%s_%s_label", std_pad_input_type[type], std_pad_button[i][0]);
				gtk_widget_set_sensitive(_gw_get_widget(cfg_std_pad.builder, bt), mode);
			}

			/* unset */
			for (i = BUT_A; i < MAX_STD_PAD_BUTTONS; i++) {
				bt = dg_obj_name("%s_%s_unset_button", std_pad_input_type[type],
				        std_pad_button[i][0]);
				gtk_widget_set_sensitive(_gw_get_widget(cfg_std_pad.builder, bt), mode);
			}
		}

		/* in sequence, unset all, default */
		gtk_widget_set_sensitive(
		        _gw_get_widget(cfg_std_pad.builder,
		                dg_obj_name("%s_notebook_button_hbox", std_pad_input_type[type])), mode);

		/* turbo delay */
		gtk_widget_set_sensitive(
		        _gw_get_widget(cfg_std_pad.builder, "standard_pad_turbo_delay_alignment"), mode);
	}

	if (mode == FALSE) {
		char *bt;

		bt = dg_obj_name("%s_%s_button", std_pad_input_type[type],
		        std_pad_button[virtual_button][0]);
		gtk_widget_set_sensitive(_gw_get_widget(cfg_std_pad.builder, bt), TRUE);

		bt = dg_obj_name("%s_%s_label", std_pad_input_type[type],
		        std_pad_button[virtual_button][0]);
		gtk_widget_set_sensitive(_gw_get_widget(cfg_std_pad.builder, bt), TRUE);
	}
}
void cfg_std_pad_enable_joystick_notebook_buttons(gint mode) {
	gtk_widget_set_sensitive(_gw_get_widget(cfg_std_pad.builder, "joystick_buttons_hbox"), mode);
	gtk_widget_set_sensitive(_gw_get_widget(cfg_std_pad.builder, "joystick_info_entry"), mode);
	gtk_widget_set_sensitive(_gw_get_widget(cfg_std_pad.builder, "joystick_notebook_button_hbox"),
	        mode);
}

gboolean cfg_std_pad_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
	gint type, virtual_button;

	if (cfg_std_pad.no_other_buttons == FALSE) {
		return (TRUE);
	}

	type = cfg_std_pad.virtual_button / MAX_STD_PAD_BUTTONS;
	virtual_button = cfg_std_pad.virtual_button - (type * MAX_STD_PAD_BUTTONS);

	if (type == KEYBOARD) {
		if (event->keyval == GDK_Escape) {
			gtk_button_set_label(cfg_std_pad.button_clicked,
			        keyval_to_name(cfg_std_pad.cfg.port.input[type][virtual_button]));
		} else {
			cfg_std_pad.cfg.port.input[type][virtual_button] = event->keyval;
			gtk_button_set_label(cfg_std_pad.button_clicked, keyval_to_name(event->keyval));
		}
		cfg_std_pad_info_entry_print(type, "");
	} else {
		/*
		 * quando sto configurando il joystick, l'unico input da tastiera
		 * che accetto e' l'escape.
		 */
		if (event->keyval == GDK_Escape) {
			gtk_button_set_label(cfg_std_pad.button_clicked,
					jsv_to_name(cfg_std_pad.cfg.port.input[type][virtual_button]));
		} else {
			return (TRUE);
		}
	}

	cfg_std_pad_enable_notebook_and_other(type, virtual_button, TRUE);

	cfg_std_pad.no_other_buttons = FALSE;
	cfg_std_pad.virtual_button = 0;

	cfg_std_pad.wait_js_input = FALSE;

	return (TRUE);
}
void cfg_std_pad_js_press_event(void) {
	_js_event jse;
	gint type, virtual_button;
	int fd;
	char device[30];
	size_t size = sizeof(jse);
	BYTE index = cfg_std_pad.cfg.port.joy_id;
	BYTE read_is_ok = FALSE;
	DBWORD value = 0;

	type = cfg_std_pad.virtual_button / MAX_STD_PAD_BUTTONS;
	virtual_button = cfg_std_pad.virtual_button - (type * MAX_STD_PAD_BUTTONS);

	if (index == name_to_jsn("NULL")) {
		if (cfg_input.child) {
			gdk_threads_enter();
			cfg_std_pad_info_entry_print(type, "Select device first");
			cfg_std_pad_enable_notebook_and_other(type, virtual_button, TRUE);
			gdk_threads_leave();
		}
		g_thread_exit(NULL);
	}

	sprintf(device, "%s%d", JS_DEV_PATH, index);
	fd = open(device, O_RDONLY | O_NONBLOCK);

	if (fd < 0) {
		if (cfg_input.child) {
			gdk_threads_enter();
			cfg_std_pad_info_entry_print(type, "Error on open device %s", device);
			cfg_std_pad_enable_notebook_and_other(type, virtual_button, TRUE);
			gdk_threads_leave();
		}
		g_thread_exit(NULL);
	}

	cfg_std_pad.no_other_buttons = TRUE;
	cfg_std_pad.wait_js_input = TRUE;

	{
		BYTE i;

		for (i = 0; i < MAX_JOYSTICK; i++) {
			if (read(fd, &jse, size) < 0) {
				if (cfg_input.child) {
					gdk_threads_enter();
					cfg_std_pad_info_entry_print(type,
					        "Error on reading controllers configurations");
					gdk_threads_leave();
				}
			}
		}
	}

	if (cfg_input.child) {
		gdk_threads_enter();
		gtk_button_set_label(cfg_std_pad.button_clicked, "...");
		cfg_std_pad_info_entry_print(type, "Press a key (ESC for the previous value \"%s\")",
		        jsv_to_name(cfg_std_pad.cfg.port.input[type][virtual_button]));
		gdk_threads_leave();
	}

	while (cfg_std_pad.wait_js_input == TRUE) {
		read_is_ok = FALSE;

		memset(&jse, 0x00, size);

		if (read(fd, &jse, size) == size) {
			if (jse.value == 0) {
				continue;
			}
			jse.type &= ~JS_EVENT_INIT;

			if (jse.type == JS_EVENT_AXIS && jse.value) {
				value = (jse.number << 1) + 1;
				if (jse.value > 0) {
					value++;
				}
			} else if (jse.type == JS_EVENT_BUTTON && jse.value) {
				value = jse.number | 0x400;
			} else {
				continue;
			}
			read_is_ok = TRUE;
			break;
		}
		gui_sleep(30);
	}

	if (read_is_ok) {
		cfg_std_pad.cfg.port.input[type][virtual_button] = value;
		if (cfg_input.child) {
			gdk_threads_enter();
			cfg_std_pad_info_entry_print(type, "");
			cfg_std_pad_enable_notebook_and_other(type, virtual_button, TRUE);
			gtk_button_set_label(cfg_std_pad.button_clicked, jsv_to_name(value));
			gdk_threads_leave();
		}
		cfg_std_pad.no_other_buttons = FALSE;
		cfg_std_pad.virtual_button = 0;
	} else {
		if (cfg_input.child) {
			gdk_threads_enter();
			cfg_std_pad_enable_notebook_and_other(type, virtual_button, TRUE);
			gdk_threads_leave();
		}
	}

	close(fd);

	g_thread_exit(NULL);
}

void cfg_std_pad_joystick_device_combobox_init(_cfg_port *cfg_port) {
	BYTE disabled_line = 0, count = 0, current_line = name_to_jsn("NULL");
	GtkComboBox *combobox;
	GtkListStore *liststore;

	combobox = _gw_get_combobox(cfg_std_pad.builder, "joystick_device_combobox");
	liststore = gtk_list_store_new(JOY_COLUMNS, G_TYPE_STRING, G_TYPE_INT);

	{
		BYTE a;
		char description[100];
		GtkTreeIter iter;

		for (a = 0; a <= MAX_JOYSTICK; a++) {
			BYTE id = a;

			if (a < MAX_JOYSTICK) {
				if (js_is_connected(id) == EXIT_ERROR) {
					continue;
				}

				if (id == cfg_port->port.joy_id) {
					current_line = count;
				}

				snprintf(description, sizeof(description), "js%d: %s", id, js_name_device(id));
			} else {
				if (count == 0) {
					break;
				}
				sprintf(description, "Disabled");
				id = name_to_jsn("NULL");
				disabled_line = count;
			}

			gtk_list_store_append(liststore, &iter);
			gtk_list_store_set(liststore, &iter, JOY_NAME, description, JOY_VALUE, id, -1);

			count++;
		}

		if (count == 0) {
			snprintf(description, sizeof(description), "No usable device");

			gtk_list_store_append(liststore, &iter);
			gtk_list_store_set(liststore, &iter, JOY_NAME, description, JOY_VALUE,
			        name_to_jsn("NULL"), -1);

			gtk_widget_set_sensitive(GTK_WIDGET(combobox), FALSE);
			cfg_std_pad_enable_joystick_notebook_buttons(FALSE);
		} else {
			gtk_widget_set_sensitive(GTK_WIDGET(combobox), TRUE);
			cfg_std_pad_enable_joystick_notebook_buttons(TRUE);
		}
	}

	gtk_combo_box_set_model(combobox, GTK_TREE_MODEL(liststore));
	g_object_unref(liststore);

	{
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();

		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), renderer, TRUE);
		gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(combobox), renderer, "text", 0);
	}

	if (count > 0) {
		if (cfg_std_pad.cfg.port.joy_id == name_to_jsn("NULL")
		        || (current_line == name_to_jsn("NULL"))) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), disabled_line);
			cfg_std_pad_enable_joystick_notebook_buttons(FALSE);
		} else {
			gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), current_line);
			cfg_std_pad_enable_joystick_notebook_buttons(TRUE);
		}

		g_signal_connect(G_OBJECT(combobox), "changed",
		        G_CALLBACK(cfg_std_pad_joystick_device_combobox_changed), NULL);
	} else {
		gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
	}
}
void cfg_std_pad_joystick_device_combobox_changed(GtkComboBox *combobox, gpointer user_data) {
	GtkTreeIter iter;
	GtkTreeModel *model;
	guint joy_id;

	model = gtk_combo_box_get_model(combobox);

	gtk_combo_box_get_active_iter(combobox, &iter);

	gtk_tree_model_get(model, &iter, JOY_VALUE, &joy_id, -1);

	if (joy_id == name_to_jsn("NULL")) {
		cfg_std_pad_enable_joystick_notebook_buttons(FALSE);
	} else {
		cfg_std_pad_enable_joystick_notebook_buttons(TRUE);
	}

	cfg_std_pad.cfg.port.joy_id = joy_id;
}

void cfg_std_pad_input_clicked(GtkButton *button, gint virtual_button) {
	gint type;

	if (cfg_std_pad.no_other_buttons == TRUE) {
		return;
	}

	cfg_std_pad.button_clicked = button;
	cfg_std_pad.virtual_button = virtual_button;

	type = virtual_button / MAX_STD_PAD_BUTTONS;
	virtual_button -= (type * MAX_STD_PAD_BUTTONS);

	cfg_std_pad_enable_notebook_and_other(type, virtual_button, FALSE);

	if (type == KEYBOARD) {
		cfg_std_pad.no_other_buttons = TRUE;
		gtk_button_set_label(cfg_std_pad.button_clicked, "...");
		cfg_std_pad_info_entry_print(type, "Press a key (ESC for the previous value \"%s\")",
		        keyval_to_name(cfg_std_pad.cfg.port.input[type][virtual_button]));

	} else {
		g_thread_create((GThreadFunc) cfg_std_pad_js_press_event, NULL, FALSE, NULL);
	}

}
void cfg_std_pad_input_unset_clicked(GtkButton *button, gint virtual_button) {
	char *obj_name;
	gint type;

	type = virtual_button / MAX_STD_PAD_BUTTONS;
	virtual_button -= (type * MAX_STD_PAD_BUTTONS);
	cfg_std_pad.cfg.port.input[type][virtual_button] = 0;

	cfg_std_pad_info_entry_print(type, "");

	obj_name = dg_obj_name("%s_%s_button", std_pad_input_type[type],
	        std_pad_button[virtual_button][0]);
	gtk_button_set_label(_gw_get_button(cfg_std_pad.builder, obj_name), "NULL");
}
void cfg_std_pad_input_in_sequence_clicked(GtkButton *button, gint type) {
	gint a, new_order[MAX_STD_PAD_BUTTONS] = {
		UP,	DOWN, LEFT, RIGHT,
		SELECT, START,
		BUT_A, BUT_B, TRB_A, TRB_B,
	};

	cfg_std_pad_info_entry_print(type, "");

	cfg_std_pad.in_sequence = TRUE;

	for (a = BUT_A; a < MAX_STD_PAD_BUTTONS; a++) {
		GtkWidget *widget;

		if (cfg_std_pad.force_exit_in_sequence == TRUE) {
			return;
		}

		widget = dg_widget_from_obj_name(cfg_std_pad.builder, "%s_%s_button",
		        std_pad_input_type[type], std_pad_button[new_order[a]][0]);

		gtk_button_clicked(GTK_BUTTON(widget));

		gui_sleep(30);

		while (cfg_std_pad.no_other_buttons == TRUE) {
			gui_flush();
			gui_sleep(30);
		}

		gui_sleep(30);
		gui_flush();
	}

	cfg_std_pad.in_sequence = FALSE;

	cfg_std_pad_enable_notebook_and_other(type, 0, TRUE);
	gui_flush();
}
void cfg_std_pad_input_unset_all_clicked(GtkButton *button, gint type) {
	gint a;

	cfg_std_pad_info_entry_print(type, "");

	for (a = BUT_A; a < MAX_STD_PAD_BUTTONS; a++) {
		cfg_std_pad_input_unset_clicked(NULL, a + (type * MAX_STD_PAD_BUTTONS));
	}
}
void cfg_std_pad_input_default_all_clicked(GtkButton *button, gint type) {
	BYTE a;

	cfg_std_pad_info_entry_print(type, "");

	cfg_file_set_kbd_joy_default(&cfg_std_pad.cfg.port, cfg_std_pad.cfg.id - 1, type);

	for (a = BUT_A; a < MAX_STD_PAD_BUTTONS; a++) {
		char text[30];

		if (type == KEYBOARD) {
			snprintf(text, sizeof(text), "%s", keyval_to_name(cfg_std_pad.cfg.port.input[type][a]));
		} else {
			snprintf(text, sizeof(text), "%s", jsv_to_name(cfg_std_pad.cfg.port.input[type][a]));
		}

		gtk_button_set_label(
		        _gw_get_button(cfg_std_pad.builder,
		                dg_obj_name("%s_%s_button", std_pad_input_type[type],
		                        std_pad_button[a][0])), text);
	}
}
void cfg_std_pad_turbo_delay_value_changed(GtkRange *range, gint type) {
	cfg_std_pad.cfg.port.turbo[type].frequency = (BYTE) gtk_range_get_value(range);
	cfg_std_pad.cfg.port.turbo[type].counter = 0;
}

void cfg_std_pad_ok_clicked(GtkWidget *widget, _cfg_port *cfg_port) {
	gtk_widget_destroy(cfg_input.child);

	/* la mamcpy deve andare necessariamente dopo il destroy */
	memcpy(cfg_port, &cfg_std_pad.cfg, sizeof(_cfg_port));
}
void cfg_std_pad_cancel_clicked(GtkWidget *widget, gpointer user_data) {
	gtk_widget_destroy(cfg_input.child);
}
void cfg_std_pad_window_destroy(GtkWidget *widget, gpointer user_data) {
	cfg_std_pad.wait_js_input = FALSE;
	cfg_std_pad.force_exit_in_sequence = TRUE;
	cfg_std_pad.in_sequence = FALSE;
	cfg_std_pad.no_other_buttons = FALSE;

	cfg_input.child = NULL;

	g_object_unref(G_OBJECT(cfg_std_pad.builder));

	if (cfg_input.father != NULL) {
		gtk_widget_show(cfg_input.father);
	}
}

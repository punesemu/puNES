/*
 * cfgstdctrl.c
 *
 *  Created on: 04/nov/2011
 *      Author: fhorse
 */

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "gtk2.h"
#include "cfgstdctrl.h"
#include "param.h"

void cfg_standard_controller_page_notebook_keyboard(GtkWidget *notebook);
void cfg_standard_controller_page_notebook_joystick(GtkWidget *notebook);
void cfg_standard_controller_combobox_select_joystick_control(GtkCellLayout *cell_layout,
		GtkCellRenderer *cell, GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data);
void cfg_standard_controller_combobox_joystick_changed(GtkComboBox *combobox);
void cfg_standard_controller_input_changed_clicked(GtkButton *button, BYTE input);
void cfg_standard_controller_js_press_event(void);
void cfg_standard_controller_ok_clicked(GtkWidget *widget, _cfg_port *cfgport);
void cfg_standard_controller_cancel_clicked(GtkWidget *widget, _cfg_port *cfgport);
void cfg_standard_controller_destroy(void);
void cfg_standard_controller_erase(GtkWidget *widget, int type);
GtkWidget *cfg_standard_controller_line_notebook(const char *description, BYTE input);
GtkWidget *cfg_standard_controller_combobox_select_joystick(void);
gboolean cfg_standard_controller_key_press_event(GtkWidget *widget, GdkEventKey *event);
gboolean cfg_standard_controller_input_is_not_ok(DBWORD value);

#define maxButtons LENGTH(paramInputP1K)
#define cfg_standard_controller_change_button_label(new_label)\
	gtk_button_set_label(cfg_std_ctrl.button_pressed, new_label);

enum {
	NAME_JOYSTICK, VALUE_JOYSTICK, N_JOYSTICK, MAX_JOYSTICK = 4
};

typedef struct {
	_cfg_port cfg;
	GtkButton *button_pressed;
	GtkWidget *button[maxButtons * 2];
	GtkWidget *jscombo;
	BYTE no_other_buttons;
	BYTE controller_input;
	BYTE wait_js_input;
} _cfg_standard_controller;

_cfg_standard_controller cfg_std_ctrl;

char cazzata[][15] = {
	"retry",
	"another time",
	"no no no",
	"in use",
	":(",
	"...",
	"ufffff",
	"i don't think"
};

void cfg_standard_controller(_cfg_port *cfgport) {
	GtkWidget *mainbox, *notebook, *okcancel;

	char title[30];

	memset(&cfg_std_ctrl, 0, sizeof(cfg_std_ctrl));
	memcpy(&cfg_std_ctrl.cfg, cfgport, sizeof(cfg_std_ctrl.cfg));

	cfg_standard_controller_toplevel = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_container_set_border_width(GTK_CONTAINER(cfg_standard_controller_toplevel), 10);
	gtk_window_set_resizable(GTK_WINDOW(cfg_standard_controller_toplevel), FALSE);

	sprintf(title, "Controller %d setup", cfgport->id);
	gtk_window_set_title(GTK_WINDOW(cfg_standard_controller_toplevel), title);

	g_signal_connect(G_OBJECT(cfg_standard_controller_toplevel), "key_press_event",
			G_CALLBACK(cfg_standard_controller_key_press_event), NULL);
	g_signal_connect(G_OBJECT(cfg_standard_controller_toplevel), "destroy",
			G_CALLBACK(cfg_standard_controller_destroy), NULL);

	mainbox = gtk_vbox_new(FALSE, SPACING);
	gtk_container_add(GTK_CONTAINER(cfg_standard_controller_toplevel), mainbox);

	notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
	gtk_box_pack_start(GTK_BOX(mainbox), notebook, FALSE, FALSE, 0);

	cfg_standard_controller_page_notebook_keyboard(notebook);
	cfg_standard_controller_page_notebook_joystick(notebook);

	okcancel = cfg_input_ok_cancel(G_CALLBACK(cfg_standard_controller_ok_clicked),
			G_CALLBACK(cfg_standard_controller_cancel_clicked), cfgport);
	gtk_box_pack_start(GTK_BOX(mainbox), okcancel, FALSE, FALSE, 0);

	gtk_widget_show_all(cfg_standard_controller_toplevel);

	gtk_widget_hide(cfg_controllers_toplevel);
}
void cfg_standard_controller_page_notebook_keyboard(GtkWidget *notebook) {
	GtkWidget *page, *frame, *vbox;
	GtkWidget *line, *label;
	char description[30];

	page = gtk_hbox_new(FALSE, SPACING);

	frame = gtk_frame_new(" Standard Pad ");
	gtk_box_pack_start(GTK_BOX(page), frame, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 10);

	vbox = gtk_vbox_new(FALSE, SPACING);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	line = cfg_standard_controller_line_notebook("A", BUT_A);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	line = cfg_standard_controller_line_notebook("B", BUT_B);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	line = cfg_standard_controller_line_notebook("Select", SELECT);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	line = cfg_standard_controller_line_notebook("Start", START);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	line = cfg_standard_controller_line_notebook("Up", UP);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	line = cfg_standard_controller_line_notebook("Down", DOWN);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	line = cfg_standard_controller_line_notebook("Left", LEFT);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	line = cfg_standard_controller_line_notebook("Right", RIGHT);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	line = cfg_standard_controller_line_notebook("Turbo A", TRBA);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	line = cfg_standard_controller_line_notebook("Turbo B", TRBB);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	{
		line = cfg_input_std_button("Erase All");
		gtk_box_pack_end(GTK_BOX(vbox), line, FALSE, FALSE, 0);

		g_signal_connect(G_OBJECT(line), "clicked", G_CALLBACK(cfg_standard_controller_erase),
				GINT_TO_POINTER(KEYBOARD));

		gtk_box_pack_end(GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 0);
	}

	line = gtk_hbox_new(FALSE, SPACING);
	{
		label = gtk_label_new("   Press ESC to erase the key");
		gtk_box_pack_start(GTK_BOX(line), label, FALSE, FALSE, 0);
	}
	gtk_box_pack_end(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	gtk_widget_show(page);

	sprintf(description, "Keyboard");
	label = gtk_label_new(description);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, label);
}
void cfg_standard_controller_page_notebook_joystick(GtkWidget *notebook) {
	GtkWidget *page, *frame, *vbox;
	GtkWidget *line, *label;
	char description[30];

	page = gtk_hbox_new(FALSE, SPACING);

	frame = gtk_frame_new(" Standard Controller ");
	gtk_box_pack_start(GTK_BOX(page), frame, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 10);

	vbox = gtk_vbox_new(FALSE, SPACING);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	line = cfg_standard_controller_line_notebook("A", BUT_A + maxButtons);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	line = cfg_standard_controller_line_notebook("B", BUT_B + maxButtons);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	line = cfg_standard_controller_line_notebook("Select", SELECT + maxButtons);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	line = cfg_standard_controller_line_notebook("Start", START + maxButtons);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	line = cfg_standard_controller_line_notebook("Up", UP + maxButtons);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	line = cfg_standard_controller_line_notebook("Down", DOWN + maxButtons);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	line = cfg_standard_controller_line_notebook("Left", LEFT + maxButtons);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	line = cfg_standard_controller_line_notebook("Right", RIGHT + maxButtons);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	line = cfg_standard_controller_line_notebook("Turbo A", TRBA + maxButtons);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	line = cfg_standard_controller_line_notebook("Turbo B", TRBB + maxButtons);
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 0);

	line = gtk_hbox_new(FALSE, SPACING);
	{
		char text[30];

		sprintf(text, "   %-25s", "Device");

		label = gtk_label_new(text);
		gtk_box_pack_start(GTK_BOX(line), label, FALSE, FALSE, 0);

		label = gtk_label_new("  ");
		gtk_box_pack_end(GTK_BOX(line), label, FALSE, FALSE, 0);

		cfg_std_ctrl.jscombo = cfg_standard_controller_combobox_select_joystick();
		gtk_box_pack_end(GTK_BOX(line), cfg_std_ctrl.jscombo, FALSE, FALSE, 0);
	}
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	line = gtk_hbox_new(FALSE, SPACING);
	{
		label = gtk_label_new("");
		gtk_box_pack_start(GTK_BOX(line), label, FALSE, FALSE, 0);
	}
	gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	{
		line = cfg_input_std_button("Erase All");
		gtk_box_pack_end(GTK_BOX(vbox), line, FALSE, FALSE, 0);

		g_signal_connect(G_OBJECT(line), "clicked", G_CALLBACK(cfg_standard_controller_erase),
				GINT_TO_POINTER(JOYSTICK));

		gtk_box_pack_end(GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 0);
	}

	line = gtk_hbox_new(FALSE, SPACING);
	{
		label = gtk_label_new("   Press ESC to erase the key");
		gtk_box_pack_start(GTK_BOX(line), label, FALSE, FALSE, 0);
	}
	gtk_box_pack_end(GTK_BOX(vbox), line, FALSE, FALSE, 0);

	gtk_widget_show(page);

	sprintf(description, "Joystick");
	label = gtk_label_new(description);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, label);
}
void cfg_standard_controller_combobox_select_joystick_control(GtkCellLayout *cell_layout,
		GtkCellRenderer *cell, GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data) {
	guint index;
	SDBWORD fd;
	char device[50];
	BYTE same_port = FALSE;

	gtk_tree_model_get(tree_model, iter, VALUE_JOYSTICK, &index, -1);

	/* il controller 1 ha priorita' sul 2 */
	if (cfg_std_ctrl.cfg.id == 1) {
		if (index == cfg_port2.port.joyID) {
			same_port = TRUE;
		}
	} else if (cfg_std_ctrl.cfg.id == 2) {
		if (index == cfg_port1.port.joyID) {
			same_port = TRUE;
		}
	}

	sprintf(device, "%s%d", JSDEVPATH, index);
	fd = open(device, O_RDONLY | O_NONBLOCK);

	if ((fd < 0) || same_port) {
		g_object_set(cell, "foreground", "Gray", "foreground-set", TRUE, NULL);
	} else {
		/* imposto il colore di default */
		g_object_set(cell, "foreground-set", FALSE, NULL);
	}

	if (fd) {
		close(fd);
	}
}
void cfg_standard_controller_combobox_joystick_changed(GtkComboBox *combobox) {
	GtkTreeIter iter;
	GtkTreeModel *model;

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));

	gtk_combo_box_get_active_iter(GTK_COMBO_BOX(combobox), &iter);

	gtk_tree_model_get(model, &iter, VALUE_JOYSTICK,
			&cfg_std_ctrl.cfg.port.joyID, -1);
}
void cfg_standard_controller_input_changed_clicked(GtkButton *button, BYTE input) {
	if (cfg_std_ctrl.no_other_buttons) {
		return;
	}

	cfg_std_ctrl.button_pressed = button;
	cfg_std_ctrl.controller_input = input;

	if (input < maxButtons) {
		/* keyboard */
		cfg_std_ctrl.no_other_buttons = TRUE;
		cfg_standard_controller_change_button_label("press a key");
	} else {
		/* joystick */
		if (!cfg_std_ctrl.no_other_buttons) {
			g_thread_create( (GThreadFunc) cfg_standard_controller_js_press_event, NULL, FALSE,
					NULL);
		}
	}
}
void cfg_standard_controller_js_press_event(void) {
	_jsevent jse;
	char device[30];
	BYTE index = cfg_std_ctrl.cfg.port.joyID;
	BYTE read_ok = FALSE;
	BYTE size = sizeof(jse);
	DBWORD value = 0;
	SDBWORD fd;
	WORD bytes;

#define _protect_change_gui(command)\
		if (cfg_standard_controller_toplevel) {\
			gdk_threads_enter();\
			command;\
			gdk_threads_leave();\
		}
#define protect_change_button_label(label)\
		_protect_change_gui(cfg_standard_controller_change_button_label(label))
#define protect_change_combobox_sensitive(state)\
		_protect_change_gui(gtk_widget_set_sensitive(GTK_WIDGET(cfg_std_ctrl.jscombo),\
				state))

	if (cfg_std_ctrl.no_other_buttons) {
		g_thread_exit(NULL);
	}

	/*
	 * i due controller non possono
	 * utilizzare lo stesso device. ed
	 */
	if (cfg_std_ctrl.cfg.id == 1) {
		if (index == cfg_port2.port.joyID) {
			g_thread_exit(NULL);
		}
	} else if (cfg_std_ctrl.cfg.id == 2) {
		if (index == cfg_port1.port.joyID) {
			g_thread_exit(NULL);
		}
	}

	sprintf(device, "%s%d", JSDEVPATH, index);
	fd = open(device, O_RDONLY | O_NONBLOCK);

	if (fd < 0) {
		g_thread_exit(NULL);
	}

	cfg_std_ctrl.no_other_buttons = TRUE;
	cfg_std_ctrl.wait_js_input = TRUE;

	{
		BYTE i;

		for (i = 0; i < 20; i++) {
			if (read(fd, &jse, size) < 0) {
				fprintf(stderr, "error on reading controllers configurations\n");
			}
		}
	}

	protect_change_button_label("press a key");
	protect_change_combobox_sensitive(FALSE);

	retry:
	while (cfg_std_ctrl.wait_js_input == TRUE) {
		read_ok = FALSE;

		memset(&jse, 0, size);

		if ((bytes = read(fd, &jse, size)) == size) {
			if (jse.value == 0) {
				continue;
			}
			jse.type &= ~JS_EVENT_INIT;

			if (jse.type == JS_EVENT_AXIS && jse.value) {
				value = (jse.number << 1) + 1;
				if (jse.value > 0) {
					value++;
				}
				//fprintf(stderr, "axis   : %s (%d)\n", jsvToName(value), value);
			} else if (jse.type == JS_EVENT_BUTTON && jse.value) {
				value = jse.number | 0x400;
				//fprintf(stderr, "button : %s (%d) %d\n", jsvToName(value), value, jse.value);
			} else {
				//fprintf(stderr, "%04X %04X %d\n", jse.type, jse.number, (SWORD) jse.value);
				continue;
			}
			read_ok = TRUE;
			break;
		}
		guiSleep(30);
	}

	if (read_ok) {
		//fprintf(stderr, "reak_ok : %s (%d)\n\n", jsvToName(value), value);
		if (cfg_standard_controller_input_is_not_ok(value)) {
			cfg_std_ctrl.wait_js_input = TRUE;
			protect_change_button_label(cazzata[(WORD) (rand() % 8) & 0x07]);
			goto retry;
		} else {
			BYTE button = cfg_std_ctrl.controller_input - maxButtons;

			cfg_std_ctrl.cfg.port.input[JOYSTICK][button] = value;
			cfg_std_ctrl.no_other_buttons = FALSE;
			cfg_std_ctrl.controller_input = 0;
			protect_change_button_label(jsvToName(value));
		}
	}

	close(fd);

	protect_change_combobox_sensitive(TRUE);

	g_thread_exit(NULL);
}
void cfg_standard_controller_ok_clicked(GtkWidget *widget, _cfg_port *cfgport) {
	gtk_widget_destroy(cfg_standard_controller_toplevel);

	/* la mamcpy deve andare necessariamente dopo il destroy */
	memcpy(cfgport, &cfg_std_ctrl.cfg, sizeof(cfg_std_ctrl.cfg));
}
void cfg_standard_controller_erase(GtkWidget *widget, int type) {
	BYTE i;

	if (type == KEYBOARD) {
		for (i = 0; i < maxButtons; i++) {
			cfg_std_ctrl.cfg.port.input[KEYBOARD][i] = 0;
			gtk_button_set_label(GTK_BUTTON(cfg_std_ctrl.button[i]), "NULL");
		}
	} else {
		for (i = maxButtons; i < maxButtons * 2; i++) {
			cfg_std_ctrl.cfg.port.input[JOYSTICK][i] = 0;
			gtk_button_set_label(GTK_BUTTON(cfg_std_ctrl.button[i]), "NULL");
		}
	}
}
void cfg_standard_controller_cancel_clicked(GtkWidget *widget, _cfg_port *cfgport) {
	gtk_widget_destroy(cfg_standard_controller_toplevel);
}
void cfg_standard_controller_destroy(void) {
	cfg_standard_controller_toplevel = NULL;

	/*
	 * nel caso sia stato premuto il tasto cancel
	 * mentre sono in attesa di un input dal joystick,
	 * chiedo di uscire dalla routine cfg_standard_js_press_event.
	 */
	if (cfg_std_ctrl.wait_js_input) {
		cfg_std_ctrl.wait_js_input = FALSE;
	}

	if (cfg_controllers_toplevel != NULL) {
		gtk_widget_show(cfg_controllers_toplevel);
	}
}
GtkWidget *cfg_standard_controller_line_notebook(const char *description, BYTE input) {
	GtkWidget *line, *label;
	char text[30];

	line = gtk_hbox_new(FALSE, SPACING);

	sprintf(text, "   %-25s", description);

	label = gtk_label_new(text);
	gtk_box_pack_start(GTK_BOX(line), label, FALSE, FALSE, 0);

	label = gtk_label_new("  ");
	gtk_box_pack_end(GTK_BOX(line), label, FALSE, FALSE, 0);

	if (input < maxButtons) {
		strcpy(text, keyvalToName(cfg_std_ctrl.cfg.port.input[KEYBOARD][input]));
	} else {
		strcpy(text, jsvToName(cfg_std_ctrl.cfg.port.input[JOYSTICK][input - maxButtons]));
	}

	cfg_std_ctrl.button[input] = cfg_input_std_button(text);
	gtk_box_pack_end(GTK_BOX(line), cfg_std_ctrl.button[input], FALSE, FALSE, 0);

	g_signal_connect(G_OBJECT(cfg_std_ctrl.button[input]), "clicked",
			G_CALLBACK(cfg_standard_controller_input_changed_clicked),
			GINT_TO_POINTER((gint) input));

	return (line);
}
GtkWidget *cfg_standard_controller_combobox_select_joystick(void) {
	GtkWidget *combobox;
	GtkListStore *model;
	GtkTreeIter iter;
	GtkCellRenderer *renderer;
	BYTE i;

	combobox = gtk_combo_box_new();

	cfg_input_resize_std_widget(combobox);

	model = gtk_list_store_new(N_JOYSTICK, G_TYPE_STRING, G_TYPE_INT);

	for (i = 0; i < MAX_JOYSTICK; i++) {
		char description[30];

		sprintf(description, "%d", i);

		gtk_list_store_append(model, &iter);
		gtk_list_store_set(model, &iter, NAME_JOYSTICK, description, VALUE_JOYSTICK, i, -1);
	}

	gtk_combo_box_set_model(GTK_COMBO_BOX(combobox), GTK_TREE_MODEL(model));
	g_object_unref(model);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), renderer, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(combobox), renderer, "text", 0);
	gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(combobox), renderer,
			cfg_standard_controller_combobox_select_joystick_control, NULL, NULL);

	if (cfg_std_ctrl.cfg.port.joyID == nameToJsn("NULL")) {
		gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
	} else {
		gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), cfg_std_ctrl.cfg.port.joyID);
	}

	g_signal_connect(G_OBJECT(combobox), "changed",
			G_CALLBACK(cfg_standard_controller_combobox_joystick_changed), NULL);

	return (combobox);
}
gboolean cfg_standard_controller_key_press_event(GtkWidget *widget, GdkEventKey *event) {
	BYTE type, index;
	gboolean value_ok;

	if (!cfg_std_ctrl.no_other_buttons) {
		return (TRUE);
	}

	if (cfg_std_ctrl.controller_input < maxButtons) {
		type = KEYBOARD;
		index = cfg_std_ctrl.controller_input;
	} else {
		type = JOYSTICK;
		index = cfg_std_ctrl.controller_input - maxButtons;
	}

	value_ok = TRUE;

	if (type == KEYBOARD) {
		if (event->keyval == GDK_Escape) {
			cfg_std_ctrl.cfg.port.input[type][index] = 0;
			cfg_standard_controller_change_button_label("NULL");
		} else {
			if (cfg_standard_controller_input_is_not_ok(event->keyval)) {
				value_ok = FALSE;
				cfg_standard_controller_change_button_label(cazzata[(WORD) (rand() % 8) & 0x07]);
			} else {
				cfg_std_ctrl.cfg.port.input[type][index] = event->keyval;
				cfg_standard_controller_change_button_label(keyvalToName(event->keyval));
			}
		}
	} else {
		/*
		 * quando sto configurando il joystick, l'unico input da tastiera
		 * che accetto e' l'escape.
		 */
		if (event->keyval == GDK_Escape) {
			cfg_std_ctrl.cfg.port.input[type][index] = 0;
			cfg_standard_controller_change_button_label("NULL");
		} else {
			return (TRUE);
		}
	}

	if (value_ok) {
		cfg_std_ctrl.no_other_buttons = FALSE;
		cfg_std_ctrl.controller_input = 0;
		if (type == JOYSTICK) {
			cfg_std_ctrl.wait_js_input = FALSE;
		}
		return (TRUE);
	}

	return (TRUE);
}
gboolean cfg_standard_controller_input_is_not_ok(DBWORD input) {
	_cfg_port *p1, *p2;
	BYTE i;

	p1 = &cfg_std_ctrl.cfg;

	if (cfg_std_ctrl.cfg.id == 1) {
		p2 = &cfg_port2;
	} else {
		p2 = &cfg_port1;
	}

	if (cfg_std_ctrl.controller_input < maxButtons) {
		/* keyboard */
		for (i = 0; i < maxButtons; i++) {
			if ((p1->port.input[KEYBOARD][i] == input) || (p2->port.input[KEYBOARD][i] == input)) {
				if (cfg_std_ctrl.controller_input == i) {
					return (EXIT_OK);
				} else {
					return (EXIT_ERROR);
				}
			}
		}
	} else {
		/* joystick */
		for (i = 0; i < maxButtons; i++) {
			if (p2->port.input[JOYSTICK][i] == input) {
				if (p1->port.joyID == p2->port.joyID) {
					return (EXIT_ERROR);
				}
			}
			if (p1->port.input[JOYSTICK][i] == input) {
				if ((cfg_std_ctrl.controller_input - maxButtons) == i) {
					return (EXIT_OK);
				} else {
					return (EXIT_ERROR);
				}
			}
		}
	}
	return (EXIT_OK);
}

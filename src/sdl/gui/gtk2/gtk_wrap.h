/*
 * gtk_wrap.h
 *
 *  Created on: 20/nov/2013
 *      Author: fhorse
 */

#ifndef GTK_WRAP_H_
#define GTK_WRAP_H_

#include "gui.h"

enum glade_h_name {
	INPUT_DIALOG,
	INPUT_STD_PAD
};

#define _gw_get_object(builder, obj, type_object) type_object(gtk_builder_get_object(builder, obj))
#define _gw_get_widget(builder, obj) _gw_get_object(builder, obj, GTK_WIDGET)
#define _gw_get_combobox(builder, obj) _gw_get_object(builder, obj, GTK_COMBO_BOX)
#define _gw_get_liststore(builder, obj) _gw_get_object(builder, obj, GTK_LIST_STORE)
#define _gw_get_label(builder, obj) _gw_get_object(builder, obj, GTK_LABEL)
#define _gw_get_button(builder, obj) _gw_get_object(builder, obj, GTK_BUTTON)
#define _gw_get_togglebutton(builder, obj) _gw_get_object(builder, obj, GTK_TOGGLE_BUTTON)
#define _gw_get_entry(builder, obj) _gw_get_object(builder, obj, GTK_ENTRY)

void gw_image_from_inline(GtkWidget *widget, const guint8 *data);

/* dialog glade function */
char *dg_obj_name(const char *fmt, ...);
GtkWidget *dg_widget_from_obj_name(GtkBuilder *builder, const char *fmt, ...);
void dg_create_gtkbuilder(GtkBuilder **builder, int id_glade_h);
void dg_signal_connect(GtkBuilder *builder, char *obj_name, char *signal, gpointer callback,
        gpointer user_data);
void dg_signal_connect_swapped(GtkBuilder *builder, char *obj_name, char *signal,
		gpointer callback, gpointer user_data);
void dg_signal_disconnect(GtkBuilder *builder, char *obj_name, char *signal);
void dg_set_sensitive(GtkBuilder *builder, char *obj_name, gboolean sensitive);
void dg_image_from_inline(GtkBuilder *builder, char *obj_name, const guint8 *data);

#endif /* GTK_WRAP_H_ */

/*
 * gtk_wrap.c
 *
 *  Created on: 20/nov/2013
 *      Author: fhorse
 */

#include "gtk_wrap.h"
#include "glade/apu_channels_glade.h"
#include "glade/input_configuration_glade.h"
#include "glade/uncompress_selection_glade.h"
#include "glade/overscan_borders_configuration_glade.h"

char *_dg_obj_name(const char *fmt, va_list argp);

void gw_image_from_inline(GtkWidget *widget, const guint8 *data) {
	GdkPixbuf *pixbuf = NULL;
	GType type = G_OBJECT_TYPE(widget);

	pixbuf = gdk_pixbuf_new_from_inline(-1, data, FALSE, NULL);

	if (type == GTK_TYPE_IMAGE_MENU_ITEM) {
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(widget),
		        gtk_image_new_from_pixbuf(pixbuf));
	} else if (type == GTK_TYPE_IMAGE) {
		gtk_image_set_from_pixbuf(GTK_IMAGE(widget), pixbuf);
	}

	g_object_unref(pixbuf);
}

/* dialog glade functions */
char *_dg_obj_name(const char *fmt, va_list argp) {
	static char obj_name[100];

	vsnprintf(obj_name, sizeof(obj_name), fmt, argp);

	return (obj_name);
}
char *dg_obj_name(const char *fmt, ...) {
	static char *obj_name;
	va_list ap;

	va_start(ap, fmt);
	obj_name = _dg_obj_name(fmt, ap);
	va_end(ap);

	return (obj_name);
}
GtkWidget *dg_widget_from_obj_name(GtkBuilder *builder, const char *fmt, ...) {
	static GtkWidget *widget;
	va_list ap;

	va_start(ap, fmt);
	widget = _gw_get_widget(builder, _dg_obj_name(fmt, ap));
	va_end(ap);

	return (widget);
}
void dg_create_gtkbuilder(GtkBuilder **builder, int id_glade_h) {
	GError *error = NULL;
	const gchar *data = NULL;

	(*builder) = gtk_builder_new();

	switch (id_glade_h) {
		case APU_CHANNELS_DIALOG:
			data =  apu_channels_glade;
			break;
		case INPUT_DIALOG:
		case INPUT_STD_PAD_DIALOG:
			data = input_configuration_glade;
			break;
		case UNCOMPRESS_SELECTION_DIALOG:
			data = uncompress_selection_glade;
			break;
		case OVERSCAN_BORDERS_DIALOG:
			data = overscan_borders_configuration_glade;
			break;
	}

	if (!gtk_builder_add_from_string((*builder), data, -1, &error)) {
		if (error != NULL) {
			g_warning("%s", error->message);
			g_error_free(error);
		}
	}
}
void dg_signal_connect(GtkBuilder *builder, const char *obj_name, const char *signal,
		GCallback callback, gpointer user_data) {
	GtkWidget *widget = _gw_get_object(builder, obj_name, GTK_WIDGET);

	g_signal_connect(G_OBJECT(widget), signal, G_CALLBACK(callback), user_data);
}
void dg_signal_connect_swapped(GtkBuilder *builder, const char *obj_name, const char *signal,
		GCallback callback, gpointer user_data) {
	GtkWidget *widget = _gw_get_object(builder, obj_name, GTK_WIDGET);

	g_signal_connect_swapped(G_OBJECT(widget), signal, G_CALLBACK(callback), user_data);
}
void dg_signal_disconnect(GtkBuilder *builder, const char *obj_name, const char *signal) {
	GtkWidget *widget = _gw_get_object(builder, obj_name, GTK_WIDGET);
	guint signal_id = g_signal_lookup(signal, G_OBJECT_TYPE(widget));
	guint handler;

	while ((handler = g_signal_handler_find(widget, G_SIGNAL_MATCH_ID, signal_id, 0, NULL, NULL,
			NULL))) {
		g_signal_handler_disconnect(widget, handler);
	}
}
void dg_set_sensitive(GtkBuilder *builder, char *obj_name, gboolean sensitive) {
	gtk_widget_set_sensitive(_gw_get_widget(builder, obj_name), sensitive);
}
void dg_image_from_inline(GtkBuilder *builder, const char *obj_name, const guint8 *data) {
	GtkWidget *widget = _gw_get_object(builder, obj_name, GTK_WIDGET);

	gw_image_from_inline(widget, data);
}

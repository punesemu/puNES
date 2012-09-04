/*
 * configurations.c
 *
 *  Created on: 02/set/2012
 *      Author: fhorse
 */

#include "gui.h"

#include "glade/audio_configuration_glade.h"
#include "glade/configurations_glade.h"
#include "glade/general_icons_inline.h"
#include "glade/video_configuration_icons_inline.h"
#include "glade/audio_configuration_icons_inline.h"

void change_image_element(const guint8 *icon_inline, gchar *element);
void video_configurations_init(void);
void audio_configurations_init(void);

void on_button_configurations_cancel_clicked(GtkWidget *widget, gpointer data);

GtkBuilder *builder;
GtkWidget *configurations_window;

void configurations_notebook(void) {
	GError *error = NULL;

	/* Create new GtkBuilder object */
	builder = gtk_builder_new();

	/* Load UI from file. If error occurs, report it and quit application.
	 * Replace "tut.glade" with your saved project. */
	if (!gtk_builder_add_from_string(builder, configurations_glade, -1, &error)) {
		g_warning("%s", error->message);
		g_free(error);
	}

	/* Get main window pointer from UI */
	configurations_window = GTK_WIDGET(gtk_builder_get_object(builder, "configurations"));

	change_image_element(mode_inline, "image_mode");

	video_configurations_init();
	audio_configurations_init();

	/* Connect signals */
	gtk_builder_connect_signals(builder, NULL);

	/* Show window. All other widgets are automatically shown by GtkBuilder */
	gtk_widget_show(configurations_window);
}
void change_image_element(const guint8 *icon_inline, gchar *element) {
	GdkPixbuf *pixbuf;
	GtkImage *image;

	pixbuf = gdk_pixbuf_new_from_inline(-1, icon_inline, FALSE, ((void *)0));
	image = GTK_IMAGE(gtk_builder_get_object(builder, element));
	gtk_image_clear(image);
	gtk_image_set_from_pixbuf(image, pixbuf);
	g_object_unref(pixbuf), pixbuf = ((void *)0);
}
void video_configurations_init(void) {
	change_image_element(rendering_inline, "image_rendering");
	change_image_element(fps_inline, "image_fps");
	change_image_element(frame_skip_inline, "image_frame_skip");
	change_image_element(scale_inline, "image_scale");
	change_image_element(effect_inline, "image_effect");
	change_image_element(filter_inline, "image_filter");
	change_image_element(overscan_inline, "image_overscan");
	change_image_element(overscan_default_inline, "image_overscan_default");
	change_image_element(palette_predefined_inline, "image_palette_predefined");
}
void audio_configurations_init(void) {
	change_image_element(sample_rate_inline, "image_sample_rate");
	change_image_element(channels_inline, "image_channels");
	change_image_element(quality_inline, "image_quality");
}


void on_button_configurations_cancel_clicked(GtkWidget *widget, gpointer data) {
	gtk_widget_destroy(configurations_window);

	/* Destroy builder, since we don't need it anymore */
	g_object_unref(G_OBJECT(builder));
}


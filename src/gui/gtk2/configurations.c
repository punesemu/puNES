/*
 * configurations.c
 *
 *  Created on: 02/set/2012
 *      Author: fhorse
 */

#include "gui.h"

#include "param.h"
#include "cfgfile.h"
#include "opengl.h"

#include "glade/audio_configuration_glade.h"
#include "glade/configurations_glade.h"
#include "glade/general_icons_inline.h"
#include "glade/video_configuration_icons_inline.h"
#include "glade/audio_configuration_icons_inline.h"

void change_image_element(const guint8 *icon_inline, gchar *element);
void populate_combobox_with_parameters(const _param *param, gint length, gint start,
        gint active, gchar *c, gchar *ls);
void populate_combobox(gchar *c, gchar *ls, gchar *text, gint index, gint value, gint active);

void general_configurations_init(void);
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

	general_configurations_init();
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
void populate_combobox_with_parameters(const _param *param, gint length, gint start,
        gint active, gchar *c, gchar *ls) {
	gint i;
	GtkComboBox *combobox;
	GtkListStore *liststore;
	GtkTreeIter it;

	liststore = GTK_LIST_STORE(gtk_builder_get_object(builder, ls));
	combobox = GTK_COMBO_BOX(gtk_builder_get_object(builder, c));

	for (i = 0; i < length - start; i++) {
		gint index = i + start;
		gtk_list_store_insert_with_values(liststore, &it, i, 0, param[index].lname, 1, index, -1);
	}

	gtk_combo_box_set_active(combobox, active - start);
}
void populate_combobox(gchar *c, gchar *ls, gchar *text, gint index, gint value, gint active) {
	GtkComboBox *combobox;
	GtkListStore *liststore;
	GtkTreeIter it;

	liststore = GTK_LIST_STORE(gtk_builder_get_object(builder, ls));
	combobox = GTK_COMBO_BOX(gtk_builder_get_object(builder, c));

	gtk_list_store_insert_with_values(liststore, &it, index, 0, text, 1, value, -1);

	if (value == active) {
		gtk_combo_box_set_active(combobox, index);
	}
}




void general_configurations_init(void) {
	/* mode */
	change_image_element(mode_inline, "image_mode");
	populate_combobox("combobox_mode", "liststore_mode", "Auto", 0, AUTO, cfg->mode);
	populate_combobox("combobox_mode", "liststore_mode", "PAL", 1, PAL, cfg->mode);
	populate_combobox("combobox_mode", "liststore_mode", "NTSC", 2, NTSC, cfg->mode);
	populate_combobox("combobox_mode", "liststore_mode", "Dendy", 3, DENDY, cfg->mode);

	//populate_combobox_whit_parameters(pMode, LENGTH(pMode), 0, cfg->mode, "combobox_mode",
	//        "liststore_mode");

	/* save now */
	change_image_element(save_now_inline, "image_save_now");
}
void video_configurations_init(void) {
	/* rendering */
	change_image_element(rendering_inline, "image_rendering");
	populate_combobox("combobox_rendering", "liststore_rendering", "Software", 0, 0, cfg->mode);
	if (opengl.supported) {
		populate_combobox("combobox_rendering", "liststore_rendering", "OpenGL", 1, 1, cfg->mode);
	}
	if (opengl.glsl.compliant) {
		populate_combobox("combobox_rendering", "liststore_rendering", "OpenGL GLSL", 2, 2, cfg->mode);
	}

	//populate_combobox_whit_parameters(pRendering, LENGTH(pRendering), 0, "liststore_rendering");
	/* fps */
	change_image_element(fps_inline, "image_fps");
	//populate_combobox_whit_parameters(pFps, LENGTH(pFps), 0, "liststore_fps");
	/* frame skip */
	change_image_element(frame_skip_inline, "image_frame_skip");
	//populate_combobox_whit_parameters(pFsk, LENGTH(pFsk), 0, "liststore_frame_skip");
	/* scale */
	change_image_element(scale_inline, "image_scale");
	//populate_combobox_whit_parameters(pSize, LENGTH(pSize), 1, "liststore_scale");

	change_image_element(effect_inline, "image_effect");
	change_image_element(filter_inline, "image_filter");
	change_image_element(overscan_inline, "image_overscan");
	change_image_element(overscan_default_inline, "image_overscan_default");
	change_image_element(palette_predefined_inline, "image_palette_predefined");
	change_image_element(palette_custom_inline, "image_palette_custom");
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


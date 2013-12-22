/*
 * cfg_apu_channels.c
 *
 *  Created on: 08/nov/2013
 *      Author: fhorse
 */

#include <string.h>
#define __GUI_BASE__
#define __GUI_SND__
#include "gui.h"
#undef __GUI_SND__
#undef __GUI_BASE__
#include "apu.h"
#include "cfg_file.h"

void apu_channels_check(void);
void apu_channels_toggle(gint channel);
void apu_channels_toggle_all(gint mode);
void apu_channels_volume_value_changed(GtkRange *range, gint channel);
gboolean apu_channels_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data);
void apu_channels_ok_clicked(GtkButton *button, gpointer user_data);
void apu_channels_cancel_clicked(GtkButton *button, gpointer user_data);
void apu_channels_window_destroy(GtkWidget *widget, gpointer user_data);

struct _apu_channels_data {
	int update;
	_config_apu cfg_save;
	GtkBuilder *builder;
	GtkWidget *window;
} apu_channels_data;

static const char apu_channels_list[][10] = {
		"square1", "square2", "triangle", "noise", "dmc", "extra", "master"
};

void apu_channels_dialog(void) {
	memset(&apu_channels_data, 0x00, sizeof(apu_channels_data));
	memcpy(&apu_channels_data.cfg_save, &cfg->apu, sizeof(_config_apu));

	dg_create_gtkbuilder(&apu_channels_data.builder, APU_CHANNELS_DIALOG);

	apu_channels_data.window = GTK_WIDGET(
	        gtk_builder_get_object(apu_channels_data.builder, "apu_channels_dialog"));

	gtk_builder_connect_signals(apu_channels_data.builder, NULL);

	/* ora collego i miei segnali */
	{
		gint i;

		for (i = APU_S1; i <= APU_EXTRA; i++) {
			dg_signal_connect_swapped(apu_channels_data.builder,
			        dg_obj_name("apu_channels_%s_checkbutton", apu_channels_list[i]), "toggled",
			        G_CALLBACK(apu_channels_toggle), GINT_TO_POINTER(i));
		}
	}

	dg_signal_connect_swapped(apu_channels_data.builder, "apu_channels_disable_all_button",
	        "clicked", G_CALLBACK(apu_channels_toggle_all), GINT_TO_POINTER(FALSE));
	dg_signal_connect_swapped(apu_channels_data.builder, "apu_channels_active_all_button",
	        "clicked", G_CALLBACK(apu_channels_toggle_all), GINT_TO_POINTER(TRUE));
	dg_signal_connect_swapped(apu_channels_data.builder, "apu_channels_defaults_button",
	        "clicked", G_CALLBACK(apu_channels_toggle_all), GINT_TO_POINTER(2));

	{
		gint i;

		for (i = APU_S1; i <= APU_MASTER; i++) {
			dg_signal_connect(apu_channels_data.builder,
			        dg_obj_name("apu_channels_%s_hscale", apu_channels_list[i]), "value-changed",
			        G_CALLBACK(apu_channels_volume_value_changed), GINT_TO_POINTER(i));
		}
	}

	dg_signal_connect(apu_channels_data.builder, "apu_channels_ok_button", "clicked",
			G_CALLBACK(apu_channels_ok_clicked), NULL);
	dg_signal_connect(apu_channels_data.builder, "apu_channels_cancel_button", "clicked",
			G_CALLBACK(apu_channels_cancel_clicked), NULL);
	g_signal_connect(G_OBJECT(apu_channels_data.window), "delete-event",
	        G_CALLBACK(apu_channels_delete_event), NULL);
	g_signal_connect(G_OBJECT(apu_channels_data.window), "destroy",
	        G_CALLBACK(apu_channels_window_destroy), NULL);

	apu_channels_check();

	gtk_widget_show(apu_channels_data.window);
}
void apu_channels_check(void) {
	BYTE i;

	apu_channels_data.update = TRUE;

	for (i = APU_S1; i <= APU_EXTRA; i++) {
		gtk_toggle_button_set_active(
		        _gw_get_togglebutton(apu_channels_data.builder,
		                dg_obj_name("apu_channels_%s_checkbutton", apu_channels_list[i])),
		        cfg->apu.channel[i]);
		gtk_range_set_value(
		        _gw_get_range(apu_channels_data.builder,
		                dg_obj_name("apu_channels_%s_hscale", apu_channels_list[i])),
		        cfg->apu.volume[i]);
	}

	gtk_range_set_value(_gw_get_range(apu_channels_data.builder, "apu_channels_master_hscale"),
	        cfg->apu.volume[i]);

	apu_channels_data.update = FALSE;
}
void apu_channels_toggle(gint channel) {
	if (apu_channels_data.update == TRUE) {
		return;
	}

	cfg->apu.channel[channel] = !cfg->apu.channel[channel];
}
void apu_channels_toggle_all(gint mode) {
	BYTE i;

	if (mode == 2) {
		for (i = APU_S1; i <= APU_MASTER; i++) {
			cfg->apu.volume[i] = 1.0f;
		}
		mode = TRUE;
	}
	/*
	 * non devo forzare cfg->apu.channel[APU_MASTER] perche'
	 * lo utilizzo per abilitare o disabilitare il suono
	 * globalmente e viene impostato altrove.
	 */
	for (i = APU_S1; i <= APU_EXTRA; i++) {
		cfg->apu.channel[i] = mode;
	}
	apu_channels_check();
	emu_pause(TRUE);
	gui_flush();
	emu_pause(FALSE);
}
void apu_channels_volume_value_changed(GtkRange *range, gint channel) {
	cfg->apu.volume[channel] = gtk_range_get_value(range);
}
void apu_channels_ok_clicked(GtkButton *button, gpointer user_data) {
	gtk_widget_destroy(apu_channels_data.window);
}
void apu_channels_cancel_clicked(GtkButton *button, gpointer user_data) {
	emu_pause(TRUE);
	memcpy(&cfg->apu, &apu_channels_data.cfg_save, sizeof(_config_apu));
	emu_pause(FALSE);

	gtk_widget_destroy(apu_channels_data.window);
}
gboolean apu_channels_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
	emu_pause(TRUE);
	memcpy(&cfg->apu, &apu_channels_data.cfg_save, sizeof(_config_apu));
	emu_pause(FALSE);

	return (FALSE);
}
void apu_channels_window_destroy(GtkWidget *widget, gpointer user_data) {
	g_object_unref(G_OBJECT(apu_channels_data.builder));
}


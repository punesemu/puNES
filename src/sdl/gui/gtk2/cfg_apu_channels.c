/*
 * cfg_apu_channels.c
 *
 *  Created on: 08/nov/2013
 *      Author: fhorse
 */

#include <string.h>
#include "gui.h"
#include "gui_snd.h"
#include "glade/apu_channels_glade.h"
#include "param.h"
#include "cfg_file.h"
#include "apu.h"

#define _get_object(out, obj)\
	out = GTK_WIDGET(gtk_builder_get_object(apu_channels_data.builder, obj));
#define _signal_connect_swapped(obj, signal, callback, gint)\
{\
	GtkWidget *tmp;\
	_get_object(tmp, obj)\
	g_signal_connect_swapped(G_OBJECT(tmp), signal, G_CALLBACK(callback), GINT_TO_POINTER(gint));\
}
#define _signal_connect(obj, signal, callback, gint)\
{\
	GtkWidget *tmp;\
	_get_object(tmp, obj)\
	g_signal_connect(G_OBJECT(tmp), signal, G_CALLBACK(callback), GINT_TO_POINTER(gint));\
}
#define _check_hscale(obj, dst)\
{\
	GtkWidget *tmp;\
	_get_object(tmp, obj)\
	gtk_range_set_value(GTK_RANGE(tmp), dst);\
}
#define channel_connect_toggled(obj, index)\
	_signal_connect_swapped(obj, "toggled", apu_channels_toggle, index)
#define channel_connect_clicked(obj, mode)\
	_signal_connect_swapped(obj, "clicked", apu_channels_toggle_all, mode)
#define volume_connect_value_changed(obj, mode)\
	_signal_connect(obj, "value-changed", apu_channels_volume_value_changed, mode)
#define channel_check_active(obj, index)\
{\
	GtkWidget *tmp;\
	_get_object(tmp, obj)\
	if (!cfg->apu.channel[index]) {\
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp), FALSE);\
	} else {\
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp), TRUE);\
	}\
}
#define volume_check(obj, index)\
	_check_hscale(obj, cfg->apu.volume[index])

void apu_channels_check(void);
void apu_channels_toggle(gint channel);
void apu_channels_toggle_all(gint mode);
void apu_channels_volume_value_changed(GtkRange *range, gint channel);
gboolean apu_channels_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data);
void apu_channels_ok_clicked(GtkButton *button, gpointer user_data);
void apu_channels_cancel_clicked(GtkButton *button, gpointer user_data);

struct _apu_channels_data {
	int update;
	_config_apu cfg_save;
	GtkBuilder *builder;
	GtkWidget *window;
} apu_channels_data;

void apu_channels_dialog(void) {
	GError *error = NULL;

	memset(&apu_channels_data, 0x00, sizeof(apu_channels_data));
	memcpy(&apu_channels_data.cfg_save, &cfg->apu, sizeof(_config_apu));

	/* Create new GtkBuilder object */
	apu_channels_data.builder = gtk_builder_new();

	/* Load UI from file. If error occurs, report it and quit application. */
	if (!gtk_builder_add_from_string(apu_channels_data.builder, apu_channels_glade, -1,
	        &error)) {
		g_warning("%s", error->message);
		g_free(error);
	}

	/* Get main window pointer from UI */
	apu_channels_data.window = GTK_WIDGET(
	        gtk_builder_get_object(apu_channels_data.builder,
	                "apu_channels_dialog"));

	/* Connect signals */
	gtk_builder_connect_signals(apu_channels_data.builder, NULL);

	/* ora collego i miei segnali */
	channel_connect_toggled("apu_channels_square1_checkbutton", APU_S1)
	channel_connect_toggled("apu_channels_square2_checkbutton", APU_S2)
	channel_connect_toggled("apu_channels_triangle_checkbutton", APU_TR)
	channel_connect_toggled("apu_channels_noise_checkbutton", APU_NS)
	channel_connect_toggled("apu_channels_dmc_checkbutton", APU_DMC)
	channel_connect_toggled("apu_channels_extra_checkbutton", APU_EXTRA)

	channel_connect_clicked("apu_channels_disable_all_button", FALSE)
	channel_connect_clicked("apu_channels_active_all_button", TRUE)
	channel_connect_clicked("apu_channels_defaults_button", 2)

	volume_connect_value_changed("apu_channels_square1_hscale", APU_S1)
	volume_connect_value_changed("apu_channels_square2_hscale", APU_S2)
	volume_connect_value_changed("apu_channels_triangle_hscale", APU_TR)
	volume_connect_value_changed("apu_channels_noise_hscale", APU_NS)
	volume_connect_value_changed("apu_channels_dmc_hscale", APU_DMC)
	volume_connect_value_changed("apu_channels_extra_hscale", APU_EXTRA)
	volume_connect_value_changed("apu_channels_master_hscale", APU_MASTER)

	_signal_connect("apu_channels_ok_button", "clicked",
	        G_CALLBACK(apu_channels_ok_clicked), 0);
	_signal_connect("apu_channels_cancel_button", "clicked",
	        G_CALLBACK(apu_channels_cancel_clicked), 0);
	g_signal_connect(G_OBJECT(apu_channels_data.window), "delete-event",
	        G_CALLBACK(apu_channels_delete_event), NULL);

	apu_channels_check();

	/* Show window. All other widgets are automatically shown by GtkBuilder */
	gtk_widget_show(apu_channels_data.window);
}
void apu_channels_check(void) {
	apu_channels_data.update = TRUE;

	channel_check_active("apu_channels_square1_checkbutton", APU_S1)
	channel_check_active("apu_channels_square2_checkbutton", APU_S2)
	channel_check_active("apu_channels_triangle_checkbutton", APU_TR)
	channel_check_active("apu_channels_noise_checkbutton", APU_NS)
	channel_check_active("apu_channels_dmc_checkbutton", APU_DMC)
	channel_check_active("apu_channels_extra_checkbutton", APU_EXTRA)

	volume_check("apu_channels_square1_hscale", APU_S1)
	volume_check("apu_channels_square2_hscale", APU_S2)
	volume_check("apu_channels_triangle_hscale", APU_TR)
	volume_check("apu_channels_noise_hscale", APU_NS)
	volume_check("apu_channels_dmc_hscale", APU_DMC)
	volume_check("apu_channels_extra_hscale", APU_EXTRA)
	volume_check("apu_channels_master_hscale", APU_MASTER)

	apu_channels_data.update = FALSE;
}
void apu_channels_toggle(gint channel) {
	if (apu_channels_data.update == TRUE) {
		return;
	}

	cfg->apu.channel[channel] = !cfg->apu.channel[channel];
}
void apu_channels_toggle_all(gint mode) {
	int index;

	if (mode == 2) {
		for (index = APU_S1; index <= APU_MASTER; index++) {
			cfg->apu.volume[index] = 1.0f;
		}
		mode = TRUE;
	}
	/*
	 * non devo forzare cfg->apu.channel[APU_MASTER] perche'
	 * lo utilizzo per abilitare o disabilitare il suono
	 * globalmente e viene impostato altrove.
	 */
	for (index = APU_S1; index <= APU_EXTRA; index++) {
		cfg->apu.channel[index] = mode;
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

/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "wdgSettingsAudio.moc"
#include "mainWindow.hpp"
#include "emu_thread.h"
#include "conf.h"
#include "audio/delay.h"
#include "audio/wave.h"

wdgSettingsAudio::wdgSettingsAudio(QWidget *parent) : QWidget(parent) {
	setupUi(this);

#if !defined (__OpenBSD__)
	connect(comboBox_Output_Devices, SIGNAL(activated(int)), this, SLOT(s_output_devices(int)));
#endif
	connect(comboBox_Buffer_Size_factor, SIGNAL(activated(int)), this, SLOT(s_audio_buffer_factor(int)));
	connect(comboBox_Sample_Rate, SIGNAL(activated(int)), this, SLOT(s_sample_rate(int)));
	connect(comboBox_Channels, SIGNAL(activated(int)), this, SLOT(s_channels(int)));
	connect(comboBox_Channels_Delay, SIGNAL(activated(int)), this, SLOT(s_channels_delay(int)));

	widget_APU_Channels->line_APU_Channels->hide();

	connect(checkBox_Swap_Duty_Cycles, SIGNAL(clicked(bool)), this, SLOT(s_swap_duty_cycles(bool)));
	connect(checkBox_Enable_Audio, SIGNAL(clicked(bool)), this, SLOT(s_enable_audio(bool)));
}
wdgSettingsAudio::~wdgSettingsAudio() {}

void wdgSettingsAudio::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}
void wdgSettingsAudio::showEvent(UNUSED(QShowEvent *event)) {
	int dim = label_Output_Devices->size().height() - 10;

	output_devices_init();

	icon_Output_Devices->setPixmap(QIcon(":/icon/icons/audio_output.svg").pixmap(dim, dim));
	icon_Buffer_Size_factor->setPixmap(QIcon(":/icon/icons/buffer_size.svg").pixmap(dim, dim));
	icon_Sample_Rate->setPixmap(QIcon(":/icon/icons/samplerate.svg").pixmap(dim, dim));
	icon_Channels->setPixmap(QIcon(":/icon/icons/channels.svg").pixmap(dim, dim));
	icon_Channels_Delay->setPixmap(QIcon(":/icon/icons/stereo_delay.svg").pixmap(dim, dim));
}

void wdgSettingsAudio::retranslateUi(QWidget *wdgSettingsAudio) {
	Ui::wdgSettingsAudio::retranslateUi(wdgSettingsAudio);
	mainwin->qaction_shcut.audio_enable->setText(checkBox_Enable_Audio->text());
	update_widget();
}
void wdgSettingsAudio::update_widget(void) {
#if defined (__OpenBSD__)
	icon_Output_Devices->setVisible(false);
	label_Output_Devices->setVisible(false);
	comboBox_Output_Devices->setVisible(false);
#endif
	audio_buffer_factor_set();
	sample_rate_set();

	{
		channels_set();

		if (cfg->channels_mode == CH_STEREO_DELAY) {
			icon_Channels_Delay->setEnabled(true);
			label_Channels_Delay->setEnabled(true);
			comboBox_Channels_Delay->setEnabled(true);
		} else {
			icon_Channels_Delay->setEnabled(false);
			label_Channels_Delay->setEnabled(false);
			comboBox_Channels_Delay->setEnabled(false);
		}
	}

	channels_delay_set();

	widget_APU_Channels->update_widget();

	checkBox_Swap_Duty_Cycles->setChecked(cfg->swap_duty);
	checkBox_Enable_Audio->setChecked(cfg->apu.channel[APU_MASTER]);
}

void wdgSettingsAudio::output_devices_init(void) {
	QString id = uQString(cfg->audio_output);
	int i;

	comboBox_Output_Devices->clear();

	snd_list_devices();

	for (i = 0; i < snd_list.playback.count; i++) {
		QString description = uQString(snd_playback_device_desc(i));
		QString id_new = uQString(snd_playback_device_id(i));

		if (i == 0) {
			description = QApplication::translate("Settings", "System Default", Q_NULLPTR);
		}

		if (description.isEmpty()) {
			break;
		}

		comboBox_Output_Devices->addItem(description, id_new);

		if (id == id_new) {
			comboBox_Output_Devices->setCurrentIndex(i);
		}
	}
}

void wdgSettingsAudio::audio_buffer_factor_set(void) {
	comboBox_Buffer_Size_factor->setCurrentIndex(cfg->audio_buffer_factor);
}
void wdgSettingsAudio::sample_rate_set(void) {
	int samplerate = 0;

	switch (cfg->samplerate) {
		case S48000:
			samplerate = 0;
			break;
		default:
		case S44100:
			samplerate = 1;
			break;
		case S22050:
			samplerate = 2;
			break;
		case S11025:
			samplerate = 3;
			break;
	}

	comboBox_Sample_Rate->setCurrentIndex(samplerate);
}
void wdgSettingsAudio::channels_set(void) {
	int channels = 0;

	switch (cfg->channels_mode) {
		case CH_MONO:
			channels = 0;
			break;
		default:
		case CH_STEREO_DELAY:
			channels = 1;
			break;
		case CH_STEREO_PANNING:
			channels = 2;
			break;
	}

	comboBox_Channels->setCurrentIndex(channels);
}
void wdgSettingsAudio::channels_delay_set(void) {
	int delay = ((cfg->stereo_delay * 100) / 5) - 1;

	comboBox_Channels_Delay->setCurrentIndex(delay);
}

void wdgSettingsAudio::s_output_devices(int index) {
	QString id_new = comboBox_Output_Devices->itemData(index).toString();
	QString id = uQString(cfg->audio_output);

	if (id != id_new) {
		ustrncpy(cfg->audio_output, (uTCHAR *) snd_list.playback.devices[index].id, usizeof(cfg->audio_output) - 1);
		emu_thread_pause();
		snd_playback_start();
		emu_thread_continue();
	}
}
void wdgSettingsAudio::s_audio_buffer_factor(int index) {
	int factor = index;

	if (cfg->audio_buffer_factor == factor) {
		return;
	}

	emu_thread_pause();
	cfg->audio_buffer_factor = factor;
	snd_playback_start();
	emu_thread_continue();
	gui_update();
}
void wdgSettingsAudio::s_sample_rate(int index) {
	int samplerate = index;

	switch (samplerate) {
		case 0:
			samplerate = S48000;
			break;
		case 1:
			samplerate = S44100;
			break;
		case 2:
			samplerate = S22050;
			break;
		case 3:
			samplerate = S11025;
			break;
	}

	if (cfg->samplerate == samplerate) {
		return;
	}

	emu_thread_pause();
	wave_close();
	cfg->samplerate = samplerate;
	snd_playback_start();
	emu_thread_continue();
	gui_update();
}
void wdgSettingsAudio::s_channels(int index) {
	int channels = index;

	switch (channels) {
		case 0:
			channels = CH_MONO;
			break;
		case 1:
			channels = CH_STEREO_DELAY;
			break;
		case 2:
			channels = CH_STEREO_PANNING;
			break;
	}

	if (cfg->channels_mode == channels) {
		return;
	}

	emu_thread_pause();
	wave_close();
	cfg->channels_mode = channels;
	snd_playback_start();
	emu_thread_continue();
	gui_update();
}
void wdgSettingsAudio::s_channels_delay(int index) {
	double delay = (double)((index + 1) * 5) / 100.0f;

	if (cfg->stereo_delay == delay) {
		return;
	}

	emu_thread_pause();
	cfg->stereo_delay = delay;
	ch_stereo_delay_set();
	emu_thread_continue();
	gui_update();
}
void wdgSettingsAudio::s_swap_duty_cycles(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->swap_duty = !cfg->swap_duty;
	emu_thread_continue();
	update_widget();
}
void wdgSettingsAudio::s_enable_audio(UNUSED(bool checked)) {
	emu_thread_pause();
	if ((cfg->apu.channel[APU_MASTER] = !cfg->apu.channel[APU_MASTER])) {
		snd_playback_start();
	} else {
		snd_playback_stop();
	}
	emu_thread_continue();
	update_widget();
}

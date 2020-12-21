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

	setFocusProxy(comboBox_Output_Devices);

	widget_Samplarate->setStyleSheet(button_stylesheet());
	widget_Channels->setStyleSheet(button_stylesheet());

#if defined (__OpenBSD__)
	icon_Output_Devices->setVisible(false);
	label_Output_Devices->setVisible(false);
	comboBox_Output_Devices->setVisible(false);
#else
	connect(comboBox_Output_Devices, SIGNAL(activated(int)), this, SLOT(s_output_devices(int)));
#endif

	pushButton_Samplarate_48000->setProperty("mtype", QVariant(S48000));
	pushButton_Samplarate_44100->setProperty("mtype", QVariant(S44100));
	pushButton_Samplarate_22050->setProperty("mtype", QVariant(S22050));
	pushButton_Samplarate_11025->setProperty("mtype", QVariant(S11025));

	connect(pushButton_Samplarate_48000, SIGNAL(toggled(bool)), this, SLOT(s_sample_rate(bool)));
	connect(pushButton_Samplarate_44100, SIGNAL(toggled(bool)), this, SLOT(s_sample_rate(bool)));
	connect(pushButton_Samplarate_22050, SIGNAL(toggled(bool)), this, SLOT(s_sample_rate(bool)));
	connect(pushButton_Samplarate_11025, SIGNAL(toggled(bool)), this, SLOT(s_sample_rate(bool)));

	connect(horizontalSlider_Buffer_Size_factor, SIGNAL(valueChanged(int)), this, SLOT(s_audio_buffer_factor(int)));

	pushButton_Channels_Mono->setProperty("mtype", QVariant(CH_MONO));
	pushButton_Channels_Stereo_Delay->setProperty("mtype", QVariant(CH_STEREO_DELAY));
	pushButton_Channels_Stereo_Panning->setProperty("mtype", QVariant(CH_STEREO_PANNING));

	connect(pushButton_Channels_Mono, SIGNAL(toggled(bool)), this, SLOT(s_channels(bool)));
	connect(pushButton_Channels_Stereo_Delay, SIGNAL(toggled(bool)), this, SLOT(s_channels(bool)));
	connect(pushButton_Channels_Stereo_Panning, SIGNAL(toggled(bool)), this, SLOT(s_channels(bool)));

	connect(horizontalSlider_Channels_Delay, SIGNAL(valueChanged(int)), this, SLOT(s_channels_delay(int)));

	widget_APU_Channels->line_APU_Channels->hide();

	connect(checkBox_Reverse_bits_DPCM, SIGNAL(clicked(bool)), this, SLOT(s_reverse_bits_dpcm(bool)));
	connect(checkBox_Swap_Duty_Cycles, SIGNAL(clicked(bool)), this, SLOT(s_swap_duty_cycles(bool)));
	connect(checkBox_Enable_Audio, SIGNAL(clicked(bool)), this, SLOT(s_enable_audio(bool)));

	{
		int w = QLabel("100%").sizeHint().width();

		label_Buffer_Size_factor_value->setFixedWidth(w);
		label_Channels_Delay_value->setFixedWidth(w);
	}
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
	int dim = fontMetrics().height();

	output_devices_init();

	icon_Audio_settings->setPixmap(QIcon(":/icon/icons/settings.svg").pixmap(dim, dim));
	icon_Output_Devices->setPixmap(QIcon(":/icon/icons/audio_output.svg").pixmap(dim, dim));
	icon_Buffer_Size_factor->setPixmap(QIcon(":/icon/icons/buffer_size.svg").pixmap(dim, dim));
	icon_Sample_Rate->setPixmap(QIcon(":/icon/icons/samplerate.svg").pixmap(dim, dim));
	icon_Channels->setPixmap(QIcon(":/icon/icons/channels.svg").pixmap(dim, dim));
	icon_Channels_Delay->setPixmap(QIcon(":/icon/icons/stereo_delay.svg").pixmap(dim, dim));
	icon_APU_Channels->setPixmap(QIcon(":/icon/icons/volume.svg").pixmap(dim, dim));
	icon_Audio_misc->setPixmap(QIcon(":/icon/icons/misc.svg").pixmap(dim, dim));
}

void wdgSettingsAudio::retranslateUi(QWidget *wdgSettingsAudio) {
	Ui::wdgSettingsAudio::retranslateUi(wdgSettingsAudio);
	mainwin->qaction_shcut.audio_enable->setText(checkBox_Enable_Audio->text());
	update_widget();
}
void wdgSettingsAudio::update_widget(void) {
	audio_buffer_factor_set();
	sample_rate_set();
	channels_set();
	channels_delay_set();

	widget_APU_Channels->update_widget();

	checkBox_Reverse_bits_DPCM->setChecked(cfg->reverse_bits_dpcm);
	checkBox_Swap_Duty_Cycles->setChecked(cfg->swap_duty);
	checkBox_Enable_Audio->setChecked(cfg->apu.channel[APU_MASTER]);

	settings_set_enabled(!info.recording_on_air);
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
void wdgSettingsAudio::sample_rate_set(void) {
	qtHelper::pushbutton_set_checked(pushButton_Samplarate_48000, false);
	qtHelper::pushbutton_set_checked(pushButton_Samplarate_44100, false);
	qtHelper::pushbutton_set_checked(pushButton_Samplarate_22050, false);
	qtHelper::pushbutton_set_checked(pushButton_Samplarate_11025, false);
	switch (cfg->samplerate) {
		case S48000:
			qtHelper::pushbutton_set_checked(pushButton_Samplarate_48000, true);
			break;
		default:
		case S44100:
			qtHelper::pushbutton_set_checked(pushButton_Samplarate_44100, true);
			break;
		case S22050:
			qtHelper::pushbutton_set_checked(pushButton_Samplarate_22050, true);
			break;
		case S11025:
			qtHelper::pushbutton_set_checked(pushButton_Samplarate_11025, true);
			break;
	}
}
void wdgSettingsAudio::audio_buffer_factor_set(void) {
	horizontalSlider_Buffer_Size_factor->setValue(cfg->audio_buffer_factor);
}
void wdgSettingsAudio::channels_set(void) {
	qtHelper::pushbutton_set_checked(pushButton_Channels_Mono, false);
	qtHelper::pushbutton_set_checked(pushButton_Channels_Stereo_Delay, false);
	qtHelper::pushbutton_set_checked(pushButton_Channels_Stereo_Panning, false);
	switch (cfg->channels_mode) {
		case CH_MONO:
			qtHelper::pushbutton_set_checked(pushButton_Channels_Mono, true);
			break;
		default:
		case CH_STEREO_DELAY:
			qtHelper::pushbutton_set_checked(pushButton_Channels_Stereo_Delay, true);
			break;
		case CH_STEREO_PANNING:
			qtHelper::pushbutton_set_checked(pushButton_Channels_Stereo_Panning, true);
			break;
	}
}
void wdgSettingsAudio::channels_delay_set(void) {
	horizontalSlider_Channels_Delay->setValue(((cfg->stereo_delay * 100) / 5) - 1);
}
void wdgSettingsAudio::settings_set_enabled(bool mode) {
	icon_Output_Devices->setEnabled(mode);
	label_Output_Devices->setEnabled(mode);
	comboBox_Output_Devices->setEnabled(mode);

	icon_Buffer_Size_factor->setEnabled(mode);
	label_Buffer_Size_factor->setEnabled(mode);
	horizontalSlider_Buffer_Size_factor->setEnabled(mode);
	label_Buffer_Size_factor_value->setEnabled(mode);

	icon_Sample_Rate->setEnabled(mode);
	label_Sample_Rate->setEnabled(mode);
	widget_Samplarate->setEnabled(mode);

	icon_Channels->setEnabled(mode);
	label_Channels->setEnabled(mode);
	widget_Channels->setEnabled(mode);

	if (cfg->channels_mode != CH_STEREO_DELAY) {
		mode = false;
	}
	icon_Channels_Delay->setEnabled(mode);
	label_Channels_Delay->setEnabled(mode);
	horizontalSlider_Channels_Delay->setEnabled(mode);
	label_Channels_Delay_value->setEnabled(mode);
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
void wdgSettingsAudio::s_sample_rate(bool checked) {
	if (checked) {
		int samplerate = QVariant(((QPushButton *)sender())->property("mtype")).toInt();

		if (cfg->samplerate == samplerate) {
			return;
		}

		emu_thread_pause();
		wave_close();
		cfg->samplerate = samplerate;
		snd_playback_start();
		emu_thread_continue();
	}
	sample_rate_set();
}
void wdgSettingsAudio::s_audio_buffer_factor(int value) {
	QFont f = label_Buffer_Size_factor_value->font();
	int factor = value;

	if (factor == 1) {
		f.setUnderline(true);
		label_Buffer_Size_factor_value->setFont(f);
	} else {
		f.setUnderline(false);
		label_Buffer_Size_factor_value->setFont(f);
	}
	label_Buffer_Size_factor_value->setText(QString("%1").arg(factor, 1));

	if (cfg->audio_buffer_factor == factor) {
		return;
	}

	emu_thread_pause();
	cfg->audio_buffer_factor = factor;
	snd_playback_start();
	emu_thread_continue();
}
void wdgSettingsAudio::s_channels(bool checked) {
	if (checked) {
		int channels = QVariant(((QPushButton *)sender())->property("mtype")).toInt();

		if (cfg->channels_mode == channels) {
			return;
		}

		emu_thread_pause();
		wave_close();
		cfg->channels_mode = channels;
		snd_playback_start();
		emu_thread_continue();
	}
	update_widget();
}
void wdgSettingsAudio::s_channels_delay(int value) {
	QFont f = label_Channels_Delay_value->font();
	int base = ((value + 1) * 5);
	double delay = (double)base / 100.0f;

	if (base == 30) {
		f.setUnderline(true);
		label_Channels_Delay_value->setFont(f);
	} else {
		f.setUnderline(false);
		label_Channels_Delay_value->setFont(f);
	}
	label_Channels_Delay_value->setText(QString("%1%").arg(base));

	if (cfg->stereo_delay == delay) {
		return;
	}

	emu_thread_pause();
	cfg->stereo_delay = delay;
	ch_stereo_delay_set();
	emu_thread_continue();
}
void wdgSettingsAudio::s_reverse_bits_dpcm(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->reverse_bits_dpcm = !cfg->reverse_bits_dpcm;
	emu_thread_continue();
	update_widget();
}
void wdgSettingsAudio::s_swap_duty_cycles(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->swap_duty = !cfg->swap_duty;
	emu_thread_continue();
	update_widget();
}
void wdgSettingsAudio::s_enable_audio(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->apu.channel[APU_MASTER] = !cfg->apu.channel[APU_MASTER];
	emu_thread_continue();
	update_widget();
}

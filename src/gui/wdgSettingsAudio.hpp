/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#ifndef WDGSETTINGSAUDIO_HPP_
#define WDGSETTINGSAUDIO_HPP_

#include <QtWidgets/QWidget>
#include "wdgSettingsAudio.hh"

class wdgSettingsAudio : public QWidget, public Ui::wdgSettingsAudio {
		Q_OBJECT

	public:
		wdgSettingsAudio(QWidget *parent = 0);
		~wdgSettingsAudio();

	private:
		void changeEvent(QEvent *event);
		void showEvent(QShowEvent *event);

	public:
		void retranslateUi(QWidget *wdgSettingsInput);
		void update_widget(void);

	private:
		void output_devices_init(void);

	private:
		void audio_buffer_factor_set(void);
		void sample_rate_set(void);
		void channels_set(void);
		void channels_delay_set(void);

	private slots:
		void s_output_devices(int index);
		void s_audio_buffer_factor(int index);
		void s_sample_rate(int index);
		void s_channels(int index);
		void s_channels_delay(int index);
		void s_swap_duty_cycles(bool checked);
		void s_enable_audio(bool checked);
};

#endif /* WDGSETTINGSAUDIO_HPP_ */

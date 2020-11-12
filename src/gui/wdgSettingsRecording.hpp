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

#ifndef WDGSETTINGSRECORDING_HPP_
#define WDGSETTINGSRECORDING_HPP_

#include <QtWidgets/QWidget>
#include <QtWidgets/QFileDialog>
#include "wdgSettingsRecording.hh"

class wdgSettingsRecording : public QWidget, public Ui::wdgSettingsRecording {
		Q_OBJECT

	public:
		wdgSettingsRecording(QWidget *parent = 0);
		~wdgSettingsRecording();

#if defined (WITH_FFMPEG)
	private:
		void changeEvent(QEvent *event);
		void showEvent(QShowEvent *event);

	public:
		void retranslateUi(QWidget *wdgSettingsRecording);
		void update_widget(void);

	private:
		void combobox_format_init(QComboBox *cb, int start, int end);
		void output_format_init(void);
		void output_resolution_init(void);
		int output_custom_control(int actual, int min, int max, int def);

	private slots:
		void s_output_audio_format(int index);
		void s_output_video_format(int index);
		void s_output_quality(int index);
		void s_output_resolution(int index);
		void s_output_custom_width(void);
		void s_output_custom_height(void);
		void s_use_emu_resolution(int state);
		void s_follow_rotation(int state);
};

// ----------------------------------------------------------------------------------------------

class wdgRecGetSaveFileName: public QFileDialog {
		Q_OBJECT

	private:
		QLabel *label_Output_Quality;
		QComboBox *comboBox_Output_Quality;
		struct _rec_cfg {
			int audio_format;
			int video_format;
			int quality;
		} rec_cfg;

	public:
		wdgRecGetSaveFileName(QWidget *parent = 0);
		~wdgRecGetSaveFileName();

	public:
		QString audio_get_save_file_name(void);
		QString video_get_save_file_name(void);

	private:
		QComboBox *init_file_types(int start, int end, int current);
		QString control_filename(int current);

	private slots:
		void s_output_audio_format(int index);
		void s_output_video_format(int index);
		void s_output_quality(int index);
#endif
};

#endif /* WDGSETTINGSRECORDING_HPP_ */

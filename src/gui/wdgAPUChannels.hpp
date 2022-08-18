/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

#ifndef WDGAPUCHANNELS_HPP_
#define WDGAPUCHANNELS_HPP_

#include <QtWidgets/QWidget>
#include "wdgAPUChannels.hh"

class wdgAPUChannels : public QWidget, public Ui::wdgAPUChannels {
	Q_OBJECT

	public:
		wdgAPUChannels(QWidget *parent = 0);
		~wdgAPUChannels();

	private:
		void changeEvent(QEvent *event);

	public:
		void update_widget(void);

	private:
		void volume_update_label(int type, int value);

	private slots:
		void s_apu_ch_checkbox(bool checked);
		void s_apu_ch_slider(int value);
		void s_apu_ch_toggle_all(bool checked);
};

#endif /* WDGAPUCHANNELS_HPP_ */

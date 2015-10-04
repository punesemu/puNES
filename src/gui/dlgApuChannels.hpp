/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

#ifndef DLGAPUCHANNELS_HPP_
#define DLGAPUCHANNELS_HPP_

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif
#include "dlgApuChannels.hh"
#include "conf.h"

class dlgApuChannels : public QDialog, public Ui::APU_channels {
		Q_OBJECT

	private:
		struct _data {
			bool update;
			bool save;
			_config_apu cfg_save;
		} data;

	public:
		dlgApuChannels(QWidget *parent);
		~dlgApuChannels();

	private:
		bool eventFilter(QObject *obj, QEvent *event);
		void update_dialog();

	private slots:
		void s_checkbox_state_changed(int state);
		void s_slider_value_changed(int value);
		void s_toggle_all_clicked(bool checked);
		void s_apply_clicked(bool checked);
		void s_discard_clicked(bool checked);
};

#endif /* DLGAPUCHANNELS_HPP_ */

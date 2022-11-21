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

#ifndef WDGNTSCFILTER_HPP_
#define WDGNTSCFILTER_HPP_

#include <QtWidgets/QWidget>
#include "ui_wdgNTSCFilter.h"

class wdgNTSCFilter : public QWidget, public Ui::wdgNTSCFilter {
	Q_OBJECT

	public:
		explicit wdgNTSCFilter(QWidget *parent = nullptr);
		~wdgNTSCFilter() override;

	private:
		void changeEvent(QEvent *event) override;

	public:
		void update_widget(void);

	private:
		void ntsc_update_paramaters(void);
		void set_sliders_spins(void);

	private slots:
		void s_slider_spin_changed(int value);
		void s_checkbox_changed(int state);
		void s_default_value_clicked(bool checked);
		void s_default_value_mv_clicked(bool checked);
		void s_reset(bool checked);
};

#endif /* WDGNTSCFILTER_HPP_ */

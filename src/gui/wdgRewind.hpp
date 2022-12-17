/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

#ifndef WDGREWIND_HPP_
#define WDGREWIND_HPP_

#include <QtWidgets/QWidget>
#include <QtCore/QTimer>
#include "ui_wdgRewind.h"
#include "common.h"

class wdgRewind : public QWidget, public Ui::wdgRewind {
	Q_OBJECT

	private:
		double step_autorepeat_timer;

	public:
		explicit wdgRewind(QWidget *parent = nullptr);
		~wdgRewind() override;

	protected:
		void changeEvent(QEvent *event) override;
		void paintEvent(QPaintEvent *event) override;

	public:
		bool step_autorepeat_timer_control(void) const;
		bool egds_rewind(void);

	private:
		void set_enable_backward(BYTE mode);
		void set_enable_forward(BYTE mode);
		void set_enable_play_pause_forward(BYTE mode);
		void first_backward(void);
		void change_factor(int *factor);

	private slots:
		void s_fast_backward(bool checked);
		void s_step_backward(bool checked);
		void s_play(bool checked);
		void s_pause(bool checked);
		void s_step_forward(bool checked);
		void s_fast_forward(bool checked);
		void s_step_released(void);
};

#endif /* WDGREWIND_HPP_ */

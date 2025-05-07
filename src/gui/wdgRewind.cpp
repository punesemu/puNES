/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#include <QtWidgets/QStyleOption>
#include <QtGui/QPainter>
#include "wdgRewind.hpp"
#include "wdgToolBar.hpp"
#include "rewind.h"
#include "gui.h"

enum wdgrewind_misc {
	WDGRWND_AUTOREPEAT_TIMER_STEP = 60,
	WDGRWND_AUTOREPEAT_MOUSE_TIMER_STEP = WDGRWND_AUTOREPEAT_TIMER_STEP + 20,
};

wdgRewind::wdgRewind(QWidget *parent) : QWidget(parent) {
	step_autorepeat_timer = gui_get_ms();

	setupUi(this);

	stylesheet_update();

	widget_buttons->setLayoutDirection(Qt::LeftToRight);

	gridLayout->setHorizontalSpacing(SPACING);
	horizontalLayout_buttons->setSpacing(SPACING);

	connect(toolButton_Fast_Backward, SIGNAL(clicked(bool)), this, SLOT(s_fast_backward(bool)));
	connect(toolButton_Step_Backward, SIGNAL(clicked(bool)), this, SLOT(s_step_backward(bool)));
	connect(toolButton_Play, SIGNAL(clicked(bool)), this, SLOT(s_play(bool)));
	connect(toolButton_Pause, SIGNAL(clicked(bool)), this, SLOT(s_pause(bool)));
	connect(toolButton_Step_Forward, SIGNAL(clicked(bool)), this, SLOT(s_step_forward(bool)));
	connect(toolButton_Fast_Forward, SIGNAL(clicked(bool)), this, SLOT(s_fast_forward(bool)));

	connect(toolButton_Step_Backward, SIGNAL(released()), this, SLOT(s_step_released()));
	connect(toolButton_Step_Forward, SIGNAL(released()), this, SLOT(s_step_released()));

	toolButton_Step_Backward->setAutoRepeatDelay(WDGRWND_AUTOREPEAT_MOUSE_TIMER_STEP);
	toolButton_Step_Backward->setAutoRepeatInterval(WDGRWND_AUTOREPEAT_MOUSE_TIMER_STEP);
	toolButton_Step_Forward->setAutoRepeatDelay(WDGRWND_AUTOREPEAT_MOUSE_TIMER_STEP);
	toolButton_Step_Forward->setAutoRepeatInterval(WDGRWND_AUTOREPEAT_MOUSE_TIMER_STEP);

	set_enable_play_pause_forward(FALSE);
}
wdgRewind::~wdgRewind() = default;

void wdgRewind::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		Ui::wdgRewind::retranslateUi(this);
	} else if (event->type() == QEvent::PaletteChange) {
		stylesheet_update();
	} else {
		QWidget::changeEvent(event);
	}
}
void wdgRewind::paintEvent(UNUSED(QPaintEvent *event)) {
	QStyleOption opt;
	QPainter p(this);

	opt.initFrom(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void wdgRewind::stylesheet_update(void) {
	if (theme::is_dark_theme()) {
		toolButton_Fast_Backward->setIcon(QIcon(":/icon/icons/rwnd_fast_backward_dtheme.svgz"));
		toolButton_Step_Backward->setIcon(QIcon(":/icon/icons/rwnd_step_backward_dtheme.svgz"));
		toolButton_Play->setIcon(QIcon(":/icon/icons/rwnd_play_dtheme.svgz"));
		toolButton_Pause->setIcon(QIcon(":/icon/icons/rwnd_pause_dtheme.svgz"));
		toolButton_Step_Forward->setIcon(QIcon(":/icon/icons/rwnd_step_forward_dtheme.svgz"));
		toolButton_Fast_Forward->setIcon(QIcon(":/icon/icons/rwnd_fast_forward_dtheme.svgz"));
	} else {
		toolButton_Fast_Backward->setIcon(QIcon(":/icon/icons/rwnd_fast_backward.svgz"));
		toolButton_Step_Backward->setIcon(QIcon(":/icon/icons/rwnd_step_backward.svgz"));
		toolButton_Play->setIcon(QIcon(":/icon/icons/rwnd_play.svgz"));
		toolButton_Pause->setIcon(QIcon(":/icon/icons/rwnd_pause.svgz"));
		toolButton_Step_Forward->setIcon(QIcon(":/icon/icons/rwnd_step_forward.svgz"));
		toolButton_Fast_Forward->setIcon(QIcon(":/icon/icons/rwnd_fast_forward.svgz"));
	}
}

bool wdgRewind::step_autorepeat_timer_control(void) const {
	if ((gui_get_ms() - step_autorepeat_timer) < WDGRWND_AUTOREPEAT_TIMER_STEP) {
		return (false);
	}
	return (true);
}
bool wdgRewind::egds_rewind(void) {
	if (rwnd.active) {
		int factor = rwnd.direction == RWND_BACKWARD ? rwnd.factor.backward : rwnd.factor.forward;

		if (rwnd.action != RWND_ACT_PAUSE) {
			rewind_frames(rewind_calculate_snap_cursor(factor, rwnd.direction));
			set_enable_backward(!rewind_is_first_snap());
			if (rewind_is_first_snap()) {
				rwnd.action = RWND_ACT_PAUSE;
				toolButton_Pause->click();
			}
			set_enable_forward(!rewind_is_last_snap());
			if (rewind_is_last_snap()) {
				rwnd.action = RWND_ACT_PAUSE;
				toolButton_Pause->click();
			}
		}
		return (true);
	}
	return (false);
}

void wdgRewind::set_enable_backward(BYTE mode) {
	toolButton_Step_Backward->setEnabled(mode);
	toolButton_Fast_Backward->setEnabled(mode);
}
void wdgRewind::set_enable_forward(BYTE mode) {
	toolButton_Step_Forward->setEnabled(mode);
	toolButton_Fast_Forward->setEnabled(mode);
}
void wdgRewind::set_enable_play_pause_forward(BYTE mode) {
	toolButton_Play->setEnabled(mode);
	toolButton_Pause->setEnabled(TRUE);
	set_enable_forward(mode);
}
void wdgRewind::first_backward(void) {
	if (!rwnd.active) {
		set_enable_play_pause_forward(TRUE);
		rewind_init_operation();
		rwnd.factor.backward = 0;
		rwnd.factor.forward = 0;
	} else {
		gui_egds_stop_rwnd();
	}
}
void wdgRewind::change_factor(int *factor) {
	switch ((*factor)) {
		case 0:
		case 64:
			(*factor) = 1;
			break;
		case 1:
			(*factor) = 2;
			break;
		case 2:
			(*factor) = 4;
			break;
		case 4:
			(*factor) = 8;
			break;
		case 8:
			(*factor) = 16;
			break;
		case 16:
			(*factor) = 32;
			break;
		case 32:
			(*factor) = 64;
			break;
	}
}

void wdgRewind::s_fast_backward(UNUSED(bool checked)) {
	first_backward();
	rwnd.action = RWND_ACT_FAST_BACKWARD;
	toolButton_Step_Backward->setChecked(false);
	toolButton_Step_Forward->setChecked(false);
	toolButton_Fast_Backward->setChecked(true);
	toolButton_Fast_Forward->setChecked(false);
	toolButton_Pause->setChecked(false);
	change_factor(&rwnd.factor.backward);
	rwnd.direction = RWND_BACKWARD;
	rwnd.factor.forward = 0;
	gui_egds_start_rwnd();
}
void wdgRewind::s_step_backward(UNUSED(bool checked)) {
	if (!step_autorepeat_timer_control()) {
		return;
	}
	first_backward();
	rwnd.action = RWND_ACT_STEP_BACKWARD;
	rewind_frames(-1);
	rwnd.factor.backward = 0;
	rwnd.factor.forward = 0;
	toolButton_Pause->click();
	set_enable_backward(!rewind_is_first_snap());
	if (rewind_is_first_snap()) {
		toolButton_Step_Backward->setChecked(false);
		rwnd.action_before_pause = RWND_ACT_PAUSE;
	} else {
		toolButton_Step_Backward->setChecked(true);
		toolButton_Pause->setChecked(false);
	}
	step_autorepeat_timer = gui_get_ms();
}
void wdgRewind::s_play(UNUSED(bool checked)) {
	rwnd.action = RWND_ACT_PLAY;
	if (!rwnd.active) {
		return;
	}
	gui_egds_stop_rwnd();
	set_enable_backward(true);
	set_enable_forward(false);
	toolButton_Step_Backward->setChecked(false);
	toolButton_Step_Forward->setChecked(false);
	toolButton_Fast_Backward->setChecked(false);
	toolButton_Fast_Forward->setChecked(false);
	toolButton_Pause->setChecked(false);
	set_enable_play_pause_forward(FALSE);
	rewind_close_operation();
	rwnd.factor.backward = 0;
	rwnd.factor.forward = 0;
}
void wdgRewind::s_pause(UNUSED(bool checked)) {
	first_backward();
	rwnd.action_before_pause = rwnd.action;
	rwnd.action = RWND_ACT_PAUSE;
	toolButton_Step_Backward->setChecked(false);
	toolButton_Step_Forward->setChecked(false);
	toolButton_Fast_Backward->setChecked(false);
	toolButton_Fast_Forward->setChecked(false);
	set_enable_backward(!rewind_is_first_snap());
	set_enable_forward(!rewind_is_last_snap());
	toolButton_Pause->setChecked(true);
	gui_egds_start_rwnd();
}
void wdgRewind::s_step_forward(UNUSED(bool checked)) {
	if (!rwnd.active || !step_autorepeat_timer_control()) {
		// il forward funziona solo dopo che si
		// e' fatto un po' di backward.
		return;
	}
	gui_egds_stop_rwnd();
	rwnd.action = RWND_ACT_STEP_FORWARD;
	rewind_frames(1);
	rwnd.factor.backward = 0;
	rwnd.factor.forward = 0;
	toolButton_Pause->click();
	set_enable_forward(!rewind_is_last_snap());
	if (rewind_is_last_snap()) {
		toolButton_Step_Forward->setChecked(false);
		rwnd.action_before_pause = RWND_ACT_PAUSE;
	} else {
		toolButton_Step_Forward->setChecked(true);
		toolButton_Pause->setChecked(false);
	}
	step_autorepeat_timer = gui_get_ms();
}
void wdgRewind::s_fast_forward(UNUSED(bool checked)) {
	if (!rwnd.active) {
		// il forward funziona solo dopo che si
		// e' fatto un po' di backward.
		return;
	}
	gui_egds_stop_rwnd();
	rwnd.action = RWND_ACT_FAST_FORWARD;
	toolButton_Step_Backward->setChecked(false);
	toolButton_Step_Forward->setChecked(false);
	toolButton_Fast_Backward->setChecked(false);
	toolButton_Fast_Forward->setChecked(true);
	toolButton_Pause->setChecked(false);
	change_factor(&rwnd.factor.forward);
	rwnd.direction = RWND_FORWARD;
	rwnd.factor.backward = 0;
	gui_egds_start_rwnd();
}
void wdgRewind::s_step_released(void) {
	((QToolButton *)sender())->setChecked(true);
}

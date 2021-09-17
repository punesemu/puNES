/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#include <QtCore/QDateTime>
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtSvg/QSvgRenderer>
#include <QtWidgets/QGraphicsItem>
#include <time.h>
#include "wdgOverlayUi.moc"
#include "mainWindow.hpp"
#include "fps.h"
#include "tas.h"
#include "input.h"
#include "input/mouse.h"
#include "fds.h"
#include "rewind.h"
#include "version.h"
#include "cpu.h"
#include "ppu.h"
#if defined (FULLSCREEN_RESFREQ)
#include "video/gfx_monitor.h"
#endif

void overlay_wdg_clear(void *widget, void *qrect);
void overlay_wdg_blit(void *widget);

static const char *info_messages_precompiled[] = {
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/*  0 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "start wav recording"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/*  1 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "stop wav recording"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/*  2 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[red]Game Genie rom not found[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/*  3 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[red]error loading Game Genie rom[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/*  4 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[red]Illegal Opcode[normal] [yellow]0x%1[normal] at [yellow]0x%2[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/*  5 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[red]error loading rom[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/*  6 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[red]FDS bios not found[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/*  7 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "disk [cyan]%1[normal] side [cyan]%2[normal] [yellow]ejected[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/*  8 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "you must [yellow]eject[normal] disk first"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/*  9 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "disk [cyan]%1[normal] side [cyan]%2[normal] [green]inserted[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 10 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "disk [cyan]%1[normal] side [cyan]%2[normal] [brown]selected[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 11 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[yellow]mapper[normal] [brown]%1[normal] [yellow]not supported[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 12 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[red]error loading patch file[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 13 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[yellow]save is impossible in Game Genie menu[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 14 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[yellow]movie playback interrupted[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 15 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[red]error[normal] loading state"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 16 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[red]state file is not for this rom[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 17 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[green]%1[normal] cheat active"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 18 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[green]%1[normal] cheats active"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 19 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "overclock enabled [green]VB[normal] [cyan]%1[normal], [green]PR[normal] [cyan]%2[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 20 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[yellow]silence, the movie has begun[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 21 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "The End"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 22 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "switched to [green]%1[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 23 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[red]error on game genie rom file[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 24 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[red]error on FDS bios file[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 25 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[red]error on shader file[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 26 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[red]error on palette file[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 27 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[red]errors[normal] on shader, use [green]'No shader'[cyan]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 28 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "switch on [cyan]%1x%2[normal] at [green]%3Hz[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 29 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[cyan]%1[normal] ID sides founds, auto switch [red]disabled[normal]"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 30 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "[red]error[normal] loading state [cyan]%1[normal], file is corrupted"),
};

static struct _shared_color {
	QColor rwnd_actual = QColor(Qt::blue);
	QColor lag = Qt::red;
	QColor no_lag = QColor(30, 128, 0);
} shared_color;
_overlay_data overlay;

void *gui_wdgoverlayui_get_ptr(void) {
	return ((void *)overlay.widget);
}

void gui_overlay_update(void) {
	if (overlay.widget) {
		overlay.widget->update_widget();
	}
}
BYTE gui_overlay_is_updated(void) {
	return (overlay.widget->update_texture);
}
void gui_overlay_enable_save_slot(BYTE mode) {
	overlay.widget->overlaySaveSlot->enable_overlay(mode);
}
void gui_overlay_set_size(int w, int h) {
	overlay.widget->setFixedSize(w, h);
}
void gui_overlay_info_init(void) {
	overlay.info_actual_message = "";
	overlay.info_messages_to_draw.clear();
}
void gui_overlay_info_emulator(void) {
	QString str = "";

	if (info.portable) {
		str += "[cyan]Portable[normal] ";
	}
	str += "[yellow]p[normal]";
	str += "[red]u[normal]";
	str += "[green]N[normal]";
	str += "[cyan]E[normal]";
	str += "[brown]S[normal]";
	str += " (by [cyan]FHorse[normal])";
	str += " " + QString(VERSION);

	overlayWidgetInfo::_append_msg(str);
}
void gui_overlay_info_append_msg_precompiled(int index, void *arg1) {
	QString msg, a1, a2, a3;

	if (index >= (int)LENGTH(info_messages_precompiled)) {
		return;
	}

	msg = overlayWidgetInfo::tr(info_messages_precompiled[index]);

	switch (index) {
		case 4:
			a1 = QString("%1").arg(cpu.opcode, 2, 16, QLatin1Char('0')).toUpper();
			a2 = QString("%1").arg((cpu.PC - 1), 4, 16, QLatin1Char('0')).toUpper();
			msg = msg.arg(a1, a2);
			break;
		case 7:
		case 9:
		case 10:
			a1 = QString("%1").arg((fds.drive.side_inserted / 2) + 1);
			a2 = QString("%1").arg((char)((fds.drive.side_inserted & 0x01) + 'A'));
			msg = msg.arg(a1, a2);
			break;
		case 11:
			a1 = QString("%1").arg(info.mapper.id);
			msg = msg.arg(a1);
			break;
		case 17:
		case 18:
		case 29:
		case 30:
			a1 = QString("%1").arg(*(int *)arg1);
			msg = msg.arg(a1);
			break;
		case 19:
			a1 = QString("%1").arg(cfg->extra_vb_scanlines);
			a2 = QString("%1").arg(cfg->extra_pr_scanlines);
			msg = msg.arg(a1, a2);
			break;
		case 22:
			a1 = (*(QString *)arg1);
			msg = msg.arg(a1);
			break;
		case 28: {
				int w = 0, h = 0, rrate = 0;

#if defined (FULLSCREEN_RESFREQ)
				gfx_monitor_mode_in_use_info(NULL, NULL, &w, &h, &rrate);
#endif
				a1 = QString("%1").arg(w);
				a2 = QString("%1").arg(h);
				a3 = QString("%1").arg(rrate);
			}
			msg = msg.arg(a1, a2, a3);
			break;
	}

	if (overlay.widget) {
		overlay.widget->overlayInfo->append_msg(msg);
	} else {
		overlayWidgetInfo::_append_msg(msg);
	}
}
void gui_overlay_blit(void) {
	overlayWidget *wdgs[] = {
		overlay.widget->overlayInfo,
		overlay.widget->overlayRewind,
		overlay.widget->overlayTAS,
		overlay.widget->overlayFloppy,
		overlay.widget->overlayFPS,
		overlay.widget->overlayFrame,
		overlay.widget->overlaySaveSlot,
		overlay.widget->overlayInputPort1,
		overlay.widget->overlayInputPort2,
		overlay.widget->overlayInputPort3,
		overlay.widget->overlayInputPort4
	};

	overlay.widget->update_texture = FALSE;

	// e' importante rispettare l'ordine sottostante.
	// 1 - prima cancellazione
	for (overlayWidget *ele : wdgs) {
		overlay_wdg_clear((void *)ele, NULL);
	}
	// 2 - seconda cancellazione con preparazione immagini
	for (overlayWidget *ele : wdgs) {
		overlay_wdg_blit((void *)ele);
	}
	// 3 - blit delle immagini
	for (overlayWidget *ele : wdgs) {
		if (ele->exchange.draw) {
			_gfx_rect rect;

			ele->exchange.draw = false;

			rect.x = ele->exchange.last_geometry.x();
			rect.y = ele->exchange.last_geometry.y();
			rect.w = ele->exchange.last_geometry.width();
			rect.h = ele->exchange.last_geometry.height();

			if ((rect.w > 0) && (rect.h > 0)) {
				gfx_overlay_blit((void *)ele->exchange.img.bits(), &rect);
			}
		}
	}

	overlay.widget->force_redraw = false;
}
void gui_overlay_slot_preview(int slot, void *buffer, uTCHAR *file) {
	if (buffer) {
		overlay.widget->overlaySaveSlot->previews[slot].image =
			QImage((uchar*)buffer, SCR_COLUMNS, SCR_ROWS, SCR_COLUMNS * sizeof(uint32_t), QImage::Format_RGB32);
		overlay.widget->overlaySaveSlot->previews[slot].image =
			overlay.widget->overlaySaveSlot->previews[slot].image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
	} else {
		overlay.widget->overlaySaveSlot->previews[slot].image = QImage();
	}
	if (file) {
		overlay.widget->overlaySaveSlot->previews[slot].fileinfo = QFileInfo(uQString(file));
	} else {
		overlay.widget->overlaySaveSlot->previews[slot].fileinfo = QFileInfo();
	}
}

void overlay_wdg_clear(void *widget, void *qrect) {
	overlayWidget *wdg = (overlayWidget *)widget;
	QRect *g = (QRect *)&wdg->geometry();
	bool erase = false;

	if (wdg->isHidden() & wdg->enabled) {
		wdg->enabled = false;
		erase = true;
	} else if (qrect) {
		g = (QRect *)qrect;
		erase = true;
	}

	if (erase) {
		_gfx_rect rect;

		overlay.widget->update_texture = TRUE;

		rect.x = g->x();
		rect.y = g->y();
		rect.w = g->width();
		rect.h = g->height();

		gfx_overlay_blit((void *)overlay.widget->clear, &rect);
	}
}
void overlay_wdg_blit(void *widget) {
	overlayWidget *wdg = (overlayWidget *)widget;

	if ((wdg->isHidden() == false) && (wdg->enabled == TRUE)) {
		bool redraw = false;

		overlay.widget->update_texture = TRUE;

		if (overlay.widget->force_redraw == true) {
			redraw = true;
		} else if (wdg->is_to_redraw() == TRUE) {
			redraw = true;
		}

		if (redraw) {
			QRect second_last = wdg->exchange.last_geometry;

			wdg->enabled = true;
			wdg->exchange.draw = true;
			wdg->exchange.img = wdg->grab().toImage();

			if ((second_last != wdg->exchange.last_geometry) && overlay.widget->geometry().contains(second_last)) {
				overlay_wdg_clear(wdg, &second_last);
			}
		}
	} else if (wdg->force_control_when_hidden && (wdg->is_to_redraw() == TRUE)) {
		wdg->grab().toImage();
	}
}

// wdgOverlayUi ------------------------------------------------------------------------------------------------------------------

wdgOverlayUi::wdgOverlayUi(QWidget *parent) : QWidget(parent) {
	QGraphicsOpacityEffect *op = new QGraphicsOpacityEffect(this);
	QFont f = font();

	f.setPointSize(10);
	setFont(f);

	clear = NULL;
	force_redraw = false;
	update_texture = FALSE;

	setupUi(this);

	setAttribute(Qt::WA_OpaquePaintEvent);

	op->setOpacity(0.999999999);
	setGraphicsEffect(op);
	setAutoFillBackground(false);

	overlayInputPort1->set_nport(0);
	overlayInputPort2->set_nport(1);
	overlayInputPort3->set_nport(2);
	overlayInputPort4->set_nport(3);
}
wdgOverlayUi::~wdgOverlayUi() {}

void wdgOverlayUi::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}
void wdgOverlayUi::resizeEvent(QResizeEvent *event) {
	uint32_t size = (((event->size().height() / 2) * event->size().width()) * 4) * gfx.device_pixel_ratio;

	if (cfg->scale == X1) {
		overlaySaveSlot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
		verticalSpacer_slot_state_up->changeSize(0, 0, QSizePolicy::Minimum, QSizePolicy::Ignored);
		verticalSpacer_slot_state_down->changeSize(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
	} else {
		overlaySaveSlot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
		verticalSpacer_slot_state_up->changeSize(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
		verticalSpacer_slot_state_down->changeSize(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
	}

	if (clear) {
		free(clear);
	}

	clear = malloc(size);
	memset(clear, 0x00, size);

	QWidget::resizeEvent(event);
}
void wdgOverlayUi::closeEvent(QCloseEvent *event) {
	if (clear) {
		free(clear);
	}
	QWidget::closeEvent(event);
}

void wdgOverlayUi::retranslateUi(QWidget *wdgOverlayUi) {
	Ui::wdgOverlayUi::retranslateUi(wdgOverlayUi);
}
void wdgOverlayUi::update_widget(void) {
	overlayInputPort1->update_widget();
	overlayInputPort2->update_widget();
	overlayInputPort3->update_widget();
	overlayInputPort4->update_widget();

	overlayInfo->update_widget();
	overlayFPS->update_widget();
	overlayFrame->update_widget();
	overlayFloppy->update_widget();
	overlayRewind->update_widget();
	overlayTAS->update_widget();

	if (cfg->scale == X1) {
		widgetFakeButtomRight->show();
	} else {
		if (tas.type == NOTAS) {
			widgetFakeButtomRight->setHidden(!overlayRewind->isHidden());
		} else {
			widgetFakeButtomRight->setHidden(!overlayTAS->isHidden());
		}
	}

	force_redraw = true;
}

// overlayWidget -----------------------------------------------------------------------------------------------------------------

overlayWidget::overlayWidget(QWidget *parent) : QWidget(parent) {
	QFont fnt = font();

	exchange.draw = false;
	padding.h = 2;
	padding.v = 1;
	base_color.fg = Qt::black;
	base_color.bg = Qt::white;
	opacity.effect = new QGraphicsOpacityEffect(this);
	fade_in.animation = new QPropertyAnimation(opacity.effect, "opacity");
	fade_out.animation = new QPropertyAnimation(opacity.effect, "opacity");
	fade_out.timer.seconds = 3;
	enabled = false;
	force_control_when_hidden = false;
	always_visible = false;
	radius = 5;

	setMinimumSize(0, 0);
	setAttribute(Qt::WA_OpaquePaintEvent);
	setAutoFillBackground(false);

	set_opacity(0.85);
}
overlayWidget::~overlayWidget() {}

void overlayWidget::paintEvent(UNUSED(QPaintEvent *event)) {
	ms_last_draw = gui_get_ms();
	exchange.last_geometry = geometry();
	update_old_value();
}

void overlayWidget::update_widget(void) {
	if (cfg->txt_on_screen) {
		show_widget();
	} else {
		hide();
	}
}
BYTE overlayWidget::is_to_redraw(void) {
	return (TRUE);
}
void overlayWidget::update_old_value(void) {}
void overlayWidget::show_widget(void) {
	show();
	enabled = true;
}
int overlayWidget::hpadtot(void) const {
	return (padding.h * 2);
}
int overlayWidget::vpadtot(void) const {
	return (padding.v * 2);
}
int overlayWidget::minimum_eight(void) const {
	// 16 pixel e' l'altezza delle immagini;
	int fm = fontMetrics().height() + vpadtot();
	int px = 16 + vpadtot();

	return (fm < px ? px : fm);
}
void overlayWidget::set_opacity(qreal opacity) {
	this->opacity.value = opacity;
	this->opacity.effect->setOpacity(this->opacity.value);
	setGraphicsEffect(this->opacity.effect);
}
void overlayWidget::draw_background(void) {
	draw_background(rect());
}
void overlayWidget::draw_background(QRect rect) {
	QPen pen;

	pen.setWidth(1);
	pen.setColor(base_color.fg);
	painter.setPen(pen);
	painter.setBrush(base_color.bg);
	painter.drawRoundedRect(rect, radius, radius);
}
void overlayWidget::fade_in_animation(void) {
	if (always_visible == true) {
		set_opacity(opacity.value);
		return;
	}
	fade_in.animation->setDuration(500);
	fade_in.animation->setStartValue(0);
	fade_in.animation->setEndValue(opacity.value);
	fade_in.animation->setEasingCurve(QEasingCurve::InBack);
	fade_in.animation->start();
	connect(fade_in.animation, SIGNAL(finished()), this, SLOT(s_fade_in_finished()));
}
void overlayWidget::fade_out_animation(void) {
	if (always_visible == true) {
		set_opacity(opacity.value);
		return;
	}
	fade_out.animation->setDuration(1000);
	fade_out.animation->setStartValue(opacity.value);
	fade_out.animation->setEndValue(0);
	fade_out.animation->setEasingCurve(QEasingCurve::OutBack);
	fade_out.animation->start();
	connect(fade_out.animation, SIGNAL(finished()), this, SLOT(s_fade_out_finished()));
}
void overlayWidget::fade_out_start_timer(void) {
	if (fade_out.animation->state() == QPropertyAnimation::Running) {
		fade_out.animation->stop();
		set_opacity(opacity.value);
	}
	fade_out.timer.enabled = true;
	fade_out.timer.start = time(NULL);
}
void overlayWidget::fade_out_tick_timer(void) {
	if ((fade_out.timer.enabled == true) && ((time(NULL) - fade_out.timer.start) >= fade_out.timer.seconds)) {
		fade_out.timer.enabled = false;
		fade_out_animation();
	}
}
QString overlayWidget::color_string(QString string, QColor color) {
	return ("<font color='" + color.name().toUpper() + "'>" + string + "</font>");
}

void overlayWidget::s_fade_in_finished(void) {
	set_opacity(opacity.value);
}
void overlayWidget::s_fade_out_finished(void) {
	enabled = false;
	set_opacity(opacity.value);
}

// overlayWidgetFPS --------------------------------------------------------------------------------------------------------------

overlayWidgetFPS::overlayWidgetFPS(QWidget *parent) : overlayWidget(parent) {};
overlayWidgetFPS::~overlayWidgetFPS() {}

QSize overlayWidgetFPS::sizeHint() const {
	return (QSize(fontMetrics().size(0, "44 fps").width() + hpadtot(), minimum_eight()));
}
void overlayWidgetFPS::paintEvent(QPaintEvent *event) {
	overlayWidget::paintEvent(event);

	painter.begin(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	draw_background();
	painter.drawText(rect(), Qt::AlignHCenter | Qt::AlignVCenter, QString("%1 fps").arg((int)old.fps));
	painter.end();
}

void overlayWidgetFPS::update_widget(void) {
	if (cfg->txt_on_screen & cfg->show_fps) {
		show_widget();
	} else {
		hide();
	}
}
BYTE overlayWidgetFPS::is_to_redraw(void) {
	if (fps.gfx != old.fps) {
		return (TRUE);
	}
	return (FALSE);
}
void overlayWidgetFPS::update_old_value(void) {
	old.fps = fps.gfx;
}

// overlayWidgetFloppy -----------------------------------------------------------------------------------------------------------

overlayWidgetFloppy::overlayWidgetFloppy(QWidget *parent) : overlayWidget(parent) {
	floppy.gray = QImage(":/pics/pics/overlay_floppy_gray.png");
	floppy.red = QImage(":/pics/pics/overlay_floppy_red.png");
	floppy.green = QImage(":/pics/pics/overlay_floppy_green.png");
};
overlayWidgetFloppy::~overlayWidgetFloppy() {}

QSize overlayWidgetFloppy::sizeHint() const {
	return (QSize(floppy.gray.size().width() + hpadtot(), minimum_eight()));
}
void overlayWidgetFloppy::paintEvent(QPaintEvent *event) {
	QPoint coords = QPoint((rect().width() - floppy.gray.size().width()) / 2,(rect().height() - floppy.gray.size().height()) / 2);

	overlayWidget::paintEvent(event);

	painter.begin(this);
	if ((fds.info.last_operation | fds.drive.disk_ejected)) {
		if (fds.drive.disk_ejected) {
			painter.drawImage(coords, floppy.gray);
		} else {
			if (fds.info.last_operation == FDS_OP_READ) {
				painter.drawImage(coords, floppy.green);
			} else {
				painter.drawImage(coords, floppy.red);
			}
			if (rwnd.action != RWND_ACT_PAUSE) {
				fds.info.last_operation = FDS_OP_NONE;
			}
		}
	}
	painter.end();
}

void overlayWidgetFloppy::update_widget(void) {
	if (cfg->txt_on_screen & fds.info.enabled) {
		show_widget();
	} else {
		hide();
	}
}

// overlayWidgetFrame --------------------------------------------------------------------------------------------------------------

overlayWidgetFrame::overlayWidgetFrame(QWidget *parent) : overlayWidget(parent) {
	old.actual_frame = 0;
	td.setDefaultFont(font());
	info();
};
overlayWidgetFrame::~overlayWidgetFrame() {}

QSize overlayWidgetFrame::sizeHint() const {
	return (QSize(td.size().width(), minimum_eight()));
}
void overlayWidgetFrame::paintEvent(QPaintEvent *event) {
	overlayWidget::paintEvent(event);

	painter.begin(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	draw_background();

	{
		static QAbstractTextDocumentLayout::PaintContext ctx;

		painter.translate(0.0f, (height() - td.size().height()) / 2.0f);

		ctx.clip = rect();
		td.documentLayout()->draw(&painter, ctx);
	}
	painter.end();
}

void overlayWidgetFrame::update_widget(void) {
	if (cfg->txt_on_screen & cfg->show_frames_and_lags & !rwnd.active & (tas.type == NOTAS)) {
		show_widget();
	} else {
		hide();
	}
}
BYTE overlayWidgetFrame::is_to_redraw(void) {
	if (ppu.frames != old.actual_frame) {
		info();
		setMinimumWidth(td.size().width());
		return (TRUE);
	}
	return (FALSE);
}
void overlayWidgetFrame::update_old_value(void) {
	old.actual_frame = ppu.frames;
}

void overlayWidgetFrame::info(void) {
	QString txt = "";

	txt += color_string("F : ", base_color.fg);
	txt += color_string(QString("%1").arg(old.actual_frame), tas.lag_actual_frame ? shared_color.lag : shared_color.rwnd_actual);
	txt += color_string(" L : ", base_color.fg);
	txt += color_string(QString("%1").arg(tas.total_lag_frames), tas.lag_actual_frame ? shared_color.lag : shared_color.no_lag);

	td.setHtml(txt);
}

// overlayWidgetInputPort --------------------------------------------------------------------------------------------------------

overlayWidgetInputPort::overlayWidgetInputPort(QWidget *parent) : overlayWidget(parent) {
	QFont fnt = font();

	input_port = 0;
	type = CTRL_DISABLED;

	std_controller.tile = QImage(":/pics/pics/overlay_controller.png");
	std_controller.up = QImage(":/pics/pics/overlay_controller_up.png");
	std_controller.left = QImage(":/pics/pics/overlay_controller_left.png");
	std_controller.select = QImage(":/pics/pics/overlay_controller_select.png");
	std_controller.start = QImage(":/pics/pics/overlay_controller_start.png");
	std_controller.but_b = QImage(":/pics/pics/overlay_controller_b.png");
	std_controller.but_a = QImage(":/pics/pics/overlay_controller_a.png");

	zapper.tile = QImage(":/pics/pics/overlay_zapper.png");
	zapper.bang = QImage(":/pics/pics/overlay_zapper_bang.png");

	snes_mouse.tile = QImage(":/pics/pics/overlay_mouse.png");
	snes_mouse.button = QImage(":/pics/pics/overlay_mouse_button.png");

	arkanoid_paddle.tile = QImage(":/pics/pics/overlay_arkanoid_paddle.png");
	arkanoid_paddle.button = QImage(":/pics/pics/overlay_arkanoid_paddle_button.png");

	oeka_kids_tablet.tile = QImage(":/pics/pics/overlay_oeka_kids_tablet.png");

	exp_port = QImage(":/pics/pics/overlay_controller_exp_port.png");

	fnt.setPointSize(8);
	setFont(fnt);

	setFixedSize(std_controller.tile.size());
};
overlayWidgetInputPort::~overlayWidgetInputPort() {}

void overlayWidgetInputPort::paintEvent(QPaintEvent *event) {
	overlayWidget::paintEvent(event);

	painter.begin(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	switch (type) {
		case CTRL_STANDARD:
			draw_std_controller();
			break;
		case CTRL_ZAPPER:
			draw_zapper();
			break;
		case CTRL_SNES_MOUSE:
			draw_snes_mouse();
			break;
		case CTRL_ARKANOID_PADDLE:
			draw_arkanoid_paddle();
			break;
		case CTRL_OEKA_KIDS_TABLET:
			draw_oeka_kids_tablet();
			break;
	}
	painter.end();
}

void overlayWidgetInputPort::update_widget(void) {
	bool hide = true;

	type = port[input_port].type;

	switch (cfg->input.controller_mode) {
		case CTRL_MODE_NES:
			if (input_port > PORT2) {
				type = CTRL_DISABLED;
			}
			break;
		case CTRL_MODE_FAMICOM:
			if (cfg->input.expansion == CTRL_ARKANOID_PADDLE) {
				if (input_port == PORT3) {
					type = cfg->input.expansion;
				} else if (input_port == PORT4) {
					type = CTRL_DISABLED;
				}
			} else if (cfg->input.expansion != CTRL_STANDARD) {
				if (input_port == PORT3) {
					type = CTRL_DISABLED;
				} else if (input_port == PORT4) {
					type = cfg->input.expansion;
				}
			}
			break;
		case CTRL_MODE_FOUR_SCORE:
			break;
	}

	if (cfg->txt_on_screen == FALSE) {
		hide = true;
	} else if ((input_port > PORT1) && (cfg->scale == X1)) {
		hide = true;
	} else if (type == CTRL_DISABLED) {
		hide = true;
	} else if (cfg->input_display) {
		hide = false;
	}

	if (hide) {
		this->hide();
	} else {
		show_widget();
	}
}
BYTE overlayWidgetInputPort::is_to_redraw(void) {
	int i;

	switch (type) {
		case CTRL_STANDARD:
			for (i = 0; i < 8; i++) {
				if (old.std_controller.data[i] != port[input_port].data[i]) {
					return (TRUE);
				}
			}
			break;
		case CTRL_ZAPPER:
			if ((old.mouse.left != gmouse.left) || (old.mouse.x != gmouse.x) || (old.mouse.y != gmouse.y)) {
				return (TRUE);
			}
			break;
		case CTRL_SNES_MOUSE:
		case CTRL_OEKA_KIDS_TABLET:
			if ((old.mouse.left != gmouse.left) || (old.mouse.right != gmouse.right) ||
				(old.mouse.x != gmouse.x) || (old.mouse.y != gmouse.y)) {
				return (TRUE);
			}
			break;
		case CTRL_ARKANOID_PADDLE:
			if ((old.mouse.left != gmouse.left) || (old.mouse.x != arkanoid[input_port & 0x01].x)) {
				return (TRUE);
			}
			break;
		default:
			break;
	}

	return (FALSE);
}
void overlayWidgetInputPort::update_old_value(void) {
	switch (type) {
		case CTRL_STANDARD:
			old.std_controller.data[UP] = port[input_port].data[UP];
			old.std_controller.data[DOWN] = port[input_port].data[DOWN];
			old.std_controller.data[LEFT] = port[input_port].data[LEFT];
			old.std_controller.data[RIGHT] = port[input_port].data[RIGHT];
			old.std_controller.data[SELECT] = port[input_port].data[SELECT];
			old.std_controller.data[START] = port[input_port].data[START];
			old.std_controller.data[BUT_B] = port[input_port].data[BUT_B];
			old.std_controller.data[BUT_A] = port[input_port].data[BUT_A];
			break;
		case CTRL_ZAPPER:
			old.mouse.left = gmouse.left;
			old.mouse.x = gmouse.x;
			old.mouse.y = gmouse.y;
			break;
		case CTRL_SNES_MOUSE:
		case CTRL_OEKA_KIDS_TABLET:
			old.mouse.left = gmouse.left;
			old.mouse.right = gmouse.right;
			old.mouse.x = gmouse.x;
			old.mouse.y = gmouse.y;
			break;
		case CTRL_ARKANOID_PADDLE:
			old.mouse.left = gmouse.left;
			old.mouse.x = arkanoid[input_port & 0x01].x;
			break;
			break;
		default:
			break;
	}
}
void overlayWidgetInputPort::set_nport(int nport) {
	if ((nport < 0) || (nport > PORT_MAX)) {
		nport = 0;
	}
	input_port = nport;
	portx = QImage(QString(":/pics/pics/overlay_controller_port%1.png").arg(nport + 1));
}

void overlayWidgetInputPort::draw_std_controller(void) {
	painter.drawImage(0, 0, std_controller.tile);
	painter.drawImage(74, 1, portx);

	if (old.std_controller.data[UP] == PRESSED) {
		painter.drawImage(12, 5, std_controller.up);
	}
	if (old.std_controller.data[DOWN] == PRESSED) {
		painter.drawImage(12, 17, std_controller.up);
	}
	if (old.std_controller.data[LEFT] == PRESSED) {
		painter.drawImage(5, 12, std_controller.left);
	}
	if (old.std_controller.data[RIGHT] == PRESSED) {
		painter.drawImage(16, 12, std_controller.left);
	}
	if (old.std_controller.data[SELECT] == PRESSED) {
		painter.drawImage(34, 18, std_controller.select);
	}
	if (old.std_controller.data[START] == PRESSED) {
		painter.drawImage(50, 18, std_controller.start);
	}
	if (old.std_controller.data[BUT_B] == PRESSED) {
		painter.drawImage(68, 14, std_controller.but_b);
	}
	if (old.std_controller.data[BUT_A] == PRESSED) {
		painter.drawImage(82, 14, std_controller.but_a);
	}
}
void overlayWidgetInputPort::draw_zapper(void) {
	painter.drawImage(0, 0, zapper.tile);

	if (cfg->input.controller_mode == CTRL_MODE_FAMICOM) {
		painter.drawImage(61, 1, exp_port);
	} else {
		painter.drawImage(74, 1, portx);
	}

	draw_mouse_coords();

	if (old.mouse.left) {
		painter.drawImage(52, 2, zapper.bang);
	}
}
void overlayWidgetInputPort::draw_snes_mouse(void) {
	painter.drawImage(0, 0, snes_mouse.tile);
	painter.drawImage(74, 1, portx);

	draw_mouse_coords();

	if (old.mouse.left) {
		painter.drawImage(7, 5, snes_mouse.button);
	}
	if (old.mouse.right) {
		painter.drawImage(11, 5, snes_mouse.button);
	}
}
void overlayWidgetInputPort::draw_arkanoid_paddle(void) {
	painter.drawImage(0, 0, arkanoid_paddle.tile);

	if (cfg->input.controller_mode == CTRL_MODE_FAMICOM) {
		painter.drawImage(61, 1, exp_port);
	} else {
		painter.drawImage(74, 1, portx);
	}

	painter.setPen(Qt::white);
	painter.drawText(61, 26, QString("x: %1").arg(old.mouse.x));

	if (old.mouse.left) {
		painter.drawImage(8, 6, arkanoid_paddle.button);
	}
}
void overlayWidgetInputPort::draw_oeka_kids_tablet(void) {
	painter.drawImage(0, 0, oeka_kids_tablet.tile);

	if (cfg->input.controller_mode == CTRL_MODE_FAMICOM) {
		painter.drawImage(61, 1, exp_port);
	} else {
		painter.drawImage(74, 1, portx);
	}
}
void overlayWidgetInputPort::draw_mouse_coords(void) {
	int x, y;

	input_read_mouse_coords(&x, &y);
	if (x < 0) {
		x = 0;
	} else if (x >= SCR_COLUMNS) {
		x = SCR_COLUMNS - 1;
	}
	if (y < 0) {
		y = 0;
	} else if (y > SCR_ROWS - 1) {
		y = SCR_ROWS -1;
	}
	painter.setPen(Qt::white);
	painter.drawText(20, 26, QString("x: %1").arg(x, 3));
	painter.drawText(61, 26, QString("y: %1").arg(y, 3));
}

// overlayWidgetRewind -----------------------------------------------------------------------------------------------------------

overlayWidgetRewind::overlayWidgetRewind(QWidget *parent) : overlayWidget(parent) {
	led = QFont("Digital Counter 7");

	led.setPointSizeF(font().pointSizeF());
	td.setDefaultFont(led);

	color.corner = QColor(234, 234, 184);
	color.border_bar = QColor(100, 100, 100);
	color.bar = QColor(224, 255, 224);
	color.actual = shared_color.rwnd_actual;
	color.total = QColor(128, 0, 0);
	color.disabled = QColor(180, 180, 180);

	act.fbackward = svg_to_image(":/icon/icons/rwnd_fast_backward.svg");
	act.backward = svg_to_image(":/icon/icons/rwnd_step_backward.svg");
	act.play = svg_to_image(":/icon/icons/rwnd_play.svg");
	act.pause = svg_to_image(":/icon/icons/rwnd_pause.svg");
	act.forward = svg_to_image(":/icon/icons/rwnd_step_forward.svg");
	act.fforward = svg_to_image(":/icon/icons/rwnd_fast_forward.svg");
};
overlayWidgetRewind::~overlayWidgetRewind() {}

QSize overlayWidgetRewind::sizeHint() const {
	return (QSize(100, minimum_eight()));
}
void overlayWidgetRewind::paintEvent(QPaintEvent *event) {
	overlayWidget::paintEvent(event);

	painter.begin(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	info.width = width();
	draw_background();
	draw_command();
	draw_corner_bar_info();
	painter.end();
}

void overlayWidgetRewind::update_widget(void) {
	if (cfg->scale == X1) {
		hide();
	} else if (tas.type == NOTAS) {
		if (cfg->txt_on_screen & rwnd.active) {
			show_widget();
		} else {
			hide();
		}
	} else {
		hide();
	}
}
BYTE overlayWidgetRewind::is_to_redraw(void) {
	if ((old.action != rwnd.action) || (old.action_before_pause != rwnd.action_before_pause) ||
		(old.actual_frame != rewind_snap_cursor()) || (old.max_frames != rewind_count_snaps())) {
		return (TRUE);
	}
	return (FALSE);
}
void overlayWidgetRewind::update_old_value(void) {
	old.action = rwnd.action;
	old.action_before_pause = rwnd.action_before_pause;
	old.actual_frame = rewind_snap_cursor();
	old.max_frames = rewind_count_snaps();
}
QString overlayWidgetRewind::seconds_to_string(_infotime *itime, _infotime::_measure max, QColor color) {
	QColor disabled = this->color.disabled;
	QString txt = "";

	if (itime->hh > 0) {
		txt += color_string(QString("%1").arg(itime->hh), itime->hh == 0 ? disabled : color);
		txt += ":";
		txt += color_string(QString("%1").arg(itime->mm, 2, 10, QLatin1Char('0')), color);
		txt += ":";
		txt += color_string(QString("%1").arg(itime->ss, 2, 10, QLatin1Char('0')), color);
		txt += ":";
	} else if (itime->mm > 0) {
		int a = itime->mm / 10;
		int b = itime->mm % 10;

		if (max == _infotime::_measure::HH) {
			txt += color_string(QString("%1").arg(itime->hh), disabled);
			txt += color_string(":", disabled);
		}
		txt += color_string(QString("%1").arg(a), a == 0 ? disabled : color);
		if (a > 0) {
			txt += color_string(QString("%1").arg(b), color);
		} else {
			txt += color_string(QString("%1").arg(b), b == 0 ? disabled : color);
		}
		txt += ":";
		txt += color_string(QString("%1").arg(itime->ss, 2, 10, QLatin1Char('0')), color);
		txt += ":";
	} else if (itime->ss > 0) {
		int a = itime->ss / 10;
		int b = itime->ss % 10;

		if (max <= _infotime::_measure::HH) {
			txt += color_string(QString("%1").arg(itime->hh), disabled);
			txt += color_string(":", disabled);
		}
		if (max <= _infotime::_measure::MM) {
			txt += color_string(QString("%1").arg(itime->mm, 2, 10, QLatin1Char('0')), disabled);
			txt += color_string(":", disabled);
		}
		txt += color_string(QString("%1").arg(a), a == 0 ? disabled : color);
		if (a > 0) {
			txt += color_string(QString("%1").arg(b), color);
		} else {
			txt += color_string(QString("%1").arg(b), b == 0 ? disabled : color);
		}
		txt += ":";
	} else {
		if (max == _infotime::_measure::HH) {
			txt += color_string(QString("%1").arg(itime->hh), disabled);
			txt += color_string(":", disabled);
		}
		if (max <= _infotime::_measure::MM) {
			txt += color_string(QString("%1").arg(itime->mm, 2, 10, QLatin1Char('0')), disabled);
			txt += color_string(":", disabled);
		}
		txt += color_string(QString("%1").arg(itime->ss, 2, 10, QLatin1Char('0')), disabled);
		txt += color_string(":", disabled);
	}
	txt += color_string(QString("%1").arg(itime->ms, 2, 10, QLatin1Char('0')), color);

	return (txt);
}
void overlayWidgetRewind::draw_command(void) {
	QRect icon, text;
	QImage *image = NULL;
	BYTE action = old.action;
	QString desc;

	painter.save();

	if (rwnd.action == RWND_ACT_PAUSE) {
		action = rwnd.action_before_pause;
	}

	switch (action) {
		case RWND_ACT_PAUSE:
			image = &act.pause;
			desc = "pause";
			break;
		case RWND_ACT_PLAY:
			if (rwnd.action == RWND_ACT_PAUSE) {
				image = &act.pause;
				desc = "pause";
			} else {
				image = &act.play;
				desc = "play";
			}
			break;
		case RWND_ACT_STEP_BACKWARD:
			image = &act.backward;
			desc = "-1";
			break;
		case RWND_ACT_FAST_BACKWARD:
			if (rwnd.action == RWND_ACT_PAUSE) {
				image = &act.pause;
				desc = "pause";
			} else {
				image = &act.fbackward;
				desc = QString("-%1x").arg(rwnd.factor.backward);
			}
			break;
		case RWND_ACT_STEP_FORWARD:
			image = &act.forward;
			desc = "+1";
			break;
		case RWND_ACT_FAST_FORWARD:
			if (rwnd.action == RWND_ACT_PAUSE) {
				image = &act.pause;
				desc = "pause";
			} else {
				image = &act.fforward;
				desc = QString("+%1x").arg(rwnd.factor.forward);
			}
			break;
	}

	if (image) {
		qreal x, y, w, h;

		x = width() - act.play.width() - padding.h;
		y = (height() - act.play.height()) / 2;
		w = act.play.width();
		h = act.play.height();
		icon = QRect(x, y, w, h);
		painter.drawImage(icon.topLeft(), (*image));

		x = 0;
		y = 0;
		w = icon.x() - padding.h;
		h = height();
		painter.setFont(led);
		text = painter.boundingRect(QRect(x, y, w, h), Qt::AlignRight | Qt::AlignVCenter, "PAUSE");
		painter.drawText(text, Qt::AlignHCenter | Qt::AlignVCenter, desc);
	}

	info.width = text.x() - hpadtot();

	painter.restore();
}
void overlayWidgetRewind::draw_corner_bar_info(void) {
	static QRect qr;
	QPen pen = painter.pen();
	int vpad = 2, hpad = 3;
	double max = this->max();
	double min = this->min();
	double value = this->value();
	double step = value / (max - min);

	//percentuale
	info.width -= padding.h;
	painter.translate(padding.h, 0);
	qr = painter.boundingRect(QRect(0, 0, info.width, height()), Qt::AlignLeft | Qt::AlignVCenter, "000%");
	painter.setPen(base_color.fg);
	painter.drawText(qr, Qt::AlignCenter, QString("%1%").arg((int)(100 * step)));

	// cornice
	info.width -= (qr.width() + padding.h);
	painter.translate(qr.width() + padding.h, 0);
	painter.setBrush(color.corner);
	painter.drawRoundedRect(QRect(0, 0, info.width, height()), radius, radius);

	// barra
	if (value > 0) {
		qr.setRect(hpad, vpad, (info.width - (hpad * 2)) * step, height() - (vpad * 2));
		painter.setPen(color.border_bar);
		painter.setBrush(color.bar);
		painter.drawRoundedRect(qr, radius, radius);
	}

	info.width -= hpad;
	painter.translate(hpad, 0);

	{
		static QAbstractTextDocumentLayout::PaintContext ctx;

		td.setHtml(info_long());

		if (td.size().width() >= info.width) {
			td.setHtml(info_short());
		}

		painter.translate(0.0f, (height() - td.size().height()) / 2.0f);

		ctx.clip = rect();
		td.documentLayout()->draw(&painter, ctx);
	}
}

int32_t overlayWidgetRewind::min(void) {
	int32_t max = this->max();

	return (max < rewind_max_buffered_snaps() ? 0 : max - rewind_max_buffered_snaps());
}
int32_t overlayWidgetRewind::max(void) {
	return (old.max_frames);
}
int32_t overlayWidgetRewind::value(void) {
	return (old.actual_frame - min());
}
QString overlayWidgetRewind::info_long(void) {
	_infotime itmax(old.max_frames);
	_infotime itactual(old.actual_frame);
	QString txt = "";

	txt += seconds_to_string(&itactual, itmax.max, color.actual);
	txt += color_string("/", base_color.fg);
	txt += seconds_to_string(&itmax, itmax.max, color.total);
	txt += color_string(" (", base_color.fg);
	txt += color_string(QString("%1").arg(old.actual_frame), color.actual);
	txt += color_string("/", base_color.fg);
	txt += color_string(QString("%1").arg(old.max_frames), color.total);
	txt += color_string(")", base_color.fg);

	return (txt);
}
QString overlayWidgetRewind::info_short(void) {
	QString txt = "";

	txt += color_string(QString("%1").arg(old.actual_frame), color.actual);
	txt += color_string("/", base_color.fg);
	txt += color_string(QString("%1").arg(old.max_frames), color.total);

	return (txt);
}

QImage overlayWidgetRewind::svg_to_image(QString resource) {
	QImage image(QSize(14, 14), QImage::Format_ARGB32);
	QPainter painter(&image);
	QSvgRenderer svg(resource);

	image.fill(Qt::transparent);
	svg.render(&painter);

	return (image);
}

// overlayWidgetTAS --------------------------------------------------------------------------------------------------------------

overlayWidgetTAS::overlayWidgetTAS(QWidget *parent) : overlayWidgetRewind(parent) {
	color.corner = QColor(211, 215, 207);
	color.bar = Qt::white;
};
overlayWidgetTAS::~overlayWidgetTAS() {}

void overlayWidgetTAS::update_widget(void) {
	if (cfg->scale == X1) {
		hide();
	} else if (tas.type == NOTAS) {
		hide();
	} else {
		if (cfg->txt_on_screen) {
			show_widget();
		} else {
			hide();
		}
	}
}
BYTE overlayWidgetTAS::is_to_redraw(void) {
	if ((old.action != rwnd.action) || (old.action_before_pause != rwnd.action_before_pause) || (old.actual_frame != tas.frame)) {
		return (TRUE);
	}
	return (FALSE);
}
void overlayWidgetTAS::update_old_value(void) {
	old.action = rwnd.action;
	old.action_before_pause = rwnd.action_before_pause;
	old.actual_frame = tas.frame;
	old.max_frames = tas.total - 1;
}

int32_t overlayWidgetTAS::min(void) {
	return (0);
}
int32_t overlayWidgetTAS::max(void) {
	return (tas.total - 1);
}
int32_t overlayWidgetTAS::value(void) {
	return (tas.frame);
}
QString overlayWidgetTAS::info_long(void) {
	_infotime itmax(old.max_frames);
	_infotime itactual(old.actual_frame);
	QString txt = "";

	txt += seconds_to_string(&itactual, itmax.max, color.actual);
	txt += color_string("/", base_color.fg);
	txt += seconds_to_string(&itmax, itmax.max, color.total);
	txt += color_string(" (", base_color.fg);
	txt += color_string(QString("%1").arg(old.actual_frame), tas.lag_actual_frame ? shared_color.lag : color.actual);
	txt += color_string("/", base_color.fg);
	txt += color_string(QString("%1").arg(old.max_frames), color.total);
	txt += color_string(" [", base_color.fg);
	txt += color_string(QString("%1").arg(tas.total_lag_frames), tas.lag_actual_frame ? shared_color.lag : shared_color.no_lag);
	txt += color_string("])", base_color.fg);

	return (txt);
}
QString overlayWidgetTAS::info_short(void) {
	QString txt = "";

	txt += color_string(QString("%1").arg(old.actual_frame), tas.lag_actual_frame ? shared_color.lag : color.actual);
	txt += color_string("/", base_color.fg);
	txt += color_string(QString("%1").arg(old.max_frames), color.total);
	txt += color_string(" [", base_color.fg);
	txt += color_string(QString("%1").arg(tas.total_lag_frames), tas.lag_actual_frame ? shared_color.lag : color.actual);
	txt += color_string("]", base_color.fg);

	return (txt);
}

// overlaySaveSlot ---------------------------------------------------------------------------------------------------------------

overlayWidgetSaveSlot::overlayWidgetSaveSlot(QWidget *parent) : overlayWidget(parent) {
	color.x1.save = Qt::red;
	color.x1.read = QColor(252, 45, 255);
	color.x1.selected = Qt::black;
	color.x1.text = Qt::black;
	color.x1.text_not_used = Qt::gray;

	color.no_preview = Qt::darkGray;
	color.previw_opacity = Qt::black;
	color.border = Qt::lightGray;
	color.border_selected = Qt::white;
	color.bar = Qt::black;
	color.bar_selected = Qt::darkGreen;
	color.slot = Qt::yellow;
	color.info = Qt::white;

	save_slot_operation = 0;
	height_row_slot = 20;
	rows = 3;
	columns = ((SAVE_SLOTS % rows) == 0) ? SAVE_SLOTS / rows : (SAVE_SLOTS / rows) + 1;

	set_opacity(0.93);
};
overlayWidgetSaveSlot::~overlayWidgetSaveSlot() {}

QSize overlayWidgetSaveSlot::sizeHint() const {
	double ratio = SCR_COLUMNS / SCR_ROWS;
	int width;

	if (cfg->scale == X1) {
		width = SAVE_SLOTS * 15 + 2;
		return (QSize(width, height_row_slot + ((width / ratio) / (11.0f / 8.0f))));
	} else {
		switch (rows) {
			default:
				width = 100;
				break;
			case 3:
				width = 130;
				break;
		}
		return (QSize((width * columns) + hpadtot(), ((width / ratio) * rows) + hpadtot()));
	}
}
void overlayWidgetSaveSlot::paintEvent(QPaintEvent *event) {
	overlayWidget::paintEvent(event);

	painter.begin(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	draw_background();
	if (cfg->scale == X1) {
		draw_slots_x1();
	} else {
		draw_slots();
	}
	fade_out_tick_timer();
	painter.end();
}

void overlayWidgetSaveSlot::enable_overlay(BYTE operation) {
	save_slot_operation = operation;
	fade_out_start_timer();
	enabled = TRUE;
}
QString overlayWidgetSaveSlot::date_and_time(int slot) {
	return (QLocale::system().toString(previews[slot].fileinfo.lastModified(), QLocale::ShortFormat));
}

void overlayWidgetSaveSlot::draw_slots_x1(void) {
	int x, y, w, h, radius;
	int x1, y1;
	static QFont f;
	static QRect rect;
	static QPen pen;

	painter.save();

	// disegno la riga di selezione dello slot
	w = width() / SAVE_SLOTS;
	h = height_row_slot;
	x1 = (w / 2) + padding.h;
	y1 = h / 2;

	if ((x1 + (SAVE_SLOTS * w)) < width()) {
		x1 += (width() - (x1 + (SAVE_SLOTS * w))) / 2;
	}

	radius = h / 3.0f;

	for (unsigned int i = 0; i < SAVE_SLOTS; i++) {
		x = x1 + (i * w);
		y = y1;

		if (save_slot.state[i]) {
			painter.setPen(Qt::NoPen);
			painter.setBrush(QColor(80, 255, 87));
			painter.drawEllipse(QPoint(x, y), radius, radius);
		}
	}

	// disegno il cursore di selezione
	{
		x = x1 + (save_slot.slot * w);
		y = y1;

		if (save_slot_operation == SAVE_SLOT_SAVE) {
			painter.setPen(Qt::NoPen);
			painter.setBrush(color.x1.save);
		} else if (save_slot_operation == SAVE_SLOT_READ) {
			painter.setPen(Qt::NoPen);
			painter.setBrush(color.x1.read);
		} else {
			pen.setColor(color.x1.selected);
			pen.setWidth(2);
			painter.setPen(pen);
			painter.setBrush(Qt::NoBrush);
		}
		painter.drawEllipse(QPoint(x, y), radius, radius);
	}

	f = font();
	f.setPixelSize(h / 2.5);

	painter.setFont(f);
	painter.setBrush(Qt::NoBrush);

	for (unsigned int i = 0; i < SAVE_SLOTS; i++) {
		x = x1 + (i * w);
		y = y1;

		rect.setRect(x - radius, y - radius, radius * 2, radius * 2);

		if (save_slot.state[i]) {
			painter.setPen(color.x1.text);
		} else {
			painter.setPen(color.x1.text_not_used);
		}
		painter.drawText(rect, Qt::AlignCenter, QString::number(i, 16).toUpper());
	}

	// disegno la preview
	w = (width() - (2 * padding.h));
	h = (height() - height_row_slot - (3 * padding.v));
	x = padding.h;
	y = height_row_slot + padding.v;

	rect.setRect(x, y, w, h);

	if (!previews[save_slot.slot].image.isNull()) {
		painter.drawImage(rect, previews[save_slot.slot].image.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	} else {
		painter.setRenderHint(QPainter::Antialiasing, false);
		painter.setPen(Qt::NoPen);
		painter.setBrush(color.no_preview);
		painter.drawRect(rect);
	}

	// disegno la riga in cui scrivere le info
	rect.setRect(x, y, w, height_row_slot);

	painter.setOpacity(0.8);
	if (!previews[save_slot.slot].image.isNull()) {
		painter.fillRect(rect, color.bar_selected);
	}

	// scrivo le info
	f.setPixelSize(w / 12.0);

	pen.setColor(color.info);
	pen.setWidth(1);

	painter.setFont(f);
	painter.setPen(pen);
	painter.setOpacity(1.0);
	painter.drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, date_and_time(save_slot.slot));

	painter.restore();
}
void overlayWidgetSaveSlot::draw_slots(void) {
	int x, y, w, h;
	static QFont f;
	static QRect rect;
	static QPen pen;

	painter.save();

	{
		double ratio = SCR_COLUMNS / SCR_ROWS;
		int x1, y1;

		w = (width() - hpadtot()) / columns;
		h = w / ratio;
		x1 = padding.h;
		y1 = padding.v;

		if ((x1 + (columns * w)) < width()) {
			x1 += (width() - (x1 + (columns * w))) / 2;
		}
		if ((y1 + (rows * h)) < height()) {
			y1 += (height() - (y1 + (rows * h))) / 2;
		}

		for (unsigned int i = 0; i < SAVE_SLOTS; i++) {
			x = x1 + ((i % columns) * w);
			y = y1 + ((i / columns) * h);

			rect.setRect(x, y, w, h);

			// disegno la preview
			if (!previews[i].image.isNull()) {
				QImage img = previews[i].image.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
				static QPainter p;

				if (i != save_slot.slot) {
					p.begin(&img);
					p.setOpacity(0.6);
					p.fillRect(img.rect(), color.previw_opacity);
					p.end();
				}
				painter.drawImage(rect, img);
			} else {
				painter.setRenderHint(QPainter::Antialiasing, false);
				painter.setPen(Qt::NoPen);
				painter.setBrush(color.no_preview);
				painter.drawRect(rect);
			}

			// disegno il bordo
			pen.setColor(color.border);
			pen.setWidth(4);

			painter.setPen(pen);
			painter.setBrush(Qt::NoBrush);
			painter.drawRect(rect);

			// disegno la riga in cui scrivere le info
			rect.setRect(x + 2, y, w - 4, height_row_slot);

			painter.setOpacity(0.8);
			if (i == save_slot.slot) {
				painter.fillRect(rect, color.bar_selected);
			} else {
				painter.fillRect(rect, color.bar);
			}

			// scrivo le info
			f = font();
			f.setPixelSize(w / 12.0);

			pen.setWidth(1);

			painter.setFont(f);
			painter.setOpacity(1.0);

			pen.setColor(color.slot);
			painter.setPen(pen);
			painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, QString(" %0").arg(i, 1, 16).toUpper());

			pen.setColor(color.info);
			painter.setPen(pen);
			painter.drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, date_and_time(i));
		}

		// disegno il cursore di selezione dello slot
		{
			x = x1 + ((save_slot.slot % columns) * w);
			y = y1 + ((save_slot.slot / columns) * h);

			rect.setRect(x, y, w, h);

			pen.setColor(color.border_selected);
			pen.setWidth(2);

			painter.setPen(pen);
			painter.setBrush(Qt::NoBrush);
			painter.drawRect(rect);
		}
	}

	painter.restore();
}

// overlayWidgetInfo -------------------------------------------------------------------------------------------------------------

overlayWidgetInfo::overlayWidgetInfo(QWidget *parent) : overlayWidget(parent) {
	base_color.bg = QColor(125, 125, 125);
	base_color.bg = QColor(50, 50, 50);
	force_control_when_hidden = true;
};
overlayWidgetInfo::~overlayWidgetInfo() {}

QSize overlayWidgetInfo::sizeHint() const {
	return (QSize(100, minimum_eight()));
}
void overlayWidgetInfo::paintEvent(QPaintEvent *event) {
	if (!isHidden() && overlay.info_actual_message.length()) {
		static const QFont f = QFont("ChronoType");
		static QAbstractTextDocumentLayout::PaintContext ctx;
		QTextDocument td;
		int x, y, w, h;

		overlayWidget::paintEvent(event);

		painter.begin(this);
		painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

		td.setDefaultFont(f);
		td.setHtml(overlay.info_actual_message);

		x = 0;
		y = 0;
		w = td.size().width();
		h = height();
		draw_background(QRect(x, y, w, h));

		painter.translate(0, (height() - td.size().height()) / 2);

		ctx.clip = rect();
		td.documentLayout()->draw(&painter, ctx);

		painter.end();
	}

	fade_out_tick_timer();
}

BYTE overlayWidgetInfo::is_to_redraw(void) {
	if (overlay.info_actual_message == "") {
		if (fade_out.timer.enabled == true) {
			return (TRUE);
		}
		if (overlay.info_messages_to_draw.length() == 0) {
			enabled = false;
			return (FALSE);
		}
		overlay.info_actual_message = overlay.info_messages_to_draw[0];
		overlay.info_messages_to_draw.removeAt(0);
		fade_in_animation();
		enabled = true;
	}
	return (TRUE);
}
void overlayWidgetInfo::append_msg(QString msg) {
	_append_msg(msg);
	enabled = true;
}

void overlayWidgetInfo::_append_msg(QString msg) {
	msg = decode_tags("[white]" + msg + "[normal]");
	overlay.info_messages_to_draw.append(msg);
}
QString overlayWidgetInfo::decode_tags(QString input) {
	static const _tags tags[] = {
		{ "[red]",    "#FF8080" },
		{ "[yellow]", "#FFFF00" },
		{ "[green]",  "#00FF00" },
		{ "[cyan]",   "#00FFFF" },
		{ "[brown]",  "#FF9B00" },
		{ "[blue]",   "#0000FF" },
		{ "[gray]",   "#B4B4B4" },
		{ "[black]",  "#000000" },
		{ "[white]",  "#FFFFFF" },
		{ "[normal]",        "" }
	};
	int i;
	unsigned int tag;
	QString output = "";

	for (i = 0; i < input.length();) {
		bool found = false;

		if (input[i] == '[') {
			for (tag = 0; tag < LENGTH(tags); tag++) {
				int len = tags[tag].name.length();

				if (input.mid(i, len) == tags[tag].name) {
					if (tags[tag].name == "[normal]") {
						output += "</font>";
					} else {
						output += "<font color='" + tags[tag].value + "'>";
					}
					found = true;
					i += len;
					break;
				}
			}
		}

		if (found) {
			continue;
		}
		output += input[i];
		i++;
	}

	return (output);
}

void overlayWidgetInfo::s_fade_in_finished(void) {
	fade_out_start_timer();
}
void overlayWidgetInfo::s_fade_out_finished(void) {
	overlay.info_actual_message = "";
	set_opacity(opacity.value);
}

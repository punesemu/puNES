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

#include <QtCore/QDateTime>
#include <QtCore/QMutex>
#include <QtCore/QRegularExpression>
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtSvg/QSvgRenderer>
#include <QtWidgets/QGraphicsItem>
#include <time.h>
#include <math.h>
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

void overlay_info_append_qstring(BYTE alignment, QString msg);

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
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 31 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "state [cyan]%1[normal] saved successfully"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 32 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "state [cyan]%1[normal] loaded successfully"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 33 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "tape play"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 34 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "tape record"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 35 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "tape stop"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 36 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "input captured"),
//: Do not translate the words contained between parentheses (example: [red] or [normal]) are tags that have a specific meaning and do not traslate %1 and %2
/* 37 */ QT_TRANSLATE_NOOP("overlayWidgetInfo", "input released"),
};

typedef struct _overlay_info_message {
	QString msg;
	BYTE alignment;
} _overlay_info_message;

struct _shared_color {
	QColor rwnd_actual = QColor(Qt::blue);
	QColor lag = Qt::red;
	QColor no_lag = QColor(30, 128, 0);
} shared_color;
struct _overlay_data {
	struct _overlay_data_ui {
		wdgOverlayUi *widget;
	} ui;
	struct _overlay_data_info {
		QMutex mutex;
		QList<_overlay_info_message> messages_to_draw;
		QString actual = "";
		BYTE alignment;
	} info;
} overlay;

void gui_overlay_update(void) {
	if (overlay.ui.widget) {
		overlay.ui.widget->update_widget();
	}
}
BYTE gui_overlay_is_updated(void) {
	return (overlay.ui.widget->update_texture);
}
void gui_overlay_enable_save_slot(BYTE mode) {
	overlay.ui.widget->overlaySaveSlot->enable_overlay(mode);
}
void gui_overlay_set_size(int w, int h) {
	// ne creo una nuova ogni volta che devo ridimensionarla ( e cio' accade
	// esclusivamente quando eseguo un gfx_set_creen()) perche' sotto windows,
	// quando nel fullscreen cambio la risoluzione, puo' succedere che il
	// device pixel ratio possa essere diverso da quello impostato nella risoluzione
	// originaria (capita su risoluzione basse tipo 640x480 o 1024x768) e quindi
	// devo forzare il ricalcolo della geometria del wdgOverlayUi.
	if (gui.start == TRUE) {
		overlay.ui.widget->update_dpr();
		overlay.ui.widget->updateGeometry();
	}
	overlay.ui.widget->setFixedSize(w, h);
}
void gui_overlay_info_init(void) {
	overlay.info.actual = "";
	overlay.info.alignment = OVERLAY_INFO_LEFT;
	overlay.info.messages_to_draw.clear();
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

	overlayWidgetInfo::_append_msg(OVERLAY_INFO_LEFT, str);
}
void gui_overlay_info_append_subtitle(uTCHAR *msg) {
	overlay_info_append_qstring(OVERLAY_INFO_CENTER, uQString(msg));
}
void gui_overlay_info_append_msg_precompiled(int index, void *arg1) {
	gui_overlay_info_append_msg_precompiled_with_alignment(OVERLAY_INFO_LEFT, index, arg1);
}
void gui_overlay_info_append_msg_precompiled_with_alignment(BYTE alignment, int index, void *arg1) {
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
		case 31:
		case 32:
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
	overlay_info_append_qstring(alignment, msg);
}
void gui_overlay_blit(void) {
	overlay.ui.widget->overlay_blit();
}
void gui_overlay_slot_preview(int slot, void *buffer, uTCHAR *file) {
	if (buffer) {
		overlay.ui.widget->overlaySaveSlot->previews[slot].image =
			QImage((uchar*)buffer, SCR_COLUMNS, SCR_ROWS, SCR_COLUMNS * sizeof(uint32_t), QImage::Format_RGB32);
		overlay.ui.widget->overlaySaveSlot->previews[slot].image =
			overlay.ui.widget->overlaySaveSlot->previews[slot].image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
	} else {
		overlay.ui.widget->overlaySaveSlot->previews[slot].image = QImage();
	}
	if (file) {
		overlay.ui.widget->overlaySaveSlot->previews[slot].fileinfo = QFileInfo(uQString(file));
	} else {
		overlay.ui.widget->overlaySaveSlot->previews[slot].fileinfo = QFileInfo();
	}
}

void overlay_info_append_qstring(BYTE alignment, QString msg) {
	if (overlay.ui.widget) {
		overlay.ui.widget->overlayInfo->append_msg(alignment, msg);
	} else {
		overlayWidgetInfo::_append_msg(alignment, msg);
	}
}

// wdgOverlayUi ------------------------------------------------------------------------------------------------------------------

wdgOverlayUi::wdgOverlayUi(QWidget *parent) : QWidget(parent) {
	QGraphicsOpacityEffect *op = new QGraphicsOpacityEffect(this);

	setFont(QFont("Sans"));

	clear = NULL;
	force_redraw = false;
	update_texture = FALSE;

	setupUi(this);

	overlayInfo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	overlayFPS->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	overlayFloppy->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	overlayFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	overlayRewind->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	overlayTAS->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	setAttribute(Qt::WA_OpaquePaintEvent);

	op->setOpacity(0.999999999);
	setGraphicsEffect(op);
	setAutoFillBackground(false);

	overlayInputPort1->set_nport(0);
	overlayInputPort2->set_nport(1);
	overlayInputPort3->set_nport(2);
	overlayInputPort4->set_nport(3);

	overlay.ui.widget = this;
	wdgs = findChildren<overlayWidget *>();

	update_dpr();
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
	uint32_t size = ((event->size().height() * event->size().width()) * 4);

	if (cfg->scale == X1) {
		overlaySaveSlot->dim_cell_x1 = cfg->fullscreen ? 20 : 20 / 1.75f;
		overlaySaveSlot->resize(overlaySaveSlot->calc_size());
		overlaySaveSlot->setSizePolicy(QSizePolicy::Preferred, cfg->fullscreen ? QSizePolicy::Preferred : QSizePolicy::Minimum);
		verticalSpacer_slot_state_up->changeSize(0, 0, QSizePolicy::Minimum, QSizePolicy::Ignored);
		verticalSpacer_slot_state_down->changeSize(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
		overlaySaveSlot->adjustSize();
	} else {
		overlaySaveSlot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
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
void wdgOverlayUi::update_dpr(void) {
	QFont font = this->font();

	font.setPointSizeF(10.0f / devicePixelRatioF());
	setFont(font);

	for (overlayWidget *ele : wdgs) {
		ele->update_dpr();
	}
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
void wdgOverlayUi::overlay_blit(void) {
	qreal dpr = devicePixelRatioF();

	update_texture = FALSE;

	// e' importante rispettare l'ordine sottostante.
	// 1 - prima cancellazione
	for (overlayWidget *ele : wdgs) {
		wdg_clear(ele, NULL, dpr);
	}
	// 2 - seconda cancellazione con preparazione immagini
	for (overlayWidget *ele : wdgs) {
		if ((ele->isHidden() == false) && (ele->enabled == TRUE)) {
			bool redraw = false;

			update_texture = TRUE;

			if (force_redraw == true) {
				redraw = true;
			} else if (ele->is_to_redraw() == TRUE) {
				redraw = true;
			}

			if (redraw) {
				QRect second_last = ele->exchange.last_geometry;

				ele->enabled = true;
				ele->exchange.draw = true;
				ele->exchange.img = ele->grab().toImage();

				if ((second_last != ele->exchange.last_geometry) && geometry().contains(second_last)) {
					wdg_clear(ele, &second_last, dpr);
				}
			}
		} else if (ele->force_control_when_hidden && (ele->is_to_redraw() == TRUE)) {
			ele->grab().toImage();
		}
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
				gfx_overlay_blit((void *)ele->exchange.img.bits(), &rect, dpr);
			}
		}
	}

	force_redraw = false;
}

void wdgOverlayUi::wdg_clear(overlayWidget *wdg, QRect *qrect, qreal dpr) {
	QRect *g = (QRect *)&wdg->geometry();
	bool erase = false;

	if (wdg->isHidden() & wdg->enabled) {
		wdg->enabled = false;
		erase = true;
	} else if (qrect) {
		g = qrect;
		erase = true;
	}

	if (erase) {
		_gfx_rect rect;

		update_texture = TRUE;

		rect.x = g->x();
		rect.y = g->y();
		rect.w = g->width();
		rect.h = g->height();

		gfx_overlay_blit((void *)clear, &rect, dpr);
	}
}

// overlayWidget -----------------------------------------------------------------------------------------------------------------

overlayWidget::overlayWidget(QWidget *parent) : QWidget(parent) {
	exchange.draw = false;
	padding.h = 2;
	padding.v = 2;
	base_color.fg = Qt::black;
	base_color.bg = Qt::white;
	opacity.effect = new QGraphicsOpacityEffect(this);
	fade_in.animation = new QPropertyAnimation(opacity.effect, "opacity");
	fade_out.animation = new QPropertyAnimation(opacity.effect, "opacity");
	fade_out.timer.seconds = 3;
	enabled = false;
	force_control_when_hidden = false;
	radius = 5;
	fade_in_duration = 500;
	fade_out_duration = 700;
	always_visible = false;

	setMinimumSize(0, 0);
	setAttribute(Qt::WA_OpaquePaintEvent);
	setAutoFillBackground(false);

	set_opacity(0.88);

	update_dpr();
}
overlayWidget::~overlayWidget() {}

void overlayWidget::paintEvent(UNUSED(QPaintEvent *event)) {
	ms_last_draw = gui_get_ms();
	exchange.last_geometry = geometry();
	update_old_value();
}

void overlayWidget::update_dpr(void) {}
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
int overlayWidget::minimum_eight(const QFont *f = NULL, int rows = 1) const {
	// 16 pixel e' l'altezza delle immagini
	qreal fm = round(((dpr_per_int(f ? QFontMetrics((*f)).height() : fontMetrics().height()) * rows) + (vpadtot() * rows)) / 2.0f) * 2.0f;
	qreal px = round((16.0f + vpadtot()) / 2.0f) * 2.0f;

	return (fm < px ? px : fm);
}
void overlayWidget::set_opacity(qreal opacity) {
	this->opacity.value = opacity;
	this->opacity.effect->setOpacity(this->opacity.value);
	setGraphicsEffect(this->opacity.effect);
}
void overlayWidget::draw_background(void) {
	draw_background(dpr_rect());
}
void overlayWidget::draw_background(QRectF rect) {
	QPen pen;

	pen.setWidthF(dpr_int(1));
	pen.setColor(base_color.fg);
	painter.setPen(pen);
	painter.setBrush(base_color.bg);
	painter.drawRoundedRect(rect, dpr_radius(), dpr_radius());
}
void overlayWidget::fade_in_animation(void) {
	if (always_visible == true) {
		set_opacity(opacity.value);
		return;
	}
	fade_in.animation->setDuration(fade_in_duration);
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
	fade_out.animation->setDuration(fade_out_duration);
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
qreal overlayWidget::dpr_per_int(int integer) const {
	return ((qreal)integer * devicePixelRatioF());
}
qreal overlayWidget::dpr_per_real(qreal real) const {
	return (real * devicePixelRatioF());
}
qreal overlayWidget::dpr_int(int integer) {
	return ((qreal)integer / devicePixelRatioF());
}
qreal overlayWidget::dpr_real(qreal real) {
	return (real / devicePixelRatioF());
}
QPointF overlayWidget::dpr_point(QPoint point) {
	return (QPointF(point) / devicePixelRatioF());
}
QPointF overlayWidget::dpr_point(int x, int y) {
	return (dpr_point(QPoint(x, y)));
}
QRectF overlayWidget::dpr_rect(void) {
	return (QRectF(dpr_int(rect().x()),
		dpr_int(rect().y()),
		dpr_int(rect().width()),
		dpr_int(rect().height())));
}
qreal overlayWidget::dpr_radius(void) {
	return (dpr_real(radius));
}
QImage overlayWidget::dpr_image(QString path) {
	QImage img = QImage(path);

	img.setDevicePixelRatio(devicePixelRatioF());
	return (img);
}
qreal overlayWidget::dpr_text_real(qreal real) {
// solo sotto windows il testo ha bisogno di una correzione
#if defined (_WIN32)
#if (QT_VERSION > QT_VERSION_CHECK(5, 14, 0)) && (QT_VERSION < QT_VERSION_CHECK(5, 15, 8))
	return (real - (devicePixelRatioF() * 0.375f));
#else
	return (real);
#endif
#else
	return (real);
#endif
}
qreal overlayWidget::dpr_text_coord(qreal coord) {
#if defined (_WIN32)
	if (devicePixelRatioF() != 1.0) {
		return (dpr_text_real(coord));
	}
#endif
	return (coord);
}
QPointF overlayWidget::dpr_text_point(QPointF point) {
#if defined (_WIN32)
	if (devicePixelRatioF() != 1.0) {
		return (QPointF(dpr_text_real(point.x()), dpr_text_real(point.y())));
	}
#endif
	return (point);
}
QRectF overlayWidget::dpr_text_rect(QRectF rect) {
#if defined (_WIN32)
	if (devicePixelRatioF() != 1.0) {
		rect.moveTopLeft(QPointF(dpr_text_real(rect.left()), dpr_text_real(rect.top())));
	}
#endif
	return (rect);
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

QSize overlayWidgetFPS::sizeHint(void) const {
	return (QSize(dpr_per_int(fontMetrics().size(0, "00 fps").width()) + hpadtot(), minimum_eight()));
}
void overlayWidgetFPS::paintEvent(QPaintEvent *event) {
	overlayWidget::paintEvent(event);

	painter.begin(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	draw_background();
	painter.drawText(dpr_text_rect(dpr_rect()), Qt::AlignCenter, QString("%1 fps").arg((int)old.fps));
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

overlayWidgetFloppy::overlayWidgetFloppy(QWidget *parent) : overlayWidget(parent) {};
overlayWidgetFloppy::~overlayWidgetFloppy() {}

QSize overlayWidgetFloppy::sizeHint(void) const {
	return (QSize(floppy.gray.size().width() + hpadtot(), minimum_eight()));
}
void overlayWidgetFloppy::paintEvent(QPaintEvent *event) {
	QPointF coords = QPointF(((qreal)rect().width() - (qreal)(floppy.gray.size().width())) / 2.0f,
		((qreal)rect().height() - (qreal)floppy.gray.size().height()) / 2.0f);

	overlayWidget::paintEvent(event);

	painter.begin(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
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
void overlayWidgetFloppy::update_dpr(void) {
	floppy.gray = dpr_image(":/pics/pics/overlay_floppy_gray.png");
	floppy.red = dpr_image(":/pics/pics/overlay_floppy_red.png");
	floppy.green = dpr_image(":/pics/pics/overlay_floppy_green.png");
}
void overlayWidgetFloppy::update_widget(void) {
	if (cfg->txt_on_screen & fds.info.enabled) {
		show_widget();
	} else {
		hide();
	}
}

// overlayWidgetFrame ------------------------------------------------------------------------------------------------------------

overlayWidgetFrame::overlayWidgetFrame(QWidget *parent) : overlayWidget(parent) {
	old.actual_frame = 0;
	info();
};
overlayWidgetFrame::~overlayWidgetFrame() {}

QSize overlayWidgetFrame::sizeHint(void) const {
	return (QSize(dpr_per_real(td.size().width()) + hpadtot(), minimum_eight()));
}
void overlayWidgetFrame::paintEvent(QPaintEvent *event) {
	overlayWidget::paintEvent(event);

	painter.begin(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	draw_background();

	{
		static QAbstractTextDocumentLayout::PaintContext ctx;

		painter.translate(dpr_text_coord(dpr_int(padding.h)), dpr_text_coord((dpr_rect().height() - td.size().height()) / 2.0f));

		ctx.clip = dpr_rect();
		td.documentLayout()->draw(&painter, ctx);
	}
	painter.end();
}

void overlayWidgetFrame::update_dpr(void) {
	td.setDefaultFont(font());
	td.setDocumentMargin(0.0f);
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
	input_port = 0;
	type = CTRL_DISABLED;
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
		case CTRL_FAMILY_BASIC_KEYBOARD:
			draw_family_basic_keyboard();
			break;
		case CTRL_SUBOR_KEYBOARD:
			draw_subor_keyboard_sb97();
			break;
	}
	painter.end();
}

void overlayWidgetInputPort::update_dpr(void) {
	QFont fnt = font();
	fnt.setPointSizeF(8.0f);
	fnt.setPixelSize(dpr_int(QFontInfo(fnt).pixelSize()));
	setFont(fnt);

	std_controller.tile = dpr_image(":/pics/pics/overlay_controller.png");
	std_controller.up = dpr_image(":/pics/pics/overlay_controller_up.png");
	std_controller.left = dpr_image(":/pics/pics/overlay_controller_left.png");
	std_controller.select = dpr_image(":/pics/pics/overlay_controller_select.png");
	std_controller.start = dpr_image(":/pics/pics/overlay_controller_start.png");
	std_controller.but_b = dpr_image(":/pics/pics/overlay_controller_b.png");
	std_controller.but_a = dpr_image(":/pics/pics/overlay_controller_a.png");

	zapper.tile = dpr_image(":/pics/pics/overlay_zapper.png");
	zapper.bang = dpr_image(":/pics/pics/overlay_zapper_bang.png");

	snes_mouse.tile = dpr_image(":/pics/pics/overlay_mouse.png");
	snes_mouse.button = dpr_image(":/pics/pics/overlay_mouse_button.png");

	arkanoid_paddle.tile = dpr_image(":/pics/pics/overlay_arkanoid_paddle.png");
	arkanoid_paddle.button = dpr_image(":/pics/pics/overlay_arkanoid_paddle_button.png");

	oeka_kids_tablet.tile = dpr_image(":/pics/pics/overlay_oeka_kids_tablet.png");

	family_basic_keyboard.tile = dpr_image(":/pics/pics/overlay_family_basic_keyboard.png");
	subor_keyboard_sb97.tile = dpr_image(":/pics/pics/overlay_subor_keyboard_sb97.png");

	exp_port = dpr_image(":/pics/pics/overlay_controller_exp_port.png");

	set_nport(input_port);

	setFixedSize(std_controller.tile.size());
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
		case CTRL_FAMILY_BASIC_KEYBOARD:
		case CTRL_SUBOR_KEYBOARD:
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
		case CTRL_FAMILY_BASIC_KEYBOARD:
		case CTRL_SUBOR_KEYBOARD:
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
	portx.setDevicePixelRatio(devicePixelRatioF());
}

void overlayWidgetInputPort::draw_std_controller(void) {
	painter.drawImage(dpr_point(0, 0), std_controller.tile);
	painter.drawImage(dpr_point(74, 1), portx);

	if (old.std_controller.data[UP] == PRESSED) {
		painter.drawImage(dpr_point(12, 5), std_controller.up);
	}
	if (old.std_controller.data[DOWN] == PRESSED) {
		painter.drawImage(dpr_point(12, 17), std_controller.up);
	}
	if (old.std_controller.data[LEFT] == PRESSED) {
		painter.drawImage(dpr_point(5, 12), std_controller.left);
	}
	if (old.std_controller.data[RIGHT] == PRESSED) {
		painter.drawImage(dpr_point(16, 12), std_controller.left);
	}
	if (old.std_controller.data[SELECT] == PRESSED) {
		painter.drawImage(dpr_point(34, 18), std_controller.select);
	}
	if (old.std_controller.data[START] == PRESSED) {
		painter.drawImage(dpr_point(50, 18), std_controller.start);
	}
	if (old.std_controller.data[BUT_B] == PRESSED) {
		painter.drawImage(dpr_point(68, 14), std_controller.but_b);
	}
	if (old.std_controller.data[BUT_A] == PRESSED) {
		painter.drawImage(dpr_point(82, 14), std_controller.but_a);
	}
}
void overlayWidgetInputPort::draw_zapper(void) {
	painter.drawImage(0, 0, zapper.tile);

	if (cfg->input.controller_mode == CTRL_MODE_FAMICOM) {
		painter.drawImage(dpr_point(61, 1), exp_port);
	} else {
		painter.drawImage(dpr_point(74, 1), portx);
	}

	draw_mouse_coords();

	if (old.mouse.left) {
		painter.drawImage(dpr_point(52, 2), zapper.bang);
	}
}
void overlayWidgetInputPort::draw_snes_mouse(void) {
	painter.drawImage(dpr_point(0, 0), snes_mouse.tile);
	painter.drawImage(dpr_point(74, 1), portx);

	draw_mouse_coords();

	if (old.mouse.left) {
		painter.drawImage(dpr_point(7, 5), snes_mouse.button);
	}
	if (old.mouse.right) {
		painter.drawImage(dpr_point(11, 5), snes_mouse.button);
	}
}
void overlayWidgetInputPort::draw_arkanoid_paddle(void) {
	painter.drawImage(dpr_point(0, 0), arkanoid_paddle.tile);

	if (cfg->input.controller_mode == CTRL_MODE_FAMICOM) {
		painter.drawImage(dpr_point(61, 1), exp_port);
	} else {
		painter.drawImage(dpr_point(74, 1), portx);
	}

	painter.setPen(Qt::white);
	painter.drawText(dpr_text_point(dpr_point(61, 25)), QString("X: %1").arg(old.mouse.x));

	if (old.mouse.left) {
		painter.drawImage(dpr_point(8, 6), arkanoid_paddle.button);
	}
}
void overlayWidgetInputPort::draw_oeka_kids_tablet(void) {
	painter.drawImage(dpr_point(0, 0), oeka_kids_tablet.tile);

	if (cfg->input.controller_mode == CTRL_MODE_FAMICOM) {
		painter.drawImage(dpr_point(61, 1), exp_port);
	} else {
		painter.drawImage(dpr_point(74, 1), portx);
	}
}
void overlayWidgetInputPort::draw_family_basic_keyboard(void) {
	painter.drawImage(dpr_point(0, 0), family_basic_keyboard.tile);

	if (cfg->input.controller_mode == CTRL_MODE_FAMICOM) {
		painter.drawImage(dpr_point(61, 1), exp_port);
	} else {
		painter.drawImage(dpr_point(74, 1), portx);
	}
}
void overlayWidgetInputPort::draw_subor_keyboard_sb97(void) {
	painter.drawImage(dpr_point(0, 0), subor_keyboard_sb97.tile);

	if (cfg->input.controller_mode == CTRL_MODE_FAMICOM) {
		painter.drawImage(dpr_point(61, 1), exp_port);
	} else {
		painter.drawImage(dpr_point(74, 1), portx);
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
	painter.drawText(dpr_text_point(dpr_point(20, 25)), QString("X: %1").arg(x, 3));
	painter.drawText(dpr_text_point(dpr_point(61, 25)), QString("Y: %1").arg(y, 3));
}

// overlayWidgetRewind -----------------------------------------------------------------------------------------------------------

overlayWidgetRewind::overlayWidgetRewind(QWidget *parent) : overlayWidget(parent) {
	color.corner = QColor(234, 234, 184);
	color.border_bar = QColor(100, 100, 100);
	color.bar = QColor(224, 255, 224);
	color.actual = shared_color.rwnd_actual;
	color.total = QColor(128, 0, 0);
	color.disabled = QColor(180, 180, 180);
};
overlayWidgetRewind::~overlayWidgetRewind() {}

QSize overlayWidgetRewind::sizeHint(void) const {
	return (QSize(100, minimum_eight()));
}
void overlayWidgetRewind::paintEvent(QPaintEvent *event) {
	overlayWidget::paintEvent(event);

	painter.begin(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	info.width = dpr_rect().width();
	draw_background();
	draw_command();
	draw_corner_bar_info();
	painter.end();
}

void overlayWidgetRewind::update_dpr(void) {
	led = font();
	led.setPointSizeF(dpr_int(9.0f));

	td.setDefaultFont(led);
	td.setDocumentMargin(0.0f);

	act.fbackward = svg_to_image(":/icon/icons/rwnd_fast_backward.svg");
	act.backward = svg_to_image(":/icon/icons/rwnd_step_backward.svg");
	act.play = svg_to_image(":/icon/icons/rwnd_play.svg");
	act.pause = svg_to_image(":/icon/icons/rwnd_pause.svg");
	act.forward = svg_to_image(":/icon/icons/rwnd_step_forward.svg");
	act.fforward = svg_to_image(":/icon/icons/rwnd_fast_forward.svg");
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
	QRectF icon, text;
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
			desc = "PAUSE";
			break;
		case RWND_ACT_PLAY:
			if (rwnd.action == RWND_ACT_PAUSE) {
				image = &act.pause;
				desc = "PAUSE";
			} else {
				image = &act.play;
				desc = "PLAY";
			}
			break;
		case RWND_ACT_STEP_BACKWARD:
			image = &act.backward;
			desc = "-1";
			break;
		case RWND_ACT_FAST_BACKWARD:
			if (rwnd.action == RWND_ACT_PAUSE) {
				image = &act.pause;
				desc = "PAUSE";
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
				desc = "PAUSE";
			} else {
				image = &act.fforward;
				desc = QString("+%1x").arg(rwnd.factor.forward);
			}
			break;
	}

	if (image) {
		qreal dpr_w = dpr_int(act.play.width());
		qreal dpr_h = dpr_int(act.play.height());
		qreal x, y, w, h;

		x = dpr_rect().width() - dpr_w - dpr_int(padding.h);
		y = (dpr_rect().height() - dpr_h) / 2.0f;
		w = dpr_w;
		h = dpr_h;

		icon = QRectF(x, y, w, h);
		painter.drawImage(icon.topLeft(), (*image));

		x = 0;
		y = 0;
		w = icon.x() - dpr_int(padding.h);
		h = dpr_rect().height();
		painter.setFont(led);
		text = painter.boundingRect(QRectF(x, y, w, h), Qt::AlignRight | Qt::AlignVCenter, "PAUSE");
		painter.drawText(dpr_text_rect(text), Qt::AlignHCenter | Qt::AlignVCenter, desc);
	}

	info.width = text.x() - dpr_int(hpadtot());

	painter.restore();
}
void overlayWidgetRewind::draw_corner_bar_info(void) {
	static QRectF qr;
	QPen pen = painter.pen();
	qreal vpad = dpr_int(2), hpad = dpr_int(3);
	qreal max = this->max();
	qreal min = this->min();
	qreal value = this->value();
	qreal step = value / (max - min);

	//percentuale
	info.width -= dpr_int(padding.h);
	painter.translate(dpr_int(padding.h), 0);
	qr = painter.boundingRect(QRectF(0, 0, info.width, dpr_rect().height()), Qt::AlignLeft | Qt::AlignVCenter, "000%");
	painter.setPen(base_color.fg);
	painter.drawText(dpr_text_rect(qr), Qt::AlignCenter, QString("%1%").arg((int)(100 * step)));

	// cornice
	info.width -= (qr.width() + dpr_int(padding.h));
	painter.translate(qr.width() + dpr_int(padding.h), 0);
	painter.setBrush(color.corner);
	painter.drawRoundedRect(QRectF(0, 0, info.width, dpr_rect().height()), dpr_radius(), dpr_radius());

	// barra
	if (value > 0) {
		qr.setRect(hpad, vpad, (info.width - (hpad * 2)) * step, dpr_rect().height() - (vpad * 2));
		painter.setPen(color.border_bar);
		painter.setBrush(color.bar);
		painter.drawRoundedRect(qr, dpr_radius(), dpr_radius());
	}

	info.width -= (hpad * 4);
	painter.translate(hpad * 2, 0);

	{
		static QAbstractTextDocumentLayout::PaintContext ctx;

		td.setHtml(info_long());

		if (td.size().width() >= info.width) {
			td.setHtml(info_short());
		}

		painter.translate(dpr_text_coord(0.0f), dpr_text_coord((dpr_rect().height() - td.size().height()) / 2.0f));

		ctx.clip = QRectF(0, 0, info.width, dpr_rect().height());
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
	image.setDevicePixelRatio(devicePixelRatioF());

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
	txt += color_string(")", base_color.fg);

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
	dim_cell_x1 = 16;
	rows = 3;
	columns = ((SAVE_SLOTS % rows) == 0) ? SAVE_SLOTS / rows : (SAVE_SLOTS / rows) + 1;
	max_size = QSize(488, 370);

	radius = 4;
	padding.h = 4;
	padding.v = 4;
};
overlayWidgetSaveSlot::~overlayWidgetSaveSlot() {}

QSize overlayWidgetSaveSlot::sizeHint(void) const {
	if (cfg->scale == X1) {
		qreal ratio = (qreal)SCR_COLUMNS / (qreal)SCR_ROWS;
		qreal width = (SAVE_SLOTS * dim_cell_x1) + hpadtot();

		return (QSize(width, vpadtot() + dim_cell_x1 + padding.v + ((width / ratio))));
	}
	return (max_size);
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

	if (save_slot_operation == SAVE_SLOT_SAVE) {
		gui_overlay_info_append_msg_precompiled(31, &save_slot.slot);
	} else if (save_slot_operation == SAVE_SLOT_READ) {
		gui_overlay_info_append_msg_precompiled(32, &save_slot.slot);
	} else {
		fade_out_start_timer();
		enabled = TRUE;
	}
}
QString overlayWidgetSaveSlot::date_and_time(int slot) {
	return (QLocale::system().toString(previews[slot].fileinfo.lastModified(), QLocale::ShortFormat));
}
QSize overlayWidgetSaveSlot::calc_size(void) {
	return (sizeHint());
}

void overlayWidgetSaveSlot::draw_slots_x1(void) {
	qreal x, y, w, h, radius;
	qreal nw, nh;
	qreal x1, y1;
	static QFont f;
	static QRectF rect;
	static QPen pen;

	painter.save();

	// disegno la riga di selezione dello slot
	nw = (this->rect().width() - hpadtot()) / (qreal)SAVE_SLOTS;
	nh = nw;
	x1 = dpr_real(padding.h + (nw / 2.0f));
	y1 = dpr_real(padding.v + (nh / 2.0f));

	w = dpr_real(nw);
	h = dpr_real(nh);

	radius = w / 2.5f;

	for (unsigned int i = 0; i < SAVE_SLOTS; i++) {
		x = x1 + (i * w);
		y = y1;

		if (save_slot.state[i]) {
			painter.setPen(Qt::NoPen);
			painter.setBrush(QColor(80, 255, 87));
			painter.drawEllipse(QPointF(x, y), radius, radius);
		}
	}

	// disegno il cursore di selezione
	{
		x = x1 + (save_slot.slot * w);
		y = y1;

		pen.setColor(color.x1.selected);
		pen.setWidthF(dpr_int(1));
		
		painter.setPen(pen);
		painter.setBrush(Qt::NoBrush);
		painter.drawEllipse(QPointF(x, y), radius, radius);
	}

	f = font();
	f.setPixelSize(h / 1.75f);

	painter.setFont(f);
	painter.setBrush(Qt::NoBrush);

	for (unsigned int i = 0; i < SAVE_SLOTS; i++) {
		x = x1 + (i * w);
		y = y1;

		rect.setRect(x - radius, y - radius, radius * 2.0f, radius * 2.0f);

		if (save_slot.state[i]) {
			painter.setPen(color.x1.text);
		} else {
			painter.setPen(color.x1.text_not_used);
		}
		painter.drawText(dpr_text_rect(rect), Qt::AlignHCenter | Qt::AlignVCenter, QString::number(i, 16).toUpper());
	}

	// disegno la preview
	x = dpr_int(padding.h);
	y = dpr_real(nh + (2 * padding.v));
	nw = (this->rect().width() - (2 * padding.h));
	nh = (this->rect().height() - nh - (3 * padding.v));

	w = dpr_real(nw);
	h = dpr_real(nh);

	rect.setRect(x, y, w, h);

	if (!previews[save_slot.slot].image.isNull()) {
		painter.drawImage(rect, previews[save_slot.slot].image.scaled(nw, nh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	} else {
		painter.setRenderHint(QPainter::Antialiasing, false);
		painter.setPen(Qt::NoPen);
		painter.setBrush(color.no_preview);
		painter.drawRect(rect);
	}

	// disegno la riga in cui scrivere le info
	rect.setHeight(dpr_int(height_row_slot));

	painter.setOpacity(0.8);
	if (!previews[save_slot.slot].image.isNull()) {
		painter.fillRect(rect, color.bar_selected);
	} else {
		painter.fillRect(rect, color.bar);
	}

	// scrivo le info
	f.setPixelSize(w / 12.0);

	pen.setWidth(dpr_int(1));

	painter.setFont(f);
	painter.setOpacity(1.0);

	pen.setColor(color.slot);
	painter.setPen(pen);
	painter.drawText(dpr_text_rect(rect), Qt::AlignLeft | Qt::AlignVCenter, QString(" %0").arg(save_slot.slot, 1, 16).toUpper());

	pen.setColor(color.info);
	painter.setPen(pen);
	painter.drawText(dpr_text_rect(rect), Qt::AlignHCenter | Qt::AlignVCenter, date_and_time(save_slot.slot));

	painter.restore();
}
void overlayWidgetSaveSlot::draw_slots(void) {
	qreal x, y, w, h;
	static QFont f;
	static QRectF rect;
	static QPen pen;

	painter.save();

	{
		qreal nw, nh;
		qreal x1, y1;

		nw = (this->rect().width() - hpadtot()) / columns;
		nh = (this->rect().height() - vpadtot()) / rows;
		x1 = dpr_real((this->rect().width() - (nw * columns)) / 2.0f);
		y1 = dpr_real((this->rect().height() - (nh * rows)) / 2.0f);

		w = dpr_real(nw);
		h = dpr_real(nh);

		for (unsigned int i = 0; i < SAVE_SLOTS; i++) {
			x = x1 + ((i % columns) * w);
			y = y1 + ((i / columns) * h);

			rect.setRect(x, y, w, h);

			// disegno la preview
			if (!previews[i].image.isNull()) {
				QImage img = previews[i].image.scaled(nw, nh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
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
			pen.setWidthF(dpr_int(padding.v / 2.0f));

			painter.setPen(pen);
			painter.setBrush(Qt::NoBrush);
			painter.drawRect(rect);

			// disegno la riga in cui scrivere le info
			rect.setRect(x + (pen.widthF() / 2.0f), y, w - pen.widthF(), dpr_real(height_row_slot));

			painter.setOpacity(0.8);
			if (i == save_slot.slot) {
				painter.fillRect(rect, color.bar_selected);
			} else {
				painter.fillRect(rect, color.bar);
			}

			// scrivo le info
			f = font();
			f.setPixelSize(w / 12.0);

			pen.setWidthF(dpr_int(1));

			painter.setFont(f);
			painter.setOpacity(1.0);

			pen.setColor(color.slot);
			painter.setPen(pen);
			painter.drawText(dpr_text_rect(rect), Qt::AlignLeft | Qt::AlignVCenter, QString(" %0").arg(i, 1, 16).toUpper());

			pen.setColor(color.info);
			painter.setPen(pen);
			painter.drawText(dpr_text_rect(rect), Qt::AlignHCenter | Qt::AlignVCenter, date_and_time(i));
		}

		// disegno il cursore di selezione dello slot
		{
			x = x1 + ((save_slot.slot % columns) * w);
			y = y1 + ((save_slot.slot / columns) * h);

			rect.setRect(x, y, w, h);

			pen.setColor(color.border_selected);
			pen.setWidthF(dpr_int(2));

			painter.setPen(pen);
			painter.setBrush(Qt::NoBrush);
			painter.drawRect(rect);
		}
	}

	painter.restore();
}

// overlayWidgetInfo -------------------------------------------------------------------------------------------------------------

overlayWidgetInfo::overlayWidgetInfo(QWidget *parent) : overlayWidget(parent) {
	//base_color.bg = QColor(125, 125, 125);
	base_color.bg = QColor(50, 50, 50);

	force_control_when_hidden = true;
	new_management = true;
	padding.h = 3;
	sec_for_word = 0.375f;
};
overlayWidgetInfo::~overlayWidgetInfo() {}

QSize overlayWidgetInfo::sizeHint(void) const {
	return (QSize(100, minimum_eight(NULL, 2)));
}
void overlayWidgetInfo::paintEvent(QPaintEvent *event) {
	if (!isHidden()) {
		int alignment, len;
		QString actual;

		overlay.info.mutex.lock();
		actual = overlay.info.actual;
		len = overlay.info.actual.length();
		alignment = overlay.info.alignment;
		overlay.info.mutex.unlock();

		if (len) {
		    static QTextOption to;
			qreal font_height = QFontMetrics(font_info).height();
			QTextDocument td;
			qreal x, y, w, h, lines = 1;

			overlayWidget::paintEvent(event);

			painter.begin(this);
			painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

			to.setWrapMode(QTextOption::WordWrap);
			to.setAlignment(Qt::AlignCenter);
			td.setDefaultFont(font_info);
			td.setDocumentMargin(0.0f);
			td.setDefaultTextOption(to);
			td.setHtml(actual);

			if (td.size().width() > dpr_rect().width()) {
				int i, divider = 20;
				qreal piece = (dpr_rect().width() / (qreal)divider);

				for (i = 5; i < divider; i++) {
					w = (piece * (qreal)i) - dpr_int(hpadtot());
					td.setTextWidth(w);
					if ((td.size().height() / font_height) <= 2.0f) {
						lines = 2;
						break;
					}
				}
			} else {
				w = td.size().width();
				td.setTextWidth(w);
			}

			switch (alignment) {
				default:
				case OVERLAY_INFO_LEFT:
					x = 0;
					break;
				case OVERLAY_INFO_CENTER:
					x = (dpr_rect().width() - td.size().width()) / 2.0f;
					break;
				case OVERLAY_INFO_RIGHT:
					x = (dpr_rect().width() - td.size().width());
					break;
			}

			w += dpr_int(hpadtot());

			if (lines == 1) {
				y = dpr_rect().height() / 2.0f;
				h = dpr_rect().height() / 2.0f;
			} else {
				y = 0;
				h = dpr_rect().height();
			}

			draw_background(QRectF(x, y, w, h));

			painter.translate(dpr_text_coord(x + dpr_int(padding.h)), dpr_text_coord(y + ((h - td.size().height()) / 2.0f)));
			td.drawContents(&painter, QRectF(0, 0, w, h));

			painter.end();
		}
	}

	fade_out_tick_timer();
}

void overlayWidgetInfo::update_dpr(void) {
	font_info = font();
	font_info.setPointSizeF(dpr_int(9.0f));
}
BYTE overlayWidgetInfo::is_to_redraw(void) {
	// nuova gestione
	if (new_management == true) {
		QString actual;

		if (overlay.info.messages_to_draw.length() > 0) {
			overlay.info.mutex.lock();
			overlay.info.alignment = overlay.info.messages_to_draw[0].alignment;
			overlay.info.actual = overlay.info.messages_to_draw[0].msg;
			overlay.info.mutex.unlock();
			overlay.info.messages_to_draw.removeAt(0);
			fade_in_animation();
			enabled = true;
			return (TRUE);
		}

		overlay.info.mutex.lock();
		actual = overlay.info.actual;
		overlay.info.mutex.unlock();

		if ((actual != "") || (fade_out.timer.enabled == true)) {
			return (TRUE);
		}

		enabled = false;
		return (FALSE);
	}

	// vecchia gestione
	if (overlay.info.actual == "") {
		if (fade_out.timer.enabled == true) {
			return (TRUE);
		}
		if (overlay.info.messages_to_draw.length() == 0) {
			enabled = false;
			return (FALSE);
		}
		overlay.info.mutex.lock();
		overlay.info.alignment = overlay.info.messages_to_draw[0].alignment;
		overlay.info.actual = overlay.info.messages_to_draw[0].msg;
		overlay.info.messages_to_draw.removeAt(0);
		overlay.info.mutex.unlock();
		fade_in_animation();
		enabled = true;
	}
	return (TRUE);
}
void overlayWidgetInfo::fade_in_animation(void) {
	// nuova gestione
	if (new_management == true) {
		QString actual;

		if (always_visible == true) {
			set_opacity(opacity.value);
			return;
		}

		overlay.info.mutex.lock();
		actual = overlay.info.actual;
		overlay.info.mutex.unlock();

		if (actual != "") {
			s_fade_in_finished();
			return;
		}

		fade_in.animation->setDuration(500);
		fade_in.animation->setStartValue(0);
		fade_in.animation->setEndValue(opacity.value);
		fade_in.animation->setEasingCurve(QEasingCurve::InBack);
		fade_in.animation->start();
		connect(fade_in.animation, SIGNAL(finished()), this, SLOT(s_fade_in_finished()));
		return;
	}

	// vecchia gestione
	overlayWidget::fade_in_animation();
}
void overlayWidgetInfo::append_msg(BYTE alignment, QString msg) {
	_append_msg(alignment, msg);
	enabled = true;
}
void overlayWidgetInfo::_append_msg(BYTE alignment, QString msg) {
	msg = decode_tags("[white]" + msg + "[normal]");
	overlay.info.messages_to_draw.append(_overlay_info_message { msg, alignment });
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
	QTextDocument td;
	QString actual;
	int words, sec;

	fade_out_start_timer();

	overlay.info.mutex.lock();
	actual = overlay.info.actual;
	overlay.info.mutex.unlock();

	td.setDefaultFont(font_info);
	td.setDocumentMargin(0.0f);
	td.setHtml(actual);

	words = td.toPlainText().split(QRegularExpression("(\\s|\\n|\\r)+")).count();
	sec = ceil(sec_for_word * (double)words);
	fade_out.timer.seconds = sec < 3 ? 3 : sec;
}
void overlayWidgetInfo::s_fade_out_finished(void) {
	overlay.info.mutex.lock();
	overlay.info.alignment = OVERLAY_INFO_LEFT;
	overlay.info.actual = "";
	overlay.info.mutex.unlock();
	set_opacity(opacity.value);
}

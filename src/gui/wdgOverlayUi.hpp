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

#ifndef WDGOVERLAYUI_HPP_
#define WDGOVERLAYUI_HPP_

#include <QtCore/QPropertyAnimation>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QLabel>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include "common.h"
#include "clock.h"

class overlayWidget : public QWidget {
	Q_OBJECT

	public:
		class _animation {
			public:
				struct _timer {
					bool enabled = false;
					time_t start = 0;
					int seconds = 0;
				} timer;
				QPropertyAnimation *animation;
		};
		struct _exchange_data {
			bool draw;
			QImage img;
			QRect last_geometry;
		} exchange;
		struct _paddings {
			int h;
			int v;
		} padding;
		struct _base_colors {
			QColor fg;
			QColor bg;
		} base_color;
		struct _opacity_data {
			QGraphicsOpacityEffect *effect;
			qreal value;
		} opacity;
		_animation fade_in;
		_animation fade_out;
		double ms_last_draw;
		bool enabled;
		bool force_control_when_hidden;
		qreal radius;

	public:
		overlayWidget(QWidget *parent);
		~overlayWidget();

	protected:
		void paintEvent(QPaintEvent *event);

	public:
		virtual void update_widget(void);
		virtual BYTE is_to_redraw(void);
		virtual void update_old_value(void);
		void show_widget(void);
		int hpadtot(void) const;
		int vpadtot(void) const;
		int minimum_eight(void) const;
		void set_opacity(qreal opacity);
		void draw_background(QPainter *painter);
		void draw_background(QPainter *painter, QRect rect);
		void fade_in_animation(void);
		void fade_out_animation(void);
		void fade_out_start_timer(void);
		void fade_out_tick_timer(void);

	public slots:
		virtual void s_fade_in_finished(void);
		virtual void s_fade_out_finished(void);
};
class overlayWidgetFPS : public overlayWidget {
	private:
		struct _old_values {
			double fps;
		} old;

	public:
		overlayWidgetFPS(QWidget *parent);
		~overlayWidgetFPS();

	protected:
		QSize sizeHint() const;
		void paintEvent(QPaintEvent *event);

	public:
		void update_widget(void);
		BYTE is_to_redraw(void);
		void update_old_value(void);
};
class overlayWidgetFloppy : public overlayWidget {
	private:
		struct _images_floppy {
			QImage gray;
			QImage red;
			QImage green;
		} floppy;

	public:
		overlayWidgetFloppy(QWidget *parent);
		~overlayWidgetFloppy();

	protected:
		QSize sizeHint() const;
		void paintEvent(QPaintEvent *event);

	public:
		void update_widget(void);
};
class overlayWidgetInputPort : public overlayWidget {
	public:
		int input_port;

	private:
		struct _old_values {
			struct _old_values_std_controller {
				BYTE data[8];
			} std_controller;
			struct _old_values_mouse {
				uint8_t left;
				uint8_t right;
				int x;
				int y;
			} mouse;
		} old;
		struct _images_std_controller {
			QImage tile;
			QImage up;
			QImage left;
			QImage select;
			QImage start;
			QImage but_a;
			QImage but_b;
		} std_controller;
		struct _images_zapper {
			QImage tile;
			QImage bang;
		} zapper;
		struct _images_snes_mouse {
			QImage tile;
			QImage button;
		} snes_mouse;
		struct _images_arkanoid_paddle {
			QImage tile;
			QImage button;
		} arkanoid_paddle;
		struct _images_oeka_kids_tablet {
			QImage tile;
		} oeka_kids_tablet;
		QImage portx;
		QImage exp_port;
		int type;

	public:
		overlayWidgetInputPort(QWidget *parent);
		~overlayWidgetInputPort();

	protected:
		void paintEvent(QPaintEvent *event);

	public:
		void update_widget(void);
		BYTE is_to_redraw(void);
		void update_old_value(void);
		void set_nport(int nport);

	private:
		void draw_std_controller(QPainter *painter);
		void draw_zapper(QPainter *painter);
		void draw_snes_mouse(QPainter *painter);
		void draw_arkanoid_paddle(QPainter *painter);
		void draw_oeka_kids_tablet(QPainter *painter);
		void draw_mouse_coords(QPainter *painter);
};
class overlayWidgetRewind : public overlayWidget {
	private:
		struct _images_action {
			QImage fbackward;
			QImage backward;
			QImage play;
			QImage pause;
			QImage forward;
			QImage fforward;
		} act;
		QFont led;

	protected:
		class _infotime {
			public:
				enum _measure { HH, MM, SS, MS };
				int hh = 0;
				int mm = 0;
				int ss = 0;
				int ms = 0;
				_measure max = _measure::MS;

			public:
				_infotime(int seconds) {
					int fseconds = seconds / machine.fps;

					hh = (fseconds / 3600);
					mm = (fseconds - (3600 * hh)) / 60;
					ss = (fseconds - (3600 * hh) - (mm * 60));
					ms = (seconds % machine.fps) * (100 / machine.fps);

					if (hh > 0) {
						max = _measure::HH;
						return;
					}
					if (mm > 0) {
						max = _measure::MM;
						return;
					}
					if (ss > 0) {
						max = _measure::SS;
						return;
					}
				}
		};
		struct _old_values {
			BYTE action;
			BYTE action_before_pause;
			int32_t actual_frame;
			int32_t max_frames;
		} old;
		struct _internal_colors {
			QColor corner;
			QColor border_bar;
			QColor bar;
			QColor actual;
			QColor total;
			QColor disabled;
		} color;
		struct _info {
			int width;
		} info;

	public:
		overlayWidgetRewind(QWidget *parent);
		~overlayWidgetRewind();

	protected:
		QSize sizeHint() const;
		void paintEvent(QPaintEvent *event);

	public:
		void update_widget(void);
		BYTE is_to_redraw(void);
		void update_old_value(void);
		QString color_string(QString string, QColor color);
		QString seconds_to_string(_infotime *itime, _infotime::_measure max, QColor color);
		void draw_command(QPainter *painter);
		void draw_corner_bar_info(QPainter *painter);

	protected:
		virtual int32_t min(void);
		virtual int32_t max(void);
		virtual int32_t value(void);
		virtual QString info_long(void);
		virtual QString info_short(void);

	private:
		QImage svg_to_image(QString resource);
};
class overlayWidgetTAS : public overlayWidgetRewind {
	private:
		QColor lag_color;

	public:
		overlayWidgetTAS(QWidget *parent);
		~overlayWidgetTAS();

	public:
		void update_widget(void);
		BYTE is_to_redraw(void);
		void update_old_value(void);
		int32_t min(void);
		int32_t max(void);
		int32_t value(void);
		QString info_long(void);
		QString info_short(void);
};
class overlayWidgetSaveSlot : public overlayWidget {
	private:
		BYTE save_slot_operation;

	public:
		overlayWidgetSaveSlot(QWidget *parent);
		~overlayWidgetSaveSlot();

	protected:
		QSize sizeHint() const;
		void paintEvent(QPaintEvent *event);

	public:
		void enable_overlay(BYTE operation);

	private:
		void draw_slots(QPainter *painter);
};
class overlayWidgetInfo : public overlayWidget {
	Q_OBJECT

	private:
		typedef struct _tags {
			QString name;
			QString value;
		} _tags;

	public:
		overlayWidgetInfo(QWidget *parent);
		~overlayWidgetInfo();

	protected:
		QSize sizeHint() const;
		void paintEvent(QPaintEvent *event);

	public:
		BYTE is_to_redraw(void);
		void append_msg(QString msg);
		static void _append_msg(QString msg);
		static QString decode_tags(QString string);

	public slots:
		void s_fade_in_finished(void);
		void s_fade_out_finished(void);
};

#include "wdgOverlayUi.hh"

class wdgOverlayUi : public QWidget, public Ui::wdgOverlayUi {
	Q_OBJECT

	public:
		void *clear;
		bool force_redraw;
		BYTE update_texture;

	public:
		wdgOverlayUi(QWidget *parent = 0);
		~wdgOverlayUi();

	protected:
		void changeEvent(QEvent *event);
		void resizeEvent(QResizeEvent *event);
		void closeEvent(QCloseEvent *event);

	public:
		void retranslateUi(QWidget *wdgOverlayUi);
		void update_widget(void);
};

typedef struct _overlay_data {
	wdgOverlayUi *widget;
	QList<QString> info_messages_to_draw;
	QString info_actual_message = "";
} _overlay_data;

extern _overlay_data overlay;

#endif /* WDGOVERLAYUI_HPP_ */

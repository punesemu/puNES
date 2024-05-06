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

#ifndef WDGOVERLAYUI_HPP_
#define WDGOVERLAYUI_HPP_

#include <QtCore/QFileInfo>
#include <QtCore/QPropertyAnimation>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QLabel>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtGui/QTextDocument>
#include "common.h"
#include "clock.h"
#include "save_slot.h"

// overlayWidget -------------------------------------------------------------------------------------------------------

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
				QPropertyAnimation *animation{};
		};
		struct _exchange_data {
			bool draw;
			QImage img;
			QRect last_geometry;
		} exchange;
		struct _paddings {
			int h;
			int v;
		} padding{};
		struct _base_colors {
			QColor fg;
			QColor bg;
		} base_color;
		struct _opacity_data {
			QGraphicsOpacityEffect *effect;
			qreal value;
		} opacity{};
		_animation fade_in;
		_animation fade_out;
		double ms_last_draw{};
		int fade_in_duration;
		int fade_out_duration;
		bool enabled;
		bool force_disable;
		bool force_control_when_hidden;
		bool always_visible;
		qreal radius;
		QPainter painter;

	public:
		explicit overlayWidget(QWidget *parent = nullptr);
		~overlayWidget() override;

	protected:
		void paintEvent(QPaintEvent *event) override;

	public:
		virtual void update_dpr(void);
		virtual void update_widget(void);
		virtual BYTE is_to_redraw(void);
		virtual void update_old_value(void);

	public:
		void show_widget(void);
		int hpadtot(void) const;
		int vpadtot(void) const;
		int minimum_eight(const QFont *font, int rows) const;
		void set_opacity(qreal value);
		void draw_background(void);
		void draw_background(QRectF rect);
		virtual void fade_in_animation(void);
		void fade_out_animation(void);
		void fade_out_start_timer(void);
		void fade_out_tick_timer(void);
		QString color_string(const QString &string, const QColor &color);
		qreal dpr_per_int(int integer) const;
		qreal dpr_per_real(qreal real) const;
		qreal dpr_int(int integer);
		qreal dpr_real(qreal real);
		QPointF dpr_point(int x, int y);
		QPointF dpr_point(QPoint point);
		QRectF dpr_rect(void);
		qreal dpr_radius(void);
		QImage dpr_image(const QString &path);
		qreal dpr_text_real(qreal real);
		QPointF dpr_text_point(QPointF point);
		QRectF dpr_text_rect(QRectF rect);
		qreal dpr_text_coord(qreal coord);

	public slots:
		virtual void s_fade_in_finished(void);
		virtual void s_fade_out_finished(void);
};

// overlayWidgetFPS ----------------------------------------------------------------------------------------------------

class overlayWidgetFPS : public overlayWidget {
	private:
		struct _old_values {
			double fps;
		} old{};

	public:
		explicit overlayWidgetFPS(QWidget *parent = nullptr);
		~overlayWidgetFPS() override;

	protected:
		QSize sizeHint(void) const override;
		void paintEvent(QPaintEvent *event) override;

	public:
		void update_widget(void) override;
		BYTE is_to_redraw(void) override;
		void update_old_value(void) override;

	private:
		double fps_value(void);
};

// overlayWidgetFrame --------------------------------------------------------------------------------------------------

class overlayWidgetFrame : public overlayWidget {
	private:
		struct _old_values {
			uint32_t actual_frame;
		} old{};
		QTextDocument td;

	public:
		explicit overlayWidgetFrame(QWidget *parent = nullptr);
		~overlayWidgetFrame() override;

	protected:
		QSize sizeHint(void) const override;
		void paintEvent(QPaintEvent *event) override;

	public:
		void update_dpr(void) override;
		void update_widget(void) override;
		BYTE is_to_redraw(void) override;
		void update_old_value(void) override;

	private:
		void update_info(void);
};

// overlayWidgetFastForward --------------------------------------------------------------------------------------------

class overlayWidgetFastForward : public overlayWidget {
	private:
		QImage icon;

	public:
		explicit overlayWidgetFastForward(QWidget *parent = nullptr);
		~overlayWidgetFastForward() override;

	protected:
		QSize sizeHint(void) const override;
		void paintEvent(QPaintEvent *event) override;

	public:
		void update_dpr(void) override;
		void update_widget(void) override;
};

// overlayWidgetFloppy -------------------------------------------------------------------------------------------------

class overlayWidgetFloppy : public overlayWidget {
	private:
		struct _images_motor {
			QImage on;
			QImage off;
			QImage read;
			QImage write;
		} motor;

	public:
		explicit overlayWidgetFloppy(QWidget *parent = nullptr);
		~overlayWidgetFloppy() override;

	protected:
		QSize sizeHint(void) const override;
		void paintEvent(QPaintEvent *event) override;

	public:
		void update_dpr(void) override;
		void update_widget(void) override;
};

// overlayWidgetInputPort ----------------------------------------------------------------------------------------------

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
		} old{};
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
		struct _images_family_basic_keyboard {
			QImage tile;
		} family_basic_keyboard;
		struct _images_subor_keyboard_sb97 {
			QImage tile;
		} subor_keyboard_sb97;
		QImage portx;
		QImage exp_port;
		int type;

	public:
		explicit overlayWidgetInputPort(QWidget *parent = nullptr);
		~overlayWidgetInputPort() override;

	protected:
		void paintEvent(QPaintEvent *event) override;

	public:
		void update_dpr(void) override;
		void update_widget(void) override;
		BYTE is_to_redraw(void) override;
		void update_old_value(void) override;

	public:
		void set_nport(int nport);

	private:
		void draw_std_controller(void);
		void draw_zapper(void);
		void draw_snes_mouse(void);
		void draw_arkanoid_paddle(void);
		void draw_oeka_kids_tablet(void);
		void draw_family_basic_keyboard(void);
		void draw_subor_keyboard_sb97(void);
		void draw_mouse_coords(void);
};

// overlayWidgetRewind -------------------------------------------------------------------------------------------------

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
		QTextDocument td;
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
				explicit _infotime(int seconds) {
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
		} old{};
		struct _internal_colors {
			QColor corner;
			QColor border_bar;
			QColor bar;
			QColor actual;
			QColor total;
			QColor disabled;
		} color;
		struct _info_dim {
			qreal width;
		} info_dim{};

	public:
		explicit overlayWidgetRewind(QWidget *parent = nullptr);
		~overlayWidgetRewind() override;

	protected:
		QSize sizeHint(void) const override;
		void paintEvent(QPaintEvent *event) override;

	public:
		void update_dpr(void) override;
		void update_widget(void) override;
		BYTE is_to_redraw(void) override;
		void update_old_value(void) override;

	protected:
		virtual int32_t min(void);
		virtual int32_t max(void);
		virtual int32_t value(void);
		virtual QString info_long(void);
		virtual QString info_short(void);

	public:
		QString seconds_to_string(_infotime *itime, _infotime::_measure max, const QColor &clr);
		void draw_command(void);
		void draw_corner_bar_info(void);

	private:
		QImage svg_to_image(const QString &resource);
};

// overlayWidgetTAS ----------------------------------------------------------------------------------------------------

class overlayWidgetTAS : public overlayWidgetRewind {
	public:
		explicit overlayWidgetTAS(QWidget *parent = nullptr);
		~overlayWidgetTAS() override;

	public:
		void update_widget(void) override;
		BYTE is_to_redraw(void) override;
		void update_old_value(void) override;

	protected:
		int32_t min(void) override;
		int32_t max(void) override;
		int32_t value(void) override;
		QString info_long(void) override;
		QString info_short(void) override;
};

// overlaySaveSlot -----------------------------------------------------------------------------------------------------

class overlayWidgetSaveSlot : public overlayWidget {
	private:
		BYTE save_slot_operation;
		int rows;
		int columns;
		QSize max_size;

	public:
		struct _previews {
			QImage image;
			QFileInfo fileinfo;
		} previews[SAVE_SLOTS_TOTAL];
		struct _internal_colors {
			struct _internal_colors_x1 {
				QColor save;
				QColor read;
				QColor selected;
				QColor text;
				QColor text_not_used;
			} x1;
			QColor no_preview;
			QColor previw_opacity;
			QColor border;
			QColor border_selected;
			QColor bar;
			QColor bar_selected;
			QColor slot;
			QColor info;
		} color;
		qreal height_row_slot;
		qreal dim_cell_x1;

	public:
		explicit overlayWidgetSaveSlot(QWidget *parent = nullptr);
		~overlayWidgetSaveSlot() override;

	protected:
		QSize sizeHint(void) const override;
		void paintEvent(QPaintEvent *event) override;

	public:
		void enable_overlay(BYTE operation);
		QString date_and_time(int slot);
		QSize calc_size(void);

	private:
		void draw_slots_x1(void);
		void draw_slots(void);
};

// overlayWidgetInfo ---------------------------------------------------------------------------------------------------

class overlayWidgetInfo : public overlayWidget {
		Q_OBJECT

	private:
		typedef struct _tags {
			QString name;
			QString value;
		} _tags;
		bool new_management;
		double sec_for_word;
		QFont font_info;

	public:
		explicit overlayWidgetInfo(QWidget *parent = nullptr);
		~overlayWidgetInfo() override;

	protected:
		QSize sizeHint(void) const override;
		void paintEvent(QPaintEvent *event) override;

	public:
		void update_dpr(void) override;
		BYTE is_to_redraw(void) override;
		void fade_in_animation(void) override;

	public:
		void append_msg(BYTE alignment, const QString &msg);
		static void _append_msg(BYTE alignment, const QString &msg);
		static QString decode_tags(QString string);

	public slots:
		void s_fade_in_finished(void) override;
		void s_fade_out_finished(void) override;
};

// wdgOverlayUi --------------------------------------------------------------------------------------------------------

#include "ui_wdgOverlayUi.h"

class wdgOverlayUi : public QWidget, public Ui::wdgOverlayUi {
	Q_OBJECT

	private:
		QList<overlayWidget *> wdgs;

	public:
		void *clear;
		bool force_redraw;
		BYTE update_texture;

	public:
		explicit wdgOverlayUi(QWidget *parent = nullptr);
		~wdgOverlayUi() override;

	protected:
		void changeEvent(QEvent *event) override;
		void resizeEvent(QResizeEvent *event) override;
		void closeEvent(QCloseEvent *event) override;

	public:
		void retranslateUi(QWidget *wdgOverlayUi);
		void update_dpr(void);
		void update_widget(void);
		void overlay_blit(void);

	private:
		void wdg_clear(overlayWidget *wdg, QRect *qrect, qreal dpr);
};

#endif /* WDGOVERLAYUI_HPP_ */

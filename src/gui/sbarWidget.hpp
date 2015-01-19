/*
 * sbarWidget.hpp
 *
 *  Created on: 13/nov/2014
 *      Author: fhorse
 */

#ifndef SBARWIDGET_HPP_
#define SBARWIDGET_HPP_

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QWidget>
#include <QtGui/QHBoxLayout>
#include <QtGui/QSlider>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QStyledItemDelegate>
#include <QtGui/QComboBox>
#include <QtGui/QStatusBar>
#else
#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSlider>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QStatusBar>
#endif
#include "application.hh"
#include "common.h"

class timelineSlider: public QSlider {
	Q_OBJECT

	public:
		int szHandle;

	public:
		timelineSlider(QWidget *parent);
		~timelineSlider();
		int sizeHandle();
};
class timeLine: public QWidget {
	Q_OBJECT

	private:
		QHBoxLayout *hbox;
		timelineSlider *slider;
		QLabel *label;
		QString lab_timeline;

	public:
		timeLine(QWidget *parent);
		~timeLine();
		int value();
		void setValue(int value, bool s_action);
		void timeline_pressed(BYTE *type);
		void timeline_released(BYTE *type);

	protected:
		bool eventFilter(QObject *obj, QEvent *event);

	private:
		void timeline_update_label(int value);

	private slots:
		void s_action_triggered(int action);
		void s_value_changed(int value);
		void s_slider_pressed();
		void s_slider_released();
};

class slotItemDelegate: public QStyledItemDelegate {
	Q_OBJECT

	public:
		slotItemDelegate(QObject *parent);
		~slotItemDelegate();

	protected:
		void paint(QPainter *painter, const QStyleOptionViewItem &option,
				const QModelIndex &index) const;
};
class slotComboBox: public QComboBox {
	Q_OBJECT

	public:
		slotComboBox(QWidget *parent);
		~slotComboBox();

	private:
		slotItemDelegate *sid;

	protected:
		void paintEvent(QPaintEvent *event);
};
class stateWidget: public QWidget {
	Q_OBJECT

	public:
		Ui::mainWindow *ui;
		QHBoxLayout *hbox;
		QPushButton *save;
		slotComboBox *slot;
		QPushButton *load;

	public:
		stateWidget(Ui::mainWindow *u, QWidget *parent);
		~stateWidget();

	protected:
		bool eventFilter(QObject *obj, QEvent *event);

	private slots:
		void s_save_clicked(bool checked);
		void s_slot_activated(int index);
		void s_load_clicked(bool checked);
};

class sbarWidget: public QStatusBar {
	Q_OBJECT

	public:
		QWidget *spacer;
		timeLine *timeline;
		stateWidget *state;

	public:
		sbarWidget(Ui::mainWindow *u, QWidget *parent);
		~sbarWidget();

		void update_statusbar();
		void update_width(int w);
};

#endif /* SBARWIDGET_HPP_ */

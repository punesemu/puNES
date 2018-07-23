/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#include "pMenu.moc"
#include "pStyle.hpp"
#include "conf.h"

class pMenuSharedInfo {
	public:
		bool mousePressed;
		int lastKey;
	public:
		pMenuSharedInfo() {
			mousePressed = false;
			lastKey = 0;
		}
		~pMenuSharedInfo() {};
} pmshare;

pMenu::pMenu(QWidget *parent = 0) : QMenu(parent) {
	newMenagement = false;
	hasCheckableItems = false;
	fromMousePressed = false;
	tabWidth = maxIconWidth = 0;
	lastAction = 0;

	setStyle(new pStyle());
	permitDisabledAction = style()->styleHint(QStyle::SH_Menu_AllowActiveAndDisabled);
}
pMenu::~pMenu() {}
void pMenu::showEvent(QShowEvent *event) {
	if (execQMenuEvent()) {
		pmshare.lastKey = 0;
		QMenu::showEvent(event);
		return;
	}

	if ((pmshare.lastKey == Qt::Key_Left) || (pmshare.lastKey == Qt::Key_Right)) {
		setFirstActionActive();
	} else {
		resetLastAction();
	}
	pmshare.lastKey = 0;
	QMenu::showEvent(event);
}
void pMenu::hideEvent(QHideEvent *event) {
	if (execQMenuEvent()) {
		QMenu::hideEvent(event);
		return;
	}

	QMenu::hideEvent(event);
	fromMousePressed = false;
	resetLastAction();
}
void pMenu::mouseMoveEvent(QMouseEvent *event) {
	if (execQMenuEvent()) {
		QMenu::mouseMoveEvent(event);
		return;
	}

	if (!rect().contains(mapFromGlobal(event->globalPos()))) {
		QObject *caused = parent();
		QMenuBar *mb = NULL;
		pMenu *m = this;

		while (caused) {
			if ((mb = qobject_cast<QMenuBar*>(caused))) {
				pMenu *hideActiveMenu = m;

				caused = 0;
				if (mb->rect().contains(mb->mapFromGlobal(event->globalPos()))) {
					QAction *mbAction = mb->actionAt(mb->mapFromGlobal(event->globalPos()));

					if (mbAction) {
						pMenu *mbActionMenu = qobject_cast<pMenu*>(mbAction->menu());

						if (hideActiveMenu != mbActionMenu) {
							hideToMenubar();
							mb->setActiveAction(mbAction);
						}
					}
				}
			} else if ((m = qobject_cast<pMenu*>(caused))) {
				if (m->rect().contains(m->mapFromGlobal(event->globalPos()))) {
					caused = 0;
				} else {
					caused = m->parent();
				}
			} else {
				caused = 0;
			}
		}
		return;
	}

	QAction *action = actionAt(event->pos());

	if (!action || action->isSeparator() || isHidden()) {
		return;
	} else {
		if (permitDisabledAction ? true : action->isEnabled())  {
			lastAction = action;
			setCurrentAction(lastAction);
			update();
		}
	}
}
void pMenu::mousePressEvent(QMouseEvent *event) {
	pmshare.mousePressed = true;

	if (execQMenuEvent()) {
		QMenu::mousePressEvent(event);
		return;
	}

	if (!rect().contains(mapFromGlobal(event->globalPos()))) {
		QObject *caused = parent();
		QMenuBar *mb = NULL;
		pMenu *m = this;

		if ((mb = qobject_cast<QMenuBar*>(caused))) {
			QMenu::mousePressEvent(event);
			return;
		}

		if (controlMenuParent(caused, event->globalPos())) {
			hide();
		}
		while (caused) {
			if ((mb = qobject_cast<QMenuBar*>(caused))) {
				pMenu *hideActiveMenu = m;

				caused = 0;
				if (!hideActiveMenu && mb->activeAction()) {
					hideActiveMenu = qobject_cast<pMenu*>(mb->activeAction()->menu());
				}
				if (hideActiveMenu) {
					hideActiveMenu->hide();
				}
				if (mb->rect().contains(mb->mapFromGlobal(event->globalPos()))) {
					QAction *mbAction = mb->actionAt(mb->mapFromGlobal(event->globalPos()));

					if (mbAction) {
						pMenu *mbActionMenu = qobject_cast<pMenu*>(mbAction->menu());

						if (hideActiveMenu != mbActionMenu) {
							mb->setActiveAction(mbAction);
						}
					}
				}
			} else if ((m = qobject_cast<pMenu*>(caused))) {
				if (m->rect().contains(m->mapFromGlobal(event->globalPos()))) {
					caused = 0;
					m->mousePressEvent(event);
				} else {
					caused = m->parent();
					if (controlMenuParent(caused, event->globalPos())) {
						m->hide();
					}
				}
			} else {
				caused = 0;
			}
		}
		return;
	}

	QAction *action = actionAt(mapFromGlobal(event->globalPos()));

	if (!action || action->isSeparator() || isHidden()) {
		return;
	} else {
		if (lastAction && (lastAction != action) && lastAction->menu() &&
				!lastAction->menu()->isHidden()) {
			lastAction->menu()->hide();
		}
		lastAction = action;
		if (lastAction->isEnabled() && lastAction->menu()) {
			if (lastAction->menu()->isHidden()) {
				pmshare.lastKey = 0;
				qobject_cast<pMenu*>(lastAction->menu())->setFromMousePressed(true);
				setCurrentAction(lastAction);
				lastAction->menu()->popup(actionPoint[actions().indexOf(lastAction)]);
			} else {
				lastAction->menu()->hide();
			}
		} else {
			setActiveAction(lastAction);
		}
		update();
	}
}
void pMenu::mouseReleaseEvent(QMouseEvent *event) {
	pmshare.mousePressed = false;

	if (execQMenuEvent()) {
		QMenu::mouseReleaseEvent(event);
		return;
	}

	QAction *action = actionAt(event->pos());

	if (!action || action->isSeparator() || isHidden()) {
		;
	} else {
		lastAction = action;
		if (triggerLastActionAndHide()) {
			return;
		}
	}
	update();
}
void pMenu::keyPressEvent(QKeyEvent *event) {
	pmshare.lastKey = event->key();

	if (execQMenuEvent()) {
		QMenu::keyPressEvent(event);
		return;
	}

	switch (event->key()) {
		case Qt::Key_Return:
		case Qt::Key_Enter:
			if (triggerLastAction()) {
				event->accept();
				return;
			}
			break;
		case Qt::Key_Left:
			if (fromMousePressed) {
				hide();
				event->accept();
				return;
			}
			break;
		default:
			break;
	}

	qobject_cast<pStyle*>(style())->newMenuMenagement = false;
	QMenu::keyPressEvent(event);
	qobject_cast<pStyle*>(style())->newMenuMenagement = newMenagement;

	if (activeAction() && (permitDisabledAction ? true : activeAction()->isEnabled())) {
		lastAction = activeAction();
		update();
	}
}
void pMenu::paintEvent(QPaintEvent *event) {
	if (execQMenuEvent()) {
		QMenu::paintEvent(event);
		return;
	}

	QPainter p(this);
	QRegion emptyArea = QRegion(rect());
	QStyleOptionMenuItem menuOpt;

	menuOpt.initFrom(this);
	menuOpt.state = QStyle::State_None;
	menuOpt.checkType = QStyleOptionMenuItem::NotCheckable;
	menuOpt.maxIconWidth = 0;
	menuOpt.tabWidth = 0;
	style()->drawPrimitive(QStyle::PE_PanelMenu, &menuOpt, &p, this);

	for (int i = 0; i < actions().count(); ++i) {
		QAction *action = actions().at(i);
		QRect adjustedActionRect = actionGeometry(action);

		if (!event->rect().intersects(adjustedActionRect)) {
			continue;
		}

		QRegion adjustedActionReg(adjustedActionRect);
		emptyArea -= adjustedActionReg;
		p.setClipRegion(adjustedActionReg);

		QStyleOptionMenuItem opt;
		initMenuStyleOption(&opt, action, lastAction);
		opt.rect = adjustedActionRect;
		style()->drawControl(QStyle::CE_MenuItem, &opt, &p, this);
	}

	const int fw = style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, this);
	if (fw) {
		QRegion borderReg;
		borderReg += QRect(0, 0, fw, height());
		borderReg += QRect(width() - fw, 0, fw, height());
		borderReg += QRect(0, 0, width(), fw);
		borderReg += QRect(0, height() - fw, width(), fw);
		p.setClipRegion(borderReg);
		emptyArea -= borderReg;

		QStyleOptionFrame frame;
		frame.rect = rect();
		frame.palette = palette();
		frame.state = QStyle::State_None;
		frame.lineWidth = style()->pixelMetric(QStyle::PM_MenuPanelWidth);
		frame.midLineWidth = 0;
		style()->drawPrimitive(QStyle::PE_FrameMenu, &frame, &p, this);
	}

	p.setClipRegion(emptyArea);
	menuOpt.state = QStyle::State_None;
	menuOpt.menuItemType = QStyleOptionMenuItem::EmptyArea;
	menuOpt.checkType = QStyleOptionMenuItem::NotCheckable;
	menuOpt.rect = rect();
	menuOpt.menuRect = rect();
	style()->drawControl(QStyle::CE_MenuEmptyArea, &menuOpt, &p, this);
}
void pMenu::setNewMenagement(bool state) {
	qobject_cast<pStyle*>(style())->newMenuMenagement = state;
	newMenagement = state;
}
bool pMenu::execQMenuEvent() {
	if (!cfg->disable_new_menu && newMenagement) {
		return (false);
	}
	return (true);
}
bool pMenu::controlMenuParent(QObject *o, const QPoint &p) {
	pMenu *m = qobject_cast<pMenu*>(o);

	if (m && !m->rect().contains(m->mapFromGlobal(p))) {
		return (true);
	}
	return (false);
}
void pMenu::updateActionRects() {
	int lastVisibleAction = getLastVisibleAction();
	QFontMetrics qfm = fontMetrics();
    QStyleOption opt;
    opt.init(this);
	const int icone = style()->pixelMetric(QStyle::PM_SmallIconSize, &opt, this);
	bool previousWasSeparator = true;

	for (int i = 0; i <= lastVisibleAction; i++) {
		QAction *act = actions().at(i);

		if (!act->isVisible() || (previousWasSeparator && act->isSeparator())) {
			continue;
		}

		previousWasSeparator = act->isSeparator();

		if (!act->isSeparator()) {
			QString s = act->text();
			int t = s.indexOf(QLatin1Char('\t'));

			if (t != -1) {
				tabWidth = qMax(int(tabWidth), qfm.width(s.mid(t + 1)));
				s = s.left(t);
			} else {
				QKeySequence seq = act->shortcut();

				if (!seq.isEmpty()) {
					tabWidth = qMax(int(tabWidth), qfm.width(seq.toString()));
				}
			}
		}
	}

	actionPoint.resize(actions().count());
	actionPoint.fill(QPoint());

	for (int i = 0; i < actions().count(); ++i) {
		QAction *act = actions().at(i);
		QIcon is = act->icon();

		if (act->isSeparator() || !act->isVisible()) {
			continue;
		}

		hasCheckableItems |= act->isCheckable();

		if (!is.isNull()) {
			maxIconWidth = qMax<uint>(maxIconWidth, icone + 4);
		}

		if (act->menu()) {
			actionPoint[i] = QPoint((geometry().x() + geometry().width()) - 1,
					geometry().y() + actionGeometry(act).y());
		} else {
			actionPoint[i] = QPoint(0, 0);
		}
	}
}
void pMenu::initMenuStyleOption(QStyleOptionMenuItem* option, QAction *action,
		QAction *currentAction) {
	QString textAndAccel = action->text();

	option->initFrom(this);

	option->palette = palette();
	option->state = QStyle::State_None;

	if (window()->isActiveWindow()) {
		option->state |= QStyle::State_Active;
	}

	if (isEnabled() && action->isEnabled() && (!action->menu() || action->menu()->isEnabled())) {
		option->state |= QStyle::State_Enabled;
	} else {
		option->palette.setCurrentColorGroup(QPalette::Disabled);
	}

	option->font = action->font().resolve(font());
	option->fontMetrics = QFontMetrics(option->font);

	if (currentAction && (currentAction == action) && !currentAction->isSeparator()) {
		option->state |= (QStyle::State_Selected
				| (pmshare.mousePressed ? QStyle::State_Sunken : QStyle::State_None));
	}
	option->menuHasCheckableItems = hasCheckableItems;

	if (!action->isCheckable()) {
		option->checkType = QStyleOptionMenuItem::NotCheckable;
	} else {
		option->checkType =
		        (action->actionGroup() && action->actionGroup()->isExclusive()) ?
		                QStyleOptionMenuItem::Exclusive : QStyleOptionMenuItem::NonExclusive;
		option->checked = action->isChecked();
	}

	if (action->menu()) {
		option->menuItemType = QStyleOptionMenuItem::SubMenu;
	} else if (action->isSeparator()) {
		option->menuItemType = QStyleOptionMenuItem::Separator;
	} else if (defaultAction() == action) {
		option->menuItemType = QStyleOptionMenuItem::DefaultItem;
	} else {
		option->menuItemType = QStyleOptionMenuItem::Normal;
	}

	if (action->isIconVisibleInMenu()) {
		option->icon = action->icon();
	}

	if (textAndAccel.indexOf(QLatin1Char('\t')) == -1) {
		QKeySequence seq = action->shortcut();

		if (!seq.isEmpty()) {
			textAndAccel += QLatin1Char('\t') + seq.toString();
		}
	}

	option->text = textAndAccel;
	option->tabWidth = tabWidth;
	option->maxIconWidth = maxIconWidth;
	option->menuRect = rect();
}
void pMenu::setFirstActionActive() {
	lastAction = 0;
	for (int i = 0; i < actions().count(); i++) {
		QAction *act = actions().at(i);

		if (!act->isSeparator() && act->isVisible() &&
				(permitDisabledAction ? true : act->isEnabled())) {
			setCurrentAction(act);
			lastAction = act;
			break;
		}
	}
	updateActionRects();
}
void pMenu::resetLastAction() {
	setActiveAction(0);
	lastAction = 0;
	updateActionRects();
}
void pMenu::setCurrentAction(QAction *action) {
	if (action) {
		bool actEnabled = action->isEnabled();

		qobject_cast<pStyle*>(style())->newMenuAllowActiveAndDisabled = true;
		action->setEnabled(false);
		setActiveAction(action);
		action->setEnabled(actEnabled);
		qobject_cast<pStyle*>(style())->newMenuAllowActiveAndDisabled = false;
	}
}
void pMenu::hideToMenubar() {
	QObject *caused = parent();
	QMenuBar *mb = NULL;
	pMenu *m = NULL;

	hide();

	while (caused) {
		if ((mb = qobject_cast<QMenuBar*>(caused))) {
			caused = 0;
		} else if ((m = qobject_cast<pMenu*>(caused))) {
			caused = m->parent();
			m->hide();
		} else {
			caused = 0;
		}
	}
}
bool pMenu::triggerLastAction() {
	if (lastAction && lastAction->isEnabled() && !lastAction->menu()) {
		lastAction->trigger();
		return (true);
	}
	return (false);
}
bool pMenu::triggerLastActionAndHide() {
	bool triggerAndHide = QVariant(lastAction->property("triggerAndHide")).toBool();

	if (triggerLastAction()) {
		if (lastAction && triggerAndHide) {
			hideToMenubar();
			return (true);
		}
	}
	return (false);
}
int pMenu::getLastVisibleAction() {
	int lastVisibleAction = actions().count() - 1;

	for (; lastVisibleAction >= 0; --lastVisibleAction) {
		const QAction *action = actions().at(lastVisibleAction);

		if (action->isVisible()) {
			if (action->isSeparator()) {
				continue;
			}
			break;
		}
	}
	return (lastVisibleAction);
}

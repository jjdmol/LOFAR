/*
 * ListWidget.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 13-feb-2013
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/ListWidget.cpp $
 *
 */

#include "ListWidget.h"
#include "lofar_scheduler.h"
#include <QFocusEvent>

ListWidget::ListWidget(QWidget *parent)
	: QListWidget(parent), itsUndefined(false)
{
}

ListWidget::~ListWidget() {
}


void ListWidget::addItems(const QStringList &names, const QStringList &checkedNames) {
	clear();
	for (QStringList::const_iterator it = names.begin(); it != names.end(); ++it) {
		QListWidgetItem *item = new QListWidgetItem(*it, this);
		if (checkedNames.contains(*it)) {
			item->setCheckState(Qt::Checked);
		}
		else {
			item->setCheckState(Qt::Unchecked);
		}
	}
	itsPreviousCheckedNames = checkedNames;
	itsUndefined = false;
}

void ListWidget::checkItems(const QStringList &items_to_select) {
	this->blockSignals(true);
	for (int i = 0; i < count(); ++i) {
		if (items_to_select.contains(item(i)->text())) {
			item(i)->setCheckState(Qt::Checked);
		}
		else {
			item(i)->setCheckState(Qt::Unchecked);
		}
	}
	itsUndefined =false;
	resetChangeDetect();

	this->blockSignals(false);
}

QStringList ListWidget::checkedItemsList(void) const {
	QStringList list;
	int cnt(count());
	for (int i = 0; i < cnt; ++i) {
		if (item(i)->checkState() == Qt::Checked) {
			list.append(item(i)->text());
		}
	}
	return list;
}

QString ListWidget::checkedItemsAsString(void) const {
	return checkedItemsList().join(",");
}

QStringList ListWidget::items(void) const {
	QStringList items;
	for (int i = 0; i < count(); ++i) {
		items.append(item(i)->text());
	}
	return items;
}

void ListWidget::setUndefined(bool enabled) {
	this->blockSignals(true);
	itsUndefined = enabled;
	itsPreviousCheckedNames.clear();
	itsPreviousNames = items();
	clear();
	if (enabled) {
		addItem(MULTIPLE_VALUE_TEXT);
	}
	else {
		addItems(itsPreviousNames);
	}
	this->blockSignals(false);
}

bool ListWidget::hasBeenChanged(void) const {
    return (!isUndefined() && (itsPreviousCheckedNames != checkedItemsList()));
}

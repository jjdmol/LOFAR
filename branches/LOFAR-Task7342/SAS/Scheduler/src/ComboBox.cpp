/*
 * ComboBox.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 6-jan-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/ComboBox.cpp $
 *
 */

#include "ComboBox.h"
#include "lofar_scheduler.h"
#include <QFocusEvent>

ComboBox::ComboBox(QWidget *parent)
	: QComboBox(parent), itsUndefined(false)
{
	QObject::connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(checkIndexChange(int)));
}

ComboBox::~ComboBox() {
}

void ComboBox::setFromString(const QString &str) {
	this->blockSignals(true);
	int i(0);
	while (i < count()) {
		if (itemText(i).compare(str) == 0) {
			QComboBox::setCurrentIndex(i);
			itsPreviousIndex = currentIndex();
			itsUndefined = false;
			if (itemText(count()-1).compare(MULTIPLE_VALUE_TEXT) == 0) {
				removeItem(count()-1);
			}
            this->blockSignals(false);
			return;
		}
		else ++i;
	}
	this->blockSignals(false);
}


void ComboBox::setUndefined(bool enabled, bool removeUndefined) {
	this->blockSignals(true);
	itsUndefined = enabled;
	if (enabled) {
		if (itemText(count()-1).compare(MULTIPLE_VALUE_TEXT) != 0) {
			addItem(MULTIPLE_VALUE_TEXT);
		}
		QComboBox::setCurrentIndex(count()-1);
		itsPreviousIndex = -1;
	}
	else {
		if (removeUndefined) {
			if (itemText(count()-1).compare(MULTIPLE_VALUE_TEXT) == 0) {
				removeItem(count()-1);
			}
		}
		QComboBox::setCurrentIndex(itsPreviousIndex);
	}
	this->blockSignals(false);
}

void ComboBox::focusOutEvent(QFocusEvent* event)
{
	checkIndexChange(currentIndex());
    QComboBox::focusOutEvent(event);
}

void ComboBox::checkIndexChange(int newIndex) {
	if (itsUndefined) {
		if (itsPreviousIndex == newIndex) {
			blockSignals(true);
			QComboBox::setCurrentIndex(count()-1);
			blockSignals(false);
		}
		else {
			itsUndefined = false;
		}
		itsPreviousIndex = currentIndex();
	}
}

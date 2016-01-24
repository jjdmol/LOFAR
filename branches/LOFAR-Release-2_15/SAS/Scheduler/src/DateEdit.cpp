/*
 * DateEdit.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 6-jan-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/DateEdit.cpp $
 *
 */

#include "DateEdit.h"
#include <QFocusEvent>

DateEdit::DateEdit(QWidget *parent)
	: QDateEdit(parent), itsUndefined(false)
{
	itsPreviousDate = date();
	QObject::connect(this,SIGNAL(dateChanged(const QDate &)), this, SLOT(checkDateChange(void)));
}

DateEdit::~DateEdit() {
}

void DateEdit::setDate(const QDate &date) {
	this->blockSignals(true);
	setSpecialValueText("");
	itsUndefined = false;
	QDateEdit::setDate(date);
	itsPreviousDate = date;
	this->blockSignals(false);
}

void DateEdit::setUndefined(bool enabled = true) {
	this->blockSignals(true);
	itsUndefined = enabled;
	if (enabled) {
		itsPreviousDate = date();
		// NOTE: if minimum date equals the date then the special value text is shown
		setSpecialValueText(MULTIPLE_VALUE_TEXT);
		QDateEdit::setDate(minimumDate());
	}
	else {
		setSpecialValueText("");
		QDateEdit::setDate(itsPreviousDate);
	}
	this->blockSignals(false);
}

void DateEdit::focusInEvent(QFocusEvent* event)
{
    this->blockSignals(true);
	if (itsUndefined) {
		QDateEdit::setDate(itsDefaultDate);
		setSpecialValueText("");
		itsPreviousDate = date();
	}

    QDateEdit::focusInEvent(event);
    this->blockSignals(false);
}

void DateEdit::focusOutEvent(QFocusEvent* event)
{
	checkDateChange();
    QDateEdit::focusOutEvent(event);
}

void DateEdit::checkDateChange(void) {
    this->blockSignals(true);
	if (itsUndefined) {
		if (itsPreviousDate == date()) { // was undefined and did not change
			// NOTE: if minimum date equals the date then the special value text is shown
			setSpecialValueText(MULTIPLE_VALUE_TEXT);
			QDateEdit::setDate(minimumDate());
		}
		else {
			itsUndefined = false;
			setSpecialValueText("");
		}
		itsPreviousDate = date();
	}
    this->blockSignals(false);
}

/*
 * DateTimeEdit.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 6-jan-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/DateTimeEdit.cpp $
 *
 */

#include "DateTimeEdit.h"
#include <QFocusEvent>
#include "astrodate.h"

DateTimeEdit::DateTimeEdit(QWidget *parent)
	: QDateTimeEdit(parent), itsUndefined(true)
{
	itsPreviousDateTime = dateTime();
	QObject::connect(this,SIGNAL(dateTimeChanged(const QDateTime &)), this, SLOT(checkDateTimeChange(void)));
}

DateTimeEdit::~DateTimeEdit() {
}

void DateTimeEdit::setDateTime(const QDateTime &datetime) {
    this->blockSignals(true);
    QDateTimeEdit::setDateTime(datetime);
    itsPreviousDateTime = QDateTimeEdit::dateTime();
    setSpecialValueText("");
    itsUndefined = false;
    this->blockSignals(false);
}

void DateTimeEdit::setUndefined(bool enabled) {
	this->blockSignals(true);
	itsUndefined = enabled;
	if (enabled) {
		itsPreviousDateTime = dateTime();
		// NOTE: if minimum date equals the date then the special value text is shown
		setSpecialValueText(itsSpecialValueText);
		QDateTimeEdit::setDateTime(minimumDateTime());
	}
	else {
		setSpecialValueText("");
		QDateTimeEdit::setDateTime(itsPreviousDateTime);
	}
	this->blockSignals(false);
}

void DateTimeEdit::focusInEvent(QFocusEvent* event)
{
	this->blockSignals(true);

	if (itsUndefined) {
		QDateTimeEdit::setDateTime(itsDefaultDateTime);
		setSpecialValueText("");
        itsPreviousDateTime = dateTime();
	}

    QDateTimeEdit::focusInEvent(event);
    this->blockSignals(false);
}

void DateTimeEdit::focusOutEvent(QFocusEvent* event)
{
	checkDateTimeChange();
    QDateTimeEdit::focusOutEvent(event);
}

void DateTimeEdit::checkDateTimeChange(void) {
    this->blockSignals(true);
	if (itsUndefined) {
		if (itsPreviousDateTime == dateTime()) { // was undefined and did not change -> re-apply opacity effect
			// NOTE: if minimum date equals the date then the special value text is shown
			setSpecialValueText(itsSpecialValueText);
			QDateTimeEdit::setDateTime(minimumDateTime());
		}
		else {
			itsUndefined = false;
			setSpecialValueText("");
		}
		itsPreviousDateTime = dateTime();
	}
    this->blockSignals(false);
}

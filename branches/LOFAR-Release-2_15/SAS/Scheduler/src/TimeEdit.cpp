/*
 * TimeEdit.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 6-jan-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/TimeEdit.cpp $
 *
 */

#include "TimeEdit.h"
#include <QFocusEvent>
#include "astrotime.h"

TimeEdit::TimeEdit(QWidget *parent)
	: QTimeEdit(parent), itsUndefined(false)
{
	itsPreviousTime = time();
	QObject::connect(this,SIGNAL(timeChanged(const QTime &)), this, SLOT(checkTimeChange(void)));
}

TimeEdit::~TimeEdit() {
}

void TimeEdit::setTime(const QTime &time) {
	this->blockSignals(true);
	QTimeEdit::setTime(time);
	itsPreviousTime = time;
	setSpecialValueText("");
	itsUndefined = false;
	this->blockSignals(false);
}

void TimeEdit::setUndefined(bool enabled) {
	this->blockSignals(true);
	itsUndefined = enabled;
	if (enabled) {
		itsPreviousTime = time();
		// NOTE: if minimum time equals the time then the special value text is shown
		setSpecialValueText(itsSpecialValueText);
		QTimeEdit::setTime(minimumTime());
	}
	else {
		setSpecialValueText("");
		QTimeEdit::setTime(itsPreviousTime);
	}
	this->blockSignals(false);
}

void TimeEdit::focusInEvent(QFocusEvent* event)
{
	this->blockSignals(true);
	if (itsUndefined) {
		QTimeEdit::setTime(itsDefaultTime);
		setSpecialValueText("");
		itsPreviousTime = time();
	}

    QTimeEdit::focusInEvent(event);
    this->blockSignals(false);
}

void TimeEdit::focusOutEvent(QFocusEvent* event)
{
    checkTimeChange();
    QTimeEdit::focusOutEvent(event);
}

void TimeEdit::checkTimeChange(void) {
    this->blockSignals(true);
	if (itsUndefined) {
		if (itsPreviousTime == time()) {
			// NOTE: if minimum time equals the time then the special value text is shown
			setSpecialValueText(itsSpecialValueText);
			QTimeEdit::setTime(minimumTime());
		}
		else {
            itsUndefined = false;
            setSpecialValueText("");
		}
    }
    this->blockSignals(false);
}

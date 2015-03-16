/*
 * LineEdit.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 6-jan-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/LineEdit.cpp $
 *
 */

#include "LineEdit.h"
#include "lofar_scheduler.h"
#include <QFocusEvent>
#include <iostream>

LineEdit::LineEdit(QWidget *parent)
    : QLineEdit(parent), itsUndefined(false), itsPreviousUndefined(false)
{
    connect(this, SIGNAL(textEdited(QString)), this, SLOT(checkTextChange()));
}

LineEdit::~LineEdit() {
}


void LineEdit::setText(const QString &text) {
	this->blockSignals(true);
	QLineEdit::setInputMask(itsInputMask);
	QLineEdit::setText(text);
    itsPreviousText = this->text();
    itsPreviousUndefined = false;
	itsUndefined = false;
	this->blockSignals(false);
}

void LineEdit::setUndefined(bool enabled = true) {
	this->blockSignals(true);
	itsUndefined = enabled;
	if (enabled) {
		blockSignals(true);
		QLineEdit::setInputMask("");
		QLineEdit::setText(MULTIPLE_VALUE_TEXT);
        itsPreviousText = text();
	}
	else {
		setInputMask(itsInputMask);
		setText(itsPreviousText);
	}
	this->blockSignals(false);
}

void LineEdit::focusInEvent(QFocusEvent* event)
{
	if (itsUndefined) {
        blockSignals(true);
        QLineEdit::setInputMask(itsInputMask);
		QLineEdit::setText(itsDefaultText);
        itsPreviousText = text();
        itsPreviousUndefined = true;
        blockSignals(false);
    }

    QLineEdit::focusInEvent(event);
}

void LineEdit::focusOutEvent(QFocusEvent* event)
{
    if (itsUndefined)
        checkTextChange();

    QLineEdit::focusOutEvent(event);
}

void LineEdit::checkTextChange(void) {
	if (itsUndefined) {
		if (itsPreviousText == text()) { // did not change?
			blockSignals(true);
			QLineEdit::setInputMask("");
			QLineEdit::setText(MULTIPLE_VALUE_TEXT);
			blockSignals(false);
		}
		else {
			itsUndefined = false;
		}
	}
}

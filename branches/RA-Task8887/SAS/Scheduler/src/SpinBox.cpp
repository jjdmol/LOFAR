/*
 * SpinBox.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 16-jun-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/SpinBox.cpp $
 *
 */

#include "SpinBox.h"

SpinBox::SpinBox(QWidget *parent)
    : QSpinBox(parent), itsUndefined(false), itsPreviousUndefined(false), itsPreviousValue(0), itsDefaultValue(0)
{
}

SpinBox::~SpinBox()
{

}

void SpinBox::setValue(int value) {
	this->blockSignals(true);
	setSpecialValueText("");
	QSpinBox::setValue(value);
	itsPreviousValue = value;
	itsUndefined = false;
	this->blockSignals(false);
}


void SpinBox::setUndefined(bool enabled = true) {
	this->blockSignals(true);
	itsUndefined = enabled;
	if (enabled) {
		itsPreviousValue = -1;
		// NOTE: if the current value is equal to the minimum value then the special value text is shown
		setSpecialValueText(MULTIPLE_VALUE_TEXT);
		QSpinBox::setValue(minimum());
	}
	else {
		setSpecialValueText("");
		QSpinBox::setValue(itsDefaultValue);
	}
	this->blockSignals(false);
}

void SpinBox::focusInEvent(QFocusEvent* event)
{
	if (itsUndefined) {
        this->blockSignals(true);
		itsPreviousUndefined = true;
		QSpinBox::setValue(itsDefaultValue);
		setSpecialValueText("");
		itsPreviousValue = value();
        this->blockSignals(false);
    }

    // You might also call the parent method.
	QSpinBox::focusInEvent(event);
}

void SpinBox::focusOutEvent(QFocusEvent* event)
{
	checkValueChange();
    // You might also call the parent method.
    QSpinBox::focusOutEvent(event);
}

void SpinBox::checkValueChange(void) {
	if (itsUndefined) {
        if (itsPreviousValue == value()) { // was undefined and did not change
            // NOTE: if the current value is equal to the minimum value then the special value text is shown
			setSpecialValueText(MULTIPLE_VALUE_TEXT);
            this->blockSignals(true);
            QSpinBox::setValue(minimum());
            this->blockSignals(false);
        }
		else {
			itsUndefined = false;
			setSpecialValueText("");
		}
		itsPreviousValue = value();
	}
}

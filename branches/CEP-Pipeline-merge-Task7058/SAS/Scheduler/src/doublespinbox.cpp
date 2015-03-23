/*
 * doublespinbox.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 24-jun-2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/doublespinbox.cpp $
 *
 */

#include "doublespinbox.h"

DoubleSpinBox::DoubleSpinBox(QWidget *parent)
    : QDoubleSpinBox(parent), itsUndefined(false), itsPreviousUndefined(false), itsPreviousValue(0), itsDefaultValue(0)
{
}

DoubleSpinBox::~DoubleSpinBox()
{

}

void DoubleSpinBox::setValue(double value) {
    this->blockSignals(true);
    setSpecialValueText("");
    QDoubleSpinBox::setValue(value);
    itsPreviousValue = value;
    itsUndefined = false;
    this->blockSignals(false);
}


void DoubleSpinBox::setUndefined(bool enabled = true) {
    this->blockSignals(true);
    itsUndefined = enabled;
    if (enabled) {
        itsPreviousValue = -1;
        // NOTE: if the current value is equal to the minimum value then the special value text is shown
        setSpecialValueText(MULTIPLE_VALUE_TEXT);
        QDoubleSpinBox::setValue(minimum());
    }
    else {
        setSpecialValueText("");
        QDoubleSpinBox::setValue(itsDefaultValue);
    }
    this->blockSignals(false);
}

void DoubleSpinBox::focusInEvent(QFocusEvent* event)
{
    if (itsUndefined) {
        itsPreviousUndefined = true;
        QDoubleSpinBox::setValue(itsDefaultValue);
        setSpecialValueText("");
        itsPreviousValue = value();
    }

    // You might also call the parent method.
    QDoubleSpinBox::focusInEvent(event);
}

void DoubleSpinBox::focusOutEvent(QFocusEvent* event)
{
    checkValueChange();
    // You might also call the parent method.
    QDoubleSpinBox::focusOutEvent(event);
}

void DoubleSpinBox::checkValueChange(void) {
    if (itsUndefined) {
        if (itsPreviousValue == value()) { // was undefined and did not change
            // NOTE: if the current value is equal to the minimum value then the special value text is shown
            setSpecialValueText(MULTIPLE_VALUE_TEXT);
            this->blockSignals(true);
            QDoubleSpinBox::setValue(minimum());
            this->blockSignals(false);
        }
        else {
            itsUndefined = false;
            setSpecialValueText("");
        }
        itsPreviousValue = value();
    }
}

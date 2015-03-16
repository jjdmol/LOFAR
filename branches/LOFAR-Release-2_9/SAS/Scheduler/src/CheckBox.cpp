/*
 * CheckBox.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 16-jun-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/CheckBox.cpp $
 *
 */

#include "CheckBox.h"

CheckBox::CheckBox(QWidget *parent)
    : QCheckBox(parent)
{
    itsPreviousCheckState = checkState();
}

CheckBox::~CheckBox()
{

}

void CheckBox::setCheckState(Qt::CheckState state) {
	this->blockSignals(true);
    QCheckBox::setCheckState(state);
    itsPreviousCheckState = state;
	this->blockSignals(false);
}

void CheckBox::setChecked(bool checked) {
    this->blockSignals(true);
    setTristate(false);
    QCheckBox::setChecked(checked);
    itsPreviousCheckState = checkState();
    this->blockSignals(false);
}


void CheckBox::setUndefined(bool tristate = true) {
	this->blockSignals(true);
    if (tristate) {
        itsPreviousCheckState = checkState();
        setTristate(true);
	}
	else {
        setTristate(false);
        setChecked(itsPreviousCheckState);
	}
	this->blockSignals(false);
}

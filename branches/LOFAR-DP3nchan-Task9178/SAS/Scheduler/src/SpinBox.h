/*
 * SpinBox.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 16-jun-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/SpinBox.h $
 *
 */

#ifndef SPINBOX_H
#define SPINBOX_H

#include "lofar_scheduler.h"
#include <QSpinBox>

class SpinBox : public QSpinBox
{
    Q_OBJECT

public:
    SpinBox(QWidget *parent = 0);
    ~SpinBox();


    void setValue(int value);
	void setUndefined(bool enabled);
	void setDefaultValue(int value) {itsDefaultValue = value;}
	inline bool isUndefined(void) {return itsUndefined;}
	bool hasBeenChanged(void) const {
		if (itsUndefined) return false;
		else if (itsPreviousUndefined) return true;
        else if (itsPreviousValue != value()) return true;
        else return false;
	}
	void resetChangeDetect(void) { itsPreviousValue = value(); }

protected:
	void focusInEvent(QFocusEvent*);
	void focusOutEvent(QFocusEvent*);

protected slots:
	void checkValueChange(void);
private:
	bool itsUndefined, itsPreviousUndefined;
    int itsPreviousValue, itsDefaultValue;
};

#endif // SPINBOX_H

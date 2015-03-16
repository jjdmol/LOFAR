/*
 * CheckBox.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 16-jun-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/CheckBox.h $
 *
 */

#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "lofar_scheduler.h"
#include <QCheckBox>

class CheckBox : public QCheckBox
{
    Q_OBJECT

public:
    CheckBox(QWidget *parent = 0);
    ~CheckBox();

    void setCheckState(Qt::CheckState state);
    void setChecked(bool checked);
    void setUndefined(bool tristate);
    inline bool isUndefined(void) {return isTristate();}
	bool hasBeenChanged(void) const {
        if (itsPreviousCheckState != checkState()) return true;
        else if (itsPreviousCheckState == Qt::PartiallyChecked && !isTristate()) return true;
		else return false;
	}
    void resetChangeDetect(void) { itsPreviousCheckState = checkState(); }

//protected:
//	void focusInEvent(QFocusEvent*);
//	void focusOutEvent(QFocusEvent*);

//protected slots:
//	void checkValueChange(void);
private:
    Qt::CheckState itsPreviousCheckState;
};

#endif // CHECKBOX_H

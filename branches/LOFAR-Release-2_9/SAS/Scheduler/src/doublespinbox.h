/*
 * SpinBox.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 24-jun-2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/doublespinbox.h $
 *
 */

#ifndef DOUBLESPINBOX_H
#define DOUBLESPINBOX_H

#include "lofar_scheduler.h"
#include <QDoubleSpinBox>

class DoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    DoubleSpinBox(QWidget *parent = 0);
    ~DoubleSpinBox();


    void setValue(double value);
    void setUndefined(bool enabled);
    void setDefaultValue(double value) {itsDefaultValue = value;}
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
    double itsPreviousValue, itsDefaultValue;
};

#endif // DOUBLESPINBOX_H

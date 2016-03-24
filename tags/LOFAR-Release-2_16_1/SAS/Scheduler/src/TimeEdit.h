/*
 * TimeEdit.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 6-jan-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/TimeEdit.h $
 *
 */

#include <QTimeEdit>
#include "astrotime.h"
#include "lofar_scheduler.h"

#ifndef TIMEEDIT_H_
#define TIMEEDIT_H_

class TimeEdit : public QTimeEdit {

	Q_OBJECT

public:
	TimeEdit(QWidget *parent);
	virtual ~TimeEdit();

	void setTime(const QTime &t);
	void setMultipleValue(void) {itsSpecialValueText = MULTIPLE_VALUE_TEXT; setUndefined(true);}
	void setUndefined(bool enabled = true);
	void setDefaultTime(const AstroTime &t) {itsDefaultTime.setHMS(t.getHours(),t.getMinutes(),t.getSeconds());}
	inline bool isUndefined(void) {return itsUndefined;}
    inline bool hasBeenChanged(void) const {return (!itsUndefined && itsPreviousTime != time());}
	void resetChangeDetect(void) {itsPreviousTime = time();}


protected:
	void focusInEvent(QFocusEvent*);
	void focusOutEvent(QFocusEvent*);

protected slots:
	void checkTimeChange(void);

private:
	bool itsUndefined;
	QString itsSpecialValueText;
	QTime itsPreviousTime, itsDefaultTime;
};

#endif /* TIMEEDIT_H_ */

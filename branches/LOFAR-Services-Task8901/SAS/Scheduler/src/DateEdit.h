/*
 * DateEdit.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 6-jan-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/DateEdit.h $
 *
 */

#include <QDateEdit>
#include "astrodate.h"

#ifndef DATEEDIT_H_
#define DATEEDIT_H_

class DateEdit : public QDateEdit {

	Q_OBJECT

public:
	DateEdit(QWidget *parent);
	virtual ~DateEdit();

	void setDate(const QDate &d);
	void setUndefined(bool enabled);
	void setDefaultDate(const AstroDate &d) {itsDefaultDate.setDate(d.getYear(),d.getMonth(),d.getDay());}
	inline bool isUndefined(void) {return itsUndefined;}
	inline bool hasBeenChanged(void) const {return itsPreviousDate != date();}
	void resetChangeDetect(void) {itsPreviousDate = date();}


protected:
	void focusInEvent(QFocusEvent*);
	void focusOutEvent(QFocusEvent*);

protected slots:
	void checkDateChange(void);

private:
	bool itsUndefined;
	QDate itsPreviousDate, itsDefaultDate;
};

#endif /* DATEEDIT_H_ */

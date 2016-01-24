/*
 * DateTimeEdit.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 6-jan-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/DateTimeEdit.h $
 *
 */

#include <QDateTimeEdit>
#include "astrodatetime.h"

#ifndef DATETIMEEDIT_H_
#define DATETIMEEDIT_H_

class DateTimeEdit : public QDateTimeEdit {

	Q_OBJECT

public:
	DateTimeEdit(QWidget *parent);
	virtual ~DateTimeEdit();

    void setDateTime(const QDateTime &datetime);
	void setMultipleValue(void) {itsSpecialValueText = MULTIPLE_VALUE_TEXT; setUndefined(true);}
    void setNotSetValue(void) {itsSpecialValueText = NOT_SET_TEXT; setUndefined(true);}
    void setUndefined(bool enabled = true);
	bool hasBeenChanged(void) const {return ((!itsUndefined) && (itsPreviousDateTime != QDateTimeEdit::dateTime()));}
	void setDefaultDateTime(const AstroDateTime &dt) {itsDefaultDateTime.setDate(QDate(dt.getYear(),dt.getMonth(),dt.getDay()));
	itsDefaultDateTime.setTime(QTime(dt.getHours(), dt.getMinutes(), dt.getSeconds()));}
	void resetChangeDetect(void) {itsPreviousDateTime = dateTime();}

	bool isUndefined(void) {return itsUndefined;}

protected:
	void focusInEvent(QFocusEvent*);
	void focusOutEvent(QFocusEvent*);

protected slots:
	void checkDateTimeChange(void);

private:
	bool itsUndefined;
	QString itsSpecialValueText;
	QDateTime itsPreviousDateTime, itsDefaultDateTime;
};

#endif /* DATETIMEEDIT_H_ */

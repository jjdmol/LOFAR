/*
 * ComboBox.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 6-jan-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/ComboBox.h $
 *
 */

#include <QComboBox>
#include <QString>

#ifndef COMBOBOX_H_
#define COMBOBOX_H_

class ComboBox : public QComboBox {

	Q_OBJECT

public:
	ComboBox(QWidget *parent);
	virtual ~ComboBox();

	void setCurrentIndex(int index, bool removeUndefined = true) {
		itsPreviousIndex = index; setUndefined(false, removeUndefined);
	}
	void setFromString(const QString &t);
	void setUndefined(bool enabled = true, bool removeUndefined = true);
	bool isUndefined(void) const {return itsUndefined;}
	bool hasBeenChanged(void) const {
		return ((itsPreviousIndex != -1) && (itsPreviousIndex != currentIndex()));
	} // -1 == MIXED, hasBeenChanged should then return false
	void resetChangeDetect(void) { itsPreviousIndex = currentIndex(); }

protected:
	void focusOutEvent(QFocusEvent*);

protected slots:
	void checkIndexChange(int);

private:
	bool itsUndefined;
	int itsPreviousIndex;
};

#endif /* COMBOBOX_H_ */

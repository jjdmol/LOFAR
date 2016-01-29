/*
 * LineEdit.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 6-jan-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/LineEdit.h $
 *
 */

#include <QLineEdit>
#include <string>

#ifndef LINEEDIT_H_
#define LINEEDIT_H_

class LineEdit : public QLineEdit {

	Q_OBJECT

public:
	LineEdit(QWidget *parent);
	virtual ~LineEdit();

	void setText(const QString &text);
	inline void setText(const std::string &text) {setText(QString(text.c_str()));}
	inline void setText(const char *text) {setText(QString(text));}
    bool hasBeenChanged(void) const {
        if (itsUndefined) return false;
        else if (itsPreviousUndefined) return true;
        else if (itsPreviousText != text()) return true;
        else return false;
    }
    void setUndefined(bool enabled);
	void setDefaultText(const QString &text) {itsDefaultText = text;}
	bool isUndefined(void) {return itsUndefined;}
	void setInputMask(const QString &mask) {QLineEdit::setInputMask(mask); itsInputMask = mask;}
	void resetChangeDetect(void) {itsPreviousText = text();}

protected:
	void focusInEvent(QFocusEvent*);
	void focusOutEvent(QFocusEvent*);

protected slots:
    void checkTextChange(void);

private:
    bool itsUndefined, itsPreviousUndefined;
	QString itsPreviousText, itsDefaultText, itsInputMask;
};

#endif /* LINEEDIT_H_ */

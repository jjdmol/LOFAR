/*
 * filterdatadialog.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : Aug 24, 2012
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/filterdatadialog.h $
 *
 */

#ifndef FILTERDATADIALOG_H
#define FILTERDATADIALOG_H

#include <QtGui/QDialog>
#include <map>
#include "ui_filterdatadialog.h"
#include "lofar_scheduler.h"

class sizeValidator : public QValidator
{
	Q_OBJECT

public:
	virtual State validate (QString & input, int & /*pos*/ ) const
	{
		if (input.isEmpty() || (input == "0")) {
			return Acceptable;
		}

		input.replace(',','.');
		input.remove(' ');
		QChar lastChar(input[input.size()-1]);
		if (lastChar.isDigit()) {
			return Acceptable;
		}
		else if (lastChar == '.') {
			return Intermediate;
		}

		int lastDigitPos(input.lastIndexOf(QRegExp("[0123456789]")));
		if (lastDigitPos < input.size()) {
			QString unitChar(input[lastDigitPos+1]);
			if (unitChar.compare("k", Qt::CaseInsensitive) == 0) {
				input = input.left(lastDigitPos+1) + "kB";
			}
			else if (unitChar.compare("m", Qt::CaseInsensitive) == 0) {
				input = input.left(lastDigitPos+1) + "MB";
			}
			else if (unitChar.compare("g", Qt::CaseInsensitive) == 0) {
				input = input.left(lastDigitPos+1) + "GB";
			}
			else if (unitChar.compare("t", Qt::CaseInsensitive) == 0) {
				input = input.left(lastDigitPos+1) + "TB";
			}
			else {
				input = input.left(lastDigitPos+1);
			}
			QString copy = input.trimmed();
			if (copy.endsWith("kB") || copy.endsWith("MB") || copy.endsWith("GB") || copy.endsWith("TB")) {
				bool *b = new bool;
				copy.left(copy.size()-2).toDouble(b);
				if (*b == true) {
					return Acceptable;
				}
				else return Invalid;
			}
		}
		return Intermediate;
	}
};

class FilterDataDialog : public QDialog
{
    Q_OBJECT

public:
    FilterDataDialog(QWidget *parent = 0);
    ~FilterDataDialog();

    QStringList selectedProjects(void) const;
    void setProjects(const QStringList &projects) {ui.listWidgetProjects->clear(); ui.listWidgetProjects->addItems(projects); ui.listWidgetProjects->selectAll();}
    std::map<dataProductTypes, bool> selectedDataTypes(void) const;
    bool applyDates(void) const {return ui.checkBoxApplyDates->isChecked();}
    bool applySizes(void) const {return ui.checkBoxApplySizes->isChecked();}
    bool hideEmpty(void) const {return ui.checkBoxHideEmpty->isChecked();}
    bool applyProjects(void) const {return itsApplyProjects;}
    bool applyDataTypes(void) const {return itsApplyDataTypes;}
    QDate minDate(void) const {return ui.dateEditEarliest->date();}
    QDate maxDate(void) const {return ui.dateEditLatest->date();}
    quint64 minFileSize(void) const; // returns minimum filesize in kB
    quint64 maxFileSize(void) const;// returns maximum filesize in kB
    bool filterApplied(void) const {return (itsApplyProjects || itsApplyDataTypes || applyDates() || applySizes());}

private:
    quint64 getSizekB(const QString &str) const;

private slots:
	void accept(void);

private:
    Ui::FilterDataDialogClass ui;
    bool itsApplyProjects, itsApplyDataTypes;
};

#endif // FILTERDATADIALOG_H

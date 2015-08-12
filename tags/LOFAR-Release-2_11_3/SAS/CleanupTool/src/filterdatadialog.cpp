/*
 * filterdatadialog.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : Aug 24, 2012
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/filterdatadialog.cpp $
 *
 */

#include "lofar_scheduler.h"
#include "filterdatadialog.h"
#include <QMessageBox>

FilterDataDialog::FilterDataDialog(QWidget *parent)
    : QDialog(parent), itsApplyProjects(false), itsApplyDataTypes(false)
{
	ui.setupUi(this);

	sizeValidator *sValidator1 = new sizeValidator();
	sizeValidator *sValidator2 = new sizeValidator();

	ui.lineEditMinSize->setValidator(sValidator1);
	ui.lineEditMaxSize->setValidator(sValidator2);

	for (int i = _BEGIN_DATA_PRODUCTS_ENUM_; i < _END_DATA_PRODUCTS_ENUM_; ++i) {
		ui.listWidgetDataTypes->addItem(QString(DATA_PRODUCTS[i]));
	}
	ui.listWidgetDataTypes->selectAll();
	connect(ui.pushButtonApply, SIGNAL(clicked()), this, SLOT(accept(void)));
	connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject(void)));
}

FilterDataDialog::~FilterDataDialog()
{

}

void FilterDataDialog::accept(void) {
	if (ui.checkBoxApplySizes->isChecked()) {
		if (!ui.lineEditMinSize->hasAcceptableInput()) {
			QMessageBox::warning(this, "wrong size specified","the specified minimum file size is invalid. Examples of valid input:\n1kB, 1.5MB, 2GB, 3.15TB");
			return;
		}
		if (!ui.lineEditMaxSize->hasAcceptableInput()) {
			QMessageBox::warning(this, "wrong size specified","the specified maximum file size is invalid. Examples of valid input:\n1kB, 1.5MB, 2GB, 3.15TB");
			return;
		}
	}
	itsApplyDataTypes = ui.listWidgetDataTypes->selectedItems().count() == ui.listWidgetDataTypes->count() ? false : true;
	itsApplyProjects = ui.listWidgetProjects->selectedItems().count() == ui.listWidgetProjects->count() ? false : true;

	done(1);
}

quint64 FilterDataDialog::minFileSize(void) const {
	return getSizekB(ui.lineEditMinSize->text());
}

quint64 FilterDataDialog::maxFileSize(void) const {
	QString strValue(ui.lineEditMaxSize->text());
	if (strValue.isEmpty()) {
		return 0;
	}
	return getSizekB(ui.lineEditMaxSize->text());
}

quint64 FilterDataDialog::getSizekB(const QString &str) const {
	int i(0);
	quint64 mult(1);
	QChar chr, dot('.');
	QString s = str.trimmed(), numberStr;
	while (i < s.length()) {
		chr = s.at(i);
		if (chr.isDigit() || (chr == dot)) {
			numberStr += chr;
		}
		else break;
		++i;
	}
	QString units(s.mid(i).trimmed());
	if (!units.isEmpty()) {
		QChar un(units[0].toLower());

		if (un == 't') {
			mult = 1073741824;
		}
		else if (un == 'g') {
			mult = 1048576;
		}
		else if (un == 'm') {
			mult = 1024;
		}
	}
	return (static_cast<quint64>(numberStr.toDouble() * mult));
}



QStringList FilterDataDialog::selectedProjects(void) const {
	QStringList list;
	foreach (QListWidgetItem* item, ui.listWidgetProjects->selectedItems()) {
		list << item->text();
	}
	return list;
}

std::map<dataProductTypes, bool> FilterDataDialog::selectedDataTypes(void) const {
	std::map<dataProductTypes, bool> dataTypes;
	for (int i = _BEGIN_DATA_PRODUCTS_ENUM_; i < _END_DATA_PRODUCTS_ENUM_; ++i) {
		dataTypes[static_cast<dataProductTypes>(i)] = ui.listWidgetDataTypes->item(i)->isSelected();
	}
	return dataTypes;
}

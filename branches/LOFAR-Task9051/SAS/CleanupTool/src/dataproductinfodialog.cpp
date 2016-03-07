#include "dataproductinfodialog.h"

DataProductInfoDialog::DataProductInfoDialog(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.checkBoxDeleted,SIGNAL(clicked()),this,SLOT(checkBoxDeletedReset(void)));
	connect(ui.checkBoxExpired,SIGNAL(clicked()),this,SLOT(checkBoxExpiredReset(void)));
}

DataProductInfoDialog::~DataProductInfoDialog()
{

}

void DataProductInfoDialog::checkBoxDeletedReset(void) {
	ui.checkBoxDeleted->toggle();
	QApplication::beep();
}

void DataProductInfoDialog::checkBoxExpiredReset(void) {
	ui.checkBoxExpired->toggle();
	QApplication::beep();
}

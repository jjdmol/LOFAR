#include "tablecolumnselectdialog.h"
#include "lofar_scheduler.h"

tableColumnSelectDialog::tableColumnSelectDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	initDialog();
}

tableColumnSelectDialog::~tableColumnSelectDialog()
{

}

void tableColumnSelectDialog::show(void) {
	for (std::vector<unsigned int>::const_iterator it = itsSelectedColumns.begin(); it != itsSelectedColumns.end(); ++it) {
		ui.listWidget_Columns->item(*it)->setSelected(true);
	}
	ui.listWidget_Columns->setFocus();
	this->showNormal();
}

void tableColumnSelectDialog::accept(void) {
	itsSelectedColumns.clear();
	for (int i = 0; i < ui.listWidget_Columns->count(); ++i) {
		if (ui.listWidget_Columns->item(i)->isSelected()) {
			itsSelectedColumns.push_back(i);
		}
	}
	this->hide();
	emit viewColumns(itsSelectedColumns);
}

void tableColumnSelectDialog::toggleSelection(void) {
	if (selectAll) {
		ui.listWidget_Columns->selectAll();
		ui.pushButton_Select->setText("Select none");
		selectAll = false;
	}
	else {
		ui.listWidget_Columns->clearSelection();
		ui.pushButton_Select->setText("Select all");
		selectAll = true;
	}
	ui.listWidget_Columns->setFocus();
}

void tableColumnSelectDialog::initDialog(void) {
	for (int i = 0; i < NR_DATA_HEADERS; ++i) {
		ui.listWidget_Columns->addItem(DATA_HEADERS[i]);
		itsSelectedColumns.push_back(i);
	}
	ui.listWidget_Columns->selectAll();
	ui.pushButton_Select->setText("Select none");
	selectAll = false;
	connect(ui.pushButton_Select, SIGNAL(clicked()), this, SLOT(toggleSelection()));
}

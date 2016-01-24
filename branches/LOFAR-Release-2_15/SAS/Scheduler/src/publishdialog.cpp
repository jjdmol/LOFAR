/*
 * publishdialog.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date    : $Date$
 * First creation : Dec 8, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/publishdialog.cpp $
 *
 */

#include "publishdialog.h"
#include "Controller.h"
#include "schedulersettings.h"
#include <QMessageBox>

PublishDialog::PublishDialog(QWidget *parent)
    : QDialog(parent), select(true)
{
	ui.setupUi(this);
	connect(ui.pushButton_select, SIGNAL(clicked()), this, SLOT(toggleSelection()));
}

PublishDialog::~PublishDialog()
{

}

void PublishDialog::show(void) {
	ui.listWidget_Weeks->setFocus();
	this->showNormal();
}

void PublishDialog::toggleSelection(void) {
	if (select) {
		ui.listWidget_Weeks->selectAll();
		ui.pushButton_select->setText("Select none");
		select = false;
	}
	else {
		ui.listWidget_Weeks->clearSelection();
		ui.pushButton_select->setText("Select all");
		select = true;
	}
	ui.listWidget_Weeks->setFocus();
}

void PublishDialog::accept(void) {
	itsPublishWeeks.clear();
	AstroDate monday;
	for (int idx = 0; idx != ui.listWidget_Weeks->count(); ++idx) {
		if (ui.listWidget_Weeks->item(idx)->isSelected()) {
			monday = ui.listWidget_Weeks->item(idx)->data(200).toInt();
			itsPublishWeeks.push_back(std::pair<unsigned, AstroDate>(monday.getWeek(), monday));
		}
	}
	if (!itsPublishWeeks.empty()) {
		this->hide();
		emit goPublish(itsPublishWeeks);
	}
}

void PublishDialog::initPublishDialog(void) {
	ui.listWidget_Weeks->clear();
	const scheduleWeekVector &weeks = Controller::theSchedulerSettings.getScheduleWeeks();
	QString weekStr;

	for (scheduleWeekVector::const_iterator it = weeks.begin(); it != weeks.end(); ++it) {
		weekStr="";
		weekStr = QString(int2String(it->first).c_str()) + QString(", starting on Monday ") + QString(it->second.toString().c_str());
		QListWidgetItem * item = new QListWidgetItem(weekStr);
		item->setData(200, it->second.toJulian()); // store the date of Monday as an int (julian day) in the listwidget item for later comparison
		ui.listWidget_Weeks->addItem(item);
	}
}

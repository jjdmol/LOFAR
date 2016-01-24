/*
 * dataslotdialog.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : June 2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/dataslotdialog.cpp $
 *
 */

#include "dataslotdialog.h"
#include "Controller.h"
#include <string>
#include <QString>

DataSlotDialog::DataSlotDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
}

DataSlotDialog::~DataSlotDialog()
{

}

void DataSlotDialog::clear() {
	ui.textDataSlots->clear();
}

void DataSlotDialog::loadData(const dataSlotMap &dataSlots) {
	ui.textDataSlots->clear();
	QString text;
	std::string stationName;
	for (dataSlotMap::const_iterator it = dataSlots.begin(); it != dataSlots.end(); ++it) {
		stationName = Controller::theSchedulerSettings.getStationName(it->first);
		if (!stationName.empty()) {
			text += QString(stationName.c_str()) + ":\n";
			for (stationDataSlotMap::const_iterator sit = it->second.begin(); sit != it->second.end(); ++ sit) {
				text += "RSP board " + QString::number(sit->first) + ": [" + QString::number(sit->second.first) + ".." + QString::number(sit->second.second) + "]\n";
			}
			text += "\n";
		}
	}
	if (text.isEmpty()) {
		ui.textDataSlots->setPlainText("No dataslots assigned for this task");
	}
	else {
		ui.textDataSlots->setPlainText(text);
	}
}


/*
 * schedulesettingsdialog.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11825 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-09-25 11:35:16 +0000 (Thu, 25 Sep 2014) $
 * First creation : 5-march-2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/schedulesettingsdialog.cpp $
 *
 */

#include "schedulesettingsdialog.h"
#include "Controller.h"
#include "SASConnection.h"
#include <iostream>
#include <sstream>
#include <QMessageBox>
#include <QProgressBar>
#include <QFileDialog>
#include <QKeyEvent>

ScheduleSettingsDialog::ScheduleSettingsDialog(Controller * controller) :
	itsController(controller) {
	ui.setupUi(this);
	createActions();

	// storage nodes header
	ui.treeWidgetStorageNodes->setColumnCount(5);
	QStringList header;
	header << "Storage node" << "Partition" << "Total Size" << "Free Space" << "Percentage full";
	ui.treeWidgetStorageNodes->setHeaderLabels(header);
	ui.treeWidgetStorageNodes->header()->resizeSection(0,230);
	ui.treeWidgetStorageNodes->header()->resizeSection(1,80);
	ui.treeWidgetStorageNodes->header()->resizeSection(2,80);
	ui.treeWidgetStorageNodes->header()->resizeSection(3,80);

	ui.edit_SAShostname->setText(Controller::theSchedulerSettings.getSASHostName());
	ui.edit_SASdatabase->setText(Controller::theSchedulerSettings.getSASDatabase());
	ui.edit_SASusername->setText(Controller::theSchedulerSettings.getSASUserName());
	ui.edit_SASpassword->setText(Controller::theSchedulerSettings.getSASPassword());

    ui.treeWidgetExcludeStrings->setColumnCount(2);
    header.clear();
    header << "permanent exclude string" << "description";
    ui.treeWidgetExcludeStrings->setHeaderLabels(header);
    ui.treeWidgetExcludeStrings->header()->resizeSection(0,250);

    connect(ui.treeWidgetExcludeStrings, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(checkExcludeString(QTreeWidgetItem*,int)));
}

ScheduleSettingsDialog::~ScheduleSettingsDialog() {

}

void ScheduleSettingsDialog::keyPressEvent(QKeyEvent *event) {
    QTreeWidgetItem *item = ui.treeWidgetExcludeStrings->currentItem();
    if (ui.treeWidgetExcludeStrings->hasFocus() && item) {
        if (event->key() == Qt::Key_Delete && !item->text(0).isEmpty()) {
            delete ui.treeWidgetExcludeStrings->currentItem();
        }
    }
}

void ScheduleSettingsDialog::checkExcludeString(QTreeWidgetItem *, int) {
    QTreeWidgetItem *item(0);
    QStringList strings;
    for(int i = 0; i < ui.treeWidgetExcludeStrings->topLevelItemCount(); ++i) {
        item = ui.treeWidgetExcludeStrings->topLevelItem(i);
        if (item->text(0).isEmpty()) {
            delete item;
            if (--i == -1) i = 0;
        }
        else if (strings.contains(item->text(0))) { // make sure the items stay unique
            delete item;
            if (--i == -1) i = 0;
        }
        else strings.append(item->text(0));
    }
    ui.treeWidgetExcludeStrings->sortItems(0, Qt::AscendingOrder);

    QTreeWidgetItem *emptyItem = new QTreeWidgetItem();
    emptyItem->setCheckState(0, Qt::Unchecked);
    emptyItem->setFlags(emptyItem->flags () | Qt::ItemIsEditable);
    ui.treeWidgetExcludeStrings->addTopLevelItem(emptyItem);
}

void ScheduleSettingsDialog::show() {
	itsLoadDefaultSettings = Controller::theSchedulerSettings.getLoadDefaultSettingsOnStartUp();
	itsSASHostName = Controller::theSchedulerSettings.getSASHostName();
	itsSASDatabase = Controller::theSchedulerSettings.getSASDatabase();
	itsSASUserName = Controller::theSchedulerSettings.getSASUserName();
	itsSASPassword = Controller::theSchedulerSettings.getSASPassword();
	itsDMHostName = Controller::theSchedulerSettings.getDMHostName();
	itsDMDatabase = Controller::theSchedulerSettings.getDMDatabase();
	itsDMUserName = Controller::theSchedulerSettings.getDMUserName();
	itsDMPassword = Controller::theSchedulerSettings.getDMPassword();
	//storage tab

	// add the storage nodes info
	ui.treeWidgetStorageNodes->clear();
	if (itsController) { // if the controller is set
		const storageHostsMap &nodes = itsController->getStorageNodes();
		const hostPartitionsMap &partitions = itsController->getStoragePartitions();
		const statesMap & states = itsController->getStorageNodesStates();
		updateStorageNodeInfoTree(nodes,states,partitions);
	}

	// SAS tab
	ui.edit_SAShostname->setText(itsSASHostName);
	ui.edit_SASdatabase->setText(itsSASDatabase);
	ui.edit_SASusername->setText(itsSASUserName);
	ui.edit_SASpassword->setText(itsSASPassword);

	// Data Monitor tab
	ui.edit_DMhostname->setText(itsDMHostName);
	ui.edit_DMdatabase->setText(itsDMDatabase);
	ui.edit_DMusername->setText(itsDMUserName);
	ui.edit_DMpassword->setText(itsDMPassword);

    // Permanent Filter tab
    ui.treeWidgetExcludeStrings->clear();
    itsExcludeStrings = Controller::theSchedulerSettings.getExcludeStrings();
    for (QMap<QString, QPair<bool, QString> >::iterator i = itsExcludeStrings.begin(); i != itsExcludeStrings.end(); ++i) {
        QStringList str;
        str << i.key() << i.value().second;
        QTreeWidgetItem *item = new QTreeWidgetItem(str);
        item->setFlags(item->flags () | Qt::ItemIsEditable);
        if (i.value().first) {
            item->setCheckState(0, Qt::Checked);
        }
        else {
            item->setCheckState(0, Qt::Unchecked);
        }
        ui.treeWidgetExcludeStrings->addTopLevelItem(item);
    }
    ui.treeWidgetExcludeStrings->sortItems(0, Qt::AscendingOrder);
    // add an emtpy item at the end of the list for the user to be able to add a new string
    QTreeWidgetItem *emptyItem = new QTreeWidgetItem();
    emptyItem->setCheckState(0, Qt::Unchecked);
    emptyItem->setFlags (emptyItem->flags () | Qt::ItemIsEditable);
    ui.treeWidgetExcludeStrings->addTopLevelItem(emptyItem);


	this->setVisible(true);
}

void ScheduleSettingsDialog::okClicked(void) {
	// general tab
	itsLoadDefaultSettings = ui.cb_LoadDefaultSettings->isChecked();

	// SAS tab
	if ((itsSASHostName != ui.edit_SAShostname->text()) ||
		(itsSASDatabase != ui.edit_SASdatabase->text()) ||
		(itsSASUserName != ui.edit_SASusername->text()) ||
		(itsSASPassword != ui.edit_SASpassword->text())) {
		updateSASConnectionSettings();
	}

	// Data Monitor tab
	itsDMHostName = ui.edit_DMhostname->text();
	itsDMDatabase = ui.edit_DMdatabase->text();
	itsDMUserName = ui.edit_DMusername->text();
	itsDMPassword = ui.edit_DMpassword->text();

    // update itsExcludeStrings
    itsExcludeStrings.clear();
    QTreeWidgetItem *item(0);
    for(int i = 0; i < ui.treeWidgetExcludeStrings->topLevelItemCount(); ++i) {
        item = ui.treeWidgetExcludeStrings->topLevelItem(i);
        const QString &txt(item->text(0));
        if (!txt.isEmpty()) {
            itsExcludeStrings[txt] = QPair<bool, QString>((item->checkState(0) == Qt::Checked), item->text(1));
        }
    }

	emit actionSaveSettings();
}

void ScheduleSettingsDialog::updateSASConnectionSettings(void) {
	itsSASHostName = ui.edit_SAShostname->text();
	itsSASDatabase = ui.edit_SASdatabase->text();
	itsSASUserName = ui.edit_SASusername->text();
	itsSASPassword = ui.edit_SASpassword->text();
	itsController->setSASConnectionSettings(itsSASUserName, itsSASPassword, itsSASDatabase, itsSASHostName);
}

void ScheduleSettingsDialog::cancelClicked(void) {
	this->close();
}

void ScheduleSettingsDialog::createActions(void) {
	connect(ui.pb_Ok, SIGNAL(clicked()), this, SLOT(okClicked()));
	connect(ui.pb_Cancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
	connect(ui.pb_RefreshStorageNodesInfo, SIGNAL(clicked()), this, SLOT(doRefreshStorageNodesInfo()));
}

void ScheduleSettingsDialog::doRefreshStorageNodesInfo(void) {
	// first get the list of allowed storage nodes (user could have changed them)
	ui.pb_RefreshStorageNodesInfo->setEnabled(false);

    ui.treeWidgetStorageNodes->clear();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QCoreApplication::processEvents();
    itsController->refreshStorageNodesInfo();

	ui.pb_RefreshStorageNodesInfo->setEnabled(true);
}


bool ScheduleSettingsDialog::checkSASsettings(void) {
	if (ui.edit_SASusername->text().isEmpty()) {
		QMessageBox::critical(this, tr("SAS user name not specified"),
				tr("SAS user name is not specified.\n Please specify this via menu 'Settings' -> menu option 'Schedule settings' -> 'SAS' tab"));
		return false;
	}
	else if (ui.edit_SASpassword->text().isEmpty()) {
		QMessageBox::critical(this, tr("SAS password not set"),
				tr("SAS password is not specified.\n Please specify this via menu 'Settings' -> menu option 'Schedule settings' -> 'SAS' tab"));
		return false;
	}
	else if (ui.edit_SASdatabase->text().isEmpty()) {
		QMessageBox::critical(this, tr("SAS database name not specified"),
				tr("SAS database name is not specified.\n Please specify this via menu 'Settings' -> menu option 'Schedule settings' -> 'SAS' tab"));
		return false;
	}
	else if (ui.edit_SAShostname->text().isEmpty()) {
		QMessageBox::critical(this, tr("SAS host name not specified"),
				tr("SAS host name is not specified.\n Please specify this via menu 'Settings' -> menu option 'Schedule settings' -> 'SAS' tab"));
		return false;
	}
	return true;
}

int ScheduleSettingsDialog::testSASconnection(bool quietWhenOk) {
	int result(-3);
	if (checkSASsettings()) {
		result = itsController->checkSASconnection(ui.edit_SASusername->text(), ui.edit_SASpassword->text(), ui.edit_SASdatabase->text(), ui.edit_SAShostname->text());
		if ((result == 0) & !quietWhenOk) {
			QMessageBox::information(this, tr("Connection to SAS OK"),
					tr("Connection tests successful. Connection to SAS is OK."));
		}
		else if (result == -1) {
			QApplication::beep();
			QMessageBox::critical(this, tr("No connection to SAS"),
					tr("Could not connect to SAS database.\n Please check SAS connection settings.\n\nError:\n") +
					itsController->lastSASError());
		}
		else if (result == -2) {
			QApplication::beep();
			QMessageBox::warning(this, tr("No write permissions to SAS database"),
					tr("You don't have write permissions to the SAS database.\n Please check the SAS user name and password"));
		}
	}
	return result;
}


void ScheduleSettingsDialog::updateStorageNodeInfoTree(const storageHostsMap &nodes, const statesMap &states, const hostPartitionsMap &hostPartitions) {
	// Storage Node, Partition, Node Status, Total Size (kb), Free Space (kb)
	QTreeWidgetItem *nodeItem, *childItem;
	QList<QTreeWidgetItem *> items;
	std::string hostName, hostStatus(" (Status unknown)");
	statesMap::const_iterator sit;
	hostPartitionsMap::const_iterator pit;
	QStringList itemValues;
	int nodeStatus, total, free;
	std::vector<std::pair<QTreeWidgetItem *, QProgressBar *> > progressBarVector;
	for (storageHostsMap::const_iterator hit = nodes.begin(); hit != nodes.end(); ++hit) {
		hostName = hit->second.itsName;
		nodeStatus = hit->second.itsStatus;
		sit = states.find(nodeStatus);
		if (sit != states.end())
			hostStatus = " (" + sit->second + ")";
		// node name and status
		hostName += hostStatus;
		nodeItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList(hostName.c_str()));
        nodeItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		nodeItem->setData(0,Qt::UserRole,hit->second.itsID); // store the storage node ID in the treewidget item for later identification of allowed storage nodes

		if (nodeStatus == 0) { // inactive status
			nodeItem->setTextColor(0,Qt::red);
		}
		// see which partition IDs belong to this host
		pit = hostPartitions.find(hit->second.itsID);
		if (pit != hostPartitions.end()) {
			for (dataPathsMap::const_iterator dpit = pit->second.begin(); dpit != pit->second.end(); ++dpit) { // iterate over all partitions for this host
					itemValues << "" << dpit->second.first.c_str() /*partition name */
					<< humanReadableUnits(dpit->second.second[0], SIZE_UNITS).c_str() /*total space*/ << humanReadableUnits(dpit->second.second[3], SIZE_UNITS).c_str() /*free space*/;
					childItem = new QTreeWidgetItem(nodeItem, itemValues);
					if (nodeStatus == 0) { // inactive status
						childItem->setTextColor(1,Qt::red);
						childItem->setTextColor(2,Qt::red);
						childItem->setTextColor(3,Qt::red);
					}
					// create a progress bar showing the percentage of free space
					QProgressBar *pBar = new QProgressBar();
					pBar->setMaximum(100);

					total = dpit->second.second[0] / 1000;
					free = dpit->second.second[3] / 1000;

					if (total != 0) {
						pBar->setValue((int)((float)(total-free) / total * 100));
					}

					progressBarVector.push_back(std::pair<QTreeWidgetItem *, QProgressBar *>(childItem, pBar));

					itemValues.clear();
			}
		}
		items.append(nodeItem);
	}
	ui.treeWidgetStorageNodes->insertTopLevelItems(0, items);
	// add the progress bars, this can only be done now because the childitems have to be added to the treewidget before setItemWidget can be called
	for (std::vector<std::pair<QTreeWidgetItem *, QProgressBar *> >::iterator pbit = progressBarVector.begin();pbit!=progressBarVector.end(); ++pbit) {
		ui.treeWidgetStorageNodes->setItemWidget(pbit->first, 4, pbit->second);
	}

	ui.treeWidgetStorageNodes->expandAll();
	ui.pb_RefreshStorageNodesInfo->setEnabled(true);
	QApplication::restoreOverrideCursor();
}


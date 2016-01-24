/*
 * schedulesettingsdialog.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 5-march-2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/schedulesettingsdialog.cpp $
 *
 */

#include "schedulesettingsdialog.h"
#include "Controller.h"
#include "astrodate.h"
#include "astrotime.h"
#include "SASConnection.h"
#include <iostream>
#include <sstream>
#include <QMessageBox>
#include <QFileDialog>

const char *storage_distributions_str[NR_STORAGE_DISTRIBUTIONS] = {"Spread (flat usage)","Least fragmented (fill up nodes)"};

ScheduleSettingsDialog::ScheduleSettingsDialog(Controller * controller) :
	itsController(controller) {
	ui.setupUi(this);
	createActions();
	ui.lineEdit_StationName->setInputMask(">nnnnn;_");
	ui.pb_ClearStationList->setEnabled(false);
	ui.pb_DeleteStation->setEnabled(false);
	ui.lineEdit_Latitude->setInputMask("#09.0000000");
	ui.lineEdit_Latitude->setText("+0.0");
	ui.lineEdit_Longitude->setInputMask("#099.0000000");
	ui.lineEdit_Longitude->setText("+0.0");

	AstroDate day(Controller::theSchedulerSettings.getEarliestSchedulingDay());
	itsEarliestDay.setDate(day.getYear(), day.getMonth(), day.getDay());
	ui.dateEdit_Earliest->setDate(itsEarliestDay);
//	ui.dateEdit_Earliest->setMinimumDate(itsEarliestDay);

	day = Controller::theSchedulerSettings.getLatestSchedulingDay();
	itsLatestDay.setDate(day.getYear(), day.getMonth(), day.getDay());
	ui.dateEdit_Latest->setDate(itsLatestDay);
//	ui.dateEdit_Latest->setMinimumDate(itsEarliestDay.addDays(1));

	AstroTime ti(Controller::theSchedulerSettings.getMinimumTimeBetweenTasks());
	itsMinTimeBetweenTasks.setHMS(ti.getHours(), ti.getMinutes(), ti.getSeconds());
	ui.timeEdit_MinTimeBetweenObservations->setTime(itsMinTimeBetweenTasks);

	itsMaxNrOptimizeIterations = Controller::theSchedulerSettings.getMaxNrOptimizations();
	ui.sbMaxNrOptimizations->setValue(itsMaxNrOptimizeIterations);
	itsUserAcceptedPenalty = Controller::theSchedulerSettings.getUserAcceptedPenalty();
	ui.sbUserAcceptedPenalty->setValue(itsUserAcceptedPenalty);
	itsAllowUnscheduleFixedTasks = Controller::theSchedulerSettings.getAllowUnscheduleFixedTasks();

//	ui.cbUnscheduleFixedTasksAllowed->setChecked(itsAllowUnscheduleFixedTasks);
	itsIsTestEnvironment = Controller::theSchedulerSettings.getIsTestEnvironment();
	ui.checkBoxTestEnvironment->setChecked(itsIsTestEnvironment);

	itsShortTermScheduleDuration = Controller::theSchedulerSettings.getShortTermScheduleDurationWeeks();
	ui.sp_ShortTermDurationWeeks->setValue(itsShortTermScheduleDuration);
	itsScheduleDuration = Controller::theSchedulerSettings.getScheduleDurationMonths();
	ui.sb_ScheduleDurationMonths->setValue(itsScheduleDuration);

	// storage nodes header
	ui.treeWidgetStorageNodes->setColumnCount(5);
	QStringList header;
	header << "Storage node" << "Partition" << "Total Size" << "Free Space" << "Percentage full";
	ui.treeWidgetStorageNodes->setHeaderLabels(header);
//	ui.treeWidgetStorageNodes->header()->setResizeMode(QHeaderView::ResizeToContents);
	ui.treeWidgetStorageNodes->header()->resizeSection(0,230);
	ui.treeWidgetStorageNodes->header()->resizeSection(1,80);
	ui.treeWidgetStorageNodes->header()->resizeSection(2,80);
	ui.treeWidgetStorageNodes->header()->resizeSection(3,80);
	// add the storage nodes itself
	/*
	QList<QTreeWidgetItem *> items;
	const storageNodeNameIDmap &storagenodes = Controller::theSchedulerSettings.getStorageNodesNameIDmapping();
	const std::map<int, storagePartitionsMap> &partitions = Controller::theSchedulerSettings.getStoragePartitionsMap();
	std::map<int, storagePartitionsMap>::const_iterator pit;
	QTreeWidgetItem *nodeItem;
	QStringList itemValues;
	for (storageNodeNameIDmap::const_iterator stit = storagenodes.begin(); stit != storagenodes.end(); ++stit) {
		nodeItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList(stit->second.c_str()));
		pit = partitions.find(stit->first);
		for (storagePartitionsMap::const_iterator ppit = pit->second.begin(); ppit != pit->second.end(); ++ppit) {
			itemValues << "" << ppit->second.first.c_str() << "online" << QString::number(ppit->second.second) << "100";
			new QTreeWidgetItem(nodeItem, itemValues);
			itemValues.clear();
		}
	    items.append(nodeItem);
	}
	ui.treeWidgetStorageNodes->insertTopLevelItems(0, items);
	*/

	// Storage distribution
	for (int i = 0; i < NR_STORAGE_DISTRIBUTIONS; ++i) {
		ui.comboBoxDataDistribution->addItem(storage_distributions_str[i]);
	}

	// init product data type storage preferences table
	ui.tableWidgetDataProductStorage->setRowCount(DP_UNKNOWN_TYPE);
	ui.tableWidgetDataProductStorage->setColumnCount(1);
//	ui.tableWidgetDataProductStorage->setSelectionMode(QAbstractItemView::MultiSelection);
	ui.tableWidgetDataProductStorage->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidgetDataProductStorage->horizontalHeader()->hide();
	ui.tableWidgetDataProductStorage->verticalHeader()->hide();
	QTableWidgetItem *item;
	for (int row = 0; row < DP_UNKNOWN_TYPE; ++row) {
		item = new QTableWidgetItem(DATA_PRODUCTS[row]);
		item->setFlags(Qt::ItemIsEnabled);
		ui.tableWidgetDataProductStorage->setItem(row,0,item);
	}
	ui.tableWidgetDataProductStorage->resizeColumnsToContents();

	ui.tableWidgetProjectStorage->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidgetProjectStorage->horizontalHeader()->hide();
	ui.tableWidgetProjectStorage->verticalHeader()->hide();

	ui.edit_SAShostname->setText(Controller::theSchedulerSettings.getSASHostName());
	ui.edit_SASdatabase->setText(Controller::theSchedulerSettings.getSASDatabase());
	ui.edit_SASusername->setText(Controller::theSchedulerSettings.getSASUserName());
	ui.edit_SASpassword->setText(Controller::theSchedulerSettings.getSASPassword());
//	ui.spinBox_DefaultTemplateTreeID->setValue(Controller::theSchedulerSettings.getSASDefaultTreeID());

	ui.tableWidgetDefaultTemplates->setColumnCount(7);
	header.clear();
	header << "ID" << "Name" << "Process type" << "Process subtype" << "Strategy" << "Status" << "Description";
	ui.tableWidgetDefaultTemplates->setHorizontalHeaderLabels(header);
	ui.tableWidgetDefaultTemplates->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidgetDefaultTemplates->horizontalHeader()->setResizeMode(QHeaderView::Interactive);

    ui.checkBoxAutoPublish->setChecked(Controller::theSchedulerSettings.getAutoPublish());
	itsPublishLocal = Controller::theSchedulerSettings.publishLocal();
	itsLocalPublishPath = Controller::theSchedulerSettings.getLocalPublishPath();
	itsSchedulerAccountName = Controller::theSchedulerSettings.getSchedulerAccountName();
	itsPrivateKeyFile = Controller::theSchedulerSettings.getPrivateKeyFile();
	itsWebServerName = Controller::theSchedulerSettings.getWebServerName();
	itsWebServerPublishPath = Controller::theSchedulerSettings.getWebServerPublishPath();

	ui.radioButton_PublishLocal->setChecked(itsPublishLocal);
	ui.radioButton_PublishToWeb->setChecked(!itsPublishLocal);
	ui.lineEdit_PublishAccountName->setText(itsSchedulerAccountName);
	ui.lineEdit_PublishPrivateKeyFile->setText(itsPrivateKeyFile);
	ui.lineEdit_PublishWebServerName->setText(itsWebServerName);
	ui.lineEdit_PublishWebServerDirectory->setText(itsWebServerPublishPath);

	ui.lineEditStorageNodeBandwidth->setText(QString::number(Controller::theSchedulerSettings.getStorageNodeBandWidth()));
	ui.radioButtonGbs->setChecked(true);

	itsPublishLocal = Controller::theSchedulerSettings.publishLocal();
	if (itsPublishLocal) {
		ui.radioButton_PublishLocal->setChecked(true);
	}
	else {
		ui.radioButton_PublishToWeb->setChecked(true);
	}
	enablePublishField();
    // allow auto publish?
    if (itsController->autoPublishAllowed()) {
        ui.checkBoxAutoPublish->setEnabled(true);
        ui.checkBoxAutoPublish->setChecked(true);
    }
    else {
        ui.checkBoxAutoPublish->setEnabled(false);
        ui.checkBoxAutoPublish->setChecked(false);
        ui.checkBoxAutoPublish->setToolTip("Auto publishing is only allowed for user lofarsys");
    }
}

ScheduleSettingsDialog::~ScheduleSettingsDialog() {

}

void ScheduleSettingsDialog::doubleClickedStation(const QModelIndex &index) {
	QStringList stationInfo(index.data(Qt::UserRole).toStringList());
	if (stationInfo.size() == 3) {
		ui.lineEdit_StationName->setText(stationInfo.at(0));
		ui.lineEdit_Latitude->setText(stationInfo.at(1));
		ui.lineEdit_Longitude->setText(stationInfo.at(2));
	}
}

void ScheduleSettingsDialog::show() {
	// load the settings from the program settings into this dialog
	itsStations = Controller::theSchedulerSettings.getStationList();

	setEarliestSchedulingDay(Controller::theSchedulerSettings.getEarliestSchedulingDay());
	setLatestSchedulingDay(Controller::theSchedulerSettings.getLatestSchedulingDay());
	itsUserAcceptedPenalty = Controller::theSchedulerSettings.getUserAcceptedPenalty();
	itsUserAcceptedPenaltyEnabled = Controller::theSchedulerSettings.getUserAcceptedPenaltyEnabled();
	itsMaxNrOptimizeIterations = Controller::theSchedulerSettings.getMaxNrOptimizations();
	itsMaxNrOptimizationsEnabled = Controller::theSchedulerSettings.getMaxNrOptimizationsEnabled();
	setMinimumTimeBetweenTasks(Controller::theSchedulerSettings.getMinimumTimeBetweenTasks());
	itsLoadDefaultSettings = Controller::theSchedulerSettings.getLoadDefaultSettingsOnStartUp();
	itsIsTestEnvironment = Controller::theSchedulerSettings.getIsTestEnvironment();
	itsShortTermScheduleDuration = Controller::theSchedulerSettings.getShortTermScheduleDurationWeeks();
	itsScheduleDuration = Controller::theSchedulerSettings.getScheduleDurationMonths();
	itsAllowUnscheduleFixedTasks = Controller::theSchedulerSettings.getAllowUnscheduleFixedTasks();
	itsSASHostName = Controller::theSchedulerSettings.getSASHostName();
	itsSASDatabase = Controller::theSchedulerSettings.getSASDatabase();
	itsSASUserName = Controller::theSchedulerSettings.getSASUserName();
	itsSASPassword = Controller::theSchedulerSettings.getSASPassword();
	itsDMHostName = Controller::theSchedulerSettings.getDMHostName();
	itsDMDatabase = Controller::theSchedulerSettings.getDMDatabase();
	itsDMUserName = Controller::theSchedulerSettings.getDMUserName();
	itsDMPassword = Controller::theSchedulerSettings.getDMPassword();
	itsPublishLocal = Controller::theSchedulerSettings.publishLocal();
	itsLocalPublishPath = Controller::theSchedulerSettings.getLocalPublishPath();
	itsSchedulerAccountName = Controller::theSchedulerSettings.getSchedulerAccountName();
	itsWebServerName = Controller::theSchedulerSettings.getWebServerName();
	itsPrivateKeyFile = Controller::theSchedulerSettings.getPrivateKeyFile();
	itsWebServerPublishPath = Controller::theSchedulerSettings.getWebServerPublishPath();
	itsMaxNrOfFilesPerStorageNode = Controller::theSchedulerSettings.getMaxNrOfFilesPerStorageNode();
	itsDemixSources = Controller::theSchedulerSettings.getDemixSources();

//	if ((!ui.edit_SASdatabase->text().isEmpty()) & (!ui.edit_SAShostname->text().isEmpty())) {
//		updateDefaultTemplates(true);
//	}
	writeDefaultTemplatesToDialog();

	// general tab
	ui.cb_LoadDefaultSettings->setChecked(itsLoadDefaultSettings);
	ui.checkBoxTestEnvironment->setChecked(itsIsTestEnvironment);

	// scheduler tab
	ui.dateEdit_Earliest->setDate(itsEarliestDay);
	ui.dateEdit_Latest->setDate(itsLatestDay);
	ui.timeEdit_MinTimeBetweenObservations->setTime(itsMinTimeBetweenTasks);
	ui.sp_ShortTermDurationWeeks->setValue(itsShortTermScheduleDuration);
	ui.sb_ScheduleDurationMonths->setValue(itsScheduleDuration);

	//storage tab
	itsStorageRaidWriteSpeed = Controller::theSchedulerSettings.getRaidMaxWriteSpeed(); // in kByte/sec
	ui.lineEditStorageRaidWriteSpeed->setText(QString::number(itsStorageRaidWriteSpeed/1000));
	ui.spinBoxStorageFillPercentage->setValue(Controller::theSchedulerSettings.getStorageFillPercentage());

	itsStorageNodeBandWidth = Controller::theSchedulerSettings.getStorageNodeBandWidth(); // in kb/s
	if (itsStorageNodeBandWidth > 999999) {
		ui.lineEditStorageNodeBandwidth->setText(QString::number(itsStorageNodeBandWidth / 1000000));
		ui.radioButtonGbs->setChecked(true);
	}
	else {
		ui.lineEditStorageNodeBandwidth->setText(QString::number(itsStorageNodeBandWidth / 1000));
		ui.radioButtonMbs->setChecked(true);
	}

	// add the storage nodes info
	ui.treeWidgetStorageNodes->clear();
	if (itsController) { // if the controller is set
		const storageHostsMap &nodes = itsController->getStorageNodes();
		const hostPartitionsMap &partitions = itsController->getStoragePartitions();
		const statesMap & states = itsController->getStorageNodesStates();
		updateStorageNodeInfoTree(nodes,states,partitions);
		updatePreferredStorageLists(nodes);
	}

	ui.tableWidgetDataProductStorage->clearSelection();
	ui.tableWidgetDataProductStorage->scrollTo(ui.tableWidgetDataProductStorage->model()->index(0,0));
	ui.tableWidgetProjectStorage->clearSelection();
	ui.tableWidgetProjectStorage->scrollTo(ui.tableWidgetProjectStorage->model()->index(0,0));

	int storage_dist(Controller::theSchedulerSettings.getStorageDistribution());
	if ((storage_dist >= 0) & (storage_dist < ui.comboBoxDataDistribution->count())) {
		ui.comboBoxDataDistribution->setCurrentIndex(storage_dist);
	}

	ui.spinBoxMaxNrFilesToNode->setValue(itsMaxNrOfFilesPerStorageNode);

	// optimization tab
	ui.cbMaxNrOptimizations->setChecked(itsMaxNrOptimizationsEnabled);
	ui.cbUserAcceptedPenalty->setChecked(itsUserAcceptedPenaltyEnabled);
	ui.sbMaxNrOptimizations->setValue(itsMaxNrOptimizeIterations);
	ui.sbMaxNrOptimizations->setEnabled(itsMaxNrOptimizationsEnabled);
	ui.sbUserAcceptedPenalty->setValue(itsUserAcceptedPenalty);
	ui.sbUserAcceptedPenalty->setEnabled(itsUserAcceptedPenaltyEnabled);

	// stations tab
	//populate the list of stations
	ui.listStations->clear();
	const stationDefinitionsMap &stations(Controller::theSchedulerSettings.getStationList());
	if (!stations.empty()) {
	QString name, latitude, longitude;
	QStringList itemInfo;
	for (stationDefinitionsMap::const_iterator it = stations.begin(); it != stations.end(); ++it) {
		latitude.setNum(it->second.first, 'f', 7);
		longitude.setNum(it->second.second, 'f', 7);
		name = it->first.c_str();
		itemInfo << name << latitude << longitude;
		QListWidgetItem *newStation = new QListWidgetItem(name + " (" + latitude + ", " + longitude + ")");
		newStation->setData(Qt::UserRole,itemInfo);
		ui.listStations->addItem(newStation);
		itemInfo.clear();
	}
	ui.lineEdit_Latitude->setText("+0.0");
	ui.lineEdit_Longitude->setText("+0.0");
	ui.lineEdit_StationName->setText("");
	ui.pb_ClearStationList->setEnabled(true);
	ui.pb_DeleteStation->setEnabled(true);
	} else {
		ui.pb_ClearStationList->setEnabled(false);
		ui.pb_DeleteStation->setEnabled(false);
	}

	// SAS tab
	ui.edit_SAShostname->setText(itsSASHostName);
	ui.edit_SASdatabase->setText(itsSASDatabase);
	ui.edit_SASusername->setText(itsSASUserName);
	ui.edit_SASpassword->setText(itsSASPassword);

	// pipelines tab
	ui.tableWidgetDemixSources->blockSignals(true);
	ui.tableWidgetDemixSources->setRowCount(itsDemixSources.count());
	for (int row = 0; row < itsDemixSources.count(); ++row) {
		ui.tableWidgetDemixSources->setItem(row, 0, new QTableWidgetItem(itsDemixSources.at(row)));
	}
	ui.tableWidgetDemixSources->blockSignals(false);

	// Data Monitor tab
	ui.edit_DMhostname->setText(itsDMHostName);
	ui.edit_DMdatabase->setText(itsDMDatabase);
	ui.edit_DMusername->setText(itsDMUserName);
	ui.edit_DMpassword->setText(itsDMPassword);

	// publish tab
    ui.checkBoxAutoPublish->setChecked(Controller::theSchedulerSettings.getAutoPublish());
	ui.radioButton_PublishLocal->setChecked(itsPublishLocal);
	ui.radioButton_PublishToWeb->setChecked(!itsPublishLocal);
	ui.lineEdit_PublishLocalPath->setText(itsLocalPublishPath);
	ui.lineEdit_PublishAccountName->setText(itsSchedulerAccountName);
	ui.lineEdit_PublishWebServerName->setText(itsWebServerName);
	ui.lineEdit_PublishPrivateKeyFile->setText(itsPrivateKeyFile);
	ui.lineEdit_PublishWebServerDirectory->setText(itsWebServerPublishPath);

	enablePublishField();
	ui.pb_Ok->setDefault(true);

	this->setVisible(true);
}

void ScheduleSettingsDialog::enablePublishField(void) {
	if (ui.radioButton_PublishLocal->isChecked()) { // local
		ui.label_PublishLocalPath->setEnabled(true);
		ui.label_PublishSCPCommand->setEnabled(false);
		ui.label_PublishWebServerPath->setEnabled(false);
		ui.label_PublishPrivateKeyFile->setEnabled(false);
		ui.pushButton_BrowseLocalPath->setEnabled(true);
		ui.pushButton_BrowsePrivateKeyFile->setEnabled(false);
		ui.lineEdit_PublishAccountName->setEnabled(false);
		ui.lineEdit_PublishPrivateKeyFile->setEnabled(false);
		ui.lineEdit_PublishWebServerName->setEnabled(false);
		ui.lineEdit_PublishWebServerDirectory->setEnabled(false);
		ui.lineEdit_PublishLocalPath->setEnabled(true);
	}
	else { // web
		ui.label_PublishLocalPath->setEnabled(false);
		ui.label_PublishSCPCommand->setEnabled(true);
		ui.label_PublishWebServerPath->setEnabled(true);
		ui.label_PublishPrivateKeyFile->setEnabled(true);
		ui.pushButton_BrowseLocalPath->setEnabled(false);
		ui.pushButton_BrowsePrivateKeyFile->setEnabled(true);
		ui.lineEdit_PublishAccountName->setEnabled(true);
		ui.lineEdit_PublishPrivateKeyFile->setEnabled(true);
		ui.lineEdit_PublishWebServerName->setEnabled(true);
		ui.lineEdit_PublishWebServerDirectory->setEnabled(true);
		ui.lineEdit_PublishLocalPath->setEnabled(false);
	}
}


AstroDate ScheduleSettingsDialog::getEarliestSchedulingDay(void) const {
	AstroDate dt(itsEarliestDay.day(), itsEarliestDay.month(), itsEarliestDay.year());
	return dt;
}

AstroDate ScheduleSettingsDialog::getLatestSchedulingDay(void) const {
	AstroDate dt(itsLatestDay.day(), itsLatestDay.month(), itsLatestDay.year());
	return dt;
}

AstroTime ScheduleSettingsDialog::getMinimumTimeBetweenObservations(void) const {
	AstroTime t(itsMinTimeBetweenTasks.hour(), itsMinTimeBetweenTasks.minute(), itsMinTimeBetweenTasks.second());
	return t;

}

void ScheduleSettingsDialog::okClicked(void) {
	// general tab
	itsLoadDefaultSettings = ui.cb_LoadDefaultSettings->isChecked();
	itsIsTestEnvironment = ui.checkBoxTestEnvironment->isChecked();

	// schedule tab
    itsEarliestDay = ui.dateEdit_Earliest->date(); // always on a monday
    itsLatestDay = ui.dateEdit_Latest->date();
	itsMinTimeBetweenTasks = ui.timeEdit_MinTimeBetweenObservations->time();
	itsShortTermScheduleDuration = ui.sp_ShortTermDurationWeeks->value();
	itsScheduleDuration = ui.sb_ScheduleDurationMonths->value();

	//stations tab

	//storage tab
	// first get the list of allowed storage nodes (user could have changed them)
	std::vector<int> allowedStorageHosts;
    QTreeWidgetItemIterator treeit(ui.treeWidgetStorageNodes);
    while (*treeit) {
        if ((*treeit)->checkState(0))
        	allowedStorageHosts.push_back((*treeit)->data(0,Qt::UserRole).toInt()); // push the storage host id in allowedStorageHosts
        ++treeit;
    }
    itsController->setAllowedStorageHosts(allowedStorageHosts); // updates the itsMayBeUsed fields of the storage nodes in the DM connection
    itsMaxNrOfFilesPerStorageNode = ui.spinBoxMaxNrFilesToNode->value();

	if (ui.radioButtonGbs->isChecked()) {
		itsStorageNodeBandWidth = ui.lineEditStorageNodeBandwidth->text().toDouble() * 1000000;
	}
	else {
		itsStorageNodeBandWidth = ui.lineEditStorageNodeBandwidth->text().toDouble() * 1000;
	}
	itsStorageRaidWriteSpeed = ui.lineEditStorageRaidWriteSpeed->text().toDouble() * 1000;

	Controller::theSchedulerSettings.setStorageFillPercentage(ui.spinBoxStorageFillPercentage->value());

	// pipelines

	itsDemixSources.clear();
	for (int row = 0; row < ui.tableWidgetDemixSources->rowCount(); ++row) {
		itsDemixSources.append(ui.tableWidgetDemixSources->item(row,0)->text());
	}
	itsDemixSources.removeDuplicates();
	itsDemixSources.sort();
	ui.tableWidgetDemixSources->blockSignals(true);
	ui.tableWidgetDemixSources->clearContents();
	ui.tableWidgetDemixSources->setRowCount(itsDemixSources.size());
	for (int i = 0; i < itsDemixSources.size(); ++i) {
		ui.tableWidgetDemixSources->setItem(i,0,new QTableWidgetItem(itsDemixSources.at(i)));
	}
	ui.tableWidgetDemixSources->blockSignals(false);

	// optimization tab
	itsUserAcceptedPenaltyEnabled = ui.cbUserAcceptedPenalty->isChecked();
	itsMaxNrOptimizationsEnabled = ui.cbMaxNrOptimizations->isChecked();
	itsUserAcceptedPenalty = ui.sbUserAcceptedPenalty->value();
	itsMaxNrOptimizeIterations = ui.sbMaxNrOptimizations->value();

	// SAS tab
	if ((itsSASHostName != ui.edit_SAShostname->text()) ||
		(itsSASDatabase != ui.edit_SASdatabase->text()) ||
		(itsSASUserName != ui.edit_SASusername->text()) ||
		(itsSASPassword != ui.edit_SASpassword->text())) {
		updateSASConnectionSettings();
		updateDefaultTemplates(true);
	}

	// Data Monitor tab
	itsDMHostName = ui.edit_DMhostname->text();
	itsDMDatabase = ui.edit_DMdatabase->text();
	itsDMUserName = ui.edit_DMusername->text();
	itsDMPassword = ui.edit_DMpassword->text();

	// publish tab
	itsPublishLocal = ui.radioButton_PublishLocal->isChecked();
	itsLocalPublishPath = ui.lineEdit_PublishLocalPath->text();
	itsSchedulerAccountName = ui.lineEdit_PublishAccountName->text();
	itsWebServerName = ui.lineEdit_PublishWebServerName->text();
	itsPrivateKeyFile = ui.lineEdit_PublishPrivateKeyFile->text();
	itsWebServerPublishPath = ui.lineEdit_PublishWebServerDirectory->text();

	Controller::theSchedulerSettings.defineScheduleAndStations(itsStations,getEarliestSchedulingDay(),getLatestSchedulingDay());

	// save the preferred storage node per data produkt type and per project
	Controller::theSchedulerSettings.setPreferredDataProductStorage(getPreferredDataProductStorage());
	Controller::theSchedulerSettings.setPreferredProjectStorage(getPreferredProjectStorage());
	// save the type of storage distribution
	Controller::theSchedulerSettings.setStorageDistribution(static_cast<storageNodeDistribution>(ui.comboBoxDataDistribution->currentIndex()));

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
	connect(ui.pb_ClearStationList, SIGNAL(clicked()), this,
			SLOT(clearStationList()));
	connect(ui.pb_AddStation, SIGNAL(clicked()), this, SLOT(addStation()));
	connect(ui.pb_DeleteStation, SIGNAL(clicked()), this, SLOT(removeStation()));
	connect(ui.pushButton_BrowseLocalPath, SIGNAL(clicked()), this, SLOT(openLocalPathBrowseDialog()));
	connect(ui.pushButton_BrowsePrivateKeyFile, SIGNAL(clicked()), this, SLOT(privateKeyBrowseDialog()));
	connect(ui.radioButton_PublishLocal,SIGNAL(toggled(bool)), this, SLOT(enablePublishField()));
	connect(ui.pb_RefreshStorageNodesInfo, SIGNAL(clicked()), this, SLOT(doRefreshStorageNodesInfo()));
	connect(ui.pushButtonUpdateDefaultTemplates, SIGNAL(clicked()), this, SLOT(updateDefaultTemplates()));
	connect(ui.listStations, SIGNAL(doubleClicked(const QModelIndex &)),this, SLOT(doubleClickedStation(const QModelIndex &)));
	connect(ui.tableWidgetDataProductStorage, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(updateDataProductStorageItem(QTableWidgetItem *)));
	connect(ui.tableWidgetProjectStorage, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(updateProjectsStorageItem(QTableWidgetItem *)));
	connect(ui.pushButtonSelectDataProductStorage, SIGNAL(clicked()), this, SLOT(selectDataProductStorageNodes()));
	connect(ui.pushButtonDeselectDataProductStorage, SIGNAL(clicked()), this, SLOT(deselectDataProductStorageNodes()));
	connect(ui.pushButtonSelectProjectStorage, SIGNAL(clicked()), this, SLOT(selectProjectsStorageNodes()));
	connect(ui.pushButtonDeselectProjectStorage, SIGNAL(clicked()), this, SLOT(deselectProjectsStorageNodes()));
	connect(ui.treeWidgetStorageNodes, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(countStorageSelection(QTreeWidgetItem *, int)));
	connect(ui.pushButtonAddDemixSource, SIGNAL(clicked()), this, SLOT(addRowToDemixSourceTable()));
	connect(ui.pushButtonDeleteDemixSource, SIGNAL(clicked()), this, SLOT(deleteDemixSourceFromTable()));
	connect(ui.tableWidgetDemixSources, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(checkDemixSourceItem()));
}

void ScheduleSettingsDialog::checkDemixSourceItem(void) {
	itsDemixSources.clear();
	QTableWidgetItem * item;
	for (int row = 0; row < ui.tableWidgetDemixSources->rowCount(); ++row) {
		item = ui.tableWidgetDemixSources->item(row,0);
		if (itsDemixSources.contains(item->text())) {
			QMessageBox::warning(this,"patch already exist","the demix source patch already exist");
			ui.tableWidgetDemixSources->setCurrentItem(item);
			ui.tableWidgetDemixSources->editItem(item);
			break;
		}
		else {
			itsDemixSources.append(item->text());
		}
	}
}


void ScheduleSettingsDialog::addRowToDemixSourceTable(void) {
	int row(ui.tableWidgetDemixSources->rowCount());
	QTableWidgetItem *item(0);
	if ((row == 0) || (!ui.tableWidgetDemixSources->item(row-1,0)->text().isEmpty())) {
		ui.tableWidgetDemixSources->insertRow(ui.tableWidgetDemixSources->rowCount());
		item = new QTableWidgetItem("");
		ui.tableWidgetDemixSources->setItem(row, 0, item);
	}
	else {
		item = ui.tableWidgetDemixSources->item(row-1,0);
	}
	ui.tableWidgetDemixSources->editItem(item);
}

void ScheduleSettingsDialog::deleteDemixSourceFromTable(void) {
	QList<QTableWidgetItem*> items(ui.tableWidgetDemixSources->selectedItems());
	for (QList<QTableWidgetItem*>::iterator it = items.begin(); it != items.end(); ++ it) {
		ui.tableWidgetDemixSources->removeRow((*it)->row());
	}
}


void ScheduleSettingsDialog::countStorageSelection(QTreeWidgetItem */*item*/, int col) {
	int count(0);
	if (col == 0) {
		QTreeWidgetItemIterator it(ui.treeWidgetStorageNodes, QTreeWidgetItemIterator::Checked);
		while (*it++) {
			++count;
		}
	}
	ui.lineEditNrOfSelectedNodes->setText(QString::number(count));
}

void ScheduleSettingsDialog::writeDefaultTemplatesToDialog(void) {
	const std::map<quint32, DefaultTemplate> &templates(Controller::theSchedulerSettings.getDefaultTemplates());
	ui.tableWidgetDefaultTemplates->setRowCount(templates.size());
	QTableWidgetItem *item;
	int row(0);
	for (std::map<quint32, DefaultTemplate>::const_iterator it = templates.begin(); it != templates.end(); ++it) {
		item = new QTableWidgetItem(QString::number(it->second.treeID));
		ui.tableWidgetDefaultTemplates->setItem(row, 0, item);
		item = new QTableWidgetItem(it->second.name);
		ui.tableWidgetDefaultTemplates->setItem(row, 1, item);
		item = new QTableWidgetItem(it->second.processType);
		ui.tableWidgetDefaultTemplates->setItem(row, 2, item);
		item = new QTableWidgetItem(it->second.processSubtype);
		ui.tableWidgetDefaultTemplates->setItem(row, 3, item);
		item = new QTableWidgetItem(it->second.strategy);
		ui.tableWidgetDefaultTemplates->setItem(row, 4, item);
		item = new QTableWidgetItem(getSasTextState(it->second.status).c_str());
		ui.tableWidgetDefaultTemplates->setItem(row, 5, item);
		item = new QTableWidgetItem(it->second.description);
		ui.tableWidgetDefaultTemplates->setItem(row++, 6, item);
	}
	ui.tableWidgetDefaultTemplates->resizeColumnsToContents();
}

void ScheduleSettingsDialog::updateDefaultTemplates(bool quiet) {
	updateSASConnectionSettings(); // get the possibly altered connection settings from the dialog
	ui.tableWidgetDefaultTemplates->clearContents();
	ui.tableWidgetDefaultTemplates->setRowCount(0);
	int result(itsController->checkSASconnection(ui.edit_SASusername->text(), ui.edit_SASpassword->text(), ui.edit_SASdatabase->text(), ui.edit_SAShostname->text()));
	if ((result == 0) || (result == -2)) {
		Controller::theSchedulerSettings.updateDefaultTemplates();
		writeDefaultTemplatesToDialog();
		if (result == -2) {
			QApplication::beep();
			QMessageBox::warning(this, tr("No write permissions to SAS database"),
					tr("Although the default templates have been successfully updated\nit appears that you don't have write permissions to the SAS database.\nPlease check the SAS user name and password"));
		}
		else if (!quiet) {
			if (Controller::theSchedulerSettings.getSchedulerDefaultTemplate() == 0) {
				QMessageBox::warning(this, tr("Scheduler default template does not exist"),
						tr("Although the default templates have been successfully updated\nthe 'Scheduler default template' could not be found in the SAS database.\n\nPlease create this default template and name it:\n'Scheduler default template'"));
			}
			else {
				QMessageBox::information(this, tr("Default templates updated"),
						tr("Default templates have been successfully updated"));
			}
		}
	}
	else if (!quiet && (result == -1)) {
		QApplication::beep();
		QMessageBox::critical(this, tr("No connection to SAS"),
				tr("Could not connect to SAS database.\n Please check SAS connection settings.\n\nError:\n") +
				itsController->lastSASError());
	}
}

void ScheduleSettingsDialog::doRefreshStorageNodesInfo(void) {
	// first get the list of allowed storage nodes (user could have changed them)
	ui.pb_RefreshStorageNodesInfo->setEnabled(false);

	bool isConnected(itsController->isDataMonitorConnected());

	std::vector<int> allowedStorageHosts;
    QTreeWidgetItemIterator treeit(ui.treeWidgetStorageNodes);
    while (*treeit) {
        if ((*treeit)->checkState(0))
        	allowedStorageHosts.push_back((*treeit)->data(0,Qt::UserRole).toInt()); // push the storage host id in allowedStorageHosts
        ++treeit;
    }
    itsController->setAllowedStorageHosts(allowedStorageHosts); // updates the itsMayBeUsed fields of the storage nodes in the DM connection

	if (isConnected) {
		ui.treeWidgetStorageNodes->clear();
		QApplication::setOverrideCursor(Qt::WaitCursor);
		QCoreApplication::processEvents();
		itsController->refreshStorageNodesInfo();
	}
	else {
		itsController->disconnectDataMonitor(); // connection might have been lost, make sure the old connection is removed
		QApplication::beep();
		if (QMessageBox::question(this, tr("Connect to Data Monitor"), tr(
				"You are currently not connected to the Data Monitor. Do you want to connect now?"),
				QMessageBox::Ok, QMessageBox::No) == QMessageBox::Ok) {
			if (itsController->connectToDataMonitor()) {
				ui.treeWidgetStorageNodes->clear();
				QApplication::setOverrideCursor(Qt::WaitCursor);
				QCoreApplication::processEvents();
				itsController->refreshStorageNodesInfo();
			}
			else {
				QApplication::restoreOverrideCursor();
				QMessageBox::critical(this, tr("No connection to Data Monitor"),
						tr("Could not connect to the Data Monitor.\n Please check Data Monitor connection settings."));
				QCoreApplication::processEvents();
			}
		}
		else { // user doesn't want to connect to data monitor
			ui.treeWidgetStorageNodes->clear();
//			QApplication::setOverrideCursor(Qt::WaitCursor);
//			QCoreApplication::processEvents();
			itsController->refreshStorageNodesInfo(false);
		}
	}
	ui.pb_RefreshStorageNodesInfo->setEnabled(true);
}


bool ScheduleSettingsDialog::checkLocalPublishPath(void) {
	QString path = ui.lineEdit_PublishLocalPath->text();
	if (!path.isEmpty()) {

#ifdef Q_OS_UNIX
	if (path.right(1).compare("/") != 0) {
		path += '/';
	}
#elif defined(Q_OS_WIN)
		if (path.right(1).compare("\\") != 0) {
			path += "\\";
		}
#endif

	}
	else {
		QMessageBox::warning(this, tr("Publish path not valid"),
				tr("The local publish path cannot be left empty.\nPlease enter a local publish path.\n"),
						QMessageBox::Ok, QMessageBox::Ok);
		ui.tabWidget->setCurrentWidget(ui.tab_Publishing);
		ui.lineEdit_PublishLocalPath->setFocus();
		return false;
	}

	QFileInfo fi(path);
	if (!fi.isDir()) {
		QMessageBox::warning(this, tr("Publish path not valid"),
				tr("The publish path does not point to an existing directory.\nPlease enter a correct directory path.\n"),
						QMessageBox::Ok, QMessageBox::Ok);
		ui.tabWidget->setCurrentWidget(ui.tab_Publishing);
		ui.lineEdit_PublishLocalPath->setFocus();
		return false;
	}
	ui.lineEdit_PublishLocalPath->setText(path);
	itsLocalPublishPath = path;
	return true;
}

void ScheduleSettingsDialog::openLocalPathBrowseDialog(void) {
	QFileDialog dialog;
	dialog.setFileMode(QFileDialog::Directory);
	if (!itsLocalPublishPath.isEmpty()) {
		QFileInfo prevPath(itsLocalPublishPath);
		if (prevPath.exists() & prevPath.isDir()) {
			dialog.setDirectory(itsLocalPublishPath);
		}
	}
	dialog.exec();
	if (dialog.result() == QDialog::Accepted) {
		QStringList files = dialog.selectedFiles();
		itsLocalPublishPath = *files.begin();
#ifdef Q_OS_UNIX
		itsLocalPublishPath += '/';
#elif defined(Q_OS_WIN)
		itsLocalPublishPath += "\\";
#endif
		ui.lineEdit_PublishLocalPath->setText(QDir::toNativeSeparators(itsLocalPublishPath));
	}
}

void ScheduleSettingsDialog::privateKeyBrowseDialog(void) {
	QFileDialog dialog;
	if (!itsPrivateKeyFile.isEmpty()) {
		QFileInfo prevFile(itsPrivateKeyFile);
		if (prevFile.exists() & prevFile.isFile()) {
			dialog.setDirectory(prevFile.absoluteDir());
		}
	}
	dialog.setFilter("private key files (*.ppk)");
	dialog.exec();
	if (dialog.result() == QDialog::Accepted) {
		QStringList files = dialog.selectedFiles();
		itsPrivateKeyFile = *files.begin();
		ui.lineEdit_PublishPrivateKeyFile->setText(QDir::toNativeSeparators(itsPrivateKeyFile));
	}
}

void ScheduleSettingsDialog::addStation(void) {
	QString name = ui.lineEdit_StationName->text();
	QString longitude = ui.lineEdit_Longitude->text();
	QString latitude = ui.lineEdit_Latitude->text();
	double flat = latitude.toDouble();
	double flon = longitude.toDouble();
	if (ui.lineEdit_StationName->text().length() < 3) {
		QMessageBox::warning(this, tr("Wrong station name"), tr(
				"Station name should be at least 3 characters"),
				QMessageBox::Ok);
		ui.lineEdit_StationName->setFocus();
	} else if ((flat < -90.0) | (flat > 90)) {
		QMessageBox::warning(this, tr("Wrong latitude"), tr(
				"Latitude should be within the [-90,+90] range"),
				QMessageBox::Ok);
		ui.lineEdit_Latitude->setFocus();
	} else if ((flon < -180) | (flon > 180)) {
		QMessageBox::warning(this, tr("Wrong longitude"), tr(
				"Longitude should be within the [-180,+180] range"),
				QMessageBox::Ok);
		ui.lineEdit_Longitude->setFocus();
	} else {
		std::pair<stationDefinitionsMap::iterator, bool> ret =
				itsStations.insert(stationDefinitionsMap::value_type(
						ui.lineEdit_StationName->text().toStdString(),
						std::pair<double, double>(flat, flon)));
		QStringList itemInfo;
		itemInfo << name << latitude << longitude;
		if (ret.second) {
			QListWidgetItem *newStation = new QListWidgetItem(name + " (" + latitude + ", " + longitude + ")");
			newStation->setData(Qt::UserRole,itemInfo);
			ui.listStations->addItem(newStation);
			ui.pb_ClearStationList->setEnabled(true);
			ui.pb_DeleteStation->setEnabled(true);
		} else {
			stationDefinitionsMap::iterator it = itsStations.find(ui.lineEdit_StationName->text().toStdString());
			it->second.first = flat;
			it->second.second= flon;
			QList<QListWidgetItem*> items = ui.listStations->findItems(ui.lineEdit_StationName->text(), Qt::MatchStartsWith);
			if (!items.isEmpty()) {
				items.front()->setText(ui.lineEdit_StationName->text() + " (" + latitude + ", " + longitude + ")");
				items.front()->setData(Qt::UserRole,itemInfo);
			}
//			QMessageBox::warning(this, tr("Station name exists"), tr(
//					"Station name already exists"), QMessageBox::Ok);
//			ui.lineEdit_StationName->setFocus();
		}
	}
}

void ScheduleSettingsDialog::removeStation(void) {
	int idx(0);
	QListWidgetItem * item2Delete;
	std::string stationName;

	while (ui.listStations->item(idx)) {
		if (ui.listStations->item(idx)->isSelected()) {
			item2Delete = ui.listStations->takeItem(idx);
			stationName = item2Delete->text().toStdString();
			stationName = stationName.substr(0, stationName.find_first_of(' '));
			itsStations.erase(stationName);
		}
		else ++idx;
	}

	if (ui.listStations->count() == 0) {
		ui.pb_ClearStationList->setEnabled(false);
		ui.pb_DeleteStation->setEnabled(false);
	}
}

void ScheduleSettingsDialog::clearStationList(void) {
	if (ui.listStations->count() > 0) {
		if (QMessageBox::question(this, tr("Clear station list"), tr(
				"Are you sure you want to clear the station list?"),
				QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok) {
			ui.listStations->clear();
			itsStations.clear();
			ui.pb_ClearStationList->setEnabled(false);
			ui.pb_DeleteStation->setEnabled(false);
		}
	}
}

void ScheduleSettingsDialog::today(void) {
	QDate day(QDate::currentDate());
	ui.dateEdit_Earliest->setDate(day);
	int dur_months = ui.sb_ScheduleDurationMonths->value();
	ui.dateEdit_Latest->setMinimumDate(day.addDays(1));
	ui.dateEdit_Latest->setDate(day.addMonths(dur_months));
}

void ScheduleSettingsDialog::scheduleStartDateChanged(QDate start) {
	ui.dateEdit_Earliest->blockSignals(true);
	int dur_months = ui.sb_ScheduleDurationMonths->value();
	ui.dateEdit_Latest->setMinimumDate(start.addDays(1));
	// always set to first day of the week (monday)
	ui.dateEdit_Earliest->setDate(ui.dateEdit_Earliest->date());
        // Removed restriction for mondays (next line) [AS]
        // .addDays(-ui.dateEdit_Earliest->date().dayOfWeek() + 1));
	QDate endDate(start.addMonths(dur_months));
	endDate = endDate.addDays(7-endDate.dayOfWeek());
	ui.dateEdit_Latest->setDate(endDate);
	ui.dateEdit_Earliest->blockSignals(false);
}

void ScheduleSettingsDialog::scheduleEndDateChanged(QDate endDate) {
	ui.dateEdit_Latest->blockSignals(true);
	// always set to last day of the week (sunday)
	endDate = endDate.addDays(7-endDate.dayOfWeek());
	ui.dateEdit_Latest->setDate(endDate);
	ui.dateEdit_Latest->blockSignals(false);
}

void ScheduleSettingsDialog::scheduleDurationChanged(int months) {
	QDate start(ui.dateEdit_Earliest->date());
	ui.dateEdit_Latest->setDate(start.addMonths(months));
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
	int checkCount(0);
	for (storageHostsMap::const_iterator hit = nodes.begin(); hit != nodes.end(); ++hit) {
		hostName = hit->second.itsName;
		nodeStatus = hit->second.itsStatus;
		sit = states.find(nodeStatus);
		if (sit != states.end())
			hostStatus = " (" + sit->second + ")";
		// node name and status
		hostName += hostStatus;
		nodeItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList(hostName.c_str()));
		nodeItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		nodeItem->setData(0,Qt::UserRole,hit->second.itsID); // store the storage node ID in the treewidget item for later identification of allowed storage nodes
		if (hit->second.itsMayBeUsed) {
			nodeItem->setCheckState(0,Qt::Checked);
			++checkCount;
		}
		else {
			nodeItem->setCheckState(0,Qt::Unchecked);
		}
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
//				}
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
	ui.lineEditNrOfSelectedNodes->setText(QString::number(checkCount));
	QApplication::restoreOverrideCursor();
}

void ScheduleSettingsDialog::updatePreferredStorageLists(const storageHostsMap &nodes) {
	ui.tableWidgetDataProductStorage->blockSignals(true);
	QTableWidgetItem *item;
	preferredDataProductStorageMap::const_iterator pit;
	const preferredDataProductStorageMap &preferredDataProductStorage(Controller::theSchedulerSettings.getPreferredDataProductStorage());
	int col(1);
	ui.tableWidgetDataProductStorage->setColumnCount(nodes.size()+1);
	for (int r = 0; r < NR_DATA_PRODUCT_TYPES; ++r) {
		for (storageHostsMap::const_iterator hit = nodes.begin(); hit != nodes.end(); ++hit) {
			item = new QTableWidgetItem(hit->second.itsName.c_str()); // storage node name
			item->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
			item->setData(Qt::UserRole,hit->second.itsID); // storage node ID
			pit = preferredDataProductStorage.find(static_cast<dataProductTypes>(r));
			if (pit != preferredDataProductStorage.end()) {
				if (std::find(pit->second.begin(), pit->second.end(), hit->second.itsID) != pit->second.end()) {
					item->setCheckState(Qt::Checked);
					item->setBackground(Qt::blue);
					item->setForeground(Qt::white);
				}
				else {
					item->setCheckState(Qt::Unchecked);
					item->setBackground(Qt::white);
					item->setForeground(Qt::black);
				}
			}
			else {
				item->setCheckState(Qt::Unchecked);
				item->setBackground(Qt::white);
				item->setForeground(Qt::black);
			}
			ui.tableWidgetDataProductStorage->setItem(r,col++,item);
		}
		col = 1;
	}
	ui.tableWidgetDataProductStorage->resizeColumnsToContents();
	ui.tableWidgetDataProductStorage->blockSignals(false);

	// project based preferred storage
	ui.tableWidgetProjectStorage->blockSignals(true);
	col = 1;
	int row(0);
	QString tooltip;
	const campaignMap &projects(Controller::theSchedulerSettings.getCampaignList());
	const preferredProjectStorageMap &preferredProjectStorage(Controller::theSchedulerSettings.getPreferredProjectStorage());
	preferredProjectStorageMap::const_iterator prit;
	ui.tableWidgetProjectStorage->setRowCount(projects.size());
	ui.tableWidgetProjectStorage->setColumnCount(nodes.size()+1);
	bool project_found;
	for (campaignMap::const_iterator projectIt = projects.begin(); projectIt != projects.end(); ++projectIt) {
		item = new QTableWidgetItem(projectIt->first.c_str()); // project name
		item->setData(Qt::UserRole,projectIt->second.id); // project ID
		tooltip = QString(projectIt->first.c_str()) + "\n" + // project name
				projectIt->second.title.c_str()	+ "\n" +
				+ "PI: " + projectIt->second.PriInvestigator.c_str() + "\n" +
				+ "CoI: " + projectIt->second.CoInvestigator.c_str() + "\n" +
				+ "contact: " + projectIt->second.contact.c_str();
		item->setToolTip(tooltip);
		item->setFlags(Qt::ItemIsEnabled);
		ui.tableWidgetProjectStorage->setItem(row,0,item);
		prit = preferredProjectStorage.find(projectIt->second.id);
		if (prit == preferredProjectStorage.end()) {
			project_found = false;
		}
		else {
			project_found = true;
		}
		for (storageHostsMap::const_iterator hit = nodes.begin(); hit != nodes.end(); ++hit) {
			item = new QTableWidgetItem(hit->second.itsName.c_str()); // storage node name
			item->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
			item->setData(Qt::UserRole,hit->second.itsID); // storage node ID
			item->setToolTip(tooltip);
			if (project_found) {
				if (std::find(prit->second.begin(), prit->second.end(), hit->first) != prit->second.end()) {
					item->setCheckState(Qt::Checked);
					item->setBackground(Qt::blue);
					item->setForeground(Qt::white);
				}
				else {
					item->setCheckState(Qt::Unchecked);
					item->setBackground(Qt::white);
					item->setForeground(Qt::black);
				}
			}
			else {
				item->setCheckState(Qt::Unchecked);
				item->setBackground(Qt::white);
				item->setForeground(Qt::black);
			}
			ui.tableWidgetProjectStorage->setItem(row,col++,item);
		}
		col = 1;
		++row;
	}
	ui.tableWidgetProjectStorage->resizeColumnsToContents();
	ui.tableWidgetProjectStorage->blockSignals(false);
}

// get the preferred storage nodes for each data product from the dialog
preferredDataProductStorageMap ScheduleSettingsDialog::getPreferredDataProductStorage(void) const {
	preferredDataProductStorageMap storageMap;
	std::vector<int> nodeIDs;
	for (int row = 0; row < ui.tableWidgetDataProductStorage->rowCount(); ++row) { // row number corresponds to the data product type enum value
		for (int col = 1; col < ui.tableWidgetDataProductStorage->columnCount(); ++col) {
			if (ui.tableWidgetDataProductStorage->item(row,col)->checkState() == Qt::Checked) {
				nodeIDs.push_back(ui.tableWidgetDataProductStorage->model()->data(ui.tableWidgetDataProductStorage->model()->index(row,col),Qt::UserRole).toInt());
			}
		}
		storageMap[static_cast<dataProductTypes>(row)] = nodeIDs;
		nodeIDs.clear();
	}
	return storageMap;
}

// get the preferred storage nodes for each project from the dialog
preferredProjectStorageMap ScheduleSettingsDialog::getPreferredProjectStorage(void) const {
	preferredProjectStorageMap storageMap;
	std::vector<int> nodeIDs;
	int project_id;
	for (int row = 0; row < ui.tableWidgetProjectStorage->rowCount(); ++row) {
		project_id = ui.tableWidgetProjectStorage->model()->data(ui.tableWidgetProjectStorage->model()->index(row,0),Qt::UserRole).toInt();
		for (int col = 1; col < ui.tableWidgetProjectStorage->columnCount(); ++col) {
			if (ui.tableWidgetProjectStorage->item(row,col)->checkState() == Qt::Checked) {
				nodeIDs.push_back(ui.tableWidgetProjectStorage->model()->data(ui.tableWidgetProjectStorage->model()->index(row,col),Qt::UserRole).toInt());
			}
		}
		storageMap[project_id] = nodeIDs;
		nodeIDs.clear();
	}
	return storageMap;
}

void ScheduleSettingsDialog::updateDataProductStorageItem(QTableWidgetItem *item) {
	if (item->checkState() == Qt::Checked) {
		item->setBackground(Qt::blue);
		item->setForeground(Qt::white);
	}
	else {
		item->setBackground(Qt::white);
		item->setForeground(Qt::black);
	}
}

void ScheduleSettingsDialog::updateProjectsStorageItem(QTableWidgetItem *item) {
	if (item->checkState() == Qt::Checked) {
		item->setBackground(Qt::blue);
		item->setForeground(Qt::white);
	}
	else {
		item->setBackground(Qt::white);
		item->setForeground(Qt::black);
	}
}

void ScheduleSettingsDialog::selectDataProductStorageNodes(void) {
	QList<QTableWidgetItem*> items(ui.tableWidgetDataProductStorage->selectedItems());
	for (QList<QTableWidgetItem*>::iterator it = items.begin(); it != items.end(); ++it) {
		(*it)->setCheckState(Qt::Checked);
		(*it)->setBackground(Qt::blue);
		(*it)->setForeground(Qt::white);
	}
}

void ScheduleSettingsDialog::deselectDataProductStorageNodes(void) {
	QList<QTableWidgetItem*> items(ui.tableWidgetDataProductStorage->selectedItems());
	for (QList<QTableWidgetItem*>::iterator it = items.begin(); it != items.end(); ++it) {
		(*it)->setCheckState(Qt::Unchecked);
		(*it)->setBackground(Qt::white);
		(*it)->setForeground(Qt::black);
	}
}
void ScheduleSettingsDialog::selectProjectsStorageNodes(void) {
	QList<QTableWidgetItem*> items(ui.tableWidgetProjectStorage->selectedItems());
	for (QList<QTableWidgetItem*>::iterator it = items.begin(); it != items.end(); ++it) {
		(*it)->setCheckState(Qt::Checked);
		(*it)->setBackground(Qt::blue);
		(*it)->setForeground(Qt::white);
	}
}

void ScheduleSettingsDialog::deselectProjectsStorageNodes(void) {
	QList<QTableWidgetItem*> items(ui.tableWidgetProjectStorage->selectedItems());
	for (QList<QTableWidgetItem*>::iterator it = items.begin(); it != items.end(); ++it) {
		(*it)->setCheckState(Qt::Unchecked);
		(*it)->setBackground(Qt::white);
		(*it)->setForeground(Qt::black);
	}
}

/*
	const storageNodeNameIDmap &storagenodes = Controller::theSchedulerSettings.getStorageNodesNameIDmapping();
	const std::map<int, storagePartitionsMap> &partitions = Controller::theSchedulerSettings.getStoragePartitionsMap();
	std::map<int, storagePartitionsMap>::const_iterator pit;
	QTreeWidgetItem *nodeItem;
	QStringList itemValues;
	for (storageNodeNameIDmap::const_iterator stit = storagenodes.begin(); stit != storagenodes.end(); ++stit) {
		nodeItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList(stit->second.c_str()));
		pit = partitions.find(stit->first);
		for (storagePartitionsMap::const_iterator ppit = pit->second.begin(); ppit != pit->second.end(); ++ppit) {
			// Storage Node, Partition, Node Status, Total Size (kb), Free Space (kb)
			itemValues << "" << ppit->second.first << itsController->getStorageNodeStatusStr(stit->first)
			<< QString::number(ppit->second.second) << itsController->getStorageNodeSpaceRemaining(stit->first, pit->first);
			new QTreeWidgetItem(nodeItem, itemValues);
			itemValues.clear();
		}
	    items.append(nodeItem);
	}
	ui.treeWidgetStorageNodes->insertTopLevelItems(0, items);
*/

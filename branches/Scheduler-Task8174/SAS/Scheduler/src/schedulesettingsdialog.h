/*
 * schedulesettingsdialog.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 5-march-2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/schedulesettingsdialog.h $
 *
 */

#ifndef SCHEDULESETTINGSDIALOG_H
#define SCHEDULESETTINGSDIALOG_H

#include <QDialog>
#include "ui_schedulesettingsdialog.h"
#include "lofar_scheduler.h"
#include "astrodate.h"
#include "astrotime.h"
#include "DataMonitorConnection.h"
#include "schedulersettings.h"
class QString;
class Controller;

extern const char *storage_distributions_str[NR_STORAGE_DISTRIBUTIONS];

class ScheduleSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    ScheduleSettingsDialog(Controller *controller);
    ~ScheduleSettingsDialog();

    const stationDefinitionsMap & getStationsList(void) const {return itsStations;}
    AstroDate getEarliestSchedulingDay(void) const; //{return itsEarliestDay;}
    AstroDate getLatestSchedulingDay(void) const; // {return itsLatestDay;}
    unsigned getMaxNrOptimizeIterations(void) const {return itsMaxNrOptimizeIterations;}
    AstroTime getMinimumTimeBetweenObservations(void) const; // const {return itsMinTimeBetweenObservations;}
    bool getUserAcceptedPenaltyEnabled(void) const {return itsUserAcceptedPenaltyEnabled;}
    unsigned int getUserAcceptedPenalty(void) const {return itsUserAcceptedPenalty;}
    bool getmaxNrOptimizationsEnabled(void) const {return itsMaxNrOptimizationsEnabled;}
    unsigned getMaxNrOptimizations(void) const {return itsMaxNrOptimizeIterations;}
    bool getLoadDefaultSettingsOnStartUp(void) const {return itsLoadDefaultSettings;}
    bool getIsTestEnvironment(void) const {return itsIsTestEnvironment;}
	unsigned getShortTermScheduleDurationWeeks(void) const {return itsShortTermScheduleDuration;}
	unsigned getScheduleDurationMonths(void) const {return itsScheduleDuration;}
	bool getAllowUnscheduleFixedTasks(void) const {return itsAllowUnscheduleFixedTasks;}
	quint16 getMaxNrOfFilesPerStorageNode(void) const {return itsMaxNrOfFilesPerStorageNode;}
	const QString &getSASDatabase(void) const {return itsSASDatabase;}
	const QString &getSASHostName(void) const {return itsSASHostName;}
	const QString &getSASUserName(void) const {return itsSASUserName;}
	const QString &getSASPassword(void) const {return itsSASPassword;}
	int getSASDefaultTreeID(void) const {return itsSASDefaultTree;}
	const QString &getDMDatabase(void) const {return itsDMDatabase;}
	const QString &getDMHostName(void) const {return itsDMHostName;}
	const QString &getDMUserName(void) const {return itsDMUserName;}
	const QString &getDMPassword(void) const {return itsDMPassword;}
    bool getAutoPublish(void) const {return ui.checkBoxAutoPublish->isChecked();}
	const QString &getLocalPublishPath(void) const {return itsLocalPublishPath;}
	const QString &getSchedulerAccountName(void) const {return itsSchedulerAccountName;}
	const QString &getPrivateKeyFile(void) const {return itsPrivateKeyFile;}
	const QString &getWebServerName(void) const {return itsWebServerName;}
	const QString &getWebServerPublishPath(void) const {return itsWebServerPublishPath;}
//	const QString &getFileNameMask(void) const {return itsFileNameMask;}
	bool publishLocal(void) const {return itsPublishLocal;}
	unsigned getMinimumNrOfStorageNodes(void) const {return itsMinNrOfStorageNodes;}
	const double &getStorageNodeBandWidth(void) const {return itsStorageNodeBandWidth;}
	const double &getRaidMaxWriteSpeed(void) const {return itsStorageRaidWriteSpeed;}
	const QStringList &getDemixSources(void) const {return itsDemixSources;}

	//update the storage nodes capacity info box
	void updateStorageNodeInfoTree(const storageHostsMap &nodes, const statesMap &states,	const hostPartitionsMap &hostPartitions);
	void updatePreferredStorageLists(const storageHostsMap &nodes);
//	const Ui::ScheduleSettingsDialogClass &getSettingsGUIClass(void) const {return ui;};

    void setStationList(const stationDefinitionsMap &stations) {itsStations = stations;}
    void setEarliestSchedulingDay(const AstroDate &day) {itsEarliestDay.setDate(day.getYear(), day.getMonth(), day.getDay());}
    void setLatestSchedulingDay(const AstroDate &day) {itsLatestDay.setDate(day.getYear(), day.getMonth(), day.getDay());}
    void setMinimumTimeBetweenTasks(const AstroTime &t) {itsMinTimeBetweenTasks.setHMS(t.getHours(), t.getMinutes(), t.getSeconds());}
	void setUserAcceptedPenalty(unsigned int penalty) {itsUserAcceptedPenalty = penalty;}
	void setUserAcceptedPenaltyEnabled(bool userPenaltyEnabled) {itsUserAcceptedPenaltyEnabled = userPenaltyEnabled;}
	void setMaxNrOptimizations(unsigned int maxOptimations) {itsMaxNrOptimizeIterations = maxOptimations;}
	void setmaxNrOptimizationsEnabled(bool maxOptEnabled) {itsMaxNrOptimizationsEnabled = maxOptEnabled;}
	void setLoadDefaultSettingsOnStartUp(bool loadDefault) {itsLoadDefaultSettings = loadDefault;}
	void setIsTestEnvironemnt(bool is_test_environment) {itsIsTestEnvironment = is_test_environment;}
	void setShortTermScheduleDurationWeeks(unsigned weeks) {itsShortTermScheduleDuration = weeks;}
	void setScheduleDurationMonths(unsigned months) {itsScheduleDuration = months;}
	void setAllowUnscheduleFixedTasks(bool allow) {itsAllowUnscheduleFixedTasks = allow;}
	void setMaxNrOfFilesPerStorageNode(quint16 nr_files) {itsMaxNrOfFilesPerStorageNode = nr_files;}
	void setSASUserName(const std::string &SAS_username) {itsSASUserName = SAS_username.c_str();}
	void setSASPassword(const std::string &SAS_password) {itsSASPassword = SAS_password.c_str();}
	void setSASDatabase(const std::string &SAS_database) {itsSASDatabase = SAS_database.c_str();}
	void setSASHostName(const std::string &SAS_hostname) {itsSASHostName = SAS_hostname.c_str();}
	void setSASDefaultTreeID(int treeID) {itsSASDefaultTree = treeID;}
	void setDMUserName(const std::string &DM_username) {itsDMUserName = DM_username.c_str();}
	void setDMPassword(const std::string &DM_password) {itsDMPassword = DM_password.c_str();}
	void setDMDatabase(const std::string &DM_database) {itsDMDatabase = DM_database.c_str();}
	void setDMHostName(const std::string &DM_hostname) {itsDMHostName = DM_hostname.c_str();}
    void setAutoPublish(bool enabled) {ui.checkBoxAutoPublish->setChecked(enabled);}
	void setLocalPublishPath(const std::string &publish_path) {itsLocalPublishPath = publish_path.c_str();}
	void setSchedulerAccountName(const std::string &account_name) {itsSchedulerAccountName = account_name.c_str();}
	void setPrivateKeyFile(const std::string &private_key_file) {itsPrivateKeyFile = private_key_file.c_str();}
	void setWebServerName(const std::string &server_name) {itsWebServerName = server_name.c_str();}
	void setWebServerPublishPath(const std::string &publish_path) {itsWebServerPublishPath = publish_path.c_str();}
//	void setFileNameMask(const std::string &file_name_mask) {itsFileNameMask = file_name_mask.c_str();}
	void setPublishLocal(bool local) {itsPublishLocal = local;}
	void setMinimumNrOfStorageNodes(unsigned min_storage_nodes) {itsMinNrOfStorageNodes = min_storage_nodes;}
	void setStorageNodeBandwidth(const double &bandwidth) {itsStorageNodeBandWidth = bandwidth;}
	void setRaidMaxWriteSpeed(const double &write_speed_kBs) {itsStorageRaidWriteSpeed = write_speed_kBs;}
	void stopStorageWaitCursor(void) {
		ui.pb_RefreshStorageNodesInfo->setEnabled(true);
		QApplication::restoreOverrideCursor();
	}

private:
	void createActions(void);
//   std::pair<std::string, std::pair<double, double> > getStationData(const QString &s) const;
	bool checkLocalPublishPath(void);
	bool checkSASsettings(void);
	int checkSASconnection(void);
	void updateSASConnectionSettings(void); // gets the sas connection settings from the dialog
	void writeDefaultTemplatesToDialog(void);
	preferredDataProductStorageMap getPreferredDataProductStorage(void) const;
	preferredProjectStorageMap getPreferredProjectStorage(void) const;

signals:
	void actionSaveSettings(void) const;

public slots:
	void show(void);
	void enablePublishField(void);

private slots:
	void addRowToDemixSourceTable(void);
	void deleteDemixSourceFromTable(void);
	void checkDemixSourceItem(void);
	void openLocalPathBrowseDialog(void);
	void privateKeyBrowseDialog(void);
	void okClicked(void);
	void cancelClicked(void);
	void addStation(void);
	void removeStation(void);
	void clearStationList(void);
	void today(void);
	void scheduleStartDateChanged(QDate);
	void scheduleEndDateChanged(QDate endDate);
	void scheduleDurationChanged(int);
	int testSASconnection(bool quietWhenOk = false);
	void doRefreshStorageNodesInfo(void);
	void updateDefaultTemplates(bool quiet = false);
	void doubleClickedStation(const QModelIndex &);
	void updateDataProductStorageItem(QTableWidgetItem *);
	void updateProjectsStorageItem(QTableWidgetItem *);
	void selectDataProductStorageNodes(void);
	void deselectDataProductStorageNodes(void);
	void selectProjectsStorageNodes(void);
	void deselectProjectsStorageNodes(void);
	void countStorageSelection(QTreeWidgetItem *, int);

private:
	Ui::ScheduleSettingsDialogClass ui;
	Controller *itsController;
    stationDefinitionsMap itsStations;
	QDate itsEarliestDay, itsLatestDay;
	QTime itsMinTimeBetweenTasks;
	QStringList itsDemixSources;
	unsigned itsUserAcceptedPenalty, itsMaxNrOptimizeIterations;
	unsigned itsShortTermScheduleDuration, itsScheduleDuration, itsMinNrOfStorageNodes;
	double itsStorageNodeBandWidth, itsStorageRaidWriteSpeed;
	bool itsUserAcceptedPenaltyEnabled, itsMaxNrOptimizationsEnabled;
	bool itsAllowUnscheduleFixedTasks, itsLoadDefaultSettings, itsIsTestEnvironment;
	QString itsSASDatabase, itsSASHostName, itsSASUserName, itsSASPassword;
	QString itsDMDatabase, itsDMHostName, itsDMUserName, itsDMPassword;
	quint16 itsMaxNrOfFilesPerStorageNode;
	int itsSASDefaultTree;
	// publish settings
	QString itsLocalPublishPath, itsWebServerPublishPath, itsWebServerName, itsSchedulerAccountName,
        itsPrivateKeyFile;
    bool itsPublishLocal;
	storageNodeDistribution itsDataDistributionScheme;

};

#endif // SCHEDULESETTINGSDIALOG_H

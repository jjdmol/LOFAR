/*
 * Controller.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 4-feb-2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/Controller.h $
 *
 */


#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <QMessageBox>
#include <vector>
#include <string>
#include "lofar_scheduler.h"
#include "task.h"
#include "Scheduler.h"
#include "schedulerdata.h"
#include "schedulergui.h"
#include "schedulesettingsdialog.h"
#include "DataHandler.h"
#include "schedulersettings.h"
#include "thrashbin.h"
#include "astrodatetime.h"
#include "OTDBtree.h"
#include "Storage.h"
#include "SASConnection.h"
#include "shifttasksdialog.h"
// todo ( This could be added in the project file, as a compiler option)
//#ifdef SCHEDULER_TEST
#include "signalhandler.h"
//#endif

class Thrashbin;
class AstroDateTime;
class DataMonitorConnection;
class ConflictDialog;
class QTableWidgetItem;
class StationTask;

enum undo_type {
	UNDO_TASK_PROPERTY,
	UNDO_COMPLETE_TASK,
	UNDO_SCHEDULER_DATA_BLOCK,
	UNDO_DELETE_TASKS
};

typedef std::vector<std::pair<unsigned, std::pair<data_headers, QVariant> > > taskPropertyUndoStack;
typedef std::vector<std::pair<unsigned, Task *> > taskUndoStack;
typedef std::vector<std::vector<Task *> > deletedTasksVector;
typedef std::vector<std::pair<undo_type, std::pair<QString, std::vector<unsigned> > > > undoTypeVector;

class Controller : public QObject
{
	Q_OBJECT
//todo
//#ifdef SCHEDULER_TEST
    friend class SignalHandler;
//#endif
public:
	Controller(QApplication &app);
	virtual ~Controller();

	// interface members to SchedulerGUI
	void start(void); // show the SchedulerGUI
//	unsigned getCurrentUndoLevel(void) const {return current_undo_level;};
	bool fixTasks();
	const Task *getScheduledTask(unsigned taskID) const {return data.getScheduledTask(taskID);}
	const Task *getInactiveTask(unsigned taskID) const {return data.getInactiveTask(taskID);}
	const Task *getUnscheduledTask(unsigned taskID) const {return data.getUnscheduledTask(taskID);}
    const Task *getTask(unsigned taskID, id_type IDtype = ID_SCHEDULER) const {return data.getTask(taskID, IDtype);}
    const StationTask *getStationTask(unsigned taskID, id_type IDtype = ID_SCHEDULER) const {return data.getStationTask(taskID, IDtype);}
    const Observation *getObservation(unsigned taskID, id_type IDtype = ID_SCHEDULER) const {return data.getObservation(taskID, IDtype);}
    const CalibrationPipeline *getCalibrationPipeline(unsigned taskID, id_type IDtype = ID_SCHEDULER) const {return data.getCalibrationPipeline(taskID, IDtype);}
    const ImagingPipeline *getImagingPipeline(unsigned taskID, id_type IDtype = ID_SCHEDULER) const {return data.getImagingPipeline(taskID, IDtype);}
    const PulsarPipeline *getPulsarPipeline(unsigned taskID, id_type IDtype = ID_SCHEDULER) const {return data.getPulsarPipeline(taskID, IDtype);}
    unsigned getNewTaskID(void) const {return data.getNewTaskID();}
//	void loadScheduleTable(void); // load the schedule table
	const char *getReservationName(unsigned reservation_id) const;
	const reservationsMap &getReservations(void) const {return data.getReservations();}
    std::pair<bool, std::pair<QString, Observation> > doReservationChecks(const Observation *orgObs, unsigned reservationID);
	// checks the task for errors that prevent it from being scheduled
	// returns pair<unscheduled_reasons, string> with the first error found in the task (unscheduled_reasons error code, string = error)
	// if the int value returned = 0 then no errors were detected
//	std::pair<unscheduled_reasons, QString> doPreScheduleChecks(const Task &task) const;
	std::pair<unscheduled_reasons, QString> doPreScheduleChecks(Task *task);
    std::pair<unscheduled_reasons, QString> setInputFilesForPipeline(Task *pPipe);
	void updateStatusBar(void);
	// undo redo interface functions
	void storeTaskPropertyUndo(unsigned taskID, data_headers property, const QVariant &value);
	void storeTaskUndo(unsigned taskID, const QString &undo_action, bool delete_task = false);
	void storeScheduleUndo(const QString &undo_action);
	void deleteLastStoredUndo(void); // removes the last added undo level

	void moveSelectedTasksRequest(unsigned task_id, const AstroDateTime &new_start_time);
	bool moveSelectedTasks(const AstroDateTime &time, moveType move_type);
	bool moveTasks(const std::map<unsigned, AstroDateTime> &startTimes);
	void deselectAllTasks(void);
	void copySelectedTasks(void);
	void copyTask(unsigned int taskID);
//	void addSelectedTask(unsigned taskID);
	void selectTasks(const std::vector<unsigned> &taskIDs); // selects multiple tasks
	void setSelectedTasks(const std::vector<unsigned> &taskIDs) {itsSelectedTasks = taskIDs;} // only sets the selected tasks, does not actually perform a selection in GUI
	void setSelectedTask(unsigned taskID) {itsSelectedTasks.clear(); itsSelectedTasks.push_back(taskID);} // only sets the selected task, does not actually perform a selection in GUI
	bool isSelected(unsigned taskID);
	void selectTask(unsigned taskID, bool singleSelection, bool selectRows = true, bool tableClick = false);
    void selectCurrentTaskGroups(selector_types type = SEL_ALL_TASKS);
	void deselectTask(unsigned taskID, bool singleSelection, bool selectRows = true);
	bool multipleSelected(void) const {return itsSelectedTasks.size() > 1;}
	unsigned lastSelectedTask(void) {if(!itsSelectedTasks.empty()) {return itsSelectedTasks.back();} else {return 0;}}
	const std::vector<unsigned> &selectedTasks(void) const {return itsSelectedTasks;}
	const std::map<std::string, std::vector<Task *> > getPublishTasks(const AstroDateTime &start, const AstroDateTime &end) const {return data.getPublishTasks(start,end);}
	const campaignMap &getCampaignList(void) const {return theSchedulerSettings.getCampaignList();} // returns the campaign (project) list
	void setSaveRequired(bool);
	bool isDeleted(unsigned taskID) const; // check if task was deleted from schedule
	bool unDelete(unsigned taskID); // un-deletes a previously deleted task (this could cause conflicts with other tasks!)
	void mergeDownloadedSASTasks(void); // used by SAS connection to initiate the creation of the downloaded SAS tasks into the scheduler
//	const Task *createSASErrorTask(const OTDBtree& SAS_tree, unsigned task_id = 0); // creates a new unscheduled task with some properties set by the erronneus SAS task tree.
    QString getSasDatabaseName(void) const {return theSchedulerSettings.getSASDatabase();}

	void unscheduleSelectedTasks(void);
	void setSelectedTasksOnHold(void);

	void setAllowedStorageHosts(const std::vector<int> &allowedStorageHosts) {
		itsDMConnection->setAllowedStorageHosts(allowedStorageHosts);
		data.setAllowedStorageHosts(allowedStorageHosts);}

	void showTaskDialog(unsigned taskID, tabIndex tab = TAB_SCHEDULE) const;
	void openMoveTasksDialog(void);

	// following used by graphicTask objects
	bool unscheduleTask(unsigned taskID);
	void scheduleTask(unsigned taskID, Task::task_status status);
	void scheduleSelectedTasks(Task::task_status new_status);
	bool setTaskOnHold(unsigned taskID, bool store_undo = true);
	void rescheduleAbortedTask(unsigned task_id, AstroDateTime new_start = 0);
	void expungeTask(unsigned treeID);
	void deleteTask(unsigned taskID);
	void showTaskStateHistory(unsigned taskID);
    void openSASTreeViewer(int treeID) const;
    void openMetaDataViewer(int treeID) const;

    // storage settings
	int refreshStorageNodesInfo(bool doConnectToDM = true); // refreshes the storage nodes availability and capacity info in the settingsdialog
	bool isDataMonitorConnected(void) const;
	void setDataMonitorConnectionButton(bool enable) {gui->setDataMonitorConnectionButton(enable);}
	const storageHostsMap &getStorageNodes(void) const {return itsDMConnection->getStorageNodes();}
	const statesMap &getStorageNodesStates(void) const {return itsDMConnection->getStates();}
	size_t getNrOfStorageNodesAvailable(void) const {return itsDMConnection->getNrOfStorageNodesAvailable();}
	const hostPartitionsMap &getStoragePartitions(void) const {return itsDMConnection->getHostPartitions();}
	int getStorageRaidID(int node_id, const std::string &raidName) const {return itsDMConnection->getStorageRaidID(node_id, raidName);}
//	const dataPathsMap &getStorageNodesDataPaths(void) const {return itsDMConnection->getDataPaths();}
	std::string getStorageNodeName(int node_id) const {return itsDMConnection->getStorageNodeName(node_id);}
	std::string getStorageRaidName(int node_id, int raid_id) const {return itsDMConnection->getStorageRaidName(node_id, raid_id);}
	int getStorageNodeID(const std::string &name) const {return itsDMConnection->getStorageNodeID(name);}
//	const std::string &getStorageNodeStatusStr(int node_id) const;
//	unsigned long getStorageNodeSpaceRemaining(int node_id, unsigned short partition_id) const {return itsDMConnection->getStorageNodeSpaceRemaining(node_id, partition_id);}
//	const statesMap &getStates(void) const {return states;}
//	const dataPathsMap &getDataPaths(void) const {return itsDMConnection->getDataPaths();}
	void setStorageNodes(const storageHostsMap &nodes) {itsDMConnection->setStorageNodes(nodes);}
	void setStoragePartitions(const hostPartitionsMap &partitions) {itsDMConnection->setStoragePartitions(partitions);}
	const AstroDateTime &now(void); // returns the current UTC time
	bool connectToDataMonitor(void);
	void disconnectDataMonitor(void);
	bool updateProjects(void); // fetches the updates project (campaign) list from SAS and updates the GUI accordingly
	void multiEditSelectedTasks(void); // opens the multi-edit taskdialiog with the cumulative properties of all selected tasks
	void refreshGUIafterMultiEdit(void);
    void synchronizeTask(const Task *pTask); // updates a single existing task after it has been downloaded again
    bool updateTask(Task *, bool createUndo);
    bool updatePipelineTask(Task *task, bool createUndo);
	bool createTask(const Task &task);
    bool createReservation(const Task *reservation);
    void createPipeline(const Pipeline *pipeline);
	std::vector<DefaultTemplate> getDefaultTemplatesFromSAS(void) {return itsSASConnection->getDefaultTemplates();}
	void defaultTemplatesUpdated(void) {gui->loadProcessTypes();}
	void setStatusText(const char *text) {gui->setStatusText(text);}
	void clearStatusText(void) {gui->clearStatusText();}
	int getSelectedObservationsTimeSpan(void) const;
    void setAutoPublish(bool enabled) {theSchedulerSettings.setAutoPublish(enabled);}
    bool autoPublishAllowed(void) {return itsAutoPublishAllowed;}

#ifdef DEBUG_SCHEDULER
	void printSelectedTasks(const std::string &callerName) const;
#endif

	// resource assignment
	bool assignStorageResources(Task *task = 0);
	bool calculateDataSlots(void);


#ifdef HAS_SAS_CONNECTION
	QString lastSASError(void) const;
	bool checkSASSettings(void);
	int checkSASconnection(const QString &username, const QString &password, const QString &DBname, const QString &hostname);
	void setSASConnectionSettings(const QString &username, const QString &password, const QString &DBname, const QString &hostname);
	void commitScheduleToSAS(void);
#endif

private:
	bool doScheduleChecks(Task *);
	std::pair<unscheduled_reasons, QString> checkPredecessorDependencies(const Task *, Task::task_status newState) const;
	void updateGraphicTasks(void) {gui->updateGraphicTasks(data.getScheduledTasks(), data.getReservations(), data.getInactiveTasks());}
	void connectSignals(void); // connect the signals from the SchedulerGUI to the Scheduler
	void loadProgramPreferences(void);
	void clearUndoRedoStack(void);
	void clearRedoStack(void);
	void checkTableItem(const QModelIndex &index); // checks the value of the table item index. Used after changes were made to a table cell
	bool possiblySave(); // check if things need to be saved and then cleans up the data
    int possiblySaveDialog();
	void cleanup(bool keepUndo = false, bool cleanSasConnection = true); // does the actual cleaning
	bool checkStationList(const std::string & stations) const; // return true if all stations mentioned in the string stations actually exist
	bool checkSettings() const;
	bool applyGUITableChanges(bool askContinue);
	void updateGUIafterSettingsLoad(void);
	void applyTaskPropertyChange(unsigned taskID, data_headers property, const QVariant &value, bool undo_redo);
	// updates the undo stack looking for deleted tasks that have been destroyed
	void updateDeletedTasksUndo(void);
	bool askOverWriteExistingTask(bool &overwrite, bool &forAll, unsigned taskID, const QString &taskName);
//	bool dataMonitorInitRequired(void); // checks to see if data monitor init is required
	bool assignManualStorageToTask(Task *pTask);
	bool assignGroupedStorage(void);
//	bool assignMinimumStorageToTask(Task *pTask);
	bool assignStorageToTask(Task *pTask);
	void rescheduleTask(unsigned task_id, AstroDateTime new_start);
	// checkEarlyTasksStatus: checks the current status of too early tasks in the SAS database and updates the tasks in the scheduler if the status was changed in SAS
	// returns false if any too early task was found which is still (PRE)SCHEDULED
	bool checkEarlyTasksStatus(void);
	bool checkPredecessorsExistence(const IDvector &predecessors) const;
    void autoPublish(void);

signals:
	void schedulerSettingsChanged(void);

public slots:
	void quit(void); // exit Scheduler application after some checks
	void deleteSelectedTasks(void);
//	void abortTask(unsigned int taskID);

private slots:
	void tableSortingChanged(void); // the sorting column and/or order was changed
//    void openTaskList(void); // handles the open task list event
    void saveTaskListAs(void); // handles the save task list as event
	bool saveSchedule(QString filename = "");
	bool saveScheduleAs(void);
	void openSchedule(void);
	bool closeSchedule(void);
	void saveSettings(void);
	void saveDefaultSettings(void);
	void loadSettings(void);
	void loadDefaultSettings(void);
	void closeSettingsDialog(void);
	void openSettingsDialog(void); // opens the settings dialog
	void createInitialSchedule(void); // creates the start schedule
	void undo(bool store_redo = true);
	void redo(void);
	void optimizeSchedule(void);
	void applyTableItemChange(unsigned, data_headers, const QVariant &, const QModelIndex &); // keep track of all changes to the schedule table made by the user
	void fixTaskErrors(void);
	void updateStatusBarOptimize(unsigned);
//	void tryRescheduleTask(unsigned, AstroDateTime);
//	void alignLeft(void);
	void showThrashBin(void);
	void thrashBinEmpty(void) const {gui->setEmptyThrashIcon();}
	void thrashBinNotEmpty(void) const {gui->setFullThrashIcon();}
	void doDestroyTasks(std::vector<unsigned>);
	void restoreTasks(const std::vector<unsigned> &tasksToRestore);
	void addTask(void);
//	void addReservation(void);
//	void checkPredecessor(unsigned) const;
	void balanceSchedule(void);
	void newSchedule(void);
	int assignResources(bool showResult = true);

#ifdef HAS_SAS_CONNECTION
	void downloadSASSchedule(void);
	void InitSynchronizeSASSchedule(void);
	void checkSASStatus(void) const;
#endif

public:
	static SchedulerSettings theSchedulerSettings; // everyone can access these settings
	static unsigned itsFileVersion; // used for storing the last read input file version

private:
    QMessageBox *possiblySaveMessageBox;
	QApplication *application;
	Scheduler scheduler;
	SchedulerGUI *gui;
	ScheduleSettingsDialog *itsSettingsDialog;
	ConflictDialog *itsConflictDialog;
	SchedulerData data;
	DataHandler *itsDataHandler;

	AstroDateTime itsTimeNow;
	std::vector<unsigned> itsSelectedTasks;
	Thrashbin itsThrashBin;

#ifdef HAS_SAS_CONNECTION
	// connection to SAS database
	SASConnection *itsSASConnection;
#endif

	DataMonitorConnection *itsDMConnection;

	// undo items
	undoTypeVector itsUndoType;
	undoTypeVector itsRedoType;
	taskPropertyUndoStack itsTaskPropertyUndoStack;
	taskPropertyUndoStack itsTaskPropertyRedoStack;
	taskUndoStack itsTaskUndoStack;
	taskUndoStack itsTaskRedoStack;
	deletedTasksVector itsDeletedTasks;
	std::vector<std::vector<unsigned> > itsDeletedTasksRedoStack;

    bool itsAutoPublishAllowed;
};

#endif /* CONTROLLER_H_ */


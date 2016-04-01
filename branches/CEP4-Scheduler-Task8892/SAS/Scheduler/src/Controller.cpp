/*
 * Controller.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * First creation : 4-feb-2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/Controller.cpp $
 *
 */

//#define HAVE_BOOST

#include "Controller.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include "lofar_scheduler.h"
#include "schedulergui.h"
#include "Scheduler.h"
#include "schedulersettings.h"
#include "GraphicResourceScene.h"
#include "conflictdialog.h"
#include "taskcopydialog.h"
#include "DigitalBeam.h"
#include "pipeline.h"
#include <QMessageBox>
#include <QDateTime>
#include <QString>
#include <string>
#include <vector>
#include <cmath>
#include <limits>
#ifdef HAS_SAS_CONNECTION
#include "SASConnection.h"
#endif
#include "DataMonitorConnection.h"

extern QString currentUser;

using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::pair;

#define STORE_UNDO false
#define STORE_REDO true

SchedulerSettings Controller::theSchedulerSettings = SchedulerSettings();
unsigned Controller::itsFileVersion = 0;

Controller::Controller(QApplication &app) :
     possiblySaveMessageBox(0), application(&app), gui(0),
    itsSettingsDialog(0), itsConflictDialog(0)
{
    itsAutoPublishAllowed = currentUser == "lofarsys" ? true : false;
#if defined Q_OS_WINDOWS || _DEBUG_
    itsAutoPublishAllowed = true;
#endif
    theSchedulerSettings.setAutoPublish(itsAutoPublishAllowed);

	Controller::theSchedulerSettings.setController(this);

	gui = new SchedulerGUI(this);
	itsSettingsDialog = new ScheduleSettingsDialog(this);
	itsConflictDialog = new ConflictDialog(this);
#ifdef HAS_SAS_CONNECTION
	itsSASConnection = new SASConnection(this);
	itsSASConnection->setLastDownloadDate(QDateTime(Controller::theSchedulerSettings.getEarliestSchedulingDay().toQDate()));
#endif
	// connection to the Data Monitor
	itsDMConnection = new DataMonitorConnection(this);
	itsDataHandler = new DataHandler(this);
	itsDataHandler->setFileName("");
	loadProgramPreferences();
	connectSignals();
	vector<std::string> headers;
	for (unsigned i = 0; i < NR_DATA_HEADERS; ++i) {
		headers.push_back(std::string(DATA_HEADERS[i]));
	}
	data.setDataHeaders(headers);
	gui->newTable(data);
	gui->setEnableScheduleMenuItems(true);
	gui->setEnableTaskListMenuItems(true);
//	gui->setExistingProjects(theSchedulerSettings.getCampaignList());
	scheduler.setData(data);
}

Controller::~Controller() {
	if (itsSettingsDialog) delete itsSettingsDialog;
	if (itsConflictDialog) delete itsConflictDialog;
	if (itsSASConnection) delete itsSASConnection;
	if (itsDMConnection) delete itsDMConnection;
	if (gui) delete gui;
	if (itsDataHandler) delete itsDataHandler;

    if (possiblySaveMessageBox) delete possiblySaveMessageBox;
}

#ifdef DEBUG_SCHEDULER
	void Controller::printSelectedTasks(const std::string &callerName) const {
		std::cout << callerName << ", currently selected tasks:" << std::endl;
		for (std::vector<unsigned>::const_iterator it = itsSelectedTasks.begin(); it != itsSelectedTasks.end(); ++it) {
			std::cout << *it << ",";
		}
		std::cout << std::endl;
	}

#endif

void Controller::setSaveRequired(bool save_required) {
	if (save_required) {
		data.setChangesMadeFlag();
	}
	else {
		data.clearChangesMadeFlag();
		data.clearUploadRequiredFlag();
	}
	gui->setSaveRequired(save_required);
}

void Controller::loadProgramPreferences(void) {
	if (itsDataHandler->loadProgramPreferences()) {
		if (Controller::theSchedulerSettings.getLoadDefaultSettingsOnStartUp()) {
			updateGUIafterSettingsLoad();
		}
	}
}

void Controller::connectSignals(void)
{
//#if defined Q_OS_WIN || _DEBUG_
	gui->connect(gui->getSchedulerGUIClass().action_Save_default_settings, SIGNAL(triggered()), this, SLOT(saveDefaultSettings()));
	gui->getSchedulerGUIClass().action_Save_default_settings->setEnabled(true);
//#else
//	gui->getSchedulerGUIClass().action_Save_default_settings->setEnabled(false);
//#endif

	// subscribe to signals from the gui->
	gui->connect(gui->getSchedulerGUIClass().action_New_schedule, SIGNAL(triggered()), this, SLOT(newSchedule()));
	gui->connect(gui->getSchedulerGUIClass().action_Save_schedule, SIGNAL(triggered()), this, SLOT(saveSchedule()));
	gui->connect(gui->getSchedulerGUIClass().action_Save_schedule_as, SIGNAL(triggered()), this, SLOT(saveScheduleAs()));
	gui->connect(gui->getSchedulerGUIClass().action_Open_Schedule, SIGNAL(triggered()), this, SLOT(openSchedule()));
//	gui->connect(gui->getSchedulerGUIClass().action_Open_task_list, SIGNAL(triggered()), this, SLOT(openTaskList()));
    gui->connect(gui->getSchedulerGUIClass().action_Save_task_list, SIGNAL(triggered()), this, SLOT(saveTaskListAs()));
	gui->connect(gui->getSchedulerGUIClass().action_Close_schedule, SIGNAL(triggered()), this, SLOT(closeSchedule()));
	gui->connect(gui->getSchedulerGUIClass().action_Create_initial_schedule, SIGNAL(triggered()), this, SLOT(createInitialSchedule()));
	gui->connect(gui->getSchedulerGUIClass().action_Optimize_schedule, SIGNAL(triggered()), this, SLOT(optimizeSchedule()));
	gui->connect(gui->getSchedulerGUIClass().action_Schedule_Settings, SIGNAL(triggered()), this, SLOT(openSettingsDialog()));
	gui->connect(gui->getSchedulerGUIClass().action_Save_settings, SIGNAL(triggered()), this, SLOT(saveSettings()));
	gui->connect(gui->getSchedulerGUIClass().action_Load_settings, SIGNAL(triggered()), this, SLOT(loadSettings()));
	gui->connect(gui->getSchedulerGUIClass().actionLoad_default_settings, SIGNAL(triggered()), this, SLOT(loadDefaultSettings()));
	gui->connect(gui->getSchedulerGUIClass().action_Balance_schedule, SIGNAL(triggered()), this, SLOT(balanceSchedule()));
	gui->connect(gui->getSchedulerGUIClass().action_Quit, SIGNAL(triggered()), this, SLOT(quit()));
	gui->connect(gui->getSchedulerGUIClass().action_Assign_resources, SIGNAL(triggered()), this, SLOT(assignResources()));
	gui->connect(gui, SIGNAL(quitApp()), this, SLOT(quit()));
	gui->connect(gui, SIGNAL(tableSortChanged()), this, SLOT(tableSortingChanged()));
	gui->connect(gui->getSchedulerGUIClass().action_Add_task, SIGNAL(triggered()), this, SLOT(addTask()));
//	gui->connect(gui->getSchedulerGUIClass().action_Add_reservation, SIGNAL(triggered()), this, SLOT(addReservation()));
	gui->connect(gui->getSchedulerGUIClass().action_Delete_task, SIGNAL(triggered()), this, SLOT(deleteSelectedTasks()));
	gui->connect(gui->getSchedulerGUIClass().action_Undo, SIGNAL(triggered()), this, SLOT(undo()));
	gui->connect(gui->getSchedulerGUIClass().action_Redo, SIGNAL(triggered()), this, SLOT(redo()));
	gui->connect(gui->getTableDelegate(), SIGNAL(tableItemChanged(unsigned, data_headers, const QVariant &, const QModelIndex &)), this, SLOT(applyTableItemChange(unsigned, data_headers, const QVariant &, const QModelIndex &)));
	gui->connect(itsSettingsDialog, SIGNAL(actionSaveSettings()), this, SLOT(closeSettingsDialog()));
	scheduler.connect(&scheduler, SIGNAL(optimizeIterationFinished(unsigned)), this, SLOT(updateStatusBarOptimize(unsigned)));
//	gui->scene()->connect(gui->scene(), SIGNAL(taskRescheduleRequest(unsigned, AstroDateTime, bool)), this, SLOT(moveTaskRequest(unsigned, const AstroDateTime &)));
//	gui->taskDialog()->connect(gui->taskDialog(), SIGNAL(taskChanged(Task &, bool)), this, SLOT(updateTask(Task &, bool)));
//	gui->taskDialog()->connect(gui->taskDialog(), SIGNAL(abortTask(unsigned int)), this, SLOT(abortTask(unsigned int)));
//	gui->taskDialog()->connect(gui->taskDialog(), SIGNAL(addNewTask(const Task &)), this, SLOT(createTask(const Task &)));
//	gui->taskDialog()->connect(gui->taskDialog(), SIGNAL(addNewReservation(const Task &)), this, SLOT(createReservation(const Task &)));
//	gui->taskDialog()->connect(gui->taskDialog(), SIGNAL(checkPredecessor(unsigned)), this, SLOT(checkPredecessor(unsigned)));
//	gui->connect(gui->getSchedulerGUIClass().action_Align_left, SIGNAL(triggered()), this, SLOT(alignLeft()));
// thrash bin connections
	gui->connect(gui->getSchedulerGUIClass().action_Thrashcan, SIGNAL(triggered()), this, SLOT(showThrashBin()));
	itsThrashBin.connect(&itsThrashBin, SIGNAL(restoreTasksRequest(const std::vector<unsigned> &)), this, SLOT(restoreTasks(const std::vector<unsigned> &)));
	itsThrashBin.connect(&itsThrashBin, SIGNAL(thrashBinIsEmpty()), this, SLOT(thrashBinEmpty()));
	itsThrashBin.connect(&itsThrashBin, SIGNAL(thrashBinContainsItems()), this, SLOT(thrashBinNotEmpty()));
	itsThrashBin.connect(&itsThrashBin, SIGNAL(destroyTasks(std::vector<unsigned>)), this, SLOT(doDestroyTasks(std::vector<unsigned>)));

#ifdef HAS_SAS_CONNECTION
	gui->connect(gui->getSchedulerGUIClass().action_DownloadSASSchedule, SIGNAL(triggered()), this, SLOT(downloadSASSchedule()));
	gui->connect(gui->getSchedulerGUIClass().action_SyncSASSchedule, SIGNAL(triggered()), this, SLOT(InitSynchronizeSASSchedule()));
	gui->connect(gui->getSchedulerGUIClass().actionCheck_SAS_status, SIGNAL(triggered()), this, SLOT(checkSASStatus()));
#endif

}

const AstroDateTime &Controller::now(void) {
	QDateTime currentTime = QDateTime::currentDateTimeUtc();
	itsTimeNow = AstroDateTime(currentTime.date().day(), currentTime.date().month(), currentTime.date().year(),
			currentTime.time().hour(), currentTime.time().minute(), currentTime.time().second());
	return itsTimeNow;
}

#ifdef HAS_SAS_CONNECTION

void Controller::checkSASStatus(void) const {
	itsSASConnection->checkSASStatus();
}

void Controller::setSASConnectionSettings(const QString &username, const QString &password, const QString &DBname, const QString &hostname) {
	Controller::theSchedulerSettings.setSASConnectionSettings(username, password, DBname, hostname);
}


void Controller::downloadSASSchedule(void) {
	//TODO: when downloading the running schedule we discard the data that is possibly in the scheduler already.
	// ask the user (if there are tasks in the schedule) if he wants to proceed and discard these.
	// Next question: which schedule settings to use? (time span of schedule, which stations are defined? etc)
	// Answers: scheduler settings: use the settings that are already loaded into the scheduler
	// if tasks from SAS contain stations that are not defined, then these tasks will be unscheduled and marked with an error.
	// we have to run checkTasks() to look for possible errors in the downloaded tasks.
	if (data.getUploadRequired()) {
		if (QMessageBox::question(0, tr("Confirm discard changes"),
				tr("Changes were made to the schedule. By downloading the schedule again those changes will be discarded.\nDo you want to continue with the download?"),
				QMessageBox::Ok,
				QMessageBox::Cancel) == QMessageBox::Cancel) {
			return;
		}
	}
	storeScheduleUndo(QObject::tr("Download SAS schedule"));
	if (checkSASSettings()) {
		gui->setStatusText("Now downloading running schedule from SAS...");
		itsSASConnection->init(theSchedulerSettings.getSASUserName(),
							theSchedulerSettings.getSASPassword(),
							theSchedulerSettings.getSASDatabase(),
							theSchedulerSettings.getSASHostName());
//		itsSASConnection.init("postgres", "", "jong", "127.0.0.1");
		int result = itsSASConnection->connect();
		if (result == 0) {
			// clear the current schedule
			cleanup(true);
//			usedTaskIDs = itsSASConnection->getUsedSASTaskIDs(); // we have to do this before any task is created or downloaded
			if (itsSASConnection->downloadAllSASTasks()) { // this will call mergeDownloadedSASTasks
//				data.setSASUsedTaskIDs(itsSASConnection->getUsedSASTaskIDs());
				gui->setStatusText("Download of running schedule from SAS completed");
			}
		}
		else if (result == -1) {
			deleteLastStoredUndo();
			QApplication::beep();
			QMessageBox::critical(0, tr("No connection to SAS"),
					tr("Could not connect to SAS database.\nPlease check SAS connection settings.\n\nError:") + itsSASConnection->lastConnectionError());

			gui->setStatusText("Error: Could not connect to SAS");
		}
		else if (result == -2) {
			deleteLastStoredUndo();
			QApplication::beep();
			QMessageBox::warning(0, tr("No write permissions to SAS database"),
					tr("You don't have write permissions to the SAS database.\n Please check the SAS user name and password"));
			gui->setStatusText("Error: No write permissions to SAS database");
		}
	}
}

void Controller::autoPublish(void) {
    std::pair<AstroDate, AstroDate> publishRange(itsSASConnection->getUploadedDateRange());
    if (publishRange.first.isSet()) {
        const AstroDate &today(itsTimeNow.date());
        if (publishRange.first < today) publishRange.first = today; // don't auto publish the past

        unsigned day = publishRange.first.getDayOfTheWeek();
        AstroDate monday = publishRange.first.subtractDays(day); // monday of current week
        scheduleWeekVector publishWeeks;
        publishWeeks.push_back(std::pair<unsigned, AstroDate>(monday.getWeek(), monday));

        while (publishRange.second >= monday.addDays(7)) {
            monday = monday.addDays(7); // next week has to be included in publish
            publishWeeks.push_back(std::pair<unsigned, AstroDate>(monday.getWeek(), monday));
        }

        QStringList weekNrStr;
        for (scheduleWeekVector::const_iterator pit = publishWeeks.begin(); pit != publishWeeks.end(); ++pit) {
            weekNrStr += QString::number(pit->first);
        }
        itsSASConnection->progressDialog().addText("Publishing week numbers:" + weekNrStr.join(QChar(',')));
        gui->publish(publishWeeks);
    }
}

void Controller::InitSynchronizeSASSchedule(void) {
	if ((data.getNrTasks() > 0) || (itsSASConnection->nrTaskToDeleteFromSAS())) {
		int result;
		if (checkSASSettings()) {
            gui->setStatusText("Now synchronizing schedule with SAS...");
			itsSASConnection->showProgressUploadDialog();
			itsSASConnection->progressDialog().addText("Checking and assigning resources...");
			result = assignResources(false);
			if (result == 0) { // resource assignment went ok
				itsSASConnection->progressDialog().addText("Resource assignment OK.");
				result = itsSASConnection->connect();
				if (result == 0) {
					// initialize the upload to SAS dialog
                    itsSASConnection->startSynchronizeProcedure(data);
				}
				else if (result == -1) { // could not connect to SAS database
					QApplication::beep();
					QString errStr(tr("Could not connect to SAS database.\nPlease check SAS connection settings.\n") +
							tr("host:") + theSchedulerSettings.getSASHostName() + tr(", database:") + theSchedulerSettings.getSASDatabase() + tr(", username:") + theSchedulerSettings.getSASUserName() +
							"\nError:" + itsSASConnection->lastConnectionError());
					QMessageBox::critical(0, tr("No connection to SAS"), errStr);
					itsSASConnection->progressDialog().addError(errStr);
					itsSASConnection->progressDialog().enableClose();
					gui->setStatusText("Error: Could not connect to SAS");
				}
				else if (result == -2) {
					QApplication::beep();
					QMessageBox::warning(0, tr("No write permissions to SAS database"),
							tr("You don't have write permissions to the SAS database.\n Please check the SAS user name and password"));
					gui->setStatusText("Error: No write permissions to SAS database");
				}
			}
			else {
				itsSASConnection->closeProgressUploadDialog();
                gui->clearStatusText();
			}
		}
	}
	else {
		QMessageBox::warning(0, tr("Empty schedule cannot be uploaded"), tr("The schedule doesn't contain any tasks.\nAn empty schedule cannot be uploaded to SAS."));
	}
}

void Controller::commitScheduleToSAS(void) {
	if (itsSASConnection->commitScheduleToSAS(data)) {
        // auto publish
        if (itsSASConnection->autoPublish()) {
            itsSASConnection->progressDialog().addText("Now starting the automatic publish of the schedule to the web-server");
            autoPublish();
        }

		data.clearUploadRequiredFlag();
		cleanup(true, false);
		mergeDownloadedSASTasks();
		gui->setStatusText(tr("schedule was successfully uploaded to SAS database"));
		itsSASConnection->addProgressInfo(tr("The new schedule was successfully uploaded to the SAS database"));
	}
	else {
		gui->setStatusText(tr("Error Uploading schedule to SAS"));
		QMessageBox::critical(0, tr("Error uploading schedule to SAS"), tr("The schedule could not correctly be uploaded to the SAS database"));
	}
	gui->writeTableData(data);
}

// ask user if he wants to overwrite existing tasks
// arguments to this function will be set according to the answers
// return value is true when user wants to continue, false if cancel was clicked
bool Controller::askOverWriteExistingTask(bool &overwrite, bool &forAll, unsigned taskID, const QString &taskName) {
	QMessageBox questionBox(QMessageBox::Question,tr("Confirm overwrite existing task"),"Task " + QString::number(taskID) +
			", '" + taskName +"' already exist.\nDo you want to overwrite this task?");
	QAbstractButton *overWriteButton = questionBox.addButton(tr("Overwrite"), QMessageBox::ActionRole);
	QAbstractButton *overWriteAllButton = questionBox.addButton(tr("Overwrite all"), QMessageBox::ActionRole);
	QAbstractButton *keepButton = questionBox.addButton(tr("Keep current"), QMessageBox::ActionRole);
	QAbstractButton *keepAllButton = questionBox.addButton(tr("Keep all"), QMessageBox::ActionRole);
	QAbstractButton *CancelDownloadButton = questionBox.addButton(tr("Cancel Download"), QMessageBox::ActionRole);
	questionBox.exec();
	if (questionBox.clickedButton() == overWriteButton) {
		overwrite = true;
		forAll = false;
		return true;
	}
	else if (questionBox.clickedButton() == overWriteAllButton) {
		overwrite = true;
		forAll = true;
		return true;
	}
	else if (questionBox.clickedButton() == keepButton) {
		overwrite = false;
		forAll = false;
		return true;
	}
	else if (questionBox.clickedButton() == keepAllButton) {
		overwrite = false;
		forAll = true;
		return true;
	}
	else if (questionBox.clickedButton() == CancelDownloadButton) {
		return false;
	}
	return false;
}


void Controller::mergeDownloadedSASTasks(void) {
	const SAStasks &sasTasks = itsSASConnection->SASTasks();
	Task::task_status status;
    vector<Task *> newPipelines;

	unsigned taskID;
	for (SAStasks::const_iterator it = sasTasks.begin(); it != sasTasks.end(); ++it) {
        taskID = it->second->getID();
        Task *pTask(0);
        Observation *pObs = dynamic_cast<Observation *>(it->second);
        StationTask *pStat(0);
        Pipeline *pPipe(0);
        if (pObs) {
            pTask = data.newObservation(taskID, OVERRIDE_SAS_TASKIDS);
            pTask->clone(pObs); // clones the properties from pObs into the newly created observation pointed to by pTask
            pTask->calculateDataFiles();
        }
        else if ((pStat = dynamic_cast<StationTask *>(it->second))) {
            pTask = data.newReservation(taskID, OVERRIDE_SAS_TASKIDS);
            pTask->clone(pStat);
		}
        else if ((pPipe = dynamic_cast<Pipeline *>(it->second))) {
            pTask = data.newPipeline(taskID, pPipe->pipelinetype(), OVERRIDE_SAS_TASKIDS);
            pTask->clone(pPipe);
            newPipelines.push_back(pTask);
		}
        else {
            debugWarn("sis", "Controller::mergeDownloadedSASTasks: task:", taskID, " is of unknown type. Skipping.");
            continue;
        }

		if (pTask) {
            // copy all properties from itsSasTasks
//            *pTask = *it->second;
           // pTask->setID(taskID); // set the ID again to the ID of the newly created task
			status = pTask->getStatus();

			if ((status == Task::SCHEDULED) || (status == Task::PRESCHEDULED)) {
				if (pTask->isStationTask()) {
                    if (data.checkStationConflicts(static_cast<StationTask *>(pTask))) {
						if (data.scheduleTask(pTask)) {
							pTask->setStatus(status);
						}
					}
					else { // there is a conflict of stations in use
						pTask->setStatus(Task::CONFLICT);
					}
				}
				else {
					if (data.scheduleTask(pTask)) {
						pTask->setStatus(status);
					}
				}
			}
			else if ((status >= Task::STARTING) && (status <= Task::COMPLETING)) {
				if (data.scheduleTask(pTask)) {
					pTask->setStatus(status);
				}
			}
			else if (status >= Task::FINISHED) {
				data.moveTaskToInactive(pTask->getID());
			}
		}
	}

	// go by all pipelines to update their input files, because these are dependent on other tasks
    for (std::vector<Task *>::const_iterator nit = newPipelines.begin(); nit != newPipelines.end(); ++nit) {
        setInputFilesForPipeline(*nit);
        static_cast<Pipeline *>(*nit)->calculateDataFiles();
	}

	data.checkStatusChanges(); // check all task in the scheduler for status changes and moves the tasks to the correct map
	if (!data.checkTasksForErrors()) {
		itsSASConnection->addProgressError("Errors were detected in some of the downloaded tasks. Please check and correct were necessary.");
	}
	updateGraphicTasks();
	gui->writeTableData(data);
    gui->updateProjectsFilter(Controller::theSchedulerSettings.getCampaignList());
	updateStatusBar();
	setSaveRequired(true);
	data.clearUploadRequiredFlag();
}


bool Controller::checkSASSettings(void) {
	if (theSchedulerSettings.getSASUserName().isEmpty()) {
		QMessageBox::critical(0, tr("SAS user name not specified"),
				tr("SAS user name is not specified.\n Please specify this via menu 'Settings' -> menu option 'Schedule settings' -> 'SAS' tab"));
		return false;
	}
	else if (theSchedulerSettings.getSASPassword().isEmpty()) {
		QMessageBox::critical(0, tr("SAS password not set"),
				tr("SAS password is not specified.\n Please specify this via menu 'Settings' -> menu option 'Schedule settings' -> 'SAS' tab"));
		return false;
	}
	else if (theSchedulerSettings.getSASDatabase().isEmpty()) {
		QMessageBox::critical(0, tr("SAS database name not specified"),
				tr("SAS database name is not specified.\n Please specify this via menu 'Settings' -> menu option 'Schedule settings' -> 'SAS' tab"));
		return false;
	}
	else if (theSchedulerSettings.getSASHostName().isEmpty()) {
		QMessageBox::critical(0, tr("SAS host name not specified"),
				tr("SAS host name is not specified.\n Please specify this via menu 'Settings' -> menu option 'Schedule settings' -> 'SAS' tab"));
		return false;
	}

	return true;
}

int Controller::checkSASconnection(const QString &username, const QString &password, const QString &DBname, const QString &hostname) {
	return itsSASConnection->testConnect(username, password, DBname, hostname);
}

QString Controller::lastSASError(void) const {
	return itsSASConnection->lastConnectionError();
}

#endif // HAS_SAS_CONNECTION
/*
void Controller::checkPredecessor(unsigned predecessor_id) const {
	if (data.predecessorExists(predecessor_id)) {
		gui->taskDialog()->updatePredecessor(true);
	}
	else {
		gui->taskDialog()->updatePredecessor(false);
	}
}
*/
void Controller::newSchedule(void) {
	if (closeSchedule()) {
		gui->newTable(data);
		gui->setEnableScheduleMenuItems(true);
		itsDataHandler->setFileName("");
	}
}

void Controller::balanceSchedule(void) {
	QMessageBox::warning(0, tr("Function not implemented"),
			tr("Balance schedule function not yet implemented"));
	return;

	if (data.getNrTasks()) {
		// TODO: implement balance schedule


	}
	else {
		QMessageBox::warning(gui, tr("No tasks defined"),
				tr("There are no tasks defined. Cannot balance an empty schedule."),tr("Close"));
	}
}

void Controller::addTask(void) {
//	unsigned newTaskID = data.getNewTaskID();
//	storeTaskUndo(newTaskID, QString("Add task ") + QString::number(newTaskID), true); // when second argument is set to true it means, when undo is applied delete the task
	gui->addTaskDialog(data.getNewTaskID());
}

//void Controller::addReservation(void) {
//	gui->addReservationDialog(data.getNewTaskID());
//}


// createTask return value: 0 - task created, -1 task not created, 1 task created but has conflict/error
bool Controller::createTask(const Task &task) {
    bool bResult(true);
    unsigned task_id(task.getID());
    Task::task_status status = task.getStatus();
    Task *new_task = cloneTask(&task);
    if (new_task) {
		// predecessor existence checks
		if (task.hasPredecessors()) {
			const IDvector &predecessors(task.getPredecessors());
			for (IDvector::const_iterator prit = predecessors.begin(); prit != predecessors.end(); ++prit) {
				if (getTask(prit->second, prit->first) == 0) {
					QMessageBox warningBox(gui);
					warningBox.setWindowTitle(tr("Task has non existing predecessors"));
					warningBox.setIcon(QMessageBox::Critical);
					warningBox.addButton(tr("Ok"), QMessageBox::NoRole);
					warningBox.setText("The task has non-existing predecessors\nThe task cannot be created before this error is resolved");
					warningBox.exec();
                    bResult = false;
				}
			}
		}
        if (bResult && (status == Task::SCHEDULED || status == Task::PRESCHEDULED)) {
//			 do (PRE)SCHEDULE checks
			std::pair<unscheduled_reasons, QString> errCode = doPreScheduleChecks(new_task);
			if (errCode.first >= SCHEDULED_TOO_EARLY) {
				QMessageBox warningBox(gui);
				warningBox.setWindowTitle(tr("Task has critical problems"));
				warningBox.setIcon(QMessageBox::Critical);
				warningBox.addButton(tr("Ok"), QMessageBox::NoRole);
				warningBox.setText(errCode.second.replace('$','\n') + "\nThe task cannot be created before this problem is resolved");
				warningBox.exec();
                bResult = false;
			}
			else {
                if (errCode.first == USER_WARNING) {
                    if (QMessageBox::question(gui, tr("Warning"),
                                              errCode.second + "\nDo you want to continue?",
                            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) {
                        bResult = false;
                    }
                }
			}
		}
		itsSelectedTasks.clear();
		itsSelectedTasks.push_back(task_id);
		gui->selectRows(itsSelectedTasks);
    }
    else { // could not clone task
        bResult = false;
    }

    if (bResult) {
        storeTaskUndo(task_id, QString("Add task ") + QString::number(task_id), true); // when second argument is set to true it means, when undo is applied delete the task
        setSaveRequired(true);
        data.addTask(new_task, false);
        if (status == Task::SCHEDULED) {
            if (doScheduleChecks(new_task)) {
                data.scheduleTask(new_task);
            }
        }
        else { // PRESCHEDULED
            data.scheduleTask(new_task);
            new_task->setStatus(Task::PRESCHEDULED);
        }
        gui->addTask(new_task);
        updateStatusBar();
    }

    return bResult;
}

bool Controller::createReservation(const Task *reservation) {
    int reservation_id = reservation->getID();
	Task *new_reservation = data.newReservation(reservation_id);
	if (new_reservation) {
		storeTaskUndo(reservation_id, QString("Add reservation ") + QString::number(reservation_id), true); // when second argument is set to true it means, when undo is applied delete the reservation
		setSaveRequired(true);
        *new_reservation = *reservation;
		data.addUsedTaskID(reservation_id);

        Task::task_status status = reservation->getStatus();
		if (status == Task::SCHEDULED || status == Task::PRESCHEDULED) {
//			 do (PRE)SCHEDULE checks
			std::pair<unscheduled_reasons, QString> errCode = doPreScheduleChecks(new_reservation);
			if (errCode.first == TASK_CONFLICT) {
				QMessageBox warningBox(gui);
				warningBox.setWindowTitle(tr("Reservation has conflicts with other tasks"));
				warningBox.setIcon(QMessageBox::Critical);
				warningBox.addButton(tr("Ok"), QMessageBox::NoRole);
				warningBox.setText(errCode.second.replace('$','\n') + "\nThe reservation cannot be created before this error/conflict is resolved");
				warningBox.exec();
                data.deleteTask(new_reservation->getID());
				deleteLastStoredUndo();
				return false;
			}
			else {
				if (status == Task::SCHEDULED) {
					if (doScheduleChecks(new_reservation)) {
						data.scheduleTask(new_reservation);
					}
				}
				else { // PRESCHEDULED
					data.scheduleTask(new_reservation);
					new_reservation->setStatus(Task::PRESCHEDULED);
				}
                gui->addTask(new_reservation);
				updateStatusBar();
			}
		}
		itsSelectedTasks.clear();
		itsSelectedTasks.push_back(reservation_id);
		gui->selectRows(itsSelectedTasks);
		return true;
	}

	return false;
}

void Controller::createPipeline(const Pipeline *pipeline) {
    Task *pTask = cloneTask(pipeline);
    data.addTask(pTask,false);
    unsigned task_id(pTask->getID());
    storeTaskUndo(task_id, QString("Add Pipeline ") + QString::number(task_id), true); // when second argument is set to true it means, when undo is applied delete the pipeline
    setSaveRequired(true);
    gui->addTask(pTask);
    updateStatusBar();
    itsSelectedTasks.clear();
    itsSelectedTasks.push_back(task_id);
    gui->selectRows(itsSelectedTasks);
}

// expungeTask: deletes a task from the scheduler without undo and without it being added to the deletedTasks list
// used by SASconnection when a task was externally deleted from SAS
void Controller::expungeTask(unsigned treeID) {
	const Task *pTask(data.getTask(treeID, ID_SAS));
	if (pTask) {
		unsigned taskID(pTask->getID());
		gui->deleteTaskFromGUI(taskID);
        data.deleteTask(taskID);
		updateStatusBar();
	}
}

// deletes a single task
void Controller::deleteTask(unsigned taskID) {
    Task *delTask(data.deleteTask(taskID, ID_SCHEDULER, false));

    if (delTask) {
        QString undo_text = "Delete task " + QString::number(taskID);
        itsUndoType.push_back(undoTypeVector::value_type(UNDO_DELETE_TASKS, std::pair<QString, vector<unsigned> >(undo_text, itsSelectedTasks )));
        gui->addUndo(undo_text);

        vector<Task *> deletedTasks;
        deletedTasks.push_back(delTask);
        gui->deleteTaskFromGUI(taskID);

        int treeID(delTask->getSASTreeID());
        if (treeID)
            itsSASConnection->addToTreesToDelete(treeID, taskID);

        itsThrashBin.addTasks(deletedTasks);
        itsDeletedTasks.push_back(deletedTasks);
        gui->setFullThrashIcon();
        clearRedoStack();
        //set status string
        gui->setStatusText(QString("Deleted task " + QString::number(taskID)));
        updateStatusBar();
        itsSelectedTasks.clear();
        setSaveRequired(true);
    }
}


// deletes selected tasks
void Controller::deleteSelectedTasks(void) {
	if (!itsSelectedTasks.empty()) {
		if (QMessageBox::question(0, tr("Delete task(s)"),
				tr("Are you sure you want to delete the selected task(s)?"),
				QMessageBox::Ok,
				QMessageBox::Cancel) == QMessageBox::Ok) {
			if (itsSelectedTasks.size() == 1) {
				QString undo_text = "Delete task " + QString::number(itsSelectedTasks.front());
				itsUndoType.push_back(undoTypeVector::value_type(UNDO_DELETE_TASKS,
						std::pair<QString, vector<unsigned> >
								(undo_text, itsSelectedTasks )));
				gui->addUndo(undo_text);

			}
			else {
				QString taskIDs;
				for (vector<unsigned>::iterator it = itsSelectedTasks.begin(); it != itsSelectedTasks.end(); ++it) {
					taskIDs.append(QString::number(*it) + ",");
				}
				taskIDs.remove(taskIDs.length()-1,1); // removes the last comma
				QString undo_text = "Delete tasks " + taskIDs;
			//	itsUndoType.push_back(std::pair<undo_type, QString>(UNDO_DELETE_TASKS, undo_text));
				itsUndoType.push_back(undoTypeVector::value_type(UNDO_DELETE_TASKS,
						std::pair<QString, vector<unsigned> >
								(undo_text, itsSelectedTasks )));

				gui->addUndo(undo_text);
			}
            vector<Task *> deletedTasks;
            unsigned treeID;

			//  *** CAUTION ! ***
			// have to block signals from GUI otherwise it will send signals when deleting graphicTasks from its view,
			// (i.e. it will generate signal selectionChanged which in turn is coupled to SLOT graphicResourceScene::handleRubberBandSelection()
			// which will indirectly alter (clear) the selection from the Controller while the following FOR loop is still active
			gui->scene()->blockSignals(true);
			for (vector<unsigned>::iterator it = itsSelectedTasks.begin(); it != itsSelectedTasks.end(); ++it) { // do the actual deletion
                Task *delTask(data.deleteTask(*it, ID_SCHEDULER, false));
                if (delTask) {
                    deletedTasks.push_back(delTask);
                    treeID = delTask->getSASTreeID();
                    if (treeID) {
                        itsSASConnection->addToTreesToDelete(treeID, *it);
                    }
                    gui->deleteTaskFromGUI(*it); // will cause signals to be generated if we don't block them (see remark just before FOR loop)
                }
			}
			gui->scene()->blockSignals(false);

			if (!deletedTasks.empty()) {
				itsThrashBin.addTasks(deletedTasks);
				itsDeletedTasks.push_back(deletedTasks);
				gui->setFullThrashIcon();
			}
			clearRedoStack();
			//set status string
			std::string statStr = "Deleted ";
			statStr += int2String(itsSelectedTasks.size()) += " task(s)";
			gui->setStatusText(statStr.c_str());
			updateStatusBar();
			itsSelectedTasks.clear();
			setSaveRequired(true);
		}
	}
}

void Controller::restoreTasks(const vector<unsigned> &tasksToRestore) {
	vector<unsigned> success;
	for (vector<unsigned>::const_iterator it = tasksToRestore.begin(); it != tasksToRestore.end(); ++it) {
		if (unDelete(*it)) {
			success.push_back(*it);
		}
	}
	if (!success.empty()) {
		itsThrashBin.removeRestoredTasks(success);
	}
	updateStatusBar();
}

void Controller::openMoveTasksDialog(void) {
	gui->openMoveTasksDialog();
}

void Controller::moveSelectedTasksRequest(unsigned task_id, const AstroDateTime &new_start_time) {
	const Task *pTask(data.getTask(task_id));
	AstroTime shift = pTask->getScheduledStart().timeDifference(new_start_time);
	moveType typeOfMove =  new_start_time < pTask->getScheduledStart() ? MOVE_LEFT : MOVE_RIGHT;
	moveSelectedTasks(shift, typeOfMove);
}

bool Controller::moveTasks(const std::map<unsigned, AstroDateTime> &startTimes) {
	storeScheduleUndo("Moving task(s)");
    std::map<unsigned, std::pair<Task *, Task::task_status> > movedTasks;
    std::vector<const Task *> updateTableTasks;
    std::vector<unsigned> updateGraphicTasks;

    // unschedule all tasks that are going to be moved so that they don't conflict with each other when doing the move
    // check the status of all tasks to be moved if above PRESCHEDULED then abort the move
    for (std::map<unsigned, AstroDateTime>::const_iterator it = startTimes.begin(); it != startTimes.end(); ++it) {
        Task *pTask(data.getTaskForChange(it->first));
        if (pTask) {
            updateTableTasks.push_back(pTask);
            if (pTask->getStatus() <= Task::PRESCHEDULED) {
                movedTasks[it->first] = std::pair<Task *, Task::task_status>(pTask, pTask->getStatus());
                data.unscheduleTask(it->first);
            }
            else {
                // status higher than PRESCHEDULED -> abort move
                undo(false);
                return false;
            }
        }
    }
    // try to move the tasks (one by one)
    // if the task that is going to be moved has status PRESCHEDULED then check if moveTask returns conflicting tasks, if yes then abort the move and undo
    // if no conflicting tasks were returned for this task move then directly schedule the task at the new time
    // if the task has state lower than PRESCHEDULED (e.g. UNSCHEDULED) then the conflicting tasks returned by moveTask can be ignored. Just continue with the move

    for (std::map<unsigned, std::pair<Task *, Task::task_status> >::const_iterator it = movedTasks.begin(); it != movedTasks.end(); ++it) {
        Task *pTask = it->second.first;
        const AstroDateTime &startTime = startTimes.find(it->first)->second;
        if (pTask->isPipeline()) {
            pTask->setScheduledStart(startTime);
            pTask->setStatus(it->second.second);
        }
        else {
            // try to move the task
            if (it->second.second == Task::PRESCHEDULED) {
                std::vector<unsigned> conflictingTasks = data.moveTask(pTask, startTime, false);
                if (!conflictingTasks.empty()) {
                    undo(false);
                    QString confstr = Vector2StringList(conflictingTasks);
                    QMessageBox::warning(gui, tr("Move not possible"),
                                         "Moving task " + QString::number(it->first) + " conflicts with task(s): " + confstr, tr("Close"));
                    return false;
                }
                else {
                    updateGraphicTasks.push_back(it->first);
                    // schedule the task at the new time
                    data.scheduleTask(pTask);
                    pTask->setStatus(it->second.second);
                }
            }
            else {
                pTask->setScheduledStart(startTime);
                pTask->setStatus(it->second.second);
            }
        }
    }

    if (!updateTableTasks.empty()) {
        gui->updateTableTasksScheduleTimes(updateTableTasks);
    }
    for (std::vector<unsigned>::const_iterator it = updateGraphicTasks.begin(); it != updateGraphicTasks.end(); ++it) {
        gui->updateGraphicTask(*it);
    }

    setSaveRequired(true);
    return true;
}

bool Controller::moveSelectedTasks(const AstroDateTime &date_time, moveType move_type) {
    if (!itsSelectedTasks.empty()) {
		storeScheduleUndo("Moving task(s)");
		AstroDateTime new_start;
		std::vector<std::pair<Task *, Task::task_status> > tasks;
		for (vector<unsigned>::iterator it = itsSelectedTasks.begin(); it != itsSelectedTasks.end(); ++it) {
			Task *pTask(data.getTaskForChange(*it));
			if (pTask) {
				if (pTask->getStatus() <= Task::PRESCHEDULED) {
					tasks.push_back(std::vector<std::pair<Task *, Task::task_status> >::value_type(pTask, pTask->getStatus()));
                    if (pTask->isScheduled()) {
                        data.unscheduleTask(*it);
                        if (pTask->hasStorage()) {
                            pTask->storage()->unAssignStorage();
                        }
                    }
				}
				else {
					QMessageBox::warning(gui, tr("Move not possible"),
							"Some tasks have a status above PRESCHEDULED.\nMoving tasks above PRESCHEDULED is not possible.\nMove is canceled.", tr("Close"));
					undo(false);
					gui->selectTasks(itsSelectedTasks); // re-select the selected tasks
					return false;
				}
			}
		}

		AstroDateTime timenow = now() + Controller::theSchedulerSettings.getMinimumTimeBetweenTasks();

		if ((move_type == MOVE_LEFT) || (move_type == MOVE_RIGHT)) { // these are relative moves so interpret only the time part of date_time (as a relative time)
			for (std::vector<std::pair<Task *, Task::task_status> >::iterator it = tasks.begin(); it != tasks.end(); ++it) {
				switch (move_type) {
				case MOVE_LEFT:
					new_start = it->first->getScheduledStart() - date_time.time();
					break;
				default: // MOVE_RIGHT
					new_start = it->first->getScheduledStart() + date_time.time();
					break;
				}
				if (it->first->isObservation() && new_start <= timenow) {
					undo(false);
					gui->selectTasks(itsSelectedTasks); // re-select the selected tasks
					QMessageBox::warning(gui, tr("Move not possible"),
							"Observations cannot be scheduled before now.\nStart time would be too early.\nMove is canceled.", tr("Close"));
					return false;
				}
                std::vector<unsigned> conflictingTasks = data.moveTask(it->first, new_start, false);
                if (!conflictingTasks.empty()) {
					unsigned taskID(it->first->getID());
					undo(false); // dangerous! it also changes all pointers such as pTask
					gui->selectTasks(itsSelectedTasks); // re-select the selected tasks
                    QString confstr = Vector2StringList(conflictingTasks);
					QMessageBox::warning(gui, tr("Move not possible"),
							"Moving task " + QString::number(taskID) + " conflicts with task(s): " + confstr + "\nMove is canceled.", tr("Close"));
					return false;
				}
			}
		}
		else if (move_type == MOVE_TO_START) { // shift to absolute specified time
			AstroDateTime earliestStartTime = tasks.front().first->getScheduledStart();
			for (std::vector<std::pair<Task *, Task::task_status> >::iterator it = tasks.begin(); it != tasks.end(); ++it) {
				if (it->first->getScheduledStart() < earliestStartTime) {
					earliestStartTime = it->first->getScheduledStart();
				}
			}

			AstroDateTime newFirstTime(date_time);
			AstroTime dif;
			bool left_right;
			if (earliestStartTime < newFirstTime) { // shift right
				dif = newFirstTime - earliestStartTime;
				left_right  = true;

			}
			else { // shift left
				dif = earliestStartTime - newFirstTime;
				left_right  = false;
			}

			for (std::vector<std::pair<Task *, Task::task_status> >::iterator it = tasks.begin(); it != tasks.end(); ++it) {
				if (left_right) {
					new_start = it->first->getScheduledStart() + dif;
				}
				else {
					new_start = it->first->getScheduledStart() - dif;
				}

				if (it->first->isObservation() && new_start <= timenow) {
					undo(false);
					gui->selectTasks(itsSelectedTasks); // re-select the selected tasks
					QMessageBox::warning(gui, tr("Move not possible"),
							"Observations cannot be scheduled before now.\nStart time would be too early.\nMove is canceled.", tr("Close"));
					return false;
				}

                 std::vector<unsigned> conflictingTasks = data.moveTask(it->first, new_start, false);
                if (!conflictingTasks.empty()) {
					unsigned taskID(it->first->getID());
					undo(false); // dangerous! it also changes all pointers such as pTask
					gui->selectTasks(itsSelectedTasks); // re-select the selected tasks
                    QString confstr = Vector2StringList(conflictingTasks);
					QMessageBox::warning(gui, tr("Move not possible"),
							"Moving task " + QString::number(taskID) + " conflicts with task(s): " + confstr + "\nMove is canceled.", tr("Close"));
					return false;
				}
			}
		}
		else { // MOVE_TO_CENTER
			AstroDateTime earliestStartTime = tasks.front().first->getScheduledStart();
			AstroDateTime latestEndTime = tasks.front().first->getScheduledEnd();
			for (std::vector<std::pair<Task *, Task::task_status> >::iterator it = tasks.begin(); it != tasks.end(); ++it) {
				if (it->first->isObservation()) { // only look at observations to determine the shift for a CENTER = LST move
					if (it->first->getScheduledStart() < earliestStartTime) {
						earliestStartTime = it->first->getScheduledStart();
					}
					if (it->first->getScheduledEnd() > latestEndTime) {
						latestEndTime = it->first->getScheduledEnd();
					}
				}
			}

			AstroDateTime newCenterTime(date_time);
			AstroDateTime oldCenterTime((latestEndTime.toJulian() + earliestStartTime.toJulian()) / 2);
			AstroTime dif = newCenterTime - oldCenterTime;

			for (std::vector<std::pair<Task *, Task::task_status> >::iterator it = tasks.begin(); it != tasks.end(); ++it) {
				new_start = it->first->getScheduledStart() + dif;

				if (it->first->isObservation() && new_start <= timenow) {
					undo(false);
					gui->selectTasks(itsSelectedTasks); // re-select the selected tasks
					QMessageBox::warning(gui, tr("Move not possible"),
							"Observations cannot be scheduled before now.\nStart time would be too early.\nMove is canceled.", tr("Close"));
					return false;
				}

                std::vector<unsigned> conflictingTasks = data.moveTask(it->first, new_start, false);
                if (!conflictingTasks.empty()) {
					unsigned taskID(it->first->getID());
					undo(false); // dangerous! it also changes all pointers such as pTask
					gui->selectTasks(itsSelectedTasks); // re-select the selected tasks
                    QString confstr = Vector2StringList(conflictingTasks);
					QMessageBox::warning(gui, tr("Move not possible"),
							"Moving task " + QString::number(taskID) + " conflicts with task(s): " + confstr, tr("Close"));
					return false;
				}
			}
		}

		// reschedule the unscheduled moved tasks
        for (std::vector<std::pair<Task *, Task::task_status> >::iterator it = tasks.begin(); it != tasks.end(); ++it) {
            if (it->second == Task::PRESCHEDULED) {
                data.scheduleTask(it->first);
            }
            it->first->setStatus(it->second);
            gui->updateTableTaskScheduleTimes(*(it->first));
            gui->updateGraphicTask(it->first->getID());
        }
		setSaveRequired(true);
	}
	return true;
}

int Controller::getSelectedObservationsTimeSpan(void) const {
	bool isSet(false);
	AstroDateTime earliestStartTime, latestEndTime;
	if (!itsSelectedTasks.empty()) {
		for (std::vector<unsigned>::const_iterator it = itsSelectedTasks.begin()+1; it != itsSelectedTasks.end(); ++it) {
			const Task *pTask(data.getTask(*it));
			if (pTask) {
				if (pTask->isObservation()/* && pTask->getScheduledStart().isSet()*/) {
					if (!isSet) {
						earliestStartTime = pTask->getScheduledStart();
						latestEndTime = pTask->getScheduledEnd();
						isSet = true;
					}
					else {
						if (pTask->getScheduledStart() < earliestStartTime) {
							earliestStartTime = pTask->getScheduledStart();
						}
						if (pTask->getScheduledEnd() > latestEndTime) {
							latestEndTime = pTask->getScheduledEnd();
						}
					}
				}
			}
		}
	}
	if (isSet) {
		return (latestEndTime - earliestStartTime).totalSeconds();
	}
	else return 0;
}

/*
void Controller::alignLeft(void) {
	storeScheduleUndo(tr("Align left"));
	data.alignLeft();
	gui->writeTableData(data);
	updateGraphicTasks();
	gui->setStatusText(tr("Align left finished"));
	setSaveRequired(true);
}
*/
void Controller::showThrashBin(void) {
	itsThrashBin.show();
}

void Controller::doDestroyTasks(vector<unsigned> destroyTasks) {
	bool taskDestroyed;
	vector<unsigned> destroyedTasks;
	for (vector<unsigned>::const_iterator dit = destroyTasks.begin(); dit != destroyTasks.end(); ++dit) {
		taskDestroyed = false;
		for (deletedTasksVector::iterator it = itsDeletedTasks.begin(); it != itsDeletedTasks.end(); ++it) {
            for (vector<Task *>::iterator tit = it->begin(); tit != it->end(); ++tit) {
                if ((*tit)->getID() == *dit) {
					destroyedTasks.push_back(*dit);
					debugInfo("si", "destroying task: ", *dit);
                    delete *tit;
					it->erase(tit);
					taskDestroyed = true;
					break;
				}
			}
			if (taskDestroyed)
				break; // goes to the next task that has to be destroyed
		}
	}
	// update the deleted tasks undo stack
	bool taskFound;
	for (vector<unsigned>::const_iterator dit = destroyedTasks.begin(); dit != destroyedTasks.end(); ++dit) {
		taskFound = false;
		for (undoTypeVector::iterator it = itsUndoType.begin(); it != itsUndoType.end(); ++it) {
			if (it->first == UNDO_DELETE_TASKS) {
				for (vector<unsigned>::iterator vit = it->second.second.begin(); vit != it->second.second.end(); ++vit) {
					if (*vit == *dit) {
						it->second.second.erase(vit);
						if (it->second.second.empty()) { // this undo step has no tasks left, remove it from the undo stack
							gui->removeUndo(it->second.first);
							itsUndoType.erase(it);
						}
						taskFound = true;
						break;
					}
				}
				if (taskFound) break;
			}
		}
	}
	gui->setStatusText(tr("Tasks were destroyed"));
}

void Controller::updateGUIafterSettingsLoad(void) {
	// new stations could be defined, update the task objects to use the new station IDs generated
	data.updateTasksStationIDs();
	data.updateStations(); // create stations if any where loaded from the default settings
	data.checkTasksForErrors(); // re-check tasks because stations may have been removed or added
	gui->updateTaskDialogStations();
	gui->updateGraphicStations();
	updateGraphicTasks();
	gui->writeTableData(data);
    gui->updateSasDatabaseName();
	updateStatusBar();
	gui->initPublishDialog(); // initialize the publish dialog with the correct week numbers
	gui->setExistingProjects(theSchedulerSettings.getCampaignList());
	gui->setTestMode(theSchedulerSettings.getIsTestEnvironment());
}

void Controller::refreshGUIafterMultiEdit(void) {
	data.checkTasksForErrors(); // re-check tasks because stations may have been removed or added
	updateGraphicTasks();
	gui->writeTableData(data);
	updateStatusBar();
}

bool Controller::updateProjects(void) {
	if (checkSASSettings()) {
		gui->setStatusText("fetching project list from SAS...");
		itsSASConnection->init(theSchedulerSettings.getSASUserName(),
				theSchedulerSettings.getSASPassword(),
				theSchedulerSettings.getSASDatabase(),
				theSchedulerSettings.getSASHostName());
		int result = itsSASConnection->connect();
		if (result == 0) {
			if (itsSASConnection->getCampaignsFromSAS()) { // stores the campaigns in theSchedulerSettings
				gui->setExistingProjects(theSchedulerSettings.getCampaignList());
				return true;
			}
			else {
				QMessageBox::critical(0, tr("No connection to SAS"),
						tr("Could not fetch the project list from SAS database ") + theSchedulerSettings.getSASDatabase());
			}
		}
		else if (result == -1) {
			QMessageBox::critical(0, tr("No connection to SAS"),
					tr("Could not connect to SAS database ") + theSchedulerSettings.getSASDatabase()
					+ tr("\n Please check SAS connection settings.\n\nError:")
					+ itsSASConnection->lastConnectionError());
			gui->setStatusText("Error: Could not connect to SAS");
		}
	}
	else {
		std::cerr << "no SAS connection" << std::endl;
	}
	return false;
}

void Controller::saveSettings(void) {
	QString filename = gui->fileDialog(tr("Save Settings"), "set", tr("Settings files (*.set)"),1);
	if (!filename.isEmpty()) {
		itsDataHandler->saveSettings(filename);
		//set status string
		std::string statStr = "Settings saved to file ";
		statStr += filename.toStdString();
		gui->setStatusText(statStr.c_str());
	}
}

void Controller::saveDefaultSettings(void) {
	if (itsDataHandler->saveSettings(PROGRAM_DEFAULT_SETTINGS_FILENAME)) {
		QDir working_dir(QDir::currentPath());
		if (working_dir.exists("WinSCP")) { // if winscp exists then we assume this is the control room pc which is a windows machine and has winSCP dir in the scheduler path
			if (QMessageBox::question(gui, tr("Upload default settings to central scheduler?"),
					tr("Default settings have been saved.\n\nAlso upload these default settings to the central scheduler on sas001?"),
					QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
				QProcess proc;
				QStringList args;
//				std::string working_dir_str(QDir::toNativeSeparators(QDir::currentPath()).toStdString());
				QString curPath(QDir::toNativeSeparators(QDir::currentPath()));
				args <<  "/privatekey=\"C:\\Scheduler\\pksas.ppk\" /script=" << curPath + "\\upload_default_settings.txt";
				proc.start("\\WinSCP\\WinSCP.com", args);
				proc.waitForFinished(-1);
//				string systemCmd(working_dir_str + "\\WinSCP\\WinSCP.com /privatekey=\"C:\\Scheduler\\pksas.ppk\" /script=" + working_dir_str + "\\upload_default_settings.txt");
//				std::system(systemCmd.c_str()); // upload .default_settings.set file to sas server
			}
		}
		else {
			QMessageBox::information(gui,"saved default settings", tr("Default settings have been saved."));
		}
	}
	else {
		QMessageBox::critical(gui,"default settings could not be saved", tr("ERROR: Default settings could not be saved."));
	}
}

void Controller::loadDefaultSettings(void) {
	QMessageBox msgBox;

	if (itsDataHandler->loadSettings(PROGRAM_DEFAULT_SETTINGS_FILENAME)) {
		updateGUIafterSettingsLoad();
		msgBox.setText(tr("Default settings have been loaded."));
	} else {
		msgBox.setText(tr("ERROR: Default settings could not be loaded."));
	}
	msgBox.exec();
}

void Controller::loadSettings(void) {
	QString filename = gui->fileDialog(tr("Load Settings"), "set", tr("Settings files (*.set)"));
	if (!filename.isEmpty()) {
		if (itsDataHandler->loadSettings(filename)) {
			updateGUIafterSettingsLoad();
			gui->setStatusText("Settings loaded");
		}
	}
}

void Controller::optimizeSchedule(void) {
	if (data.getNrTasks()) {
		gui->setProgressBarMaximum(Controller::theSchedulerSettings.getMaxNrOptimizations());
		gui->setStatusText("Optimize in progress");
		storeScheduleUndo(QObject::tr("Optimize schedule"));
		//	data.create_undo_level();
		//	data.setSaveRequired(true);
		//	registerChange(SCHEDULER_DATA_BLOCK_CHANGE);
		int result = scheduler.optimize();
		//	data.checkPredecessors(); // check time limits on predecessor tasks
		if (result == 0) {
			debugInfo("s", "Optimization finished.");
		} else if (result == 1) {
			debugInfo("s", "Optimization finished, user accepted penalty was reached.");
		} else if (result == 2) {
			debugInfo("s", "Optimization finished, the maximum number of iterations was reached.");
		} else if (result == 3) {
			debugInfo("s", "Optimization finished, there are no more scheduled tasks that may be changed");
		}
		gui->writeTableData(data);
		updateGraphicTasks();
		gui->setStatusText("Optimization finished");
		gui->disableProgressBar();
		debugInfo("si", "Penalty obtained: ", data.getPenalty());
		debugInfo("si", "# of successfully scheduled tasks: ", data.getNrScheduled());
		setSaveRequired(true);
	}
	else {
		QMessageBox::warning(gui, tr("No tasks defined"),
				tr("There are no tasks defined. Cannot optimize an empty schedule."),tr("Close"));
	}
}

void Controller::updateStatusBar(void) { gui->updateStatusBar(data.getNrScheduled(), data.getNrUnscheduled(),
		data.getNrInactive(), data.getNrReservations(), data.nrOfErrorTasks(), data.getNrPipelines());
}

void Controller::updateStatusBarOptimize(unsigned optimize_iteration) {
	gui->updateStatusBar(data.getNrScheduled(), data.getNrUnscheduled(),
				data.getNrInactive(), data.getNrReservations(), data.nrOfErrorTasks(), optimize_iteration);
}

void Controller::applyTableItemChange(unsigned taskID, data_headers property, const QVariant &value, const QModelIndex &index) {
	checkTableItem(index);
	Task *pTask = data.getTaskForChange(taskID);
	Task::task_status status(pTask->getStatus());

	// create undo
	storeTaskUndo(taskID, QString("Task ") + QString::number(taskID) +  " " + DATA_HEADERS[property] + " change");

	if (index.column() == TASK_STATUS) { // user is trying to change the status, don't apply status just yet, first do the preschedule checks
		QString newstatus(value.toString());
		if ((newstatus == task_states_str[Task::PRESCHEDULED]) || (newstatus == task_states_str[Task::SCHEDULED])) {
			std::pair<unscheduled_reasons, QString> errCode = doPreScheduleChecks(pTask);
            if (errCode.first == BEAM_DURATION_DIFFERENT) {
				if (QMessageBox::question(gui, tr("Beam duration different"),
						errCode.second.replace('$','\n') + "\nDo you still want to set the task to " + newstatus + "?",
						QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) {
					// don't apply the status change
					deleteLastStoredUndo();
					return;
				}
				else {
					applyTaskPropertyChange(taskID, property, value, STORE_UNDO); // apply the status change
					updateStatusBar();
				}
			}
            else if (errCode.first == USER_WARNING) {
                if (QMessageBox::question(gui, tr("Warning"),
                                          errCode.second + "\nDo you still want to set the task to " + newstatus + "?",
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) {
                    deleteLastStoredUndo();
                    return;
                }
            }

			else if (errCode.first == SCHEDULED_TOO_EARLY) {
				QMessageBox::warning(gui, tr("Start time too early"),
						QString("Error: ") + errCode.second.replace('$','\n') + "\n\nThe change will not be applied!");
					// don't apply the status change -> undo the change
				deleteLastStoredUndo();
			}
			else if (errCode.first >= TASK_CONFLICT) {
				if (pTask->isScheduled()) {
					data.unscheduleTask(taskID);
					pTask->clearAllStorageConflicts();
                    if (pTask->hasStorage()) {
                        pTask->storage()->unAssignStorage();
                        pTask->storage()->generateFileList();
                    }
                }
				if (errCode.first == TASK_CONFLICT) {
					pTask->setStatus(Task::CONFLICT);
				}
				else {
					pTask->setStatus(Task::ERROR);
				}
				updateStatusBar();
				QMessageBox warningBox(gui);
				warningBox.setWindowTitle(tr("Task has errors/conflicts"));
				warningBox.setIcon(QMessageBox::Critical);
				warningBox.addButton(tr("Ok"), QMessageBox::NoRole);
				warningBox.setText(errCode.second.replace('$','\n') + "\nThe task cannot be set to (PRE)SCHEDULED before the problem is resolved");
				warningBox.exec();
			}
			else {
				applyTaskPropertyChange(taskID, property, value, STORE_UNDO); // apply the status change
				updateStatusBar();
			}
		}
		else {
			applyTaskPropertyChange(taskID, property, value, STORE_UNDO); // apply the change
		}
	}
	// for changes made in the table other than status changes
	else {
		// do the preschedule checks after the change has been applied
		// If there is an error ask the user if he still wants to apply the change,
		// if he wants to apply the change the task should be set to error state
		// if not undo the last stored undo

		applyTaskPropertyChange(taskID, property, value, STORE_UNDO); // apply the change regarding if error or not
		Task *pTask = data.getTaskForChange(taskID);
		if ((status == Task::PRESCHEDULED) || (status == Task::SCHEDULED)) {
			std::pair<unscheduled_reasons, QString> errCode = doPreScheduleChecks(pTask);
			if (errCode.first == BEAM_DURATION_DIFFERENT) {
				QMessageBox::warning(gui, tr("Beam duration different"), errCode.second.replace('$','\n'), QMessageBox::Ok);
			}
            else if (errCode.first == USER_WARNING) {
                if (QMessageBox::question(gui, tr("Warning"),
                                          errCode.second + "\nDo you still want to apply the change?",
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) {
                    undo(false);
                }
            }
			else if (errCode.first == SCHEDULED_TOO_EARLY) {
				QMessageBox::warning(gui, tr("Start time too early"),
						QString("Error: ") + errCode.second.replace('$','\n') + "\n\nThe change will not be applied!");
					// don't apply the status change -> undo the change
				undo(false);
			}
			else if (errCode.first >= TASK_CONFLICT) {
				if (QMessageBox::question(gui, tr("Task has errors/conflicts"),
						QString("Error: ") + errCode.second.replace('$','\n') + "\n\nIf the change is applied the task will get the ERROR/CONFLICT status.\nDo you still want to apply the change?",
						QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) {
					// don't apply the status change -> undo the change
					undo(false);
				}
				else {
					data.unscheduleTask(taskID);
					pTask->clearAllStorageConflicts();
                    if (pTask->hasStorage()) {
                        pTask->storage()->unAssignStorage();
                        pTask->storage()->generateFileList();
                    }
                    if (errCode.first == TASK_CONFLICT) {
						pTask->setStatus(Task::CONFLICT);
					}
					else {
						pTask->setStatus(Task::ERROR);
					}
					updateStatusBar();
				}
			}
		}
	}
    gui->updateTask(pTask);
	setSaveRequired(true);
}

void Controller::tableSortingChanged(void) {
	gui->setErrorCells(data.getErrorTasks());
}

void Controller::checkTableItem(const QModelIndex &index) {
	switch (index.column()) {
	case TASK_TYPE: // task type
	case TASK_NAME:
	case PROJECT_ID:
	case CONTACT_NAME:
	case CONTACT_PHONE:
	case CONTACT_EMAIL:
		break;
/*
	case PREDECESSORS: // predecessor ID
		if (index.model()->data(index).toString().isEmpty()) {
			gui->clearErrorIndex(index); // mark cell as repaired because it is an empty cell
		}
		else {
			if (data.predecessorExists(index.model()->data(index).toUInt())) // if the predecessor exists
				gui->clearErrorIndex(index); // mark cell as repaired
			else
				gui->addErrorIndex(index); // mark cell as error
		}
		break;
*/
	case PRIORITY: // priority
		break;
	case STATION_ID: // station IDs
		if (checkStationList(index.model()->data(index).toString().toStdString())) {
			gui->clearErrorIndex(index);
		} else {
			gui->addErrorIndex(index);
		}
		break;
	case STORAGE_SIZE: // storage
	case ANTENNA_MODE: // antenna mode
	case FILTER_TYPE: // filter type
	case CLOCK_FREQUENCY: // clock frequencies
	case FIXED_DAY: // fixed day and time (1/0)
	case FIXED_TIME:
	case TASK_STATUS: // task status
	case NR_OF_SUBBANDS: // number of subbands (spinbox)
	case NIGHT_TIME_WEIGHT_FACTOR: // night time weight factor (spinbox)
		break;
//	case PRED_MIN_TIME_DIF: // time edit
//	case PRED_MAX_TIME_DIF:
	case TASK_DURATION: // duration
		break;
	case WINDOW_MINIMUM_TIME:
	case WINDOW_MAXIMUM_TIME:
		break;
	case FIRST_POSSIBLE_DATE: // date edit
	case LAST_POSSIBLE_DATE:
		break;
	case PLANNED_START: // date-time edit
	case PLANNED_END:
		break;
	}
}

bool Controller::checkStationList(const std::string & stations) const {
	size_t p = 0, i = 0;
	if (!stations.empty()) {
		while (true) {
			i = stations.find(';', p);
			if (i != std::string::npos) {
				if (!(theSchedulerSettings.stationExist(stations.substr(p, i-p)))) {
					return false;
				}
			}
			else { // extract last station and exit loop
				if (!(theSchedulerSettings.stationExist(stations.substr(p, stations.size()-p)))) {
					return false;
				}
				else return true;
			}
			p = i + 1;
		}
	} else return true; // no error in empty list
	return true;
}

void Controller::storeTaskUndo(unsigned taskID, const QString &undo_action, bool delete_task) {
//	itsUndoType.push_back(std::pair<undo_type, QString>(UNDO_COMPLETE_TASK, undo_action));

	//std::pair<undo_type, std::pair<QString, std::vector<unsigned> > >

	itsUndoType.push_back(undoTypeVector::value_type(UNDO_COMPLETE_TASK,
			std::pair<QString, vector<unsigned> >( undo_action, vector<unsigned>() ) ));


	if (delete_task) {
        itsTaskUndoStack.push_back(taskUndoStack::value_type(taskID, new Task(0)) );
	}
	else {
        itsTaskUndoStack.push_back(taskUndoStack::value_type(taskID, cloneTask(data.getTask(taskID))) );
	}
	gui->addUndo(undo_action);
}

void Controller::storeTaskPropertyUndo(unsigned taskID, data_headers property, const QVariant &value) {
	QString undo_text("Task " + QString::number(taskID) + " " + DATA_HEADERS[property] + " change");
	itsUndoType.push_back(undoTypeVector::value_type(UNDO_TASK_PROPERTY,
			std::pair<QString, vector<unsigned> >( undo_text, vector<unsigned>() ) ));
	itsTaskPropertyUndoStack.push_back(taskPropertyUndoStack::value_type (taskID,
			std::pair<data_headers, QVariant>(property, value)));
	gui->addUndo(undo_text);
	setSaveRequired(true);
}

void Controller::storeScheduleUndo(const QString &undo_action) {
//	itsUndoType.push_back(std::pair<undo_type, QString>(UNDO_SCHEDULER_DATA_BLOCK, undo_action));
	itsUndoType.push_back(undoTypeVector::value_type(UNDO_SCHEDULER_DATA_BLOCK,
			std::pair<QString, vector<unsigned> >
					( undo_action, vector<unsigned>() ) ));
	data.create_undo_level();
	gui->addUndo(undo_action);
}

void Controller::deleteLastStoredUndo(void) {
	if (!itsUndoType.empty()) {
		undo_type ut = itsUndoType.back().first;
		itsUndoType.pop_back();
		if (ut == UNDO_COMPLETE_TASK) {
            delete itsTaskUndoStack.back().second;
			itsTaskUndoStack.pop_back();
		}
		else if (ut == UNDO_SCHEDULER_DATA_BLOCK) {
			data.deleteLastStoredUndo();
		}
		else if (ut == UNDO_TASK_PROPERTY) {
			itsTaskPropertyUndoStack.pop_back();
		}
	}
	gui->removeLastUndo();
}

void Controller::applyTaskPropertyChange(unsigned taskID, data_headers property, const QVariant &value, bool undo_redo) {
	QVariant taskValue;
	Task *pTask = data.getTaskForChange(taskID);

	Task::task_status currentStatus, newStatus;
	switch (property) {
	case TASK_ID:
	case SAS_ID:
		return;
	case TASK_NAME:
		taskValue = QString(pTask->getTaskName());
		pTask->setTaskName(value.toString().toStdString());
		break;
	case TASK_DESCRIPTION:
		taskValue = QString(pTask->SASTree().description().c_str());
		pTask->setTaskDescription(value.toString().toStdString());
		break;
	case PROJECT_ID:
		taskValue = QString(pTask->getProjectName());
		pTask->setProjectName(value.toString().toStdString());
		break;
	case CONTACT_NAME:
		taskValue = QString(pTask->getContactName());
		pTask->setContactName(value.toString().toStdString());
		break;
	case CONTACT_PHONE:
		taskValue = QString(pTask->getContactPhone());
		pTask->setContactPhone(value.toString().toStdString());
		break;
	case CONTACT_EMAIL:
		taskValue = QString(pTask->getContactEmail());
		pTask->setContactEmail(value.toString().toStdString());
		break;
//	case TASK_TYPE:
//		taskValue = QString(pTask->getTypeStr());
//		pTask->setType(value.toString().toStdString());
//		break;
	case PREDECESSORS:
        taskValue = pTask->getPredecessorsString();
		pTask->setPredecessors(value.toString());
		break;
	case PRED_MIN_TIME_DIF:
		taskValue = QString(pTask->getPredecessorMinTimeDif().toString().c_str());
		pTask->setPredecessorMinTimeDif(AstroTime(value.toString()));
		break;
	case PRED_MAX_TIME_DIF:
		taskValue = QString(pTask->getPredecessorMaxTimeDif().toString().c_str());
		pTask->setPredecessorMaxTimeDif(value.toString());
		break;
	case PRIORITY:
		taskValue = pTask->getPriority();
		pTask->setPriority(value.toDouble());
		break;
	case FIRST_POSSIBLE_DATE:
		taskValue = QString(pTask->getWindowFirstDay().toString().c_str());
		pTask->setWindowFirstDay(value.toString());
		break;
	case LAST_POSSIBLE_DATE:
		taskValue = QString(pTask->getWindowLastDay().toString().c_str());
		pTask->setWindowLastDay(value.toString());
		break;
	case WINDOW_MINIMUM_TIME:
		taskValue = QString(pTask->getWindowMinTime().toString().c_str());
		pTask->setWindowMinTime(value.toString());
		break;
	case WINDOW_MAXIMUM_TIME:
		taskValue = QString(pTask->getWindowMaxTime().toString().c_str());
		pTask->setWindowMaxTime(value.toString());
		break;
	case TASK_DURATION:
		taskValue = QString(pTask->getDuration().toString().c_str());
//		pTask->setDuration(value.toString().toStdString());
		data.changeTaskDuration(taskID, value.toString());
//        pTask->storage()->recalculateCheck();
		break;
	case PLANNED_START:
		taskValue = QString(pTask->getScheduledStart().toString().c_str());
//		pTask->setScheduledStart(value.toString().toStdString());
		data.changeTaskStartTime(taskID, value.toString());
		break;
	case PLANNED_END:
		taskValue = QString(pTask->getScheduledEnd().toString().c_str());
//		pTask->setScheduledEnd(value.toString().toStdString());
		data.changeTaskEndTime(taskID, value.toString());
//		pTask->recalculateCheck();
		break;
	case FIXED_DAY:
		taskValue = pTask->getFixedDay();
		pTask->setFixDay(value.toBool());
		break;
	case FIXED_TIME:
		taskValue = pTask->getFixedTime();
		pTask->setFixTime(value.toBool());
		break;
	case TASK_STATUS:
		taskValue = QString(pTask->getStatusStr());
		newStatus = taskStatusFromString(value.toString().toStdString());
		if (pTask->isScheduled()) { // currently scheduled?
			if ( (newStatus < Task::PRESCHEDULED) || (newStatus == Task::OBSOLETE) ) { // not scheduled anymore?
				data.unscheduleTask(pTask->getID());
				pTask->clearAllStorageConflicts();
                if (pTask->hasStorage()) {
                    pTask->storage()->unAssignStorage();
                    pTask->storage()->generateFileList();
                }
				pTask->setStatus(newStatus);
			}
			else if (newStatus == Task::SCHEDULED) {
				if (doScheduleChecks(pTask)) {
					pTask->setStatus(newStatus);
				}
			}
			else {
				pTask->setStatus(newStatus);
			}
		}
		else { // currently task is not scheduled
			if (pTask->getStatus() == Task::ERROR) {
				if (!data.checkTask(pTask)) { // task still has errors?
					// TODO: mark the error cell with gui->addErrorIndex(index)
					QMessageBox::critical(gui, tr("Task has errors"),
							tr("The task still has errors. Cannot change its status."),tr("Close"));
					QApplication::beep();
					return;
				}
			}
			if (newStatus == Task::PRESCHEDULED) {
				if (!pTask->isPipeline()) {
					data.scheduleTask(pTask);
				}
				pTask->setStatus(Task::PRESCHEDULED);
			}
			else if (newStatus == Task::SCHEDULED) {
				if (doScheduleChecks(pTask)) {
					if (!pTask->isPipeline()) {
						data.scheduleTask(pTask);
					}
					else {
						pTask->setStatus(Task::SCHEDULED);
					}
				}
			}
			else if ((newStatus == Task::STARTING) || (newStatus == Task::ACTIVE)) {
				data.scheduleTask(pTask);
				pTask->setStatus(newStatus);
			}
			else {
				pTask->setStatus(newStatus);
			}
		}
		updateStatusBar();
		break;
	case UNSCHEDULED_REASON:
		break;
	case STORAGE_SIZE:
		break;
	case _END_DATA_HEADER_ENUM_:
		break;
	default:
		break;
	}

    if (pTask->isStationTask()) {
        StationTask *pStationTask = static_cast<StationTask *>(pTask); // no expensive dynamic_cast needed because we know it is a StationTask

        switch (property) {
        case FILTER_TYPE:
            taskValue = QString(pStationTask->getFilterTypeStr());
            pStationTask->setFilterType(value.toString().toStdString());
            break;
        case ANTENNA_MODE:
            taskValue = QString(pStationTask->getAntennaModeStr());
            pStationTask->setAntennaMode(value.toString().toStdString());
            break;
        case STATION_ID:
            taskValue = QString(pStationTask->getStationNamesStr().c_str());
            currentStatus = pStationTask->getStatus();
            if (pStationTask->isScheduled()) {
                data.unscheduleTask(taskID);
                pStationTask->setStations(value.toString(),';');
                pStationTask->clearAllStorageConflicts();
                if (pStationTask->hasStorage()) {
                    pStationTask->storage()->unAssignStorage();
                    pStationTask->storage()->generateFileList();
                }
                data.scheduleTask(pStationTask);
                pStationTask->setStatus(currentStatus);
            }
            else {
                pStationTask->setStations(value.toString(),';');
            }
//            pStationTask->recalculateCheck();
            break;
        case CLOCK_FREQUENCY:
            taskValue = QString(pStationTask->getStationClockStr());
            pStationTask->setStationClock(value.toString().toStdString());
//            pStationTask->recalculateCheck();
            break;
        default:
            break;
        }

        if (pStationTask->isObservation()) {
            Observation *pObs = static_cast<Observation *>(pStationTask);
            switch (property) {
            case NIGHT_TIME_WEIGHT_FACTOR:
                taskValue = pObs->getNightTimeWeightFactor();
                pObs->setNightTimeWeightFactor(value.toUInt());
                break;
            default:
                break;
            }
        }

    }

	if (undo_redo) {
		itsTaskPropertyRedoStack.push_back(taskPropertyUndoStack::value_type (pTask->getID(),
				std::pair<data_headers, QVariant>(property, taskValue)));
	}
	else {
		itsTaskPropertyUndoStack.push_back(taskPropertyUndoStack::value_type (pTask->getID(),
				std::pair<data_headers, QVariant>(property, taskValue)));
	}
	setSaveRequired(true);
}

void Controller::undo(bool store_redo) {
	if (!itsUndoType.empty()) {
		QApplication::setOverrideCursor(Qt::WaitCursor);
		undoTypeVector::value_type undo_data = itsUndoType.back();
		itsUndoType.pop_back();
		if (store_redo) {
			itsRedoType.push_back(undo_data);
		}
		if (undo_data.first == UNDO_TASK_PROPERTY) {
			taskPropertyUndoStack::value_type val = itsTaskPropertyUndoStack.back();
			itsTaskPropertyUndoStack.pop_back();
			applyTaskPropertyChange(val.first, val.second.first, val.second.second, store_redo);
            gui->updateTableTask(data.getTask(val.first));
			gui->updateGraphicTask(val.first);
			//set status string
			gui->setStatusText("Task " + QString::number(val.first) + DATA_HEADERS[val.second.first] + " change undone");
		}
		else if (undo_data.first == UNDO_COMPLETE_TASK) {
			taskUndoStack::value_type val = itsTaskUndoStack.back(); // val contains the task as it was before the action that has to be undone was applied
			itsTaskUndoStack.pop_back(); // remove task from undo stack
            if (val.second->getID()) { // if task is non-zero then this is not a delete action
				Task *task = data.getTaskForChange(val.first);  // contains current task properties that will be undone
				if (task) { // task could have been deleted before this undo step in which case we would have a null pointer exception
					if (store_redo) {
                        itsTaskRedoStack.push_back(taskUndoStack::value_type (val.first, cloneTask(task))); // stores current properties in redo stack
					}
					Task::task_status current_state = task->getStatus(); // the current state
                    Task::task_status prev_state = val.second->getStatus(); // the state before the change was applied that is now being undone
                    task->clone(val.second); // copy the previous task values to the existing task
					if (!task->isPipeline()) {
						if ( (prev_state >= Task::FINISHED) && (current_state >= Task::PRESCHEDULED) && (current_state < Task::FINISHED)) { // go from a scheduled to an inactive state
							data.unscheduleTask(task->getID());
							task->setStatus(prev_state);
							data.moveTaskToInactive(task->getID());
						}
						else if ((prev_state >= Task::PRESCHEDULED) && (prev_state < Task::FINISHED) && (current_state < Task::PRESCHEDULED)) { // go from an unscheduled to a scheduled state
							data.scheduleTask(task);
							task->setStatus(prev_state);
						}
						else if ((prev_state < Task::PRESCHEDULED) && (current_state >= Task::PRESCHEDULED) && (current_state < Task::FINISHED)) { // go from a scheduled to an unscheduled state
							data.unscheduleTask(task->getID());
							task->setStatus(prev_state);
						}
					}
					updateStatusBar();
                    gui->updateTask(task);
				}
				else {
					gui->removeLastUndo();
					gui->setStatusText("Changes to task " + QString::number(val.first) + " could not be undone because it was destroyed");
                    delete val.second; // release memory of task that came from undo stack
                    return;
				}
			}
			else { // when task ID is zero it means delete task (it was added to undo stack by the add task function)
				Task *task = data.getTaskForChange(val.first);
				if (store_redo) {
                    itsTaskRedoStack.push_back(taskUndoStack::value_type (0, cloneTask(task)));
				}
				gui->deleteTaskFromGUI(val.first);
                data.deleteTask(val.first);
				updateStatusBar();
			}
            delete val.second; // release memory from undo task
			//set status string
			gui->setStatusText("Task " + QString::number(val.first) + " change undone");
		}
		else if (undo_data.first == UNDO_SCHEDULER_DATA_BLOCK) {
			if (data.undo(store_redo)) { // schedule undo
				data.checkTasksForErrors();
				gui->writeTableData(data);
				updateGraphicTasks();
				gui->selectTasks(itsSelectedTasks); // re-select the selected tasks
				updateStatusBar();
				gui->setStatusText("Schedule changes undone");
			}
			else {
				gui->setStatusText("No more schedule changes to undo");
				debugWarn("s", "No more data undo levels");
			}
		}
		else if (undo_data.first == UNDO_DELETE_TASKS) { // undo deletion of tasks
			if (!itsDeletedTasks.empty()) {
                vector<Task *> &deletedTasks = itsDeletedTasks.back();
				unsigned task_id;
				vector<unsigned> taskIDvec;
                Task::task_status status;
                for (vector<Task *>::iterator it = deletedTasks.begin(); it != deletedTasks.end(); ++it) {
                    task_id = (*it)->getID();
					taskIDvec.push_back(task_id);
                    status = (*it)->getStatus();
                    data.addTask(*it, false);
                    if ((*it)->isScheduled()) {
                        data.scheduleTask(*it);
                        (*it)->setStatus(status);
                    }
                    else if (status >= Task::FINISHED) {
                        data.moveTaskToInactive(task_id);
                    }
                    gui->addTask(*it);
                    itsSASConnection->removeFromSASTaskToDelete((*it)->getSASTreeID());
				}
				itsThrashBin.removeRestoredTasks(taskIDvec);
                itsDeletedTasks.pop_back();
				if (store_redo) {
					itsDeletedTasksRedoStack.push_back(taskIDvec);
				}
				updateStatusBar();
			}
		}
		gui->undo(store_redo);
	}
	if (itsUndoType.empty()) {
		setSaveRequired(false);
	}
	QApplication::restoreOverrideCursor();
}

void Controller::redo(void) {
	if (!itsRedoType.empty()) {
		QApplication::setOverrideCursor(Qt::WaitCursor);
		undoTypeVector::value_type redo_data = itsRedoType.back();
		itsRedoType.pop_back();
		itsUndoType.push_back(redo_data);
		if (redo_data.first == UNDO_TASK_PROPERTY) {
			taskPropertyUndoStack::value_type val = itsTaskPropertyRedoStack.back();
			itsTaskPropertyRedoStack.pop_back();
			applyTaskPropertyChange(val.first, val.second.first, val.second.second, STORE_UNDO);
            gui->updateTableTask(data.getTask(val.first));
			gui->updateGraphicTask(val.first);
			//set status string
			gui->setStatusText("Task " + QString::number(val.first) + DATA_HEADERS[val.second.first] + " change redone");
		}
		else if (redo_data.first == UNDO_COMPLETE_TASK) {
			taskUndoStack::value_type val = itsTaskRedoStack.back();
			itsTaskRedoStack.pop_back();
			if (val.first) { // when task ID is not zero the task exist
				Task *task = data.getTaskForChange(val.first);
				if (task) {
                    itsTaskUndoStack.push_back(taskUndoStack::value_type (val.first, cloneTask(task)));
					Task::task_status current_state = task->getStatus(); // the current state of the task
                    Task::task_status new_state = val.second->getStatus(); // the state before the undo was applied
                    task->clone(val.second); // copy the previous ('undone') task values to the existing task
					if (!task->isPipeline()) {
						if ((new_state >= Task::FINISHED) && (current_state >= Task::PRESCHEDULED)  && (current_state < Task::FINISHED)) { // go from scheduled to an inactive state
							data.unscheduleTask(task->getID());
							task->setStatus(new_state);
							data.moveTaskToInactive(task->getID());
						}
						else if ((new_state >= Task::PRESCHEDULED) && (new_state < Task::FINISHED) && (current_state < Task::PRESCHEDULED)) { // go from an unscheduled to a scheduled state
							data.scheduleTask(task);
							task->setStatus(current_state);
						}
						else if ((new_state < Task::PRESCHEDULED) && (current_state >= Task::PRESCHEDULED) && (current_state < Task::FINISHED)) { // go from a scheduled to an unscheduled state
							data.unscheduleTask(task->getID());
							task->setStatus(new_state);
						}
						else if ((new_state >= Task::PRESCHEDULED) && (new_state < Task::FINISHED) && (current_state >= Task::FINISHED)) { // go from an inactive to a scheduled state
							data.scheduleTask(task); // we need to reschedule the task because the state changed from an inactive to active
							task->setStatus(new_state);
						}
					}
					updateStatusBar();
                    gui->updateTask(task);
				}
                delete val.second; // release memory of undo task
			}
			else { // special case: add the previously deleted task
                Task *pTask = val.second;
                if (!data.addTask(pTask)) {
                    Task::task_status status(pTask->getStatus());
                    if (!pTask->isPipeline() && pTask->isScheduled()) { // we need to schedule the task again if it was scheduled before it got deleted
                        data.scheduleTask(pTask);
                        pTask->setStatus(status);
                    }
                    itsTaskUndoStack.push_back(taskUndoStack::value_type (pTask->getID(), new Task(0)));
                    gui->addTask(pTask);
                }
                else {
                    qDebug() << "Controller::redo : task cannot be re-added : ID:" << pTask->getID();
                }
            }
			//set status string
			gui->setStatusText("Task " + QString::number(val.first) + " change redone");
			updateStatusBar();
		}
		else if (redo_data.first == UNDO_SCHEDULER_DATA_BLOCK) {
			if (data.redo()) { // schedule undo
				data.checkTasksForErrors();
				gui->writeTableData(data);
				updateGraphicTasks();
				gui->selectTasks(itsSelectedTasks); // re-select the selected tasks
				updateStatusBar();
				gui->setStatusText("Schedule changes redone");
			}
			else {
				gui->setStatusText("No more schedule changes to redo");
				debugWarn("s", "No more data redo levels");
			}
		}
        else if (redo_data.first == UNDO_DELETE_TASKS) { // redo deletion of tasks
			if (!itsDeletedTasksRedoStack.empty()) {
                vector<Task *> deletedTasks;
				vector<unsigned> &taskIDvec = itsDeletedTasksRedoStack.back();
                gui->scene()->blockSignals(true);
                unsigned treeID;
                for (vector<unsigned>::iterator it = taskIDvec.begin(); it != taskIDvec.end(); ++it) {
                    Task *delTask(data.deleteTask(*it, ID_SCHEDULER, false));
                    if (delTask) {
                        deletedTasks.push_back(delTask);
                        treeID = delTask->getSASTreeID();
                        if (treeID) {
                            itsSASConnection->addToTreesToDelete(treeID, *it);
                        }
                        gui->deleteTaskFromGUI(*it); // will cause signals to be generated if we don't block them (see remark just before FOR loop)
                    }
					else {
						debugWarn("si","Redo delete: could not find task: ", *it); // should never occur
					}
				}
                gui->scene()->blockSignals(false);
                if (!deletedTasks.empty()) {
                    itsDeletedTasks.push_back(deletedTasks);
                    itsThrashBin.addTasks(deletedTasks);
                    gui->setFullThrashIcon();
                }
				itsDeletedTasksRedoStack.pop_back();
				updateStatusBar();
			}
		}
		gui->redo();
		setSaveRequired(true);
	}
	QApplication::restoreOverrideCursor();
}

void Controller::quit()
{
	itsSASConnection->disconnect();
	if (possiblySave())
	{
		itsDataHandler->saveProgramPreferences();
		application->quit();
	}
}

bool Controller::checkSettings() const {
	return true;
}

// Dialogbox asking for saving of schedule project
int Controller::possiblySaveDialog()
{

    // Save the pointer to the message box to allow sending signals from
    // outside threads (testing)
    possiblySaveMessageBox = new QMessageBox();
    QMessageBox *possiblySaveMessageBox = new QMessageBox();
    possiblySaveMessageBox->setWindowTitle(      tr("Save schedule project") );
    possiblySaveMessageBox->setText(             tr("Schedule project contains unsaved changes\nDo you want to save the schedule project?") );
    possiblySaveMessageBox->setStandardButtons(  QMessageBox::Save | QMessageBox::No | QMessageBox::Cancel);
    possiblySaveMessageBox->setDefaultButton(    QMessageBox::Save);
    possiblySaveMessageBox->setEscapeButton(     QMessageBox::Cancel);

    // execute and wait
   return possiblySaveMessageBox->exec();
}

// check if data needs to be saved before cleaning up data
// returns true if data saving went OK or when no data needs to be saved.
// returns false if user decided to cancel the save or something else went wrong
// with saving the data.
// TODO: The return value of this function mixes error state and selected choices.
bool Controller::possiblySave()
{
    // check in SchedulingData if save required
    if (!data.getSaveRequired() )
        return true;

    // ask user if he wants to save project file
    int answer = possiblySaveDialog();

    // If user answered no or cancel we can exit with true exit state ( no error
    if (answer == QMessageBox::No)
        return true;

    if (answer == QMessageBox::Cancel)
        return false;

    // If save is selected attempt save If this fails return false
    if (answer == QMessageBox::Save && saveSchedule())
        return true;
    else// default return is false
        return false;
}


void Controller::cleanup(bool keepUndo, bool cleanSasConnection) {
	itsSelectedTasks.clear();
	gui->clearTasks();
	if (cleanSasConnection) {
		itsSASConnection->cleanup();
	}
	data.cleanup();
	if (!keepUndo) {
		clearUndoRedoStack();
	}
//	gui->updateGraphicStations();
	gui->cleanup();
	gui->newTable(data);
	itsThrashBin.emptyThrashBin();
	gui->setEmptyThrashIcon();
	setSaveRequired(false);
}

void Controller::clearUndoRedoStack(void) {
	itsTaskPropertyUndoStack.clear();
	itsTaskPropertyRedoStack.clear();
    for (taskUndoStack::iterator uit = itsTaskUndoStack.begin(); uit != itsTaskUndoStack.end(); ++uit) delete uit->second;
	itsTaskUndoStack.clear();
    for (taskUndoStack::iterator rit = itsTaskRedoStack.begin(); rit != itsTaskRedoStack.end(); ++rit) delete rit->second;
    itsTaskRedoStack.clear();
	itsUndoType.clear();
	itsRedoType.clear();
	gui->clearUndoRedo();
}

void Controller::clearRedoStack(void) {
	itsTaskPropertyRedoStack.clear();
    for (taskUndoStack::iterator rit = itsTaskRedoStack.begin(); rit != itsTaskRedoStack.end(); ++rit) delete rit->second;
    itsTaskRedoStack.clear();
    itsRedoType.clear();
	gui->clearRedo();
}

/*
void Controller::openTaskList(void)
{
	if (possiblySave()) {
		QString filename = gui->openTaskFileDialog();
		if (!filename.isEmpty()) {
			cleanup();
			if(itsDataHandler->readCSVFile(filename, data))
			{
				scheduler.setData(data);
				gui->writeTableData(data);
				gui->setCurrentFileName(filename);
				gui->setEnableTaskListMenuItems(true);
			}
		}
	}
}
*/

void Controller::saveTaskListAs(void) {
	bool saveAll(true);
	std::vector<const Task *> tasks;
	std::vector<unsigned> selectedTaskIDs(gui->getSelectedRowsTaskIDs());
	if (!selectedTaskIDs.empty()) {
		QMessageBox msgBox;
		msgBox.setText("Do you want to export all tasks or only the selection?");
		QPushButton *selectionButton = msgBox.addButton("Selection", QMessageBox::NoRole);
		QPushButton *allButton = msgBox.addButton("All", QMessageBox::YesRole);
		msgBox.setDefaultButton(allButton);
		msgBox.setIcon(QMessageBox::Question);
		msgBox.exec();
		if (msgBox.clickedButton() == selectionButton) {
			saveAll = false;
			for (std::vector<unsigned>::const_iterator it = selectedTaskIDs.begin(); it != selectedTaskIDs.end(); ++it) {
				tasks.push_back(data.getTask(*it));
			}
		}
	}
	if (saveAll) {
		std::vector<unsigned> taskIDs(gui->getShownTaskIDs());
		for (std::vector<unsigned>::const_iterator it = taskIDs.begin(); it != taskIDs.end(); ++it) {
			tasks.push_back(data.getTask(*it));
		}
	}
	QString filename = gui->saveTaskFileDialog();
	if (!filename.isEmpty()) {
		itsDataHandler->writeCSVFile(filename, tasks);
	}
}

bool Controller::saveScheduleAs(void) {
	QString filename = gui->saveProjectDialog();
	if (!filename.isEmpty()) {
		return saveSchedule(filename);
	}
	else return false;
}

bool Controller::saveSchedule(QString filename) {
	if (filename.isEmpty()) {
		filename = itsDataHandler->getFileName();
	}
	if (filename.isEmpty()) {
		QString filename = gui->saveProjectDialog();
		if (!filename.isEmpty()) {
			bool result = itsDataHandler->saveSchedule(filename, data);
			if (result) {
				//set status string
				QString statStr = "Schedule saved to " + filename;
				gui->setStatusText(statStr);
				gui->setCurrentFileName(filename);
				setSaveRequired(false);
				return true;
			}
			return result;
		}
		else return false;
	}
	else {
		if (itsDataHandler->saveSchedule(filename, data)) {
			QString statStr = "Schedule saved to " + filename;
			gui->setStatusText(statStr);
			gui->setCurrentFileName(filename);
			setSaveRequired(false);
			return true;
		}
		else return false;
	}
}

void Controller::openSchedule(void) {
	if (possiblySave()) {
		QString filename = gui->openProjectDialog();
		if (!filename.isEmpty()) {
			QApplication::setOverrideCursor(Qt::WaitCursor);
			cleanup();
			if(itsDataHandler->openSchedule(filename, data)) {
				scheduler.setData(data);
				gui->newTable(data);
                gui->updateProjectsFilter(Controller::theSchedulerSettings.getCampaignList());
				gui->setCurrentFileName(filename);
				gui->updateSceneTimeSpan();
				gui->updateTaskDialogStations();
				gui->updateGraphicStations();
				updateGraphicTasks();
				gui->setEnableScheduleMenuItems(true);
				gui->initPublishDialog(); // initialize the publish dialog with the correct week numbers
				//set status string
				QString statStr = "Schedule loaded from " + filename;
				gui->setStatusText(statStr);
				updateStatusBar();
			}
			QApplication::restoreOverrideCursor();
		}
	}
}

bool Controller::closeSchedule(void) {
	if (possiblySave()) {
		cleanup();
		gui->clearCurrentFileName();
		gui->setEnableScheduleMenuItems(false);
		updateStatusBar();
		gui->setStatusText("Ready");
		return true;
	}
	else return false;
}

void Controller::openSettingsDialog(void) {
	itsSettingsDialog->show();
}

void Controller::closeSettingsDialog() {
	if (checkSettings()) {
		theSchedulerSettings.updateSettings(itsSettingsDialog);
		scheduler.updateSettings();
		itsSettingsDialog->close();
        itsSASConnection->setAutoPublishEnabled(theSchedulerSettings.getAutoPublish());
		updateGUIafterSettingsLoad();
		emit schedulerSettingsChanged();
	}
}

bool Controller::connectToDataMonitor(void) {
	bool bResult(true);
	if (!itsDMConnection->isOpen()) {
		QMessageBox mb(QMessageBox::Information, "Please wait for connection to data monitor",
				"Trying to connect to the data monitor.\n Please wait.");
		mb.show();
		QApplication::setOverrideCursor(Qt::WaitCursor);
		QCoreApplication::processEvents(); // force update to paint dialog

		if (itsDMConnection->init(theSchedulerSettings.getDMUserName(),
					theSchedulerSettings.getDMPassword(),
					theSchedulerSettings.getDMDatabase(),
					theSchedulerSettings.getDMHostName())) {
			if (itsDMConnection->connect()) {
				itsDMConnection->updateStorageNodes();
				itsSettingsDialog->updateStorageNodeInfoTree(itsDMConnection->getStorageNodes(), itsDMConnection->getStates(), itsDMConnection->getHostPartitions());
				mb.hide();
			}
			else {
				bResult = false;
			}
		}
		else {
			bResult = false;
		}
		QApplication::restoreOverrideCursor();
	}
	else {
		bResult = false;
	}

	gui->setDataMonitorConnectionButton(bResult);
	return bResult;
}

void Controller::disconnectDataMonitor(void) {
	itsDMConnection->disconnect();
	gui->setDataMonitorConnectionButton(false);
}

bool Controller::isDataMonitorConnected(void) const {
	bool isConnected(itsDMConnection->isOpen());
	gui->setDataMonitorConnectionButton(isConnected);
	return isConnected;
}

int Controller::refreshStorageNodesInfo(bool doConnectToDM) {
	if (doConnectToDM) {
		if (itsDMConnection->isOpen()) {
			//		if (connectToDataMonitor()) {
			if (itsDMConnection->updateStorageNodes()) {
				// update the storage node information used for scheduling (wich is stored in the datablock)
				// update the storage node information in the scheduler settings information used at startup of scheduler to fill the settingsdialog with last known storage nodes
				data.initStorage(); // also clears all task claims on the storage nodes
				// show the updated storage node information in the settings dialog
				itsSettingsDialog->updateStorageNodeInfoTree(itsDMConnection->getStorageNodes(), itsDMConnection->getStates(), itsDMConnection->getHostPartitions());
				return 0; // OK
			}
			else return 1; // not connected to data monitor
		}
        else { // not yet connected to the data monitor
            if (connectToDataMonitor()) {
                if (itsDMConnection->updateStorageNodes()) {
                    // update the storage node information used for scheduling (wich is stored in the datablock)
                    // update the storage node information in the scheduler settings information used at startup of scheduler to fill the settingsdialog with last known storage nodes
                    data.initStorage(); // also clears all task claims on the storage nodes
                    // show the updated storage node information in the settings dialog
                    itsSettingsDialog->updateStorageNodeInfoTree(itsDMConnection->getStorageNodes(), itsDMConnection->getStates(), itsDMConnection->getHostPartitions());
                    return 0; // OK
                }
                else return 1; // not connected to data monitor
            }
            else {
                QMessageBox msgBox;
                msgBox.setText("Could not connect to the data monitor.");
                msgBox.setInformativeText("Do you want to continue?");
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::Yes);
                int ret = msgBox.exec();
                if (ret == QMessageBox::Yes) {
                    data.initStorage(); // also clears all task claims on the storage nodes
                    itsSettingsDialog->updateStorageNodeInfoTree(itsDMConnection->getStorageNodes(), itsDMConnection->getStates(), itsDMConnection->getHostPartitions());
                    return 0; // OK, user didn't want to connect to data monitor but does want to continue
                }
                else {
                    itsSettingsDialog->stopStorageWaitCursor();
                    return 1; // not connected to data monitor
                }
            }
        }
    }
	else {
		data.initStorage(); // also clears all task claims on the storage nodes
		itsSettingsDialog->updateStorageNodeInfoTree(itsDMConnection->getStorageNodes(), itsDMConnection->getStates(), itsDMConnection->getHostPartitions());
		return 0;
	}
}

void Controller::createInitialSchedule(void)
{
	if (data.getNrTasks()) {
		gui->setStatusText("Creating Initial schedule");
		data.checkTasksForErrors();
		gui->writeTableData(data);
		const errorTasksMap &errorTasks = data.getErrorTasks();
		if (!errorTasks.empty()) {
			gui->setErrorCells(errorTasks);
			QMessageBox mb("Error(s) in task(s)",
					"The marked task properties have errors!\nDo you want to continue?\n Tasks with errors will get error status and will not be scheduled",
					QMessageBox::Warning,
					QMessageBox::Yes,
					QMessageBox::No | QMessageBox::Escape | QMessageBox::Default,
					QMessageBox::NoButton);
			const QRect &gui_pos = gui->geometry();
			mb.setGeometry(gui_pos.x()+gui_pos.width()/3, gui_pos.y()+gui_pos.height()/3, mb.width(), mb.height());
			int answer = mb.exec();
			if (answer == QMessageBox::Yes) {
				data.markErrorTasksStatus(); // changes the error tasks status to ERROR
			}
			else {
				return;
			}
		}// no new (unmarked) error tasks were found, it's save to continue
		storeScheduleUndo(QObject::tr("Create initial schedule"));
		scheduler.createStartSchedule();
		setSaveRequired(true);
		gui->setEnableScheduleMenuItems(true);

		// update views
		gui->writeTableData(data);
		updateGraphicTasks();
		gui->setStatusText("Initial schedule created");
		updateStatusBar();
	}
	else {
		QMessageBox::warning(gui, tr("No tasks defined"),
				tr("There are no tasks defined. Cannot create a start schedule."),tr("Close"));
	}
}

void Controller::start()
{
	// create scheduling data storage
	gui->show(); // shows the GUI
}

void Controller::fixTaskErrors() {
	errorTasksMap tasks = data.getErrorTasks();
	gui->setErrorCells(tasks);
	if (!tasks.empty()) {

		QMessageBox mb("Error(s) in task(s)",
				"The marked task properties have errors!\nDo you want to Continue?\n Tasks with errors will get error status and skipped from further calculation",
				QMessageBox::Warning,
				QMessageBox::Yes,
				QMessageBox::No | QMessageBox::Escape | QMessageBox::Default,
				QMessageBox::NoButton);
		const QRect &gui_pos = gui->geometry();
		mb.setGeometry(gui_pos.x()+gui_pos.width()/3, gui_pos.y()+gui_pos.height()/3, mb.width(), mb.height());
		int answer = mb.exec();
		if (answer == QMessageBox::Yes) {
		}
	}
}

/*
void Controller::tryRescheduleTask(unsigned task_id, AstroDateTime start_time) {
	// check if multiple tasks are selected, if so try to 'shift' them all
	if (multipleSelected())  {
		const Task *pTask(data.getTask(task_id));
		AstroDateTime new_start;
		AstroTime dif = pTask->getScheduledStart().timeDifference(start_time);
		bool negative = start_time < pTask->getScheduledStart() ? true : false;
		storeScheduleUndo("Reschedule of multiple tasks");
		bool save_required(true);
		for (std::vector<unsigned>::const_iterator it = itsSelectedTasks.begin(); it != itsSelectedTasks.end(); ++it) {
			const Task *task(data.getTask(*it));
			if (negative) {
				new_start = task->getScheduledStart() - dif;
			}
			else {
				new_start = task->getScheduledStart() + dif;
			}
			if (!scheduler.tryRescheduleTask(*it, new_start)) {
				undo();
				deleteLastStoredUndo();
				save_required = false;
				break;
			}
		}
		setSaveRequired(save_required);
	}
	else {
		// TODO: check if the move of the task is not beyond the predecessor limits (if the task has predecessors)
		storeTaskUndo(task_id, QString("Reschedule task ") + QString::number(task_id));
		if (scheduler.tryRescheduleTask(task_id, start_time)) {
			setSaveRequired(true);
		}
		else {
			deleteLastStoredUndo();
		}
	}
	for (std::vector<unsigned>::const_iterator it = itsSelectedTasks.begin(); it != itsSelectedTasks.end(); ++it) {
		gui->updateTask(*it,Task::OBSERVATION);
	}
}
*/

void Controller::rescheduleTask(unsigned task_id, AstroDateTime new_start) {
	const Task *pTask = data.getTask(task_id);
	if ( (new_start >= pTask->getFirstPossibleDateTime()) & (new_start <= pTask->getLastPossibleDateTime()) ) {
		storeTaskUndo(task_id, QString("Reschedule aborted task ") + QString::number(task_id));
		if (scheduler.rescheduleAbortedTask(task_id, new_start)) {
			setSaveRequired(true);
            gui->updateTask(pTask);
			updateStatusBar();
		}
		else {
			deleteLastStoredUndo();
		}
	}
	else {
		QMessageBox::warning(0, tr("Cannot reschedule task"),
				tr("Could not reschedule the task.\nThe task's scheduling window (first possible date, last possible date) does not allow the task to be scheduled in the future."));
		QApplication::beep();
	}
}

void Controller::rescheduleAbortedTask(unsigned task_id, AstroDateTime new_start) {
	if (new_start.isSet()) {
		rescheduleTask(task_id,new_start);
	}
	else { // reschedule the task at now
		const AstroDateTime &timenow(now());
		if ((timenow >= Controller::theSchedulerSettings.getEarliestSchedulingDay()) | (timenow <= Controller::theSchedulerSettings.getLatestSchedulingDay())) {
			rescheduleTask(task_id,timenow + Controller::theSchedulerSettings.getMinimumTimeBetweenTasks());
		}
		else {
			QMessageBox::warning(0, tr("Cannot reschedule task"),
					tr("The current time is outside of the defined scheduling range.\nTasks can only be scheduled in the future."));
			QApplication::beep();
		}
	}
}

void Controller::showTaskDialog(unsigned taskID, tabIndex tab) const {
	const Task *pTask = data.getTask(taskID);
	if (pTask) {
		gui->showTaskDialog(pTask, tab);
	}
	else {
		QMessageBox::critical(gui,QObject::tr("Could not find task"), QObject::tr("Could not find task:") + QString::number(taskID));
	}
}

void Controller::showTaskStateHistory(unsigned taskID) {
	const Task *pTask = data.getTask(taskID);
	if (pTask) {
		int treeID(pTask->getSASTreeID());
		if (treeID != 0) {
			itsSASConnection->showTaskStateHistory(treeID);
		}
		else {
			QMessageBox::warning(0,QObject::tr("No history for new task"), QObject::tr("This seems to be a new task which is not in the SAS database yet"));
		}
	}
}

void Controller::openSASTreeViewer(int treeID) const {
    QString parsetTree(itsSASConnection->getTreeParset(treeID));
	OTDBtree otdb_tree(itsSASConnection->getTreeInfo(treeID));
	if (!parsetTree.isEmpty()) {
		gui->parsetTreeView(parsetTree, otdb_tree);
	}
	else {
		QMessageBox::warning(0, tr("Empty tree parset returned"),
				tr("SAS could not create a tree parset for tree ") + QString::number(treeID));
		QApplication::beep();
	}
}

void Controller::openMetaDataViewer(int treeID) const {
    QString metaData(itsSASConnection->getMetaData(treeID));
    if (!metaData.isEmpty()) {
        gui->parsetTreeView(metaData);
    }
    else {
        QMessageBox::warning(0, tr("No meta-data available"),
                tr("No meta-data is available for tree ") + QString::number(treeID));
        QApplication::beep();
    }
}


bool Controller::checkPredecessorsExistence(const IDvector &predecessors) const {
	for (IDvector::const_iterator prit = predecessors.begin(); prit != predecessors.end(); ++prit) {
		if (getTask(prit->second, prit->first) == 0) {
			return false;
		}
	}
	return true;
}

bool Controller::updatePipelineTask(Task *task, bool createUndo) {
    unsigned task_id = task->getID();

    Task *pTask = data.getPipelineForChange(task_id); // a pointer to the task to change
	if (pTask) {

		if (createUndo) {
			storeTaskUndo(task_id, QString("Changes to pipeline task ") + QString::number(task_id));
		}

		// predecessor existence checks
        if (task->hasPredecessors()) {
            if (!checkPredecessorsExistence(task->getPredecessors())) {
				if (QMessageBox::question(gui->taskDialog(), tr("Task has non existing predecessors"),
						"The task has non-existing predecessors\nIf you continue the task status will be set to ERROR.\nDo you want to continue?",
						QMessageBox::Yes,
						QMessageBox::No) == QMessageBox::No) {
					if (createUndo) deleteLastStoredUndo();
					return false;
				}
                else {
                    pTask->clone(task);
                    pTask->calculateDataFiles();
					pTask->setStatus(Task::ERROR);
					pTask->setReason(unscheduled_reason_str[PREDECESSOR_NOT_FOUND]);
                    gui->updateTask(pTask);
					gui->taskDialog()->updateStatus(Task::ERROR);
					setSaveRequired(true);
					return true;
				}
			}
		}

        Task::task_status prevState(pTask->getStatus()), new_status(task->getStatus());

		if (new_status <= Task::PRESCHEDULED) {

			// now check what needs to be changed and then apply the change
			if (new_status == Task::PRESCHEDULED) { // task is just changed to a scheduled state
                task->clearReason();
                std::pair<unscheduled_reasons, QString> errCode = doPreScheduleChecks(task);
				if (errCode.first == BEAM_DURATION_DIFFERENT) {
					if (QMessageBox::question(gui->taskDialog(), tr("Beam duration different from observation duration"),
							errCode.second.replace('$','\n') + tr("\nDo you want to continue?"),
							QMessageBox::Yes,
							QMessageBox::No) == QMessageBox::No) {
						if (createUndo)	deleteLastStoredUndo();
						return false;
					}
				}
				else if (errCode.first != 0) { // the task has an error
					if (QMessageBox::question(gui->taskDialog(), tr("Task has errors"),
							errCode.second.replace('$','\n') + tr("\nIf you continue the task status will be set to ERROR.\nDo you want to continue?"),
							QMessageBox::Yes,
							QMessageBox::No) == QMessageBox::No) {
						if (createUndo)	deleteLastStoredUndo();
						return false;
					}
					else {
                        pTask->clone(task);
						if ((prevState >= Task::PRESCHEDULED) && (prevState <= Task::ACTIVE)) {
							data.unscheduleTask(task_id);
							pTask->clearAllStorageConflicts();
                            pTask->storage()->unAssignStorage();
                            pTask->storage()->generateFileList();
						}
						pTask->setStatus(Task::ERROR);
						gui->taskDialog()->updateStatus(Task::ERROR);
						setSaveRequired(true);
						return true;
					}
				}
                pTask->clone(task);
                pTask->calculateDataFiles();
				data.scheduleTask(pTask);
				pTask->setStatus(Task::PRESCHEDULED);
			}
			else if (new_status == Task::SCHEDULED) {
                task->clearReason();
                pTask->clone(task);
                pTask->calculateDataFiles();
				if (doScheduleChecks(pTask)) {
					data.scheduleTask(pTask);
				}
				else {
					pTask->setStatus(prevState);
				}
			}
			else if (prevState == Task::ERROR) { // previously task had error, currently it is not (pre)scheduled
                pTask->clone(task);
                data.checkTask(pTask);
			}
			else {
                pTask->clone(task);
            }

			// upload the ON_HOLD status directly to SAS. If not possible (e.g. no connection) issue warning
			if ((new_status == Task::ON_HOLD) && (prevState != Task::ON_HOLD)) {
				setTaskOnHold(task_id, false);
			}

            gui->updateTask(pTask);
			setSaveRequired(true);
			return true;
		}
		else return false; // may not be edited if status is above SCHEDULED
	}
	else {
		debugErr("sis", "Controller::updatePipelineTask: task:", task_id, " not found");
		return false;
	}
}

// updateTask returns true if the task was updated and false otherwise
bool Controller::updateTask(Task *task, bool createUndo) { // task contains the new attributes of the task that need to be changed
    unsigned task_id = task->getID();
	Task *pTask = data.getTaskForChange(task_id); // a pointer to the task to change
	if (pTask) {
        Task::task_status prev_status(pTask->getStatus()), new_status(task->getStatus());
		// check status of task. if status <= SCHEDULED then the task may be edited otherwise don't allow changes
		if (prev_status <= Task::SCHEDULED) {
			// create an undo level for the changes
            if (new_status == Task::SCHEDULED && !doScheduleChecks(task)) {
				return false;
			}
			if (createUndo) {
				storeTaskUndo(task_id, QString("Changes to task ") + QString::number(task_id));
			}

			// check if (PRE)SCHEDULE checks need to be done
			if ((new_status == Task::PRESCHEDULED) || (new_status == Task::SCHEDULED)) {
				// new status is one of the scheduled states?
                std::pair<unscheduled_reasons, QString> errCode = doPreScheduleChecks(task);
				if (errCode.first == BEAM_DURATION_DIFFERENT) {
					if (QMessageBox::question(gui->taskDialog(), tr("Beam duration different from observation duration"),
							errCode.second.replace('$','\n') + tr("\nDo you want to continue?"),
							QMessageBox::Yes,
							QMessageBox::No) == QMessageBox::No) {
						if (createUndo)	deleteLastStoredUndo();
						return false;
					}
				}
                else if (errCode.first == USER_WARNING) {
                    if (QMessageBox::question(gui, tr("Warning"),
                                              errCode.second + "\nDo you want to continue?",
                            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) {
                        if (createUndo)	deleteLastStoredUndo();
                        return false;
                    }
                }

				else if (errCode.first != NO_ERROR) { // the task has an error
					if (QMessageBox::question(gui->taskDialog(), tr("Task has errors/conflict"),
							errCode.second.replace('$','\n') + tr("\nIf you continue the task status will be set to ERROR/CONFLICT.\nDo you want to continue?"),
							QMessageBox::Yes,
							QMessageBox::No) == QMessageBox::No) {
						if (createUndo)	deleteLastStoredUndo();
						return false;
					}
					else {
                        pTask->clone(task);
                        pTask->calculateDataFiles();
						if ((prev_status >= Task::PRESCHEDULED) && (prev_status <= Task::ACTIVE)) {
							data.unscheduleTask(task_id);
							pTask->clearAllStorageConflicts();
                            if (pTask->hasStorage()) {
                                pTask->storage()->unAssignStorage();
                                pTask->storage()->generateFileList();
                            }
						}
						if (errCode.first == TASK_CONFLICT) {
							pTask->setStatus(Task::CONFLICT);
							gui->taskDialog()->updateStatus(Task::CONFLICT);
						}
						else {
							pTask->setStatus(Task::ERROR);
							gui->taskDialog()->updateStatus(Task::ERROR);
						}
                        gui->updateTask(pTask);
						setSaveRequired(true);
						return true;
					}
				}
				if (new_status == Task::SCHEDULED) {
                    if (!doScheduleChecks(task)) {
						if (createUndo) deleteLastStoredUndo();
						return false;
					}
				}
			}

            // now check what needs to change and apply the change

			// TODO: When a task is put on SCHEDULED then ScheduleTask should be called. This function checks a.o. if storage is assigned correctly.
			// if storage is not assigned correctly the task doesn't get its new state SCHEDULED.
			// it should then be put back on its original state and the last stored undo information should be removed

			if ((prev_status >= Task::PRESCHEDULED) && (prev_status <= Task::ACTIVE)) { // if task was scheduled on stations
                if (!task->isScheduled()) { // task is not scheduled anymore
					data.unscheduleTask(task_id);
					pTask->clearAllStorageConflicts();
                    if (pTask->hasStorage()) {
                        pTask->storage()->unAssignStorage();
                        pTask->storage()->generateFileList();
                    }
				}
                else if (pTask->isStationTask() && task->isStationTask()) {
                    StationTask *pt(static_cast<StationTask *>(pTask));
                    StationTask *po(static_cast<StationTask *>(task));
                    if (pt->getStations() != po->getStations()) { // task is still scheduled but its stations were changed (unschedule and reschedule to update the stations bookkeeping)
                        data.unscheduleTask(task_id);
                        pt->setStations(po->getStations()); // set the new stations
                        if (new_status == Task::SCHEDULED) {
                            if (doScheduleChecks(pt)) {
                                data.scheduleTask(pt);
                            }
                        }
                        else {
                            data.scheduleTask(pt);
                            pt->setStatus(new_status);
                        }
                    }
                }
                else if ((pTask->getScheduledStart() != task->getScheduledStart()) ||
                        (pTask->getScheduledEnd() != task->getScheduledEnd()) ||
                        (pTask->getDuration() != task->getDuration())) { // still scheduled but its scheduling times were changed
                    data.changeTaskSchedule(task_id, task->getScheduledStart(), task->getScheduledEnd());
				}
                pTask->clone(task);
			}
			else if ((new_status == Task::PRESCHEDULED) || (new_status == Task::SCHEDULED)) { // task is just changed to a scheduled state
                task->clearReason();
                pTask->clone(task);
				if (new_status == Task::SCHEDULED) {
					if (doScheduleChecks(pTask)) {
						data.scheduleTask(pTask);
					}
				}
				else {
					data.scheduleTask(pTask);
					pTask->setStatus(new_status);
				}
			}
			else if (prev_status == Task::ERROR) { // previously task had error, currently it is not (pre)scheduled
                pTask->clone(task);
				data.checkTask(pTask);
			}
			else {
                pTask->clone(task);
			}
            pTask->calculateDataFiles();

			// upload the on hold status directly to SAS. If not possible (e.g. no connection) issue warning
			if ((new_status == Task::ON_HOLD) && (prev_status != Task::ON_HOLD)) {
				setTaskOnHold(task_id, false);
			}

            gui->updateTask(pTask);
			updateStatusBar();
			setSaveRequired(true);
			return true;
		}
		else return false; // may not be edited if status above SCHEDULED
	}
	else {
		debugErr("sis", "Controller::updateTask: task:", task_id, " not found");
		return false;
	}
}

// synchronizeTask updates externally (i.e. OTDB) modified tasks in the scheduler
// parameter task holds the updated task
// if the task already exists in the scheduler then update its properties,
// if it does not yet exist then create it
void Controller::synchronizeTask(const Task *pTask) {
    unsigned treeID(pTask->getSASTreeID()), taskID(pTask->getID());
    Task::task_status state(pTask->getStatus());

    data.deleteTask(treeID, ID_SAS); // unschedules and deletes the existing task

    // create a new copyt of the task to add to the scheduler's datablock
    Task *pClone = cloneTask(pTask);

    // Add the task to the unscheduled tasks, dont check if the ID is free becuase it could just be an update of a scheudler task already in memory
    if (pClone->isStationTask()) {
        data.addTask(pClone, false);
        if (state >= Task::PRESCHEDULED && state <= Task::SCHEDULED) {
            data.scheduleTask(pClone);
            pClone->setStatus(state);
        }
        if (state >= Task::FINISHED) {
            data.moveTaskToInactive(taskID);
        }
    }
    else if (pClone->isPipeline()) {
        Pipeline *pPipe(static_cast<Pipeline *>(pClone));
        data.addTask(pPipe, false); // adds the task to the unscheduled tasks
        setInputFilesForPipeline(pPipe);
        pPipe->calculateDataFiles();
        if (state >= Task::PRESCHEDULED && state <= Task::SCHEDULED) {
            data.scheduleTask(pPipe);
            pPipe->setStatus(state);
        }
    }

    gui->updateTask(pClone);
    updateStatusBar();
}

/*
void Controller::synchronizeTask(const Task &task) {
	unsigned treeID(task.getSASTreeID()), taskID(task.getID());
	Task::task_status state(task.getStatus());
	Task *schedulerTask = data.getTaskForChange(treeID, ID_SAS);
	if (schedulerTask) {
		Task::task_status prev_status(schedulerTask->getStatus());

		if ((prev_status >= Task::PRESCHEDULED) && (prev_status <= Task::ACTIVE)) { // if task was scheduled on stations
			data.unscheduleTask(taskID);
		}
		if (prev_status >= Task::FINISHED && state < Task::FINISHED) {
			Task *pTask(data.moveTaskFromInactive(taskID));
			if (pTask) {
				schedulerTask = pTask;
			}
		}

		taskID = schedulerTask->getID();
        *schedulerTask = task; // THIS DOES NOT WORK FOR DERIVED CLASSES BECAUSE THEIR PROPERTIES ARE NOT COPIED (ONLY THE BASE CLASS PROPERTIES ARE COPIED THROUGH BASE POINTERS!!!)
		schedulerTask->setID(taskID);
	}
    else { // task not found in scheduler , create it now
		if (task.isPipeline()) {
            schedulerTask = data.newPipeline(taskID, static_cast<const Pipeline &>(task).pipelinetype(), OVERRIDE_SAS_TASKIDS); // this should always succeed because OVERRIDE_SAS_TASKIDS is true
			if (schedulerTask) {
				taskID = schedulerTask->getID();
				*schedulerTask = task;
				schedulerTask->setID(taskID);
                setInputFilesForPipeline(schedulerTask);
				schedulerTask->calculateDataFiles();
			}
			else {
				std::cerr << "Controller::synchronizeTask: Could not create Pipeline task for tree:" << treeID << std::endl;
				return;
			}
		}
		else { // TODO: Also tasks of type SYSTEM are for the moment created here to prevent SEGFAULTs. But it should maybe be created in another way than with newTask?
			schedulerTask = data.newTask(taskID, OVERRIDE_SAS_TASKIDS); // this should always succeed because OVERRIDE_SAS_TASKIDS is true
			if (schedulerTask) {
				taskID = schedulerTask->getID();
				*schedulerTask = task;
				schedulerTask->setID(taskID);
			}
			else {
				std::cerr << "Controller::synchronizeTask: Could not create task for tree:" << treeID << std::endl;
				return;
			}
		}
	}

	if (task.isScheduled()) {
		data.scheduleTask(schedulerTask);
		Task *pTask = data.getTaskForChange(taskID);
		pTask->setStatus(task.getStatus());
	}
	else {
		if (state >= Task::FINISHED) {
			data.moveTaskToInactive(taskID);
		}
	}

	gui->updateTask(treeID, ID_SAS, task.getType());
	updateStatusBar();
}
*/

/*
void Controller::abortTask(unsigned int taskID) {
	Task *pTask = data.getScheduledTaskForChange(taskID);
	if (pTask) {
		Task::task_status status = pTask->getStatus();
		if ((status == Task::ACTIVE) | (status == Task::STARTING) | (status == Task::SCHEDULED)) {
			QString taskStr = tr("Aborting task (") + QString::number(taskID) + ") " + pTask->getTaskName() + ", SAS_ID:" + QString::number(pTask->getSASTreeID());
			if (QMessageBox::question(0, taskStr,
					taskStr + "\n" + tr("The abort will be instantaneously committed to SAS.\nThe abort cannot be undone.\nDo you really want to abort the task?"),
					QMessageBox::Yes,
					QMessageBox::No) == QMessageBox::Yes) {
				if (itsSASConnection->abortTask(pTask->getSASTreeID())) {
					pTask->setStatus(Task::ABORTED);
					data.moveTaskToInactive(taskID);
					gui->updateTask(taskID);
					updateStatusBar();
				}
				else {
					QMessageBox::critical(0, tr("Could not abort the task"),
							tr("Could not abort the task. Probably the task is not in the ACTIVE or STARTING state"));
				}
			}
		}
	}
}
*/

const char *Controller::getReservationName(unsigned reservation_id) const {
	const Task *pRes = data.getReservation(reservation_id);
	if (pRes) return pRes->getTaskName();
	else return 0;
}

std::pair<unscheduled_reasons, QString> Controller::doPreScheduleChecks(Task *task) {
	// only add checks to this function that really should block the task from being (pre)scheduled
	std::pair<unscheduled_reasons, QString> error;
	error.first = NO_ERROR;

	// check for zero duration
	if (!task->getDuration().isSet()) {
		task->setReason(unscheduled_reason_str[ZERO_DURATION]);
		error.first = ZERO_DURATION;
		error.second = unscheduled_reason_str[ZERO_DURATION];
		return error;
	}

	// start time not set
	if (!task->getScheduledStart().isSet()) {
		task->setReason(unscheduled_reason_str[START_TIME_NOT_SET]);
		error.first = START_TIME_NOT_SET;
		error.second = unscheduled_reason_str[START_TIME_NOT_SET];
		return error;
	}
	// stop time not set
	if (!task->getScheduledEnd().isSet()) {
		task->setReason(unscheduled_reason_str[END_TIME_NOT_SET]);
		error.first = END_TIME_NOT_SET;
		error.second = unscheduled_reason_str[END_TIME_NOT_SET];
		return error;
	}

	QDateTime currentTime = QDateTime::currentDateTimeUtc();
	AstroDateTime now = AstroDateTime(currentTime.date().day(), currentTime.date().month(), currentTime.date().year(),
			currentTime.time().hour(), currentTime.time().minute(), currentTime.time().second());
	const AstroTime &minTime(Controller::theSchedulerSettings.getMinimumTimeBetweenTasks());
	if (task->isObservation() || task->isPipeline()) {
		if (task->getScheduledStart() < now + minTime) {
			task->setReason(unscheduled_reason_str[SCHEDULED_TOO_EARLY]);
			error.first = SCHEDULED_TOO_EARLY;
			error.second = unscheduled_reason_str[SCHEDULED_TOO_EARLY];
			return error;
		}
	}

	error = checkPredecessorDependencies(task, Task::PRESCHEDULED);
	if (error.first != NO_ERROR) {
		task->setReason(unscheduled_reason_str[error.first]);
		return error;
	}

    if (task->isObservation()) {
        Observation *pObs(static_cast<Observation *>(task));
		// check for unspecified antenna mode
        if (pObs->getAntennaMode() == UNSPECIFIED_ANTENNA_MODE) {
            pObs->setReason(unscheduled_reason_str[ANTENNA_MODE_UNSPECIFIED]);
			error.first = ANTENNA_MODE_UNSPECIFIED;
			error.second = unscheduled_reason_str[ANTENNA_MODE_UNSPECIFIED];
			return error;
		}
		// check for unspecified clock
        if (pObs->getStationClock() == UNSPECIFIED_CLOCK) {
            pObs->setReason(unscheduled_reason_str[UNSPECIFIED_CLOCK]);
			error.first = CLOCK_FREQUENCY_UNSPECIFIED;
			error.second = unscheduled_reason_str[CLOCK_FREQUENCY_UNSPECIFIED];
			return error;
		}
		// check for unspecified filter
        if (pObs->getFilterType() == UNSPECIFIED_FILTER) {
            pObs->setReason(unscheduled_reason_str[UNSPECIFIED_FILTER]);
			error.first = FILTER_TYPE_UNSPECIFIED;
			error.second =  unscheduled_reason_str[FILTER_TYPE_UNSPECIFIED];
			return error;
		}
		// check for incompatible antenna mode and filter type
        if (QString(pObs->getAntennaModeStr()).left(3) != QString(pObs->getFilterTypeStr()).left(3)) {
            pObs->setReason(unscheduled_reason_str[INCOMPATIBLE_ANTENNA_AND_FILTER]);
			error.first = INCOMPATIBLE_ANTENNA_AND_FILTER;
			error.second =  unscheduled_reason_str[INCOMPATIBLE_ANTENNA_AND_FILTER];
			return error;
		}
		// check for incompatible station clock and filter settings
        station_clock clock(pObs->getStationClock());
        station_filter_type filter(pObs->getFilterType());
		if ((clock != UNSPECIFIED_CLOCK) & (filter != UNSPECIFIED_FILTER)) {
			if ((clock == clock_160Mhz) & ((filter == LBA_10_70) | (filter == LBA_30_70) | (filter == HBA_170_230))) { /* compatible 160MHz */ }
			else if ((clock == clock_200Mhz) & ((filter == LBA_10_90) | (filter == LBA_30_90) | (filter == HBA_110_190) | (filter == HBA_210_250))) { /* compatible 200MHz */ }
			else {
                pObs->setReason(unscheduled_reason_str[INCOMPATIBLE_CLOCK_AND_FILTER]);
				error.first = INCOMPATIBLE_CLOCK_AND_FILTER;
				error.second = unscheduled_reason_str[INCOMPATIBLE_CLOCK_AND_FILTER];
				return error;
			}
		}
		// check for empty station list
        if (pObs->getStations().empty()) {
            pObs->setReason(unscheduled_reason_str[NO_STATIONS_DEFINED]);
			error.first = NO_STATIONS_DEFINED;
			error.second = unscheduled_reason_str[NO_STATIONS_DEFINED];
			return error;
		}

		// check for forbidden station combinations and empty station list / non-existing stations
        unscheduled_reasons reason = data.checkTaskStations(pObs);
		if (reason != NO_ERROR) {
			error.first = reason;
            error.second = pObs->getReason().c_str();
			return error;
		}

		// check for empty subband lists in digital beams
		int i = 0;
        bool showRelativeCoordWarning(false);
        const std::map<unsigned, DigitalBeam> &digiBeams = pObs->getDigitalBeams();
		for (std::map<unsigned, DigitalBeam>::const_iterator it = digiBeams.begin(); it != digiBeams.end(); ++it) {
			if (it->second.nrSubbands() == 0) {
                pObs->setReason(unscheduled_reason_str[EMPTY_SUBBAND_LIST]);
				error.first = EMPTY_SUBBAND_LIST;
				error.second = QString(unscheduled_reason_str[EMPTY_SUBBAND_LIST]) + " for beam " + QString::number(i);
				return error;
			}
            // check for relative TAB coordinates (if 0,0 is specified than most likely the user did not change the angles to absolute coordinates yet)
            if (!showRelativeCoordWarning) {
                const std::map<unsigned, TiedArrayBeam> &Tabs(it->second.tiedArrayBeams());
                for (std::map<unsigned, TiedArrayBeam>::const_iterator tabit = Tabs.begin(); tabit != Tabs.end(); ++tabit) {
                    if (tabit->second.isCoherent() && tabit->second.angle1() < std::numeric_limits<double>::epsilon() && tabit->second.angle2() < std::numeric_limits<double>::epsilon()) {
                        showRelativeCoordWarning = true;
                        break;
                    }
                }
            }
            ++i;
        }

		// check the total number of subbands
        unsigned short nrSubbands(pObs->getNrOfSubbands());
        switch (pObs->getBitMode()) {
/*
		case 2:
			if (nrSubbands > MAX_DATASLOTS_2_BITS) {
                pObs->setReason(unscheduled_reason_str[TOO_MANY_SUBBANDS]);
				error.first = TOO_MANY_SUBBANDS;
				error.second = unscheduled_reason_str[TOO_MANY_SUBBANDS] + "\nFor 2 bits mode only " + QString::number(MAX_DATASLOTS_2_BITS) + " subbands are allowed";
				return error;
			}
			break;
*/
		case 4:
			if (nrSubbands > MAX_DATASLOTS_4_BITS) {
                pObs->setReason(unscheduled_reason_str[TOO_MANY_SUBBANDS]);
				error.first = TOO_MANY_SUBBANDS;
				error.second = QString(unscheduled_reason_str[TOO_MANY_SUBBANDS]) + "\nFor 4 bits mode only " + QString::number(MAX_DATASLOTS_4_BITS) + " subbands are allowed";
				return error;
			}
			break;
		case 8:
			if (nrSubbands > MAX_DATASLOTS_8_BITS) {
                pObs->setReason(unscheduled_reason_str[TOO_MANY_SUBBANDS]);
				error.first = TOO_MANY_SUBBANDS;
				error.second = QString(unscheduled_reason_str[TOO_MANY_SUBBANDS]) + "\nFor 8 bits mode only " + QString::number(MAX_DATASLOTS_8_BITS) + " subbands are allowed";
				return error;
			}
			break;
		case 16:
			if (nrSubbands > MAX_DATASLOTS_16_BITS) {
                pObs->setReason(unscheduled_reason_str[TOO_MANY_SUBBANDS]);
				error.first = TOO_MANY_SUBBANDS;
				error.second = QString(unscheduled_reason_str[TOO_MANY_SUBBANDS]) + "\nFor 16 bits mode only " + QString::number(MAX_DATASLOTS_16_BITS) + " subbands are allowed";
				return error;
			}
			break;
		default: // unknown bit mode
            pObs->setReason(unscheduled_reason_str[UNKNOWN_BITMODE]);
			error.first = UNKNOWN_BITMODE;
			error.second = unscheduled_reason_str[UNKNOWN_BITMODE];
			return error;
		}

		// check beam duration difference from observation duration
		QString beamDurStr;
        const AstroTime &duration(pObs->getDuration());
		for (std::map<unsigned, DigitalBeam>::const_iterator it = digiBeams.begin(); it != digiBeams.end(); ++it) {
			if ((it->second.duration().totalSeconds() > 0) && (it->second.duration() != duration)) {
				beamDurStr += QString(it->second.target().c_str()) + "(" + it->second.duration().toString().c_str() + ")\n";
			}
		}
        const Observation::RTCPsettings &rtcp(pObs->getRTCPsettings());
        TaskStorage::enableDataProdukts odp(pObs->storage()->getOutputDataProductsEnabled());

        if ((odp.coherentStokes || odp.incoherentStokes /*|| odp.complexVoltages*/) && !rtcp.flysEye && (pObs->totalNrTABs() == 0)) {
			error.first = NO_TABS_DEFINED;
			error.second = unscheduled_reason_str[NO_TABS_DEFINED];
            pObs->setReason(unscheduled_reason_str[NO_TABS_DEFINED]);
			return error;
		}

        if (odp.incoherentStokes && (pObs->nrIncoherentTABs() == 0)) {
			error.first = NO_INCOHERENT_TABS_DEFINED;
			error.second = unscheduled_reason_str[NO_INCOHERENT_TABS_DEFINED];
            pObs->setReason(unscheduled_reason_str[NO_INCOHERENT_TABS_DEFINED]);
			return error;
		}

        if (odp.coherentStokes && (pObs->nrCoherentTABs() == 0) && (pObs->nrTABrings() == 0) && !rtcp.flysEye) {
			error.first = NO_COHERENT_TABS_DEFINED;
			error.second = unscheduled_reason_str[NO_COHERENT_TABS_DEFINED];
            pObs->setReason(unscheduled_reason_str[NO_COHERENT_TABS_DEFINED]);
			return error;
		}

        /* NOT FOR COBALT
		if (odp.coherentStokes && (rtcp.coherentChannelsPerSubband != 0)) {
			if (fmod((double)rtcp.channelsPerSubband, rtcp.coherentChannelsPerSubband) > 0) {
				error.first = WRONG_CHANNEL_COLLAPSE;
				error.second = "Channels per subband must be integer multiple or equal to coherent channels per subband";
                pObs->setReason(unscheduled_reason_str[WRONG_CHANNEL_COLLAPSE]);
				return error;
			}
		}
        */

        if (odp.coherentStokes && rtcp.coherentChannelsPerSubband == 0) {
            error.first = WRONG_CHANNEL_COLLAPSE;
            error.second = "coherent channels per subband setting may not be zero";
            pObs->setReason("coherent channels per subband setting may not be zero");
            return error;
        }

        /* NOT FOR COBALT
		if (odp.incoherentStokes && (rtcp.incoherentChannelsPerSubband != 0)) {
			if (fmod((double)rtcp.channelsPerSubband, rtcp.incoherentChannelsPerSubband) > 0) {
				error.first = WRONG_CHANNEL_COLLAPSE;
				error.second = "Channels per subband must be integer multiple or equal to incoherent channels per subband";
                pObs->setReason(unscheduled_reason_str[WRONG_CHANNEL_COLLAPSE]);
				return error;
			}
		}
        */

        if (odp.incoherentStokes && rtcp.incoherentChannelsPerSubband == 0) {
            error.first = WRONG_CHANNEL_COLLAPSE;
            error.second = "incoherent channels per subband setting may not be zero";
            pObs->setReason("incoherent channels per subband setting may not be zero");
            return error;
        }

        if ((!odp.incoherentStokes) && (!odp.coherentStokes) && (!odp.correlated)) {
			error.first = NO_DATA_OUTPUT_SELECTED;
			error.second = unscheduled_reason_str[NO_DATA_OUTPUT_SELECTED];
            pObs->setReason(unscheduled_reason_str[NO_DATA_OUTPUT_SELECTED]);
			return error;
		}


		// check for conflicts with overlapping tasks
        if (!data.checkStationConflicts(pObs)) {
			error.first = TASK_CONFLICT;
            error.second = pObs->getReason().c_str();
			return error;
		}

        if (pObs->getReservation() != 0) {
            std::pair<bool, std::pair<QString, Observation> > result(doReservationChecks(pObs, pObs->getReservation()));
			if (result.first) {
				error.first = NOT_COMPATIBLE_WITH_RESERVATION;
				error.second = result.second.first;
				return error;
			}
		}

		// (non severe tests) such as beamduration test should be the last test (user is still allowed to set the task to PRESCHEDULED
		if (!beamDurStr.isEmpty()) {
			error.first = BEAM_DURATION_DIFFERENT;
			error.second = QString(unscheduled_reason_str[BEAM_DURATION_DIFFERENT]) + " (" + duration.toString().c_str() + ") for beam:\n" + beamDurStr;
			return error;
		}

        if (showRelativeCoordWarning) {
            error.first = USER_WARNING;
            error.second = "(0.0, 0.0) TAB coordinates detected. Did you forget to specify the TABs in absolute coordinates?";
        }


	} // END if task->isObservation()
	else if (task->isStationTask()) { // e.g. MAINTENANCE or RESERVATION
        StationTask *psTask(static_cast<StationTask *>(task));
		// check for conflicts with overlapping tasks
        if (!data.checkStationConflicts(psTask)) {
			error.first = TASK_CONFLICT;
            error.second = psTask->getReason().c_str();
			return error;
		}
	}
	else if (task->isPipeline()) { // PIPELINE checks here
        Pipeline *pPipe(static_cast<Pipeline *>(task));
        itsSASConnection->translateMomPredecessors(pPipe->getPredecessorsForChange());

		// check demixing and averaging settings
		// demix freq step should be multiple of (averaging freq step)
		// same for demix time step and averaging time step
        // but this should only lead to error when demixing is actually done
        if (pPipe->isCalibrationPipeline()) {
            CalibrationPipeline *pCalPipe(static_cast<CalibrationPipeline *>(pPipe));
            if (pCalPipe->demixingEnabled()) {
                const DemixingSettings &demixing(pCalPipe->demixingSettings());
                // TODO: see Redmine issue #4923. The check if there are sources to demix cannot be used yet.
                // the demix freq step and time step still need checking because the demixer will always be started in the pipeline and throw an assertion if these are wrong
                // needs fixing in the pipeline first.
                if ((demixing.freqStep() != 0) && (demixing.timeStep() != 0)) {
                    if ((demixing.demixFreqStep() % demixing.freqStep() != 0) || (demixing.demixTimeStep() % demixing.timeStep() != 0)) {
                        error.first = INCOMPATIBLE_DEMIX_SETTINGS;
                        error.second = unscheduled_reason_str[INCOMPATIBLE_DEMIX_SETTINGS];
                        task->setReason(unscheduled_reason_str[INCOMPATIBLE_DEMIX_SETTINGS]);
                        return error;
                    }
                }
                else {
                    error.first = INCOMPATIBLE_DEMIX_SETTINGS;
                    error.second = unscheduled_reason_str[INCOMPATIBLE_DEMIX_SETTINGS];
                    task->setReason(unscheduled_reason_str[INCOMPATIBLE_DEMIX_SETTINGS]);
                    return error;
                }

                // check for unknown demix sources
                const QStringList &demix_src = theSchedulerSettings.getDemixSources();
                QStringList dsra(demixing.demixAlways().split(",", QString::SkipEmptyParts));
                QStringList dsri(demixing.demixIfNeeded().split(",", QString::SkipEmptyParts));
                QStringList invalid_srcs;
                foreach (const QString &src, dsra) {
                    if (!demix_src.contains(src)) {
                        invalid_srcs.append(src);
                    }
                }
                foreach (const QString &src, dsri) {
                    if (!demix_src.contains(src)) {
                        invalid_srcs.append(src);
                    }
                }
                if (!invalid_srcs.isEmpty()) {
                    error.first = UNKNOWN_DEMIX_SOURCE;
                    error.second = QString("Unknown demix sources specified:") + invalid_srcs.join(",");
                    task->setReason(error.second.toStdString());
                    return error;
                }
            }
        }

        // TODO: setInputFilesForPipeline should probably not be done here. Only set the input files when a task is downloaded from SAS or when it is just loaded from disk
        // now the enabled flags (user selection) gets reset by calling setInputFilesForPipeline which is also a bug. This should not be the case
        error = setInputFilesForPipeline(pPipe);

        //WK code commented out
//        if (pPipe->isCalibrationPipeline() &&
//            !task->storage()->getEqualityInputOutputProducts())
//        {
//            error.first = INPUT_OUTPUT_LOCATION_MISMATCH1;
//            error.second = unscheduled_reason_str[INPUT_OUTPUT_LOCATION_MISMATCH2];
//            return error;
//        }


		if (error.first != NO_ERROR) return error;
	}

	if (error.first == NO_ERROR) {
        if (task->hasStorage()) {
            task->calculateDataFiles();
            task->storage()->generateFileList();
        }
	}
    // Check here if the input output locations are the same
    // Check added due to #8174
//    if (task->isPipeline())
//    {
//        // TODO: This is incredibly ugly!!!
//        Pipeline *pipeline = dynamic_cast<Pipeline *>(task);
//
//        if (pipeline->isCalibrationPipeline() &&
//            !task->storage()->getEqualityInputOutputProducts())
//        {
//            error.first = INPUT_OUTPUT_LOCATION_MISMATCH2;
//            error.second = unscheduled_reason_str[INPUT_OUTPUT_LOCATION_MISMATCH2];
//            return error;
//        }
//    }

	// if we arrrive here no errors in the task
	task->clearReason();

	return error;
}

std::pair<unscheduled_reasons, QString> Controller::checkPredecessorDependencies(const Task *pTask, Task::task_status newState) const {
	// check predecessors statuses. They must have at least PRESCHEDULED state
	std::pair<unscheduled_reasons, QString> error;
	const IDvector &predecessorIDs(pTask->getPredecessors());
	if (!predecessorIDs.empty()) {
		Task::task_status predState;
		// check if all predecessors are equal or beyond the SCHEDULED status, if not this task may not yet be scheduled
		const Task *predTask;
		for (IDvector::const_iterator predit = predecessorIDs.begin(); predit != predecessorIDs.end(); ++ predit) {
			predTask = data.getTask(predit->second, predit->first);
			if (predTask) {
				predState = predTask->getStatus();
				if ((predState < newState) || (predState > Task::ABORTED)) {
					error.first = PREDECESSOR_UNSCHEDULED;
					error.second = "The task cannot be scheduled because not all its predecessors are (pre)scheduled..aborted.\nThe predecessors may not have a lower state.";
					return error; // not all predecessors in correct state, cannot schedule the current task
				}
                else if (pTask->getScheduledStart() < predTask->getRealEnd() + theSchedulerSettings.getMinimumTimeBetweenTasks()) {
					error.first = TOO_CLOSE_OR_BEFORE_PREDECESSOR;
					error.second = "The start time is before or too close to the predecessor task " + QString::number(predTask->getID());
					return error; // not all predecessors in correct state, cannot schedule the current task
				}
			}
			else {
				error.first = PREDECESSOR_NOT_FOUND;
				error.second = QString("The task cannot be scheduled because the predecessor ") + QString::number(predit->second) + " could not be found";
				return error; // one of the predecessors could not be found
			}
		}
	}
	return error;
}


// doReservationChecks checks if the task is compatible with the reservation specified in the task
// returns pair<bool, Task>; bool is true -> changes needed, new compatible Task with changes is returned as second part of pair
// if bool is false means task is already compatible and no changes are needed or the reservation was not found
std::pair<bool, std::pair<QString, Observation> > Controller::doReservationChecks(const Observation *orgObs, unsigned reservationID) {
    std::pair<bool, std::pair<QString, Observation> > returnValue;
	returnValue.first = false;
    returnValue.second.second.clone(orgObs);
    Observation &compatibleTask(returnValue.second.second);
	QString &msg(returnValue.second.first);

	if (reservationID) {
        const StationTask *reservation = data.getReservation(reservationID);
		if (reservation) {
			msg = "The following changes are needed to make the task compatible with the reservation:\n";
			// check if task is no longer than the reservation
			const AstroTime &reservationDuration(reservation->getDuration());
            if (orgObs->getDuration() > reservationDuration) {
				compatibleTask.setDuration(reservationDuration); // shorten task
				msg += QString("- task duration shortened to ") + reservationDuration.toString().c_str() +"\n";
				returnValue.first = true;
			}
            if (orgObs->isStationTask()) {
                // check if the stations of the task are a subset of the reserved stations
                const taskStationsMap &stations = orgObs->getStations();
                const taskStationsMap &resStations = reservation->getStations();
                std::vector<std::string> newStations;
                bool stationsChanged(false);
                QString removedStations("- removed stations:");
                for (taskStationsMap::const_iterator it = stations.begin(); it != stations.end(); ++it) {
                    if (resStations.find(it->first) != resStations.end()) {
                        newStations.push_back(it->first);
                    }
                    else {
                        removedStations += QString(it->first.c_str()) + " ";
                        stationsChanged = true;
                    }
                }
                compatibleTask.setStations(newStations);

                if (stationsChanged) {
                    msg += removedStations += "\n";
                    returnValue.first = true;
                }

                // check if antenna settings are the same
                if ((reservation->getAntennaMode() != UNSPECIFIED_ANTENNA_MODE) && (orgObs->getAntennaMode() != reservation->getAntennaMode())) {
                    compatibleTask.setAntennaMode(reservation->getAntennaMode());
                    msg += QString("- antenna mode changed from ") + orgObs->getAntennaModeStr() + " to " + compatibleTask.getAntennaModeStr() + "\n";
                    returnValue.first = true;
                }
                if ((reservation->getStationClock() != UNSPECIFIED_CLOCK) && (orgObs->getStationClock() != reservation->getStationClock())) {
                    compatibleTask.setStationClock(reservation->getStationClock());
                    msg += QString("- station clock changed from ") + orgObs->getStationClockStr() + " to " + compatibleTask.getStationClockStr() + "\n";
                    returnValue.first = true;
                }
                if ((reservation->getFilterType() != UNSPECIFIED_FILTER) && (orgObs->getFilterType() != reservation->getFilterType())) {
                    compatibleTask.setFilterType(reservation->getFilterType());
                    msg += QString("- station filter changed from ") + orgObs->getFilterTypeStr() + " to " + compatibleTask.getFilterTypeStr() + "\n";
                    returnValue.first = true;
                }
            }


			// check the scheduled times of the task and if needed find new schedule times within reservation

            const AstroDateTime &scheduledStart = orgObs->getScheduledStart();
			const AstroDateTime &resStart = reservation->getScheduledStart();
			const AstroDateTime &resEnd = reservation->getScheduledEnd();
			if ((scheduledStart < resStart) || (scheduledStart > resEnd)) {
				AstroDateTime start(resStart);
                data.findFirstOpportunity(orgObs, start, reservationID);
				compatibleTask.setScheduledStart(start);
				msg += QString("- scheduled start changed from ") +
                        orgObs->getScheduledStart().toString().c_str() +
							" to " + compatibleTask.getScheduledStart().toString().c_str() + "\n";
				returnValue.first = true;
			}
			// minimum and maximum time window
            if ((orgObs->getWindowFirstDay() < resStart) || (orgObs->getWindowFirstDay() > resEnd)) {
				compatibleTask.setWindowFirstDay(resStart);
				msg += QString("- first possible date changed from ") +
                        orgObs->getWindowFirstDay().toString().c_str() +
						" to " + compatibleTask.getWindowFirstDay().toString().c_str() + "\n";
				returnValue.first = true;
			}
            if ((orgObs->getWindowLastDay() < resStart) || (orgObs->getWindowLastDay() > resEnd)) {
				compatibleTask.setWindowLastDay(resEnd);
				msg += QString("- last possible date changed from ") +
                        orgObs->getWindowLastDay().toString().c_str() +
						" to " + compatibleTask.getWindowLastDay().toString().c_str() + "\n";
				returnValue.first = true;
			}
            if (orgObs->getWindowMinTime() < resStart.getTime()) {
				compatibleTask.setWindowMinTime(resStart.getTime());
				msg += QString("- first possible time changed from ") +
                        orgObs->getWindowMinTime().toString().c_str() +
						" to " + compatibleTask.getWindowMinTime().toString().c_str() + "\n";
				returnValue.first = true;
			}
            if (orgObs->getWindowMaxTime() < resEnd.getTime()) {
				compatibleTask.setWindowMaxTime(resEnd.getTime());
				msg += QString("- last possible time changed from ") +
                        orgObs->getWindowMaxTime().toString().c_str() +
						" to " + compatibleTask.getWindowMaxTime().toString().c_str() + "\n";
				returnValue.first = true;
			}
		}
		else {
			debugWarn("sis","reservation with ID:", reservationID, " not found!");
		}
	}
	return returnValue;
}


void Controller::multiEditSelectedTasks(void) {
	bool showReservationWarning(false), showScheduledWarning(false);
	std::vector<Task *> selectedTasks;
	Task *pTask;
	for (std::vector<unsigned>::iterator it = itsSelectedTasks.begin(); it != itsSelectedTasks.end(); ++it) {
		pTask = data.getTaskForChange(*it);
		if (pTask) {
			if (pTask->getType() != Task::RESERVATION) {
				if (pTask->getStatus() <= Task::PRESCHEDULED) {
					selectedTasks.push_back(pTask);
				}
				else {
					showScheduledWarning = true;
				}
			}
			else {
				showReservationWarning = true;
			}
		}
	}
	if (showReservationWarning || showScheduledWarning) {
		if (!selectedTasks.empty()) {
			if (QMessageBox::question(gui, tr("Some tasks cannot be edited"),
					tr("Tasks with status SCHEDULED and above and reservations cannot be (multi)edited.\n These tasks will be de-selected. Do you want to continue?"),
					QMessageBox::Yes,
					QMessageBox::No) == QMessageBox::Yes) {
				std::vector<unsigned> newSelection;
				for (std::vector<Task *>::const_iterator it = selectedTasks.begin(); it != selectedTasks.end(); ++it) {
					newSelection.push_back((*it)->getID());
				}
				selectTasks(newSelection); // sets the new selection (without reservations)
				if (!selectedTasks.empty()) {
					gui->multiEditTasks(selectedTasks);
					return;
				}
			}
			else return; // do not continue
		}
		else {
			QMessageBox::warning(gui, tr("None of the selected tasks can be edited"),
								tr("Tasks with status SCHEDULED and above and reservations cannot be (multi)edited.\n There are no other tasks left in the selection that can be (multi-)edited"));
			return;
		}
	}
	if (!selectedTasks.empty()) {
		gui->multiEditTasks(selectedTasks);
		return;
	}
	else {
		QMessageBox::warning(0, tr("Not tasks to multi-edit"),
				tr("No tasks remain for multi-edit."));
		QApplication::beep();
	}
}

void Controller::selectCurrentTaskGroups(selector_types type) {
	if (!itsSelectedTasks.empty()) {
		QApplication::setOverrideCursor(Qt::WaitCursor);
		std::vector<unsigned> groups;
		for (std::vector<unsigned>::const_iterator it = itsSelectedTasks.begin(); it != itsSelectedTasks.end(); ++it) {
			const Task *pTask(data.getTask(*it));
			if (pTask) {
				unsigned group(pTask->getGroupID());
				if (group != 0) {
					if (find(groups.begin(), groups.end(), group) == groups.end()) {
						groups.push_back(group);
					}
				}
			}
		}
		std::vector<unsigned> tasksToSelect;
		for (std::vector<unsigned>::const_iterator groupIt = groups.begin(); groupIt != groups.end(); ++groupIt) {
			std::vector<unsigned> groupTasks(data.tasksInGroup(*groupIt, type));
			tasksToSelect.insert(tasksToSelect.end(), groupTasks.begin(), groupTasks.end());
		}
		if (!tasksToSelect.empty()) {
            selectTasks(tasksToSelect);
            QApplication::restoreOverrideCursor();
        }
		else {
            QApplication::restoreOverrideCursor();
            QMessageBox::information(gui, "No tasks to select", "No tasks of this type could be found to select");
		}
	}
}


void Controller::selectTask(unsigned taskID, bool singleSelection, bool selectRows, bool tableClick) {
	if (singleSelection) {
		itsSelectedTasks.clear();
		gui->selectTask(taskID, singleSelection, selectRows, tableClick);
		itsSelectedTasks.push_back(taskID);
	}
	else {
		if (find(itsSelectedTasks.begin(), itsSelectedTasks.end(), taskID) == itsSelectedTasks.end()) { // not already selected?
			gui->selectTask(taskID, singleSelection, selectRows, tableClick);
			itsSelectedTasks.push_back(taskID);
		}
	}
}

void Controller::deselectTask(unsigned taskID, bool singleSelection, bool selectRows) {
	std::vector<unsigned>::iterator it = find(itsSelectedTasks.begin(), itsSelectedTasks.end(), taskID);
	if (it != itsSelectedTasks.end()) { // is it currently selected?
		gui->deselectTask(taskID, singleSelection, selectRows);
		itsSelectedTasks.erase(it);
	}
}

void Controller::selectTasks(const std::vector<unsigned> &taskIDs) {
	itsSelectedTasks.clear();
	for (std::vector<unsigned>::const_iterator it = taskIDs.begin(); it != taskIDs.end(); ++it) {
		if ((find(itsSelectedTasks.begin(), itsSelectedTasks.end(), *it)) == itsSelectedTasks.end()) {
			itsSelectedTasks.push_back(*it);
		}
	}
	gui->selectTasks(itsSelectedTasks);
}

bool Controller::isSelected(unsigned taskID) {
	return (find(itsSelectedTasks.begin(), itsSelectedTasks.end(), taskID) != itsSelectedTasks.end());
}

void Controller::deselectAllTasks(void) {
	itsSelectedTasks.clear();
	gui->clearSelection();
}

bool Controller::isDeleted(unsigned taskID) const {
	for (deletedTasksVector::const_iterator it = itsDeletedTasks.begin(); it != itsDeletedTasks.end(); ++it) {
        for (vector<Task *>::const_iterator tit = it->begin(); tit != it->end(); ++tit) {
            if ((*tit)->getID() == taskID) return true;
		}
	}
	return false;
}


bool Controller::unDelete(unsigned taskID) {
	for (deletedTasksVector::iterator it = itsDeletedTasks.begin(); it != itsDeletedTasks.end(); ++it) {
        for (vector<Task *>::iterator tit = it->begin(); tit != it->end(); ++tit) {
            if ((*tit)->getID() == taskID) { // found the task
                Task::task_status status((*tit)->getStatus());
                data.addTask(*tit, false);
                if ((*tit)->isScheduled()) {
                    data.scheduleTask(*tit);
                    (*tit)->setStatus(status);
                }
                else if (status >= Task::FINISHED) {
                    data.moveTaskToInactive(taskID);
                }
                gui->addTask(*tit);
                itsSASConnection->removeFromSASTaskToDelete((*tit)->getSASTreeID());
                it->erase(tit); // deletes the task from the deletedTasksVector

                // update the undo info so that it doesn't try to undo the deletion of this task again
                for (undoTypeVector::iterator uit = itsUndoType.begin(); uit != itsUndoType.end(); ++uit) {
                    if (uit->first == UNDO_DELETE_TASKS) {
                        for (vector<unsigned>::iterator vit = uit->second.second.begin(); vit != uit->second.second.end(); ++vit) {
                            if (*vit == taskID) {
                                uit->second.second.erase(vit);
                                if (uit->second.second.empty()) { // this undo step has no tasks left, remove it from the undo stack
                                    gui->removeUndo(uit->second.first);
                                    itsUndoType.erase(uit);
                                }
                                return true;
                            }
                        }
                    }
                }
            }
		}
	}
	return false;
}

void Controller::unscheduleSelectedTasks(void) {
	if (!itsSelectedTasks.empty()) {
		storeScheduleUndo(QObject::tr("unschedule task(s)"));
		Task *pTask;
		for (std::vector<unsigned>::const_iterator it = itsSelectedTasks.begin(); it != itsSelectedTasks.end(); ++it) {
			if (data.unscheduleTask(*it)) {
				pTask = data.getTaskForChange(*it);
                pTask->clearAllStorageConflicts();
                if (pTask->hasStorage()) {
                    pTask->storage()->unAssignStorage();
                    pTask->storage()->generateFileList();
                }
                gui->removeTaskFromScene(*it);
                gui->updateTableTask(pTask);
                updateStatusBar();
			}
		}
	}
}

void Controller::setSelectedTasksOnHold(void) {
	if (!itsSelectedTasks.empty()) {
		bool undo_needed(false), warning(false);
		storeScheduleUndo(QObject::tr("set (multiple) task(s) on hold"));
		Task::task_status status;
		Task *pTask;
		std::vector<int> SAStrees;
		for (std::vector<unsigned>::const_iterator it = itsSelectedTasks.begin(); it != itsSelectedTasks.end(); ++it) {
			pTask = data.getTaskForChange(*it);
			status = pTask->getStatus();
			if (status <= Task::SCHEDULED) {
				if (!((status == Task::UNSCHEDULED) || (status == Task::ERROR))) {
					data.unscheduleTask(*it);
					pTask->clearAllStorageConflicts();
                    if (pTask->hasStorage()) {
                        pTask->storage()->unAssignStorage();
                        pTask->storage()->generateFileList();
                    }
					gui->removeTaskFromScene(*it);
				}
				undo_needed = true;
				pTask->setStatus(Task::ON_HOLD);
                gui->updateTableTask(pTask);
				SAStrees.push_back(pTask->getSASTreeID());
			}
			else warning = true;
		}
		if (!undo_needed) {
			deleteLastStoredUndo();
		}
		else {
			updateStatusBar();
			if (QMessageBox::question(gui, tr("Directly upload to SAS"),
					tr("Do you want to upload the ON_HOLD status directly to SAS?"),
					QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
				if (!itsSASConnection->setTasksOnHold(SAStrees)) {
					QMessageBox::warning(0, tr("Could not update the states in SAS"),
							tr("Could not update the states in SAS database. Maybe there is a connection error?"));
				}
			}
		}
		if (warning) {
			QMessageBox::warning(0, tr("Some task(s) could not be set on hold"),
					tr("Only tasks that have the status UNSCHEDULED or (PRE)SCHEDULED or ERROR can be put on hold"));
			QApplication::beep();
		}
	}
}

bool Controller::setTaskOnHold(unsigned taskID, bool store_undo) {
	Task *pTask = data.getTaskForChange(taskID);
	Task::task_status status = pTask->getStatus();
	if (status <= Task::SCHEDULED) {
		if (store_undo) {
			storeTaskPropertyUndo(taskID, TASK_STATUS, QString(pTask->getStatusStr()));
		}
		if (!((status == Task::UNSCHEDULED) || (status == Task::ERROR))) {
			data.unscheduleTask(taskID);
			pTask->clearAllStorageConflicts();
            if (pTask->hasStorage()) {
                pTask->storage()->unAssignStorage();
                pTask->storage()->generateFileList();
            }
            gui->removeTaskFromScene(taskID);
		}
		pTask->setStatus(Task::ON_HOLD);
        gui->updateTableTask(pTask);
		updateStatusBar();
		if (QMessageBox::question(gui, tr("Directly upload to SAS"),
				tr("Do you want to upload the ON_HOLD status directly to SAS?"),
				QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
			if (!itsSASConnection->setTaskOnHold(pTask->getSASTreeID())) {
				QMessageBox::warning(0, tr("Could not set the state in SAS"),
						tr("Could put the task ON_HOLD in the SAS database. Maybe there is a connection error?"));
			}
		}
		return true;
	}
	else {
		QMessageBox::warning(0, tr("Cannot put the task on hold"),
				tr("Task with status: ") + pTask->getStatusStr() + tr(" cannot be put ON_HOLD"));
		QApplication::beep();
		return false;
	}
}

void Controller::copyTask(unsigned int taskID) {
	bool undo_necessary(false);
	TaskCopyDialog *copyDialog = new TaskCopyDialog(gui);
	int nrCopies = copyDialog->exec();
	AstroDateTime startTime(copyDialog->getStartTime());
	const AstroTime &timeStep(copyDialog->getTimeStep());
	const Task::task_status newState(copyDialog->getTaskState());
	bool applyNewStartTime = startTime.isSet() ? true : false;
	bool applyTimeStep = timeStep.isSet() ? true : false;

	if (nrCopies) {
		std::vector<unsigned int> newTasks;
		storeScheduleUndo("create copy(s) of task " + QString::number(taskID));
		const Task *pTask(data.getTask(taskID));
		if (pTask) {
			unsigned int newTaskID;
            Task *newTask(0);
            Observation *obs(0);
            Pipeline *pipe(0);
			for (int copy = 0; copy < nrCopies; ++copy) {
                newTaskID = data.getNewTaskID();
                newTask = cloneTask(pTask);
                newTask->setID(newTaskID);
                data.addTask(newTask, false);
                newTasks.push_back(newTaskID);
                if (applyNewStartTime) {
                    if (applyTimeStep & (copy > 0)) {
                        startTime += timeStep + newTask->getDuration();
                    }
                    newTask->setScheduledStart(startTime);
                }
                newTask->resetRealTimes();
                obs = dynamic_cast<Observation *>(newTask);
                if (obs) {
                    obs->resetBeamDurations();
                    obs->clearDataSlots();
                }
                newTask->setOriginalTreeID(theSchedulerSettings.getSASDefaultTreeID(newTask));
                newTask->setSASTreeID(0);
                newTask->setMoMID(0);
                newTask->setTaskName(std::string("(copy of) ") + newTask->getTaskName());
                if (newTask->hasStorage()) {
                    newTask->storage()->clearStorageCheckResults();
                    newTask->storage()->unAssignStorage();
                }
                newTask->clearAllConflicts();
                newTask->clearReason();
                newTask->clearPenalty();
                pipe = dynamic_cast<Pipeline *>(newTask);
                if (pipe) {
                    setInputFilesForPipeline(pipe);
                }
                if (newTask->hasStorage()) {
                    newTask->calculateDataFiles();
                }
                if (newState == Task::PRESCHEDULED) {
                    std::pair<unscheduled_reasons, QString> errCode(doPreScheduleChecks(newTask));
                    if (errCode.first == BEAM_DURATION_DIFFERENT) {
                        if (QMessageBox::question(gui, tr("Beam duration different"),
                                                  errCode.second.replace('$','\n') + "\nDo you want to maximize the beam durations (Recommended)",
                                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
                            static_cast<Observation *>(newTask)->resetBeamDurations();
                        }
                        data.scheduleTask(newTask);
                        newTask->setStatus(Task::PRESCHEDULED);
                    }
                    else if (errCode.first == USER_WARNING) {
                        if (QMessageBox::question(gui, tr("Warning"),
                                                  errCode.second + "\nDo you want to continue?",
                                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) {
                            deleteLastStoredUndo();
                            return;
                        }
                        data.scheduleTask(newTask);
                        newTask->setStatus(Task::PRESCHEDULED);
                    }
                    else if (errCode.first == NO_ERROR) {
                        data.scheduleTask(newTask);
                        newTask->setStatus(Task::PRESCHEDULED);
                    }
                    else if (errCode.first == TASK_CONFLICT) {
                        newTask->setStatus(Task::CONFLICT);
                        QMessageBox warningBox(gui);
                        warningBox.setWindowTitle(tr("Task has conflict"));
                        warningBox.setIcon(QMessageBox::Critical);
                        warningBox.addButton(tr("Ok"), QMessageBox::NoRole);
                        warningBox.setText(errCode.second.replace('$','\n') + "\nThe task cannot be set to PRESCHEDULED before the problem is resolved");
                        warningBox.exec();
                    }
                    else if (errCode.first > TASK_CONFLICT) {
                        newTask->setStatus(Task::ERROR);
                        QMessageBox warningBox(gui);
                        warningBox.setWindowTitle(tr("Task has critical errors"));
                        warningBox.setIcon(QMessageBox::Critical);
                        warningBox.addButton(tr("Ok"), QMessageBox::NoRole);
                        warningBox.setText(errCode.second.replace('$','\n') + "\nThe task cannot be set to PRESCHEDULED before the problem is resolved");
                        warningBox.exec();
                    }
                }
                else {
                    newTask->setStatus(Task::UNSCHEDULED);
                }
                gui->addTask(newTask);
            }
		}

		if (undo_necessary) {
			gui->sortTable();
			selectTasks(newTasks);
			updateStatusBar();
			setSaveRequired(true);
		}
		else {
			deleteLastStoredUndo();
		}
        updateStatusBar();
	}
	delete copyDialog;
}

// setInputFilesForPipeline checks if the task has dependencies on data products from predecessor tasks
// then gets the data product information and checks and sets the correct info for these dependencies in the task
// These dependencies have to be checked and set before a task is set to SCHEDULED
std::pair<unscheduled_reasons, QString> Controller::setInputFilesForPipeline(Task *pPipe) {
	std::pair<unscheduled_reasons, QString> error;
		std::vector<const Task *> predecessors;
        const IDvector &predecessorIDs(pPipe->getPredecessors());
		if (!predecessorIDs.empty()) {
			// collect all predecessor tasks in a vector for faster searching
			const Task *predTask;
			for (IDvector::const_iterator predit = predecessorIDs.begin(); predit != predecessorIDs.end(); ++ predit) {
				predTask = data.getTask(predit->second, predit->first);
				if (predTask) {
					predecessors.push_back(predTask);
				}
				else {
					error.first = PREDECESSOR_NOT_FOUND;
					error.second = QString("The predecessor task ") + QString::number(predit->second) + " could not be found.";
					return error; // one of the predecessors could not be found
				}
			}

			// task has predecessors, check all input data products identifications of this task and search the
			// output data products of the predecessors for matching identifications
			// if a match is found then copy the corresponding filenames and location arrays into the inputDataProduct of this task
			// if no match then error condition data product not found in predecessor(s) tasks, task cannot be scheduled
            std::map<dataProductTypes, TaskStorage::inputDataProduct> &inputDataProducts(pPipe->storage()->getInputDataProductsForChange());

			int idxIt;
			bool foundit;
            std::map<dataProductTypes, TaskStorage::outputDataProduct>::const_iterator pit;
			QString sapstr;
			QStringList locations, filenames;
			std::vector<bool> skipVec;
			bool resetSkipVector, SyncSkipWithPredecessor;

			for (dataProductTypes dpType = _BEGIN_DATA_PRODUCTS_ENUM_; dpType < _END_DATA_PRODUCTS_ENUM_-1; dpType = dataProductTypes(dpType + 1)) {
                if (pPipe->storage()->isInputDataProduktEnabled(dpType)) { // is this input data product type enabled?
                    TaskStorage::inputDataProduct &dp = inputDataProducts[dpType]; // also creates the record in the inputDataProducts map if it doesn't exist yet
					resetSkipVector = (dp.skip.empty() /*|| (dp.skip.size() != (unsigned)dp.filenames.size())*/); //  the skip vector should only be synchronized with the predecessor skip vector the first time (i.e. when it is not yet set)
					storageVector storageVec;

					for (QStringList::const_iterator identit = dp.identifications.begin(); identit != dp.identifications.end(); ++identit) {
						foundit = false;
						for (std::vector<const Task *>::const_iterator predit = predecessors.begin(); predit != predecessors.end(); ++predit) {
                            if ((*predit)->hasStorage()) {
                                const TaskStorage *predStorage((*predit)->storage());
                            const std::map<dataProductTypes, TaskStorage::outputDataProduct> &pred_output(predStorage->getOutputDataProducts());
							pit = pred_output.find(dpType);
							if (pit != pred_output.end()) {
								idxIt = pit->second.identifications.indexOf(*identit);
								if (idxIt != -1) { // found?
                                    storageVector predecessorStorageVec(predStorage->getStorageLocations(dpType));
									unsigned psz(predecessorStorageVec.size());
									if (psz != 0) {
										// copy the filenames and locations pointed to by this identification to the input data product list of this task
										if (pit->second.filenames.size() == pit->second.locations.size()) {
                                            if ((dpType == DP_CORRELATED_UV) || (dpType == DP_COHERENT_STOKES) || (dpType == DP_INCOHERENT_STOKES)) { // for these data product types copy only the files that have the corresponding SAP
												const QString &identification(pit->second.identifications.at(idxIt));
												int i(identification.indexOf(".SAP"));
												if (i != -1) { // does it contain a reference to a specific SAP?
													sapstr = identification.mid(i+1, identification.indexOf('.',i+1) - i - 1);
													SyncSkipWithPredecessor = (pit->second.skip.size() == (unsigned)pit->second.filenames.size());
													for (int i = 0; i < pit->second.filenames.size(); ++ i) {
														const QString &filename(pit->second.filenames.at(i));
														if (filename.contains(sapstr)) {
															filenames.push_back(filename);
															locations.push_back(pit->second.locations.at(i));
															storageVec.push_back(predecessorStorageVec.at(i % psz));
															if (resetSkipVector) {
																if (SyncSkipWithPredecessor) {
																	skipVec.push_back(pit->second.skip.at(i));
																}
																else {
																	skipVec.push_back(false);
																}
															}
														}
													}
												}
												else { // no specific SAP specified in identification, just copy all files
													filenames += pit->second.filenames;
													locations += pit->second.locations;
													storageVec.insert(storageVec.end(), predecessorStorageVec.begin(), predecessorStorageVec.end());
													if (resetSkipVector) {
														if (pit->second.skip.size() == (unsigned)pit->second.filenames.size()) {
															skipVec.insert(skipVec.end(), pit->second.skip.begin(), pit->second.skip.end());
														}
														else {
															skipVec.assign(filenames.size(), false);
														}
													}
												}
											}
											else { // for all other data product types copy all files
												filenames += pit->second.filenames;
												locations += pit->second.locations;
												storageVec.insert(storageVec.end(), predecessorStorageVec.begin(), predecessorStorageVec.end());
												if (resetSkipVector) {
													if (pit->second.skip.size() == (unsigned)pit->second.filenames.size()) {
														skipVec.insert(skipVec.end(), pit->second.skip.begin(), pit->second.skip.end());
													}
													else {
														skipVec.assign(filenames.size(), false);
													}
												}
											}
											// also set the input file size
                                            const std::pair<double, unsigned> &outputSizes(predStorage->getOutputFileSizes(dpType));
                                            pPipe->storage()->setInputFileSizes(dpType, std::pair<double, unsigned>(outputSizes.first, filenames.size()));

                                            // Pulsar Pipeline: for Stokes register the polarization type in the pipeline
                                            // which is needed to determine the number of output files for the pulsar pipeline
                                            PulsarPipeline *pPulsarPipe = dynamic_cast<PulsarPipeline *>(pPipe);
                                            if (pPulsarPipe && (*predit)->isObservation()) {
                                                const Observation *predObs(static_cast<const Observation *>(*predit));
                                                pPulsarPipe->setCoherentType(predObs->getRTCPsettings().coherentType);
                                                pPulsarPipe->setIncoherentType(predObs->getRTCPsettings().incoherentType);
                                            }
										}
										else {
											std::cerr << "filenames and locations array of task: " << (*predit)->getID() << " for data product: " << DATA_PRODUCTS[dpType] << " are not equal in length!" << std::endl;
										}
										foundit = true;
										break; // breaks out of predecessor search loop for the current identification
									}
								}
							}
                        }
						}
						if (!foundit) {
							// one of the identifications was not found in any of the predecessor tasks
							// this is an error, scheduling of this task cannot continue
							error.first = INPUT_DATA_PRODUCT_NOT_FOUND;
							error.second = QString("The task cannot be scheduled because the ") + DATA_PRODUCTS[dpType] + " input data product with identification:" + *identit + " could not be found in its predecessor tasks.\nAre all predecessor tasks correctly defined for this task?";
							return error; // identification not found
						}
					}
					// set storage location IDs equal to the accumulation of the predecessor output storage vec's
                    pPipe->storage()->addInputStorageLocations(dpType, storageVec);
					dp.filenames = filenames;
					dp.locations = locations;
					if (!resetSkipVector) {
						if (dp.skip.size() != (unsigned)dp.filenames.size()) {
							dp.skip.assign(dp.filenames.size(), false);
						}
					}
					else {
						dp.skip = skipVec;
					}
					filenames.clear();
					locations.clear();
					skipVec.clear();
				}
			}
		}
		else {
			error.first = INPUT_DATA_PRODUCT_NOT_FOUND;
			error.second = QString("The task cannot be scheduled because it doesn't have a predecessor defined");
			return error;
		}

	return error;
}

bool Controller::doScheduleChecks(Task *pTask) {
	// check if all output data products have storage assigned to them, if not don't allow to set the task to SCHEDULED
    if (pTask->hasStorage()) {
        if (!pTask->storage()->checkStorageAssigned()) {
            QMessageBox::warning(gui, tr("Cannot schedule the task"),
				tr("The task cannot be scheduled because some of its output data products do not have storage assigned.\nFirst assign resources to the task in the PRESCHEDULED state."));
            return false; // no storage assigned to (some) data products
        }
    }

	std::pair<unscheduled_reasons, QString> result = checkPredecessorDependencies(pTask, Task::SCHEDULED);
	if (result.first != NO_ERROR) {
		QMessageBox::warning(gui, tr("Cannot schedule the task"),result.second);
		return false;
	}

//WK code commented out
//    if (pTask->isPipeline())
//    {
//        // TODO: This is incredibly ugly!!!
//        Pipeline *pipeline = dynamic_cast<Pipeline *>(pTask);
//        if (pipeline->isCalibrationPipeline() &&
//            !pTask->storage()->getEqualityInputOutputProducts())
//        {
//            QMessageBox::warning(gui,
//              tr("Error during scheduling")
//                     ,tr("Task input and output are different, #8174, LOC3. Retry assigning resources"));
//            return false;
//        }
//    }

	return true;
}

void Controller::copySelectedTasks(void) {
	if (!itsSelectedTasks.empty()) {
		bool undo_necessary(false);
		TaskCopyDialog *copyDialog = new TaskCopyDialog(gui);
		int nrCopies = copyDialog->exec();
		AstroDateTime startTime(copyDialog->getStartTime());
		const AstroTime &timeStep(copyDialog->getTimeStep());
		const Task::task_status newState(copyDialog->getTaskState());
		bool applyNewStartTime = startTime.isSet() ? true : false;

		// first sort the selected tasks on start time
		const Task *pTask(0);
		std::vector<const Task *> sortedTasks;
		for (vector<unsigned>::iterator it = itsSelectedTasks.begin(); it != itsSelectedTasks.end(); ++it) {
			pTask = data.getTask(*it);
			if (pTask) {
				sortedTasks.push_back(pTask);
			}
		}
		sort(sortedTasks.begin(), sortedTasks.end(), cmp_taskScheduledStart());

		if (nrCopies) {
			std::vector<unsigned int> newTasks;
			storeScheduleUndo("create copies of multiple tasks");
			Task *newTask(0);
			unsigned int newTaskID;
			AstroDateTime firstTaskStart(sortedTasks.front()->getScheduledStart());
			AstroTime shift(startTime - sortedTasks.front()->getScheduledStart()); // the shift from the original start of the first task to the new start time of the first task
			for (int copy = 0; copy < nrCopies; ++copy) {
                for (vector<const Task *>::iterator it = sortedTasks.begin(); it != sortedTasks.end(); ++it) {
                    const AstroDateTime &origTaskStartTime((*it)->getScheduledStart());
                    newTask = cloneTask(*it);
                    newTaskID = data.getNewTaskID();
                    newTask->setID(newTaskID);
                    if (data.addTask(newTask, false)) {
                        newTasks.push_back(newTaskID);
                        if (applyNewStartTime) {
                            newTask->setScheduledStart(origTaskStartTime + shift);
                        }
                        int defaultTreeID(theSchedulerSettings.getSASDefaultTreeID(newTask));
                        if (defaultTreeID != newTask->getOriginalTreeID()) {
                            newTask->setOriginalTreeID(defaultTreeID);
                        }
                        newTask->resetRealTimes();
                        newTask->setSASTreeID(0);
                        newTask->setMoMID(0);
                        newTask->setTaskName(std::string("(copy of) ") + newTask->getTaskName());
                        if (newTask->hasStorage()) {
                            newTask->storage()->unAssignStorage();
                            newTask->storage()->clearStorageCheckResults();
                        }
                        if (newTask->isObservation()) {
                            Observation *pObs(static_cast<Observation *>(newTask));
                            pObs->resetBeamDurations();
                            pObs->clearDataSlots();
                        }
                        newTask->clearAllConflicts();
                        newTask->clearReason();
                        newTask->clearPenalty();
                        newTask->calculateDataFiles();
                        undo_necessary = true;
                        if (newState == Task::PRESCHEDULED) {
                            data.scheduleTask(newTask);
                            newTask->setStatus(newState);
                        }
                        else {
                            newTask->setStatus(Task::UNSCHEDULED);
                        }
                        gui->addTask(newTask);
                    }
                }
                shift = newTask->getScheduledEnd() + timeStep - firstTaskStart;
                if (!undo_necessary) {
                    deleteLastStoredUndo();
                }
            }
			// after all copying is done:
			if (undo_necessary) {
				selectTasks(newTasks);
				updateStatusBar();
				setSaveRequired(true);
			}
			else {
				deleteLastStoredUndo();
			}
		}
	}
}

bool Controller::unscheduleTask(unsigned taskID) {
	Task *pTask = data.getTaskForChange(taskID);
	if (pTask) {
		storeTaskPropertyUndo(taskID,TASK_STATUS, QString(pTask->getStatusStr()));
		if (data.unscheduleTask(taskID)) {
			pTask->clearAllStorageConflicts();
            if (pTask->hasStorage()) {
                pTask->storage()->unAssignStorage();
            }
			if (pTask->isStationTask()) {
				gui->removeTaskFromScene(taskID);
			}
            gui->updateTableTask(pTask);
			updateStatusBar();
			return true;
		}
		else {
			deleteLastStoredUndo();
			return false;
		}
	}
	else return false;
}

void Controller::scheduleTask(unsigned taskID, Task::task_status new_status) {
	Task *pTask = data.getTaskForChange(taskID);
	Task::task_status current_status(pTask->getStatus());

	if (new_status == Task::PRESCHEDULED) {
		if ((current_status < Task::PRESCHEDULED) || (current_status == Task::SCHEDULED)) {
			std::pair<unscheduled_reasons, QString> errCode = doPreScheduleChecks(pTask);
			if (errCode.first == NOT_COMPATIBLE_WITH_RESERVATION) {
				QMessageBox warningBox(gui);
				warningBox.setWindowTitle(tr("Task does not meet reservation constraints"));
				warningBox.setIcon(QMessageBox::Critical);
				warningBox.addButton(tr("Ok"), QMessageBox::NoRole);
				warningBox.setText(errCode.second.replace('$','\n') + "\nThe task cannot be set to PRESCHEDULED before the conflict is resolved");
				warningBox.exec();
                gui->updateTask(pTask);
				return;
			}
            if (errCode.first == BEAM_DURATION_DIFFERENT) {
				if (QMessageBox::question(gui, tr("Beam duration different"),
						errCode.second.replace('$','\n') + "\nDo you still want to set the task to PRESCHEDULED?",
						QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) {
					// don't apply the status change
					return;
				}
			}
            else if (errCode.first == USER_WARNING) {
                if (QMessageBox::question(gui, tr("Warning"),
                                          errCode.second + "\nDo you want to continue?",
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) {
                    return;
                }
            }
			else if (errCode.first == TASK_CONFLICT) {
				QMessageBox warningBox(gui);
				warningBox.setWindowTitle(tr("Task has conflicts with other tasks"));
				warningBox.setIcon(QMessageBox::Critical);
				warningBox.addButton(tr("Ok"), QMessageBox::NoRole);
				warningBox.setText(errCode.second.replace('$','\n') + "\nThe task cannot be set to PRESCHEDULED before the conflict is resolved");
				warningBox.exec();
				pTask->setStatus(Task::CONFLICT);
                gui->updateTask(pTask);
				return;
			}
			else if (errCode.first == SCHEDULED_TOO_EARLY) {
				QMessageBox warningBox(gui);
				warningBox.setWindowTitle(tr("Task is scheduled too early"));
				warningBox.setIcon(QMessageBox::Critical);
				warningBox.addButton(tr("Ok"), QMessageBox::NoRole);
				warningBox.setText(errCode.second.replace('$','\n') + "\nThe task cannot be set to PRESCHEDULED");
				warningBox.exec();
                gui->updateTask(pTask);
				return;
			}
			else if (errCode.first > TASK_CONFLICT) {
				if (pTask->isScheduled()) {
					if (!pTask->isPipeline()) {
						data.unscheduleTask(taskID);
					}
					pTask->clearAllStorageConflicts();
                    if (pTask->hasStorage()) {
                        pTask->storage()->unAssignStorage();
                        pTask->storage()->generateFileList();
                    }
					pTask->setStatus(Task::ERROR);
                    gui->updateTask(pTask);
					updateStatusBar();
				}
				QMessageBox warningBox(gui);
				warningBox.setWindowTitle(tr("Task contains error(s)"));
				warningBox.setIcon(QMessageBox::Critical);
				warningBox.addButton(tr("Cancel"), QMessageBox::NoRole);
				warningBox.setText(errCode.second.replace('$','\n') + "\nThe task cannot be set to (PRE)SCHEDULED before the error is resolved");
				warningBox.exec();
                gui->updateTask(pTask);
				return;
			}
			storeTaskPropertyUndo(taskID, TASK_STATUS, QString(pTask->getStatusStr()));
			if (!pTask->isPipeline()) { // pipelines do not have to be unscheduled first
				data.unscheduleTask(taskID);
				if (data.scheduleTask(pTask)) {
					pTask->setStatus(Task::PRESCHEDULED);
				}
			}
			else { // for pipeline
				pTask->setStatus(Task::PRESCHEDULED);
			}
            gui->updateTask(pTask);
		}
		else {
			QMessageBox::warning(0, tr("Cannot set the task to PRESCHEDULED"),
					tr("The task can only be set PRESCHEDULED when the current status is below PRESCHEDULED or the status is SCHEDULED."));
			QApplication::beep();
		}
	}
	else if (new_status == Task::SCHEDULED) {
		Task::task_type type(pTask->getType());
		if (current_status == Task::PRESCHEDULED) {
			if (doScheduleChecks(pTask)) {
				storeTaskUndo(taskID, "Set task " + QString::number(taskID) + " to SCHEDULED");
				if (type != Task::PIPELINE) {
					data.unscheduleTask(taskID);
				}
				data.scheduleTask(pTask);
			}
            gui->updateTask(pTask);
		}
		else {
			QMessageBox::warning(0, tr("Could not set the task to SCHEDULED"),
					tr("Tasks can only be set SCHEDULED when their current task status is PRESCHEDULED."));
			QApplication::beep();
		}
	}
}

void Controller::scheduleSelectedTasks(Task::task_status new_status) {
	bool warn(false), undo_needed(false);
	std::vector<Task *> sortedTasks;
	for (std::vector<unsigned>::const_iterator it = itsSelectedTasks.begin(); it != itsSelectedTasks.end(); ++it) {
		Task *pTask = data.getTaskForChange(*it);
		sortedTasks.push_back(pTask);
	}
	sort(sortedTasks.begin(), sortedTasks.end(), cmp_taskScheduledStart());
	Task::task_status current_status;
	if (new_status == Task::SCHEDULED) {
		storeScheduleUndo("Set multiple tasks on SCHEDULED");
		for (std::vector<Task *>::const_iterator tit = sortedTasks.begin(); tit != sortedTasks.end(); ++tit) {
			Task::task_type type((*tit)->getType());
			current_status = (*tit)->getStatus();
			if ((current_status == Task::PRESCHEDULED) && (new_status == Task::SCHEDULED)) {
				if (doScheduleChecks((*tit))) {
					if (type != Task::PIPELINE) {
						data.unscheduleTask((*tit)->getID());
					}
					data.scheduleTask((*tit));
					undo_needed = true;
                    gui->updateTask(*tit);
				}
			}
			else if (current_status == new_status) { } // nop
			else warn = true;
		}
		if (!undo_needed) {
			deleteLastStoredUndo();
		}
		if (warn) {
			QMessageBox::warning(0, tr("Could not set some tasks to SCHEDULED"),
					tr("Tasks can only be set SCHEDULED when their current task status is PRESCHEDULED."));
			QApplication::beep();
		}
	}
	else {
		storeScheduleUndo("Set multiple tasks on PRESCHEDULED");
        bool apply_prescheduled, early_warning(false), error_warning(false), conflict_warning(false), apply_all_beam_duration_diffs(false),
                ignore_all_rel_coord_warnings(false);
		for (std::vector<Task *>::const_iterator tit = sortedTasks.begin(); tit != sortedTasks.end(); ++tit) {
			apply_prescheduled = false;
			Task::task_type type((*tit)->getType());
			unsigned taskID = (*tit)->getID();
			Task::task_status current_status((*tit)->getStatus());
			if ((current_status < Task::PRESCHEDULED) || (current_status == Task::SCHEDULED)) {
				std::pair<unscheduled_reasons, QString> errCode(doPreScheduleChecks(*tit));
				if (errCode.first == NO_ERROR) {
					apply_prescheduled = true;
				}
                else if (errCode.first == BEAM_DURATION_DIFFERENT) {
					if (apply_all_beam_duration_diffs) apply_prescheduled = true;
					else {
						int choice(QMessageBox::question(gui, tr("Beam duration different"),
								errCode.second.replace('$','\n') + "\nDo you still want to set the task to PRESCHEDULED?",
								QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No, QMessageBox::Yes));
						if (choice == QMessageBox::No) {
							// don't apply the status change
						}
						else if (choice == QMessageBox::YesToAll) {
							apply_all_beam_duration_diffs = true;
							apply_prescheduled = true;
						}
						else apply_prescheduled = true;
					}
				}
                else if (errCode.first == USER_WARNING) {
                    if (ignore_all_rel_coord_warnings) apply_prescheduled = true;
                    else {
                        int choice(QMessageBox::question(gui, tr("Warning"),
                                                         errCode.second.replace('$','\n') + "\nDo you still want to set the task to PRESCHEDULED?",
                                                         QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No, QMessageBox::Yes));
                        if (choice == QMessageBox::No) {
                            // don't apply the status change
                        }
                        else if (choice == QMessageBox::YesToAll) {
                            ignore_all_rel_coord_warnings = true;
                            apply_prescheduled = true;
                        }
                    }
                }
                else if (errCode.first == SCHEDULED_TOO_EARLY) {
					if ((*tit)->isScheduled()) {
						(*tit)->clearAllStorageConflicts();
                        if ((*tit)->hasStorage()) {
                            (*tit)->storage()->unAssignStorage();
                            (*tit)->storage()->generateFileList();
                        }
						data.unscheduleTask(taskID);
                        gui->updateTask(*tit);
					}
					early_warning = true;
				}
				else if (errCode.first == TASK_CONFLICT) {
					if ((*tit)->isScheduled()) {
						(*tit)->clearAllStorageConflicts();
                        if ((*tit)->hasStorage()) {
                            (*tit)->storage()->unAssignStorage();
                            (*tit)->storage()->generateFileList();
                        }
                        data.unscheduleTask(taskID);
                        gui->updateTask(*tit);
					}
					(*tit)->setStatus(Task::CONFLICT);
					conflict_warning = true;
					// don't apply the status change
				}
				else if (errCode.first > TASK_CONFLICT) {
					if ((*tit)->isScheduled()) {
						(*tit)->clearAllStorageConflicts();
                        if ((*tit)->hasStorage()) {
                            (*tit)->storage()->unAssignStorage();
                            (*tit)->storage()->generateFileList();
                        }
                        data.unscheduleTask(taskID);
                        gui->updateTask(*tit);
					}
					(*tit)->setStatus(Task::ERROR);
					error_warning = true;
					// don't apply the status change
				}
				else apply_prescheduled = true;

				if (apply_prescheduled) {
					if (type != Task::PIPELINE) {
						if ((*tit)->isScheduled()) {
							data.unscheduleTask(taskID);
						}
					}
					data.scheduleTask((*tit));
					(*tit)->setStatus(Task::PRESCHEDULED);
					undo_needed = true;
				}
                gui->updateTask(*tit);
			}
			else if (current_status == Task::PRESCHEDULED) { } // nop
			else warn = true;
		}

		if (early_warning) {
			QMessageBox warningBox(gui);
			warningBox.setWindowTitle(tr("Task(s) scheduled too early"));
			warningBox.setIcon(QMessageBox::Critical);
			warningBox.addButton(tr("Ok"), QMessageBox::NoRole);
			warningBox.setText("One or more task(s) are scheduled too early or too close to now.\nCheck start time(s) of these tasks before setting them to (PRE)SCHEDULED");
			warningBox.exec();
		}
		if (conflict_warning) {
			QMessageBox warningBox(gui);
			warningBox.setWindowTitle(tr("Task(s) with conflicts"));
			warningBox.setIcon(QMessageBox::Critical);
			warningBox.addButton(tr("Ok"), QMessageBox::NoRole);
			warningBox.setText("One or more task(s) are conflicting with other tasks.\nThese task(s) cannot be set to (PRE)SCHEDULED before the conflict is resolved");
			warningBox.exec();
		}
		if (error_warning) {
			QMessageBox warningBox(gui);
			warningBox.setWindowTitle(tr("Task(s) with errors"));
			warningBox.setIcon(QMessageBox::Critical);
			warningBox.addButton(tr("Ok"), QMessageBox::NoRole);
			warningBox.setText("One or more task(s) have errors.\nThese task(s) cannot be set to (PRE)SCHEDULED before these errors are resolved");
			warningBox.exec();
		}

		if (!undo_needed) {
			deleteLastStoredUndo();
		}
		else {
			updateStatusBar();
		}
		if (warn) {
			QMessageBox::warning(0, tr("Could not set some tasks to PRESCHEDULED"),
					tr("Tasks can only be set PRESCHEDULED when their current task status is below PRESCHEDULED or the status is SCHEDULED."));
			QApplication::beep();
		}
	}
}


bool Controller::checkEarlyTasksStatus(void) {
	bool bResult(true);
	// first check the actual status of tasks that have a start time less than now + minimum_time_between_observations
	// if the status is still (PRE)SCHEDULED then this is a conflict
	// if the status changed in the mean time then download the task again from SAS and update in the scheduler
	std::vector<Task *> tasks(data.getScheduledTasksVector());
    std::vector<Pipeline *> pipelines(data.getScheduledPipelinesVector());
	tasks.insert(tasks.end(), pipelines.begin(), pipelines.end()); // add pipelines
	int treeID;
	for (std::vector<Task *>::const_iterator it = tasks.begin(); it != tasks.end(); ++it) {
        if ((*it)->getScheduledStart() <= now()) {
			treeID = (*it)->getSASTreeID();
			if ((itsSASConnection->connect() == 0) && (treeID != 0)) { // only do the sas check for apparently too early tasks that are already in SAS , not for new tasks
				Task::task_status status(itsSASConnection->getTaskStatus(treeID));
                AstroDateTime start(itsSASConnection->getScheduledStartTime(treeID));
                if (status < Task::STARTING && start.isSet()) { // *** the task has not been started yet (nor is it running or finished or aborted), this is a too early task now
					// set conflict
                    if ((*it)->hasStorage()) {
                        (*it)->storage()->clearStorageCheckResults();
                        (*it)->storage()->unAssignStorage();
                    }
                    (*it)->setConflict(CONFLICT_STORAGE_TIME_TOO_EARLY);
					itsConflictDialog->addConflict(*it, CONFLICT_STORAGE_TIME_TOO_EARLY);
					bResult = false;
				}
			}
			else { // this is a new task with a too early start time, that is a conflict
                if ((*it)->hasStorage()) {
                    (*it)->storage()->clearStorageCheckResults();
                    (*it)->storage()->unAssignStorage();
                }
                (*it)->setConflict(CONFLICT_STORAGE_TIME_TOO_EARLY);
				itsConflictDialog->addConflict(*it, CONFLICT_STORAGE_TIME_TOO_EARLY);
				bResult = false;
			}
		}
	}
	return bResult;
}

int Controller::assignResources(bool showResult) {
	int retVal(0);
	itsConflictDialog->clearAllConflicts();
	// first check the actual status of tasks that have a start time less than now + minimum_time_between_observations
	// if the status is still (PRE)SCHEDULED then this is a conflict
	// if the status changed in the mean time then download the task again from SAS and update in the scheduler
	if (!checkEarlyTasksStatus()) {
		retVal = 2; // some tasks are scheduled too early
	}

	if (retVal == 0) {
		int ret = refreshStorageNodesInfo();
		// ret:
		// 0: refresh ok
		// 1: no connection to data monitor don't continue
		// 2: user clicked cancel when asked to connect to the data monitor

		if (ret == 0) { // refresh ok
			if (!assignStorageResources()) {
				retVal = 3; // storage resource assignment conflicts detected
			}
		}
		else if (ret == 2) { // user clicked cancel when asked to connect to the data monitor
			return -1;
		}
		else retVal = ret;

		if ((retVal != 1) & (!calculateDataSlots())) {
			retVal = 4; // data slots conflict detected
		}
	}

	switch (retVal) {
	case 0:
		if (showResult) {
			QMessageBox::information(gui,tr("Resource assignment ok"),tr("Resources are successfully assigned to all future prescheduled and scheduled tasks"));
		}
		break;
	case 1:
//		QMessageBox::critical(0, tr("No connection to Data Monitor"),
//				tr("Could not connect to the Data Monitor.\nPlease check Data Monitor connection settings"));
		break;
	case 2:
		QMessageBox::warning(gui,tr("Resource assignment conflicts detected"),tr("Some task(s) are scheduled in the past!\nStart time needs to be at least 3 minutes after now"));
		itsConflictDialog->show();
		break;
	case 3:
		QMessageBox::warning(gui,tr("Resource assignment conflicts detected"),tr("A storage resource conflict was detected!"));
		itsConflictDialog->show();
		break;
	case 4:
		QMessageBox::warning(gui,tr("Resource assignment conflicts detected"),tr("A dataslot resource conflict was detected!"));
		itsConflictDialog->show();
		break;
	}

	return retVal;
}


bool Controller::calculateDataSlots(void) {
	// TODO calculate the data slots for all OBSERVATION tasks (RESERVATION should reserve a number of data slots)
	// the number of data slots for a task are equal to the total number of subbands that are requested.
	bool bResult(true);

	// steps:
	// 1) get a list of pointers all future scheduled tasks (status prescheduled or scheduled)
    const std::vector<Observation *> tasks = data.getFutureObservationsSortStartTime();
	// clear all assigned data slots for these tasks
    for (std::vector<Observation *>::const_iterator it = tasks.begin(); it != tasks.end(); ++it) {
		if ((*it)->getStatus() == Task::PRESCHEDULED) { // tasks with SCHEDULED status keep their assigned resources
			(*it)->clearDataSlots();
			(*it)->clearConflict(CONFLICT_BITMODE);
			(*it)->clearConflict(CONFLICT_OUT_OF_DATASLOTS);
		}
	}

	std::vector<unsigned> overlappingTasks;
	std::map<unsigned short, unsigned> upperDataSlotUsed; // key = RSP board number, value contains the maximum dataslotnr that is currently taken
	stationDataSlotMap newDataSlots; // the newly assigned dataslot ranges for a task (first = RSP board, second = range of dataslots)
	unsigned short bitMode;
	unsigned nrOfSubbands;
	bool bitModeConflict, outOfDataSlots;

	// for each task, do:
    for (std::vector<Observation *>::const_iterator it = tasks.begin(); it != tasks.end(); ++it) {
        // get bit mode for this task (parallel tasks on the same stations should have same bit mode
        bitModeConflict = false;
        outOfDataSlots = false;
        bitMode = (*it)->getBitMode();
        nrOfSubbands = (*it)->getNrOfSubbands();
        // 3) for each station assigned to the task find out which are the parallel tasks
        const std::map<std::string, unsigned> &stations =  (*it)->getStations();
        for (std::map<std::string, unsigned>::const_iterator statit = stations.begin(); statit != stations.end(); ++statit) {
            newDataSlots.clear();
            upperDataSlotUsed.clear();
            const Station *station = data.getStation(statit->second);
            if (station) {
                overlappingTasks = station->getTaskswithinTimeSpan((*it)->getScheduledStart(), (*it)->getScheduledEnd());
                // 4) for each parallel task on this station make a note of the used data slots
                for (std::vector<unsigned>::const_iterator otit = overlappingTasks.begin(); otit != overlappingTasks.end(); ++otit) {
                    if (*otit != (*it)->getID()) { // Station::getOverlappingTasks also returns the current task which obviously always overlaps with itself (exclude this task)
                        StationTask *parallelTask = data.getScheduledTask(*otit); // the next overlapping task on this station
                        if (parallelTask) {
                            if (!bitModeConflict) { // if the current task already has a bit mode conflict
                                // check that bit mode is the same for parallel tasks
                                if (parallelTask->isObservation()) {
                                    Observation *parallelObs(static_cast<Observation *>(parallelTask));
                                    if (parallelObs->getBitMode() == bitMode) {
                                        const stationDataSlotMap & dataSlots = parallelTask->getStationDataSlots(statit->second);
                                        if (!dataSlots.empty()) { // are the data slots set for this station in this task?
                                            // iterate over the RSP boards
                                            for (unsigned short RSPBoard = 0; RSPBoard <= 3; ++ RSPBoard) {
                                                stationDataSlotMap::const_iterator dsit = dataSlots.find(RSPBoard);
                                                if (dsit != dataSlots.end()) {
                                                    // the parallel task already uses some of the dataslots on this RSP board
                                                    // take a note of the maximum occupied data slot number on this board
                                                    upperDataSlotUsed[RSPBoard] = std::max(upperDataSlotUsed[RSPBoard], dsit->second.second);
                                                }
                                            }
                                        }
                                    }
                                    else { // there is a bit mode conflict for this task and the parallel task
                                        (*it)->setConflict(CONFLICT_BITMODE);
                                        parallelObs->setConflict(CONFLICT_BITMODE);
                                        itsConflictDialog->addDataSlotConflict(*it, CONFLICT_BITMODE, statit->first, parallelObs->getID());
                                        bitModeConflict = true;
                                        bResult = false;
                                    }
                                }
                            }
                            else {
                                parallelTask->setConflict(CONFLICT_BITMODE);
                            }
                        }
                    }
                }

			// 2d) for every station in the task assign N free dataslots (where N equals the total number of subbands for this task)
			if (!bitModeConflict && !outOfDataSlots) {
				unsigned maxDataSlotNr = (*it)->getNrOfDataslotsPerRSPboard()-1;

				// assign free dataslots
				std::map<unsigned short, unsigned>::const_iterator udit; // used dataslot iterator
				unsigned subbandsLeft(nrOfSubbands);
				unsigned maxNrToAssign;
				while (subbandsLeft && !outOfDataSlots) {
					for (unsigned short RSPBoard = 0; RSPBoard <= 3; ++ RSPBoard) {
						udit = upperDataSlotUsed.find(RSPBoard);
						if (udit == upperDataSlotUsed.end()) { // board is completely free
							maxNrToAssign = std::min(subbandsLeft-1, (unsigned)maxDataSlotNr);
							newDataSlots[RSPBoard] = std::pair<unsigned, unsigned>(0, maxNrToAssign);
							subbandsLeft -= (maxNrToAssign+1);
						}
						else { // board is partially taken
							// udit->second = maximum data slot occupied on the RSP board
							if (udit->second < maxDataSlotNr) {
								maxNrToAssign = std::min(udit->second + subbandsLeft, (unsigned)maxDataSlotNr); // the maximum dataslot number that can be assigned on this board
								newDataSlots[RSPBoard] = std::pair<unsigned, unsigned>(udit->second+1, maxNrToAssign); // the range of dataslots that will be assigned to this task on this board
								subbandsLeft -= (maxNrToAssign - udit->second); // the number of subbands that still have to be assigned to the task
							}
							else if (RSPBoard == 3) outOfDataSlots = true;
						}
						if (subbandsLeft == 0) break; // w're done for this station
					}
					if (subbandsLeft > 0) {
						outOfDataSlots = true;
						break;
					}
				}
				if (!outOfDataSlots) {
					(*it)->assignDataSlots(statit->second, newDataSlots); // TODO: we have to set the list for every antenna field (stations) in the task!!!
//					std::cout << "assigning data slots to task: " << (*it)->getID() << ", for station:" << statit->first << std::endl;
//					for (stationDataSlotMap::const_iterator vit = newDataSlots.begin(); vit != newDataSlots.end(); ++vit) {
//						std::cout << "RSPBoard:" << vit->first << ", data slots:" << vit->second.first << ".." << vit->second.second << std::endl;
//					}
				}
				else {
					(*it)->setConflict(CONFLICT_OUT_OF_DATASLOTS);
					itsConflictDialog->addDataSlotConflict(*it, CONFLICT_OUT_OF_DATASLOTS, statit->first);
					bResult = false;
				}
			}
		}
	}
		// 2e) get the next scheduled task and redo steps 2b,c,d until at end of scheduled tasks
	}
	return bResult;
}


bool Controller::assignManualStorageToTask(Task *pTask) {
    if (pTask->hasStorage()) {
        TaskStorage *taskStorage(pTask->storage());
        // check and when possible assign the task's manually requested resources
        const storageMap &locations(taskStorage->getStorageLocations());
        //check if the minimum required number of nodes is assigned
        std::map<dataProductTypes, int> minNodes(taskStorage->getMinimumNrOfStorageNodes());
        storageMap::const_iterator sit;
        for (std::map<dataProductTypes, int>::const_iterator it = minNodes.begin(); it != minNodes.end(); ++it) {
            sit = locations.find(it->first);
            if (sit != locations.end()) {
                if (sit->second.size() < static_cast<unsigned>(it->second)) {
                    pTask->setConflict(CONFLICT_STORAGE_MINIMUM_NODES);
                    taskStorage->setOutputDataProductAssigned(it->first, false);
                    // add the conflict to the conflicts dialog
                    itsConflictDialog->addStorageConflict(pTask, it->first, CONFLICT_STORAGE_MINIMUM_NODES);
                    return false;
                }
            }
        }

        std::vector<storageResult> result = data.addStorageToTask(pTask, locations);
        if (result.empty()) { // no conflicts?
            // clear the task's storage conflicts
            pTask->clearAllStorageConflicts();
            taskStorage->clearStorageCheckResults();
            return true;
        }
        else {
            taskStorage->setStorageCheckResult(result);
            for (std::vector<storageResult>::const_iterator confit = result.begin(); confit != result.end(); ++confit) {
                pTask->setConflict(confit->conflict);
                taskStorage->setOutputDataProductAssigned(confit->dataProductType, false);
                // add the conflict to the conflicts dialog
                itsConflictDialog->addStorageConflict(pTask, confit->dataProductType, confit->conflict, confit->storageNodeID, confit->storageRaidID);
            }
            return false;
        }
    }
    else return true;
}


bool Controller::assignStorageToTask(Task *pTask) {
    bool bResult(true);
    if (pTask->hasStorage()) {
        TaskStorage *taskStorage(pTask->storage());
        data.removeStorageForTask(pTask->getID());
        taskStorage->unAssignStorage();
        taskStorage->clearStorageCheckResults();
        const dataFileMap &dataFileSizes = taskStorage->getOutputFileSizes();
        if (dataFileSizes.empty()) {
            itsConflictDialog->addStorageConflict(pTask, DP_UNKNOWN_TYPE, CONFLICT_STORAGE_NO_DATA);
            return false;
        }

        double singleFileBW;
        unsigned nrFiles, minNrOfNodes, nrFilesPerNode;

        if (pTask->isPipeline()) {
            Pipeline *pipe = static_cast<Pipeline *>(pTask);
            setInputFilesForPipeline(pipe);
            // for pipelines typically the input node is also the output node except for imaging pipeline and skyImage (skyImkages get merged together)
            const storageMap &inputLocations(taskStorage->getInputStorageLocations());
            std::vector<storageResult> scResultTotal;
            for (dataFileMap::const_iterator dpit = dataFileSizes.begin(); dpit != dataFileSizes.end(); ++dpit) { // output number of files and file sizes
                if (dpit->first == DP_SKY_IMAGE) { // for sky image files may be merged to a single node
                    // TODO: for imaging pipeline write distribution scheme (merge skyImages to one node for each image)
                }
                else if (dpit->first == DP_PULSAR) {
                    storageVector combinedStorage;

                    storageMap::const_iterator iit = inputLocations.find(DP_COHERENT_STOKES);
                    if (iit != inputLocations.end()) {
                        combinedStorage.insert( combinedStorage.end(), iit->second.begin(), iit->second.end() );
                    }
                    iit = inputLocations.find(DP_INCOHERENT_STOKES);
                    if (iit != inputLocations.end()) {
                        combinedStorage.insert( combinedStorage.end(), iit->second.begin(), iit->second.end() );
                    }

                    if (!combinedStorage.empty()) {
                        std::vector<storageResult> scResult = data.addStorageToTask(pipe, dpit->first, combinedStorage, false);
                        scResultTotal.insert(scResultTotal.end(), scResult.begin(), scResult.end());
                    }
                    else bResult = false; // no input data for task
                }
                else {
                    // get the locations used for the input data product from which this output data product type is generated
                    // for the moment this typically will be DP_CORRELATED_UV
                    storageMap::const_iterator iit = inputLocations.find(DP_CORRELATED_UV);
                    if (iit != inputLocations.end()) {
                        std::vector<storageResult> scResult = data.addStorageToTask(pipe, dpit->first, iit->second, false);
                        scResultTotal.insert(scResultTotal.end(), scResult.begin(), scResult.end());
                    }
                    else bResult = false; // storage locations for input data product type not found
                }
            }
            if (scResultTotal.empty()) { // no conflicts?
                // clear the task's storage conflicts
                pipe->clearAllStorageConflicts();
                taskStorage->clearStorageCheckResults();
                bResult = true;
            }
            else {
                taskStorage->setStorageCheckResult(scResultTotal);
                for (std::vector<storageResult>::const_iterator confit = scResultTotal.begin(); confit != scResultTotal.end(); ++confit) {
                    pTask->setConflict(confit->conflict);
                    taskStorage->setOutputDataProductAssigned(confit->dataProductType, false);
                    // add the conflict to the conflicts dialog
                    itsConflictDialog->addStorageConflict(pTask, confit->dataProductType, confit->conflict, confit->storageNodeID, confit->storageRaidID);
                }
                bResult = false;
            }
        }
        else { // for observations and other non-pipeline tasks
            std::map<dataProductTypes, int> minimumNrNodes = taskStorage->getMinimumNrOfStorageNodes();
            sortMode sort_mode;
            std::vector<int> emptyVec;
            std::vector<int> &preferred_nodes = emptyVec;
            std::vector<int> extra_nodes, usable_storage_nodes(itsDMConnection->getUsableStorageNodeIDs()); // all existing storage node IDs

            // the distribution algorithm used
            const storageNodeDistribution &distribution(theSchedulerSettings.getStorageDistribution());
            if (distribution == STORAGE_DISTRIBUTION_FLAT_USAGE) {
                sort_mode = SORT_USAGE;
            }
            else if (distribution == STORAGE_DISTIBUTION_LEAST_FRAGMENTED) {
                sort_mode = SORT_SIZE;
            }
            else sort_mode = SORT_NONE;

            // storage node selection by preferred project nodes?
            storage_selection_mode mode(taskStorage->getStorageSelectionMode());
            bool project_preferred_nodes;
            if ((mode == STORAGE_MODE_MAXIMUM_PROJECT_PREFERRED) || (mode == STORAGE_MODE_MINIMUM_PROJECT_PREFERRED)) project_preferred_nodes = true;
            else project_preferred_nodes = false;

            const preferredDataProductStorageMap &pdps(theSchedulerSettings.getPreferredDataProductStorage());
            const preferredProjectStorageMap &pps(theSchedulerSettings.getPreferredProjectStorage());
            int project_id(theSchedulerSettings.getCampaignInfo(pTask->getProjectName()).id);

            preferredDataProductStorageMap::const_iterator pnit;
            preferredProjectStorageMap::const_iterator ppit;

            // ***********************************************************************
            // ******************* SEARCH FOR SUITABLE LOCATIONS *********************
            // ***********************************************************************

            // STEP2: search storageLocations for all dataProducts in sequence according to individual file size (large to small) of the dataProduct (i.e. sequence of sortedDataFiles)

            if (project_preferred_nodes) {
                ppit = pps.find(project_id);
                if (ppit != pps.end()) {
                    if (ppit->second.empty()) {
                        preferred_nodes = usable_storage_nodes;
                    }
                    else {
                        preferred_nodes = ppit->second;
                        for (std::vector<int>::const_iterator asit = usable_storage_nodes.begin(); asit != usable_storage_nodes.end(); ++asit) {
                            if (std::find(preferred_nodes.begin(), preferred_nodes.end(), *asit) == preferred_nodes.end()) {
                                extra_nodes.push_back(*asit);
                            }
                        }
                    }
                }
                else {
                    preferred_nodes = usable_storage_nodes;
                }
            }

            dataFileMap::const_iterator dfit;
            for (std::map<dataProductTypes, int>::const_iterator dpit = minimumNrNodes.begin(); dpit != minimumNrNodes.end(); ++dpit) {
                if (!project_preferred_nodes) {
                    // data type preferred nodes
                    pnit = pdps.find(dpit->first); // are there preferred storage nodes for this data product type specified?
                    if (pnit != pdps.end()) {
                        if (pnit->second.empty()) {
                            preferred_nodes = usable_storage_nodes;
                        }
                        preferred_nodes = pnit->second;
                        for (std::vector<int>::const_iterator asit = usable_storage_nodes.begin(); asit != usable_storage_nodes.end(); ++asit) {
                            if (std::find(preferred_nodes.begin(), preferred_nodes.end(), *asit) == preferred_nodes.end()) {
                                extra_nodes.push_back(*asit);
                            }
                        }
                    }
                    else {
                        preferred_nodes = usable_storage_nodes;
                    }
                }

                if (dpit->second == -1) {  // the bandwidth required for a single file of this dataproduct exceeds the single storage node network bandwidth
                    pTask->setConflict(CONFLICT_STORAGE_SINGLE_FILE_BW_TOO_HIGH);
                    itsConflictDialog->addStorageConflict(pTask, dpit->first, CONFLICT_STORAGE_SINGLE_FILE_BW_TOO_HIGH);
                    bResult = false;
                    break;
                }
                // get number of available nodes
                unsigned nrOfAvailableNodes(Controller::theSchedulerSettings.getNrOfStorageNodesAvailable());
                if (nrOfAvailableNodes == 0) {
                    pTask->setConflict(CONFLICT_STORAGE_NO_NODES);
                    itsConflictDialog->addStorageConflict(pTask, dpit->first, CONFLICT_STORAGE_NO_NODES);
                    return false; // no sense in continuing when no storage nodes are available
                }

                minNrOfNodes = dpit->second; // the minimum number of storage nodes REQUIRED for this data product
                dfit = dataFileSizes.find(dpit->first);
                storageLocationOptions preferred_locations, extra_locations;
                bool found_sufficient_nodes(false);
                if (dfit != dataFileSizes.end()) {
                    nrFiles = dfit->second.second; // dataFileSizes.at(dpit->first).second;
                    unsigned minNrFilesPerNode = static_cast<unsigned>(ceil((float)nrFiles / nrOfAvailableNodes)); // the minimum number of files that have to be written to one storage node if all available nodes are used
                    unsigned maxNrFilesPerNode = static_cast<unsigned>(floor((float)nrFiles / minNrOfNodes)); // the maximum number of files that can be written to one storage node according to bandwidth limitations
                    nrFilesPerNode = minNrFilesPerNode;
                    singleFileBW = dfit->second.first / (double) pTask->getDuration().totalSeconds() * 8; // kbit/sec

			// calculate the mininum number of files that have to fit on one storage node
			if (nrOfAvailableNodes >= minNrOfNodes) {
				if (minNrFilesPerNode <= maxNrFilesPerNode) {
					while (!found_sufficient_nodes) {
						preferred_locations = data.getStorageLocationOptions(dpit->first, pTask->getScheduledStart(), pTask->getScheduledEnd(), dfit->second.first, singleFileBW, nrFilesPerNode, sort_mode, preferred_nodes);
						if (preferred_locations.size() <= minNrOfNodes) {
							extra_locations = data.getStorageLocationOptions(dpit->first, pTask->getScheduledStart(), pTask->getScheduledEnd(), dfit->second.first, singleFileBW, nrFilesPerNode, sort_mode, extra_nodes);
                            if (((preferred_locations.size() + extra_locations.size()) * nrFilesPerNode >= nrFiles) && (preferred_locations.size() + extra_locations.size() >= minNrOfNodes)) {
								found_sufficient_nodes = true;
								break;
							}
						}
						else {
							if (preferred_locations.size() * nrFilesPerNode >= nrFiles) {
								found_sufficient_nodes = true;
								break;
							}
						}
						if (++nrFilesPerNode > maxNrFilesPerNode) { // nr files per node too high will exceed bandwidth to node
							break;
						}
					}
                            if (!found_sufficient_nodes) {
                                taskStorage->setStorageCheckResult(data.getLastStorageCheckResult());
                                pTask->setConflict(CONFLICT_STORAGE_NO_OPTIONS);
                                itsConflictDialog->addStorageConflict(pTask, dpit->first, CONFLICT_STORAGE_NO_OPTIONS);
                                bResult = false;
                                break;
                            }
                        }
                        else {
                            pTask->setConflict(CONFLICT_STORAGE_TOO_FEW_NODES);
                            itsConflictDialog->addStorageConflict(pTask, dpit->first, CONFLICT_STORAGE_TOO_FEW_NODES);
                            bResult = false;
                            break;
                        }

                        if (bResult) {
                            // ***********************************************************************
                            // ************* DISTRIBUTION OF DATA OVER STORAGE NODES *****************
                            // ***********************************************************************
                            // use the maximum number of suitable storage nodes (= suitable_locations.size())
                            bool sufficient_locations(false);
                            storageVector locations;
                            unsigned maxFilesToNodes(MAX_UNSIGNED);

                            if ((mode == STORAGE_MODE_MAXIMUM_DATA_TYPE_PREFERRED) || (mode == STORAGE_MODE_MAXIMUM_PROJECT_PREFERRED)) {
                                for (storageLocationOptions::const_iterator sit = preferred_locations.begin(); sit != preferred_locations.end(); ++sit) { // iterate over available locations
                                    for (nodeStorageOptions::const_iterator nsit = sit->second.begin(); nsit != sit->second.end(); ++nsit) { // iterates over the raids
                                        locations.push_back(storageVector::value_type(sit->first, nsit->raidID)); // use only the first raid option of each storage node available
                                        // also determine the maximum number of files that can be written to a single suitable node, needed to determine the minimum number of extra (non-preferred) nodes needed in addition to the preferred nodes
                                        maxFilesToNodes = std::min(maxFilesToNodes, nsit->nrUnits);
                                        if (locations.size() * nrFilesPerNode >= nrFiles) {
                                            sufficient_locations = true;
                                            break; // don't assign more storage nodes than the number of files written
                                        }
                                    }
                                    if (sufficient_locations) break;
                                }
                                if (!sufficient_locations && (locations.size() < minNrOfNodes)) { // do we need extra locations (non preferred nodes)? If so, use as few as possible of these
                                    for (storageLocationOptions::const_iterator sit = extra_locations.begin(); sit != extra_locations.end(); ++sit) {
                                        for (nodeStorageOptions::const_iterator nsit = sit->second.begin(); nsit != sit->second.end(); ++nsit) {
                                            locations.push_back(storageVector::value_type(sit->first, nsit->raidID));
                                            maxFilesToNodes = std::min(maxFilesToNodes, nsit->nrUnits);
                                            if ((locations.size() * maxFilesToNodes >= nrFiles) & (locations.size() >= minNrOfNodes)) {
                                                sufficient_locations = true;
                                                break; // don't assign more storage nodes than the number of files written
                                            }
                                        }
                                        if (sufficient_locations) break;
                                    }
                                }
                            }


                            else if ((mode == STORAGE_MODE_MINIMUM_DATA_TYPE_PREFERRED) || (mode == STORAGE_MODE_MINIMUM_PROJECT_PREFERRED)) {
                                bool inserted(false);
                                vector<std::pair<int, storageOption> > smallest_vec; // first = node ID
                                for (storageLocationOptions::const_iterator sit = preferred_locations.begin(); sit != preferred_locations.end(); ++sit) {
                                    for (nodeStorageOptions::const_iterator nsit = sit->second.begin(); nsit != sit->second.end(); ++nsit) {
                                        maxFilesToNodes = std::min(maxFilesToNodes, nsit->nrUnits); // determine the maximum number of files that can be written to a single suitable node
                                        for (vector<std::pair<int, storageOption> >::iterator ssit = smallest_vec.begin(); ssit != smallest_vec.end(); ++ssit) {
                                            if (nsit->remainingSpacekB < ssit->second.remainingSpacekB) { // sort according to free space in smallest_vec
                                                smallest_vec.insert(ssit, std::pair<int, storageOption>(sit->first, *nsit)); // insert the smallest free space raid arrays in smallest_vec
                                                inserted = true;
                                                break;
                                            }
                                        }
                                        if (!inserted) {
                                            smallest_vec.push_back(std::pair<int, storageOption>(sit->first, *nsit)); // put at the end (it's has the largest free space up to now)
                                            break; // only one raid array per node here
                                        }
                                    }
                                    if (smallest_vec.size() * nrFilesPerNode >= nrFiles) {
                                        sufficient_locations = true;
                                        break; // don't assign more storage nodes than the number of files written
                                    }
                                }

                                if (!sufficient_locations) { // do we need extra locations (non preferred nodes)? If so, use as few as possible of these
                                    for (storageLocationOptions::const_iterator sit = extra_locations.begin(); sit != extra_locations.end(); ++sit) {
                                        for (nodeStorageOptions::const_iterator nsit = sit->second.begin(); nsit != sit->second.end(); ++nsit) {
                                            maxFilesToNodes = std::min(maxFilesToNodes, nsit->nrUnits);
                                            for (vector<std::pair<int, storageOption> >::iterator ssit = smallest_vec.begin(); ssit != smallest_vec.end(); ++ssit) {
                                                if (nsit->remainingSpacekB < ssit->second.remainingSpacekB) { // sort according to free space in smallest_vec
                                                    smallest_vec.insert(ssit, std::pair<int, storageOption>(sit->first, *nsit)); // insert the smallest free space raid arrays in smallest_vec
                                                    inserted = true;
                                                    break;
                                                }
                                            }
                                            if (!inserted) {
                                                smallest_vec.push_back(std::pair<int, storageOption>(sit->first, *nsit)); // put at the end (it has the largest free space up to now)
                                                break; // only one raid array per node here
                                            }
                                        }
                                        if ((smallest_vec.size() * maxFilesToNodes >= nrFiles) & (smallest_vec.size() >= minNrOfNodes)) {
                                            break; // don't assign more storage nodes than the number of files written
                                        }
                                    }
                                }
                                for (unsigned i = 0; i < minNrOfNodes; ++i) {
                                    locations.push_back(pair<int,int>(smallest_vec.at(i).first, smallest_vec.at(i).second.raidID));
                                }
                            }

                            // report the storage to the storage nodes which will also set the storage in the task
                            data.addStorageToTask(pTask, dpit->first, locations, true);
                        }
                    }
                    else {
                        pTask->setConflict(CONFLICT_STORAGE_TOO_FEW_NODES);
                        itsConflictDialog->addStorageConflict(pTask, dpit->first, CONFLICT_STORAGE_TOO_FEW_NODES);
                        bResult = false;
                        break;
                    }
                }
            }
        }
    }
    return bResult;
}

bool Controller::assignGroupedStorage(void) { // not for manual assignment of storage
	bool bResult(true);
	std::map<unsigned, std::vector<Task *> > groupedTasks = data.getGroupedTasks(Task::PRESCHEDULED);

	std::vector<unsigned> emptyGroups;
	if (!groupedTasks.empty()) {
		std::vector<Task *> subGroupTasks;
        TaskStorage *tStorage;
		for (std::map<unsigned, std::vector<Task *> >::iterator groupIt = groupedTasks.begin(); groupIt != groupedTasks.end(); ++groupIt) {
			for (std::vector<Task *>::const_iterator taskit = groupIt->second.begin(); taskit != groupIt->second.end(); ++taskit) {
                if ((*taskit)->hasStorage()) {
                    tStorage = (*taskit)->storage();
                    if (tStorage->getStorageSelectionMode() != STORAGE_MODE_MANUAL) { // don't assign grouped storage to tasks that have manual storage assignment
                        data.removeStorageForTask((*taskit)->getID());
                        tStorage->unAssignStorage();
                        tStorage->clearStorageCheckResults();
                        (*taskit)->clearAllStorageConflicts();
                        subGroupTasks.push_back(*taskit);
                    }
                }
            }
			if (subGroupTasks.empty()) {
				emptyGroups.push_back(groupIt->first);
			}
			else {
				groupIt->second = subGroupTasks; // removes all 'manual storage' tasks
				subGroupTasks.clear();
			}
		}
		// remove groups that have no tasks with automatic storage assignment left
		for (std::vector<unsigned>::const_iterator eit = emptyGroups.begin(); eit != emptyGroups.end(); ++eit) {
			groupedTasks.erase(*eit);
		}
		if (groupedTasks.empty()) return bResult; // if nothing left return

	// get number of available storage nodes
	unsigned nrOfAvailableNodes(Controller::theSchedulerSettings.getNrOfStorageNodesAvailable());
	if (nrOfAvailableNodes > 0) {
		sortMode sort_mode;

		// the distribution algorithm used
		const storageNodeDistribution &distribution(theSchedulerSettings.getStorageDistribution());
		if (distribution == STORAGE_DISTRIBUTION_FLAT_USAGE) {
			sort_mode = SORT_USAGE;
		}
		else if (distribution == STORAGE_DISTIBUTION_LEAST_FRAGMENTED) {
			sort_mode = SORT_SIZE;
		}
		else sort_mode = SORT_NONE;

		const preferredDataProductStorageMap &pdps(theSchedulerSettings.getPreferredDataProductStorage());
		const preferredProjectStorageMap &pps(theSchedulerSettings.getPreferredProjectStorage());
		std::vector<int> extra_nodes, usable_storage_nodes(itsDMConnection->getUsableStorageNodeIDs()); // all existing storage node IDs


	//determine grouped tasks combined storage needs and settings
	dataFileMap combinedOutput;
    storage_selection_mode mode;
	std::map<dataProductTypes, int> combinedMinimumNrNodes;
	std::map<dataProductTypes, double> maxSingleFileBW;
	std::map<dataProductTypes, double>::iterator bwit;
	dataFileMap::iterator dfit;
	std::map<dataProductTypes, int>::iterator mit;
	int project_id(0);
	std::pair<unscheduled_reasons, QString> error;
	for (std::map<unsigned, std::vector<Task *> >::const_iterator groupIt = groupedTasks.begin(); groupIt != groupedTasks.end(); ++groupIt) { // 2
		combinedOutput.clear();
		combinedMinimumNrNodes.clear();
		if (!groupIt->second.empty()) {
            mode = groupIt->second.front()->storage()->getStorageSelectionMode();
			project_id = theSchedulerSettings.getCampaignInfo(groupIt->second.front()->getProjectName()).id; // should be the same for all tasks within group (no check done)
			for (std::vector<Task *>::const_iterator taskit = groupIt->second.begin(); taskit != groupIt->second.end(); ++taskit) { // 3
                TaskStorage *task_storage((*taskit)->storage());
				if ((*taskit)->isPipeline()) {
                    Pipeline *pipe(static_cast<Pipeline *>(*taskit));
                    error = setInputFilesForPipeline(pipe);
					if (error.first == NO_ERROR) {
                        pipe->calculateDataFiles();
                        const storageMap &inputStorageLocations(task_storage->getInputStorageLocations());
						storageMap::const_iterator inpcorit = inputStorageLocations.find(DP_CORRELATED_UV);
						if (inpcorit != inputStorageLocations.end()) {
                            if (task_storage->isOutputDataProduktEnabled(DP_INSTRUMENT_MODEL)) { // set output storage nodes equal to input storage nodes for these type of data products
                                std::vector<storageResult> result = data.addStorageToTask(pipe, DP_INSTRUMENT_MODEL, inpcorit->second, false);
								if (!result.empty()) {
									for (std::vector<storageResult>::const_iterator sit = result.begin(); sit != result.end(); ++sit) {
										if (sit->conflict != CONFLICT_NO_CONFLICT) {
                                            itsConflictDialog->addStorageConflict(pipe, sit->dataProductType, sit->conflict);
										}
                                        pipe->setConflict(sit->conflict);
									}
									bResult = false;
								}
							}
                            if (task_storage->isOutputDataProduktEnabled(DP_CORRELATED_UV)) {

								// inpcorit->second bevat alle storage nodes, dus ook die van de eerdere SAPs waardoor
								// dit dataprodukt de storage nodes van SAP000 van de input krijgt en niet die van SAP001
                                std::vector<storageResult> result = data.addStorageToTask(pipe, DP_CORRELATED_UV, inpcorit->second, false);
								if (!result.empty()) {
									for (std::vector<storageResult>::const_iterator sit = result.begin(); sit != result.end(); ++sit) {
										if (sit->conflict != CONFLICT_NO_CONFLICT) {
                                            itsConflictDialog->addStorageConflict(pipe, sit->dataProductType, sit->conflict);
										}
                                        pipe->setConflict(sit->conflict);
									}
									bResult = false;
								}
							}

                            task_storage->generateFileList();
						}
					}
				}

				double taskDuration((*taskit)->getDuration().totalSeconds() * 8);
                const dataFileMap &dataFileSizes = task_storage->getOutputFileSizes();
                if (task_storage->getStorageSelectionMode() != mode) {
					itsConflictDialog->addConflict(*taskit, CONFLICT_GROUP_STORAGE_MODE_DIFFERENT);
					bResult = false;
				}
				if (!dataFileSizes.empty()) {
					// summing individual file size for same data product types
					for (dataFileMap::const_iterator dit = dataFileSizes.begin(); dit != dataFileSizes.end(); ++dit) { // 4
						if ((*taskit)->isObservation() || ((dit->first != DP_INSTRUMENT_MODEL) && (dit->first != DP_CORRELATED_UV))) { // skip this for instrument model and correlated, which is already set above
							dfit = combinedOutput.find(dit->first);
							if (dfit != combinedOutput.end()) {
								dfit->second.first += dit->second.first;

							}
							else {
								combinedOutput[dit->first] = std::pair<double, unsigned>(dit->second.first, dit->second.second);
							}
							//determine the maximum single file bandwidth for each data product type in the group of tasks
							double currentBW = dit->second.first / taskDuration; // kbit/sec
							bwit = maxSingleFileBW.find(dit->first);
							if (bwit != maxSingleFileBW.end()) {
								bwit->second = std::max(bwit->second, currentBW);
							}
							else {
                                maxSingleFileBW[dit->first] = currentBW;
							}
						}
					}
					// determining the overall minimum number of nodes needed for each data product type
                    std::map<dataProductTypes, int> minimumNrnodes = task_storage->getMinimumNrOfStorageNodes();
					for (std::map<dataProductTypes, int>::const_iterator minit = minimumNrnodes.begin(); minit != minimumNrnodes.end(); ++minit) {
						if ((*taskit)->isObservation() || ((minit->first != DP_INSTRUMENT_MODEL) && (minit->first != DP_CORRELATED_UV))) { // skip for instrument model and correlated,they have been set
							if (minit->second == -1) {
								(*taskit)->setConflict(CONFLICT_STORAGE_SINGLE_FILE_BW_TOO_HIGH);
								itsConflictDialog->addStorageConflict((*taskit), minit->first, CONFLICT_STORAGE_SINGLE_FILE_BW_TOO_HIGH);
								bResult = false;
							}
							else {
								mit = combinedMinimumNrNodes.find(minit->first);
								if (mit != combinedMinimumNrNodes.end()) {
									mit->second = std::max(mit->second, minit->second);
								}
								else {
									combinedMinimumNrNodes[minit->first] = minit->second;
								}
							}
						}
					}
				}
				else {
					itsConflictDialog->addConflict(*taskit, CONFLICT_STORAGE_NO_DATA);
					bResult = false;
				}
			} // END 3
		}
		else {
			std::cout << "Controller::assignGroupedStorage: Warning: trying to assign storage to group:" << groupIt->first << " in which there are no tasks" << std::endl;
			continue;
		}

	if (bResult) { // only do the actual group storage assignment if no conflicts are found

	if (!combinedOutput.empty()) {

		// storage node selection by preferred project nodes?
		//	storage_selection_mode mode(pTask->getStorageSelectionMode());
		bool project_preferred_nodes;
		if ((mode == STORAGE_MODE_MAXIMUM_PROJECT_PREFERRED) || (mode == STORAGE_MODE_MINIMUM_PROJECT_PREFERRED)) project_preferred_nodes = true;
		else project_preferred_nodes = false;


//	double singleFileBW;
	unsigned nrFiles(0), minNrOfNodes(0), nrFilesPerNode(0);

	std::vector<int> preferred_nodes;// = emptyVec;


	// ***********************************************************************
	// ******************* SEARCH FOR SUITABLE LOCATIONS *********************
	// ***********************************************************************

	// STEP2: search storageLocations for all dataProducts in sequence according to individual file size (large to small) of the dataProduct (i.e. sequence of sortedDataFiles)

	preferredDataProductStorageMap::const_iterator pnit;
	preferredProjectStorageMap::const_iterator ppit;

	if (project_preferred_nodes) {
		ppit = pps.find(project_id);
		if (ppit != pps.end()) {
			if (ppit->second.empty()) {
				preferred_nodes = usable_storage_nodes;
			}
			else {
				preferred_nodes = ppit->second;
				for (std::vector<int>::const_iterator asit = usable_storage_nodes.begin(); asit != usable_storage_nodes.end(); ++asit) {
					if (std::find(preferred_nodes.begin(), preferred_nodes.end(), *asit) == preferred_nodes.end()) {
						extra_nodes.push_back(*asit);
					}
				}
			}
		}
		else {
			preferred_nodes = usable_storage_nodes;
		}
	}

	storageLocationOptions preferred_locations, extra_locations, common_pref_locations, common_extra_locations;
	for (std::map<dataProductTypes, int>::const_iterator dpit = combinedMinimumNrNodes.begin(); dpit != combinedMinimumNrNodes.end(); ++dpit) { // 5
		for (std::vector<Task *>::const_iterator taskit = groupIt->second.begin(); taskit != groupIt->second.end(); ++taskit) { //6

		if (!project_preferred_nodes) {
			pnit = pdps.find(dpit->first); // are there preferred storage nodes for this data product type specified?
			if (pnit != pdps.end()) {
				if (pnit->second.empty()) {
					preferred_nodes = usable_storage_nodes;
				}
				else {
					preferred_nodes = pnit->second;
					for (std::vector<int>::const_iterator asit = usable_storage_nodes.begin(); asit != usable_storage_nodes.end(); ++asit) {
						if (std::find(preferred_nodes.begin(), preferred_nodes.end(), *asit) == preferred_nodes.end()) {
							extra_nodes.push_back(*asit);
						}
					}
				}
			}
			else {
				preferred_nodes = usable_storage_nodes;
			}
		}

		// now search for storage locations using the combined storage requirements (only search ones, not for all tasks separately,
		// final check will be done when really assigning the storage to each grouped task
		minNrOfNodes = dpit->second; // the minimum number of storage nodes REQUIRED for this data product
		dfit = combinedOutput.find(dpit->first);
		bwit = maxSingleFileBW.find(dpit->first);
		if (dfit != combinedOutput.end()) {
			nrFiles = dfit->second.second;
			unsigned minNrFilesPerNode = static_cast<unsigned>(ceil((float)nrFiles / nrOfAvailableNodes)); // the minimum number of files the selected nodes should be able to hold
			unsigned maxNrFilesPerNode = static_cast<unsigned>(floor((float)nrFiles / minNrOfNodes)); // the maximum number of files that can be written to one storage node according to bandwidth limitations
			nrFilesPerNode = minNrFilesPerNode;

			// calculate the minimum number of files that have to fit on one storage node
			if (nrOfAvailableNodes >= minNrOfNodes) {
				if (minNrFilesPerNode <= maxNrFilesPerNode) {
					preferred_locations = data.getStorageLocationOptions(dpit->first, (*taskit)->getScheduledStart(), (*taskit)->getScheduledEnd(), dfit->second.first, bwit->second, nrFilesPerNode, sort_mode, preferred_nodes);
					extra_locations = data.getStorageLocationOptions(dpit->first, (*taskit)->getScheduledStart(), (*taskit)->getScheduledEnd(), dfit->second.first, bwit->second, nrFilesPerNode, sort_mode, extra_nodes);

                    if (preferred_locations.size() + extra_locations.size() >= minNrOfNodes) {

					preferred_nodes.clear();
					extra_nodes.clear();
					for (storageLocationOptions::const_iterator cpit = preferred_locations.begin(); cpit != preferred_locations.end(); ++cpit) {
						preferred_nodes.push_back(cpit->first);
					}
					for (storageLocationOptions::const_iterator cpit = extra_locations.begin(); cpit != extra_locations.end(); ++cpit) {
						extra_nodes.push_back(cpit->first);
					}

					if (preferred_nodes.empty() && extra_nodes.empty()) {
						(*taskit)->setConflict(CONFLICT_STORAGE_NO_OPTIONS);
						itsConflictDialog->addStorageConflict((*taskit), dpit->first, CONFLICT_STORAGE_NO_OPTIONS);
						bResult = false;
						break;
					}

					if (nrFilesPerNode > maxNrFilesPerNode) { // nr files per node too high will exceed bandwidth to node
						(*taskit)->setConflict(CONFLICT_STORAGE_EXCEEDS_BANDWIDTH);
						itsConflictDialog->addStorageConflict((*taskit), dpit->first, CONFLICT_STORAGE_EXCEEDS_BANDWIDTH);
						bResult = false;
						break;
					}
                }
                    else {
                        (*taskit)->setConflict(CONFLICT_STORAGE_MINIMUM_NODES);
                        itsConflictDialog->addStorageConflict((*taskit), dpit->first, CONFLICT_STORAGE_MINIMUM_NODES);
                        bResult = false;
                        break;
                    }
				}
				else {
					(*taskit)->setConflict(CONFLICT_STORAGE_EXCEEDS_BANDWIDTH);
					itsConflictDialog->addStorageConflict((*taskit), dpit->first, CONFLICT_STORAGE_EXCEEDS_BANDWIDTH);
					bResult = false;
					break;
				}
			}
			else {
				(*taskit)->setConflict(CONFLICT_STORAGE_TOO_FEW_NODES);
				itsConflictDialog->addStorageConflict((*taskit), dpit->first, CONFLICT_STORAGE_TOO_FEW_NODES);
				bResult = false;
				break;
			}
		}


	if (bResult) {
		// check which nodes they have in common, assign common nodes with checking!
		// ***********************************************************************
		// ************* DISTRIBUTION OF DATA OVER STORAGE NODES *****************
		// ***********************************************************************
		// use the maximum number of suitable storage nodes (= suitable_locations.size())
		// only keep the locations that are common to all tasks for this data product
		if (taskit == groupIt->second.begin()) { // for first task search, keep all options
			common_pref_locations = preferred_locations;
			common_extra_locations = extra_locations;
		}
		else {
			storageLocationOptions new_common_pref_locations, new_common_extra_locations;
			for (storageLocationOptions::const_iterator sit = common_pref_locations.begin(); sit != common_pref_locations.end(); ++sit) {
				for (nodeStorageOptions::const_iterator nsit = sit->second.begin(); nsit != sit->second.end(); ++nsit) { // iterates over the raids
					if (storageLocationsContains(preferred_locations, sit->first, nsit->raidID)) {
						new_common_pref_locations.push_back(*sit);
					}
				}
			}
			common_pref_locations = new_common_pref_locations;
			for (storageLocationOptions::const_iterator sit = common_extra_locations.begin(); sit != common_extra_locations.end(); ++sit) {
				for (nodeStorageOptions::const_iterator nsit = sit->second.begin(); nsit != sit->second.end(); ++nsit) { // iterates over the raids
					if (storageLocationsContains(extra_locations, sit->first, nsit->raidID)) {
						new_common_extra_locations.push_back(*sit);
					}
				}
			}
			common_extra_locations = new_common_extra_locations;
		}
		}
	} // 6, end of search for all task in this group for the current data product


		unsigned maxFilesToNodes(MAX_UNSIGNED);
		storageVector locations;

		bool sufficient_locations(false);
		// select enough locations from the common location solutions
		if ((mode == STORAGE_MODE_MAXIMUM_DATA_TYPE_PREFERRED) || (mode == STORAGE_MODE_MAXIMUM_PROJECT_PREFERRED)) {
			for (storageLocationOptions::const_iterator sit = common_pref_locations.begin(); sit != common_pref_locations.end(); ++sit) { // iterate over available locations
				for (nodeStorageOptions::const_iterator nsit = sit->second.begin(); nsit != sit->second.end(); ++nsit) { // iterates over the raids
					locations.push_back(storageVector::value_type(sit->first, nsit->raidID)); // use only the first raid option of each storage node available
					// also determine the maximum number of files that can be written to a single suitable node, needed to determine the minimum number of extra (non-preferred) nodes needed in addition to the preferred nodes
					maxFilesToNodes = std::min(maxFilesToNodes, nsit->nrUnits);
					if (locations.size() * nrFilesPerNode >= nrFiles) {
						sufficient_locations = true;
//						break; // don't assign more storage nodes than the number of files written
					}

				}
//				if (sufficient_locations) break;
			}
			if (!sufficient_locations && (locations.size() < minNrOfNodes)) { // do we need extra locations (non preferred nodes)? If so, use as few as possible of these
				for (storageLocationOptions::const_iterator sit = common_extra_locations.begin(); sit != common_extra_locations.end(); ++sit) {
					for (nodeStorageOptions::const_iterator nsit = sit->second.begin(); nsit != sit->second.end(); ++nsit) {
						locations.push_back(storageVector::value_type(sit->first, nsit->raidID));
						maxFilesToNodes = std::min(maxFilesToNodes, nsit->nrUnits);
						if ((locations.size() * maxFilesToNodes >= nrFiles) && (locations.size() >= minNrOfNodes)) {
							sufficient_locations = true;
							break; // don't assign more storage nodes than the number of files written
						}
					}
					if (sufficient_locations) break;
				}
			}
		}
			else if ((mode == STORAGE_MODE_MINIMUM_DATA_TYPE_PREFERRED) || (mode == STORAGE_MODE_MINIMUM_PROJECT_PREFERRED)) {
				bool inserted(false);
				vector<std::pair<int, storageOption> > smallest_vec; // first = node ID
				for (storageLocationOptions::const_iterator sit = common_pref_locations.begin(); sit != common_pref_locations.end(); ++sit) {
					for (nodeStorageOptions::const_iterator nsit = sit->second.begin(); nsit != sit->second.end(); ++nsit) {
						maxFilesToNodes = std::min(maxFilesToNodes, nsit->nrUnits); // determine the maximum number of files that can be written to a single suitable node
						for (vector<std::pair<int, storageOption> >::iterator ssit = smallest_vec.begin(); ssit != smallest_vec.end(); ++ssit) {
							if (nsit->remainingSpacekB < ssit->second.remainingSpacekB) { // sort according to free space in smallest_vec
								smallest_vec.insert(ssit, std::pair<int, storageOption>(sit->first, *nsit)); // insert the smallest free space raid arrays in smallest_vec
								inserted = true;
								break;
							}
						}
						if (!inserted) {
							smallest_vec.push_back(std::pair<int, storageOption>(sit->first, *nsit)); // put at the end (it's has the largest free space up to now)
							break; // only one raid array per node here
						}
					}
					if (smallest_vec.size() * nrFilesPerNode >= nrFiles) {
						sufficient_locations = true;
						break; // don't assign more storage nodes than the number of files written
					}
				}

				if (!sufficient_locations && (locations.size() < minNrOfNodes)) { // do we need extra locations (non preferred nodes)? If so, use as few as possible of these
					for (storageLocationOptions::const_iterator sit = common_extra_locations.begin(); sit != common_extra_locations.end(); ++sit) {
						for (nodeStorageOptions::const_iterator nsit = sit->second.begin(); nsit != sit->second.end(); ++nsit) {
							maxFilesToNodes = std::min(maxFilesToNodes, nsit->nrUnits);
							for (vector<std::pair<int, storageOption> >::iterator ssit = smallest_vec.begin(); ssit != smallest_vec.end(); ++ssit) {
								if (nsit->remainingSpacekB < ssit->second.remainingSpacekB) { // sort according to free space in smallest_vec
									smallest_vec.insert(ssit, std::pair<int, storageOption>(sit->first, *nsit)); // insert the smallest free space raid arrays in smallest_vec
									inserted = true;
									break;
								}
							}
							if (!inserted) {
								smallest_vec.push_back(std::pair<int, storageOption>(sit->first, *nsit)); // put at the end (it has the largest free space up to now)
								break; // only one raid array per node here
							}
						}
						if ((smallest_vec.size() * maxFilesToNodes >= nrFiles) & (smallest_vec.size() >= minNrOfNodes)) {
							break; // don't assign more storage nodes than the number of files written
						}
					}
				}
				for (unsigned i = 0; i < minNrOfNodes; ++i) {
					locations.push_back(pair<int,int>(smallest_vec.at(i).first, smallest_vec.at(i).second.raidID));
				}
			}

		// finally assign the common storage locations for this data product to all tasks in the group and check if the result is ok (i.e. no conflicts)
		for (std::vector<Task *>::const_iterator taskit = groupIt->second.begin(); taskit != groupIt->second.end(); ++taskit) { // 7
            if ((*taskit)->storage()->isOutputDataProduktEnabled(dpit->first)) {
				std::vector<storageResult> result = data.addStorageToTask(*taskit, dpit->first, locations, false);
				if (!result.empty()) {
					for (std::vector<storageResult>::const_iterator sit = result.begin(); sit != result.end(); ++sit) {
						if (sit->conflict != CONFLICT_NO_CONFLICT) {
							itsConflictDialog->addStorageConflict(*taskit, sit->dataProductType, sit->conflict);
						}
						(*taskit)->setConflict(sit->conflict);
					}
					bResult = false;
				}
			}
		} // END 7
	} // 5
	for (std::vector<Task *>::const_iterator taskit = groupIt->second.begin(); taskit != groupIt->second.end(); ++taskit) {
		if ((*taskit)->isPipeline()) {
            Pipeline *pipe(static_cast<Pipeline *>(*taskit));
            setInputFilesForPipeline(pipe);
			// we have to re-assign the storage for the pipeline because the predecessor (observation or pipeline) might have changed in the previous loop
			for (dataProductTypes dp = _BEGIN_DATA_PRODUCTS_ENUM_; dp < _END_DATA_PRODUCTS_ENUM_; dp = dataProductTypes(dp + 1)) {
                if (dp != DP_SKY_IMAGE && pipe->storage()->isOutputDataProduktEnabled(dp)) { // for SKY_IMAGE input nodes are not equal to output nodes
                    const storageMap &inputStorageLocations(pipe->storage()->getInputStorageLocations());
					storageMap::const_iterator inpcorit = inputStorageLocations.find(DP_CORRELATED_UV);
					if (inpcorit != inputStorageLocations.end()) {
                        data.addStorageToTask(*taskit, dp, inpcorit->second, false);
					}
				}
			}
		}
        (*taskit)->storage()->generateFileList();
	}

	}
	}
	} // END for loop over individual groups
	}
	else return false; // no storage nodes available
	}

	return bResult;
}


bool Controller::assignStorageResources(Task *task) {
	bool bResult(true);
	//	if (refreshStorageNodesInfo()) {
	if (task) {
		// assign storage only to the specified task

		gui->updateTaskDialog(task); // update task dialog if it shows the current task (needed for the storage tree to show the conflict info)
	}
	else {
		data.clearStorageClaims();
		// assign storage resources to all SCHEDULED tasks
		// step 1: iterate over all scheduled tasks and try to re-assign storage to these tasks according to their storage claims already set
		//		   check the user assigned resources. If insufficient storage is assigned notify user and ask how to continue.
		//         (options are: let me change/add resources for this task, ignore and continue (in which case the task is left on prescheduled status and its conflicts are updated,
		//          and the last option is abort assigning resources.
		const std::vector<Task *> scheduledTasks = data.getScheduledTasksSortStartTime();
		std::vector<storageResult> res;
		const AstroTime &minTime(Controller::theSchedulerSettings.getMinimumTimeBetweenTasks());

        TaskStorage *tStorage(0);
		for (std::vector<Task *>::const_iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
            tStorage = (*it)->storage();
            if (tStorage) {
                if ((*it)->getScheduledStart() >= now() + minTime) { // TODO time needs to be after a minimum time after now (minimum time needs to be added to settingsdialog)
                    res = data.addStorageToTask(*it, tStorage->getStorageLocations());
                    if (res.empty()) {
                        (*it)->clearAllStorageConflicts();
                    }
                    else {
                        for (std::vector<storageResult>::const_iterator scit = res.begin(); scit != res.end(); ++scit) {
                            itsConflictDialog->addStorageConflict(*it, scit->dataProductType, scit->conflict, scit->storageNodeID, scit->storageRaidID);
                        }
                        bResult = false; // tasks with status SCHEDULED should have storage assigned already and will only be checked if storage resources are still ok
                    }
                }
            }
        }

		// now try to do the same for prescheduled tasks
		const std::vector<Task *> preScheduledTasks = data.getPreScheduledTasksSortStartTime();
        for (std::vector<Task *>::const_iterator it = preScheduledTasks.begin(); it != preScheduledTasks.end(); ++it) {
            tStorage = (*it)->storage();
            if (tStorage) {
                if ((*it)->getScheduledStart() >= now() + minTime) { // TODO time needs to be after a minimum time after now (minimum time needs to be added to settingsdialog)
                    storage_selection_mode mode(tStorage->getStorageSelectionMode());
                    if (mode == STORAGE_MODE_MANUAL) { // both for observations and pipelines
                        if (assignManualStorageToTask(*it)) {
                            (*it)->clearAllStorageConflicts();
                        }
                        else bResult = false;
                        tStorage->generateFileList();
                    }
                    else if ((*it)->getGroupID() == 0) { // grouped observations are assigned resources together in automatic storage selection modes
                        if (!assignStorageToTask(*it))
                            bResult = false;
                        (*it)->clearAllStorageConflicts();
                        tStorage->generateFileList();
                    }
                }
            }
        }
    }


	// now assign storage to grouped OBSERVATIONS
	// observations in the same group get the same storage nodes per subband for correlated data
	// the actual assignment for tasks with automatic selected storage (non manual) is done later on.

	if (!assignGroupedStorage()) bResult = false;

	gui->updateTaskDialog(); // update task dialog (needed for the storage tree to show the conflict info)
	return bResult;
}


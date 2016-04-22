/*
 * schedulergui.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Feb 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/schedulergui.cpp $
 *
 */

#include <QtGui>
#include <QBoxLayout>
#include "qlofardatamodel.h"
#include <QTableView>
#include <QDesktopWidget>
#include <QLCDNumber>
#include <QFileDialog>
#include <sstream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cstdlib> // needed for system() call in publish()
#include "lofar_scheduler.h"
#include "schedulergui.h"
#include "schedulerdata.h"
#include "Controller.h"
#include "GraphicResourceScene.h"
#include "graphicstoragescene.h"
#include "GraphicTask.h"
#include "FileUtils.h"
#include "parsettreeviewer.h"
#include "shifttasksdialog.h"

using std::ofstream;
using std::string;
using std::endl;

const char * DATA_HEADERS[NR_DATA_HEADERS] = { "task ID", "SAS ID", "MoM ID", "group ID", "project ID", "task name", "planned start (UTC)", "planned end (UTC)", "duration",
        "task type", "task status" , "cluster", "error reason", "task description", "stations", "reservation", "priority", "fix day", "fix time",
		"first possible date", "last possible date", "window min time", "window max time", "antenna mode", "clock", "filter", "# subbands",
        "contact name", "phone", "e-mail", "predecessors", "pred. min time dif", "pred. max time dif", "night wf.", "data size"};

extern QString currentUser;


SchedulerGUI::SchedulerGUI(Controller *controller)
    : QMainWindow(0), itsSortColumn(PLANNED_START), itsSortOrder(Qt::DescendingOrder), itsModel(0), itsDelegate(0), itsSearchBar(0),
    itsSearchLineEdit(0), itsSearchNextButton(0), itsSearchPreviousButton(0), itsSearchColumnOnlyCheckBox(0),
    itsIsTestMode(false), itsIsSaveRequired(false), itsEnablePublish(false),
    itsController(controller)

    {
	ui.setupUi(this);

#if defined Q_OS_WINDOWS || _DEBUG_
    itsEnablePublish = true;
#endif

    if (currentUser == "lofarsys") {
    	itsEnablePublish = true;
    }

    itsEnablePublish = true;

	createMainToolbar();
	createGraphicDock();
	createTableDock();
//    createGraphicStorageDock();
	createStatusBar();
	createSearchBar();

	itsTaskDialog = new TaskDialog(this, itsController);
	itsTreeViewer = new ParsetTreeViewer();

	connectInternalSignals();


	// center the GUI in the middle of the current screen
	QRect geometry = this->geometry();
	QDesktopWidget * pDesktop = QApplication::desktop();
	QRect screenGeometry = pDesktop->screenGeometry(pDesktop->screenNumber());
	int xpos = (screenGeometry.width() - geometry.width()) / 2;
	int ypos = (screenGeometry.height() - geometry.height()) / 2;
	this->setGeometry(xpos, ypos, geometry.width(), geometry.height());
	clearCurrentFileName();
	itsWindowTitle = "LOFAR Scheduler";
	updateWindowTitle();

    // the timer to update the current time - line
    itsTimer = new QTimer(this);
    itsTimer->setSingleShot(false);
    connect(itsTimer, SIGNAL(timeout()), this, SLOT(updateCurrentTime()));
    itsTimer->setInterval(1000); // update every minute
    itsTimer->start();
}

SchedulerGUI::~SchedulerGUI()
{
	delete itsModel;
	delete itsSearchLineEdit;
	delete itsSearchNextButton;
	delete itsSearchPreviousButton;
	delete itsSearchColumnOnlyCheckBox;
	delete itsSearchBar;
	// status bar components
    delete itsStatusBarUsedDatabase;
	delete itsStatusBarProgress;
	delete itsStatusBarTaskStatus;
	delete itsStatusBarStatusText;
	delete itsStatusBar;
    // main toolbar
    delete itsLCDtimer;
    delete itsMainToolBar;
	//graphic view components
	delete itsZoomOutAction;
	delete itsZoomInAction;
	delete itsGraphicViewToolbar;
	delete itsGraphicView;
	delete itsGraphicResourceScene;
	delete itsGraphicDockMainLayout;
	delete itsGraphicDockWidgetContents;
	delete itsGraphicDock;
	// table view components
	delete itsTableView;
	delete closeSearchBarAct;
	delete itsSearchPreviousAct;
	delete itsSearchNextAct;
	delete itsSearchColumnOnlyCheckBoxAct;
	delete closeIcon;
	delete itsTaskDialog;
}

void SchedulerGUI::updateCurrentTime(void) {
    const QDateTime &UTC(QDateTime::currentDateTime().toUTC());
    itsGraphicResourceScene->updateCurrentTime(UTC);
    itsLCDtimer->display(UTC.toString("hh:mm:ss"));
}

void SchedulerGUI::createMainToolbar(void) {
    itsMainToolBar = new QToolBar(tr("Main"));
    addToolBar(Qt::TopToolBarArea, itsMainToolBar);
    itsMainToolBar->setMovable(false);

    // new schedule
    ui.action_New_schedule->setIcon(QIcon(tr(":/icons/new_schedule.png")));
    ui.action_New_schedule->setText(tr("New schedule"));
    itsMainToolBar->addAction(ui.action_New_schedule);
    // open schedule
    ui.action_Open_Schedule->setIcon(QIcon(tr(":/icons/open.png")));
    ui.action_Open_Schedule->setText(tr("Open schedule"));
    itsMainToolBar->addAction(ui.action_Open_Schedule);
    // save schedule
    ui.action_Save_schedule->setIcon(QIcon(tr(":/icons/save.png")));
    ui.action_Save_schedule->setText(tr("Save schedule"));
    itsMainToolBar->addAction(ui.action_Save_schedule);

    // undo
    itsMainToolBar->addSeparator();
    ui.action_Undo->setIcon(QIcon(tr(":/icons/undo.png")));
    ui.action_Undo->setText(tr("Undo"));
    itsMainToolBar->addAction(ui.action_Undo);
    //redo
    ui.action_Redo->setIcon(QIcon(tr(":/icons/redo.png")));
    ui.action_Redo->setText(tr("Redo"));
    itsMainToolBar->addAction(ui.action_Redo);
    // add
    ui.action_Add_task->setIcon(QIcon(tr(":/icons/add_task.png")));
    ui.action_Add_task->setText(tr("Add task"));
    itsMainToolBar->addAction(ui.action_Add_task);
    // delete
    ui.action_Delete_task->setIcon(QIcon(tr(":/icons/delete_task.png")));
    ui.action_Delete_task->setText(tr("Delete task"));
    itsMainToolBar->addAction(ui.action_Delete_task);
    //thrashcan
    ui.action_Thrashcan->setIcon(QIcon(tr(":/icons/empty_trash.png")));
    ui.action_Thrashcan->setText(tr("Show thrashcan"));
    itsMainToolBar->addAction(ui.action_Thrashcan);
    // find
    ui.action_Find->setIcon(QIcon(tr(":/icons/find.png")));
    ui.action_Find->setText(tr("Find"));
    itsMainToolBar->addAction(ui.action_Find);

    itsMainToolBar->addSeparator();
    // download schedule
    ui.action_DownloadSASSchedule->setIcon(QIcon(tr(":/icons/download.png")));
    ui.action_DownloadSASSchedule->setText(tr("Download current schedule"));
    itsMainToolBar->addAction(ui.action_DownloadSASSchedule);
    // synchronize with SAS
    ui.action_SyncSASSchedule->setIcon(QIcon(tr(":/icons/synchronize.png")));
    ui.action_SyncSASSchedule->setText(tr("Synchronize with SAS"));
    itsMainToolBar->addAction(ui.action_SyncSASSchedule);

    itsMainToolBar->addSeparator();
    // create initial schedule create_initial_schedule.png
    ui.action_Create_initial_schedule->setIcon(QIcon(tr(":/icons/create_initial_schedule.png")));
    ui.action_Create_initial_schedule->setText(tr("Create initial schedule"));
    itsMainToolBar->addAction(ui.action_Create_initial_schedule);
    // optimize schedule
    ui.action_Optimize_schedule->setIcon(QIcon(tr(":/icons/optimize.png")));
    ui.action_Optimize_schedule->setText(tr("Optimize schedule"));
    itsMainToolBar->addAction(ui.action_Optimize_schedule);
    // equalize schedule
    ui.action_Balance_schedule->setIcon(QIcon(tr(":/icons/load-balancing.png")));
    ui.action_Balance_schedule->setText(tr("Balance schedule"));
    itsMainToolBar->addAction(ui.action_Balance_schedule);
    // assign resources
    itsMainToolBar->addAction(ui.action_Assign_resources);

    itsMainToolBar->addSeparator();
    ui.actionPublish_schedule->setIcon(QIcon(tr(":/icons/publish.png")));
    ui.actionPublish_schedule->setText(tr("Publish schedule"));
    itsMainToolBar->addAction(ui.actionPublish_schedule);

    ui.actionPublish_schedule->setEnabled(itsEnablePublish);
    if (!itsEnablePublish) {
    	ui.actionPublish_schedule->setToolTip("Publishing is only enabled when you are lofarsys");
    }

    itsMainToolBar->addSeparator();
    ui.action_Schedule_Settings->setIcon(QIcon(tr(":/icons/settings.png")));
    ui.action_Schedule_Settings->setText(tr("Schedule settings"));
    itsMainToolBar->addAction(ui.action_Schedule_Settings);
    itsMainToolBar->addSeparator();
    ui.actionConnect_to_Data_monitor->setIcon(QIcon(tr(":/icons/connect_datamonitor.png")));
    ui.actionConnect_to_Data_monitor->setCheckable(true);
    itsMainToolBar->addAction(ui.actionConnect_to_Data_monitor);
    itsMainToolBar->addSeparator();
    // add empty space by using an empty widget with automatic stretch
    QWidget* empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    itsMainToolBar->addWidget(empty);
    itsLCDtimer = new QLCDNumber(itsMainToolBar);
    itsLCDtimer->setSegmentStyle(QLCDNumber::Flat);
    itsLCDtimer->setFrameStyle(QLCDNumber::Sunken);
    itsLCDtimer->setFrameShape(QLCDNumber::WinPanel);
    itsLCDtimer->setDigitCount(8);
    itsLCDtimer->setToolTip("current UTC");
    itsLCDtimer->display("00:00:00");
    itsMainToolBar->addWidget(itsLCDtimer);
}

void SchedulerGUI::setExistingProjects(const campaignMap &projects) {
	itsTaskDialog->setExistingProjects(projects);
    updateProjectsFilter(projects);
}

void SchedulerGUI::updateDataMonitorConnectionStatus(void) {
	ui.actionConnect_to_Data_monitor->setChecked(itsController->isDataMonitorConnected());
}


void SchedulerGUI::setSaveRequired(bool save_required) {
	if (save_required != itsIsSaveRequired) {
		itsIsSaveRequired = save_required;
		updateWindowTitle();
		if (save_required) {
			ui.action_Save_schedule->setEnabled(true);
		}
		else {
			ui.action_Save_schedule->setEnabled(false);
		}
	}
}

/*
void SchedulerGUI::setTestMode(bool enable_test_mode) {
	if (enable_test_mode) {
		gui->setWindowTitle("LOFAR Scheduler (" + itsCurrentFileName + ")");
	}
	else {
		gui->setWindowTitle("** TEST MODE ** LOFAR Scheduler (" + itsCurrentFileName + ")");
	}
}
*/

void SchedulerGUI::updateWindowTitle(void) {
	QString fullTitle(itsWindowTitle + " (" + itsCurrentFileName);
	if (itsIsSaveRequired) {
		fullTitle += "*)";
	}
	else {
		fullTitle += ")";
	}
	if (itsIsTestMode) {
		fullTitle += " ### TEST MODE ###";
	}
	setWindowTitle(fullTitle);
}

void SchedulerGUI::showTaskDialog(const Task *task, tabIndex tab) {
	itsTaskDialog->show(task, tab);
}

void SchedulerGUI::multiEditTasks(std::vector<Task *> &tasks) {
	itsTaskDialog->showMultiEdit(tasks);
}

void SchedulerGUI::updateTaskDialogStations(void) {
	itsTaskDialog->loadAvailableStations();
}

void SchedulerGUI::setShortcutKeys(void) {
}

void SchedulerGUI::horizontalView(void) {
	ui.actionVertical_view->setChecked(false);
	ui.actionHorizontal_view->setChecked(true);
//	itsTableDock->setAllowedAreas(Qt::LeftDockWidgetArea);
//	itsGraphicDock->setAllowedAreas(Qt::LeftDockWidgetArea);
	splitDockWidget(itsGraphicDock, itsTableDock, Qt::Vertical);
}

void SchedulerGUI::verticalView(void) {
	ui.actionVertical_view->setChecked(true);
	ui.actionHorizontal_view->setChecked(false);
//	itsTableDock->setAllowedAreas(Qt::TopDockWidgetArea);
//	itsGraphicDock->setAllowedAreas(Qt::TopDockWidgetArea);
	splitDockWidget(itsTableDock, itsGraphicDock, Qt::Horizontal);
}

void SchedulerGUI::updateGraphicStations() {
	itsGraphicResourceScene->updateStationTimeLines();
}

void SchedulerGUI::updateSceneTimeSpan(void) {
	itsGraphicResourceScene->setNewTimeSpan();
}

void SchedulerGUI::addTask(const Task *pTask) {
    if (pTask->isStationTask()) {
        itsGraphicResourceScene->addTask(pTask);
	}
	unsigned rows = itsModel->rowCount() + 1;
	itsModel->setRowCount(rows);
    updateTableTask(pTask, rows-1);
}

void SchedulerGUI::deleteTaskFromGUI(unsigned int taskID, Task::task_type type) {
	if (type != Task::PIPELINE) {
		itsGraphicResourceScene->removeTaskFromScene(taskID);
	}
	// delete task from table
	QList<QStandardItem *> results = itsModel->findItems(QString(int2String(taskID).c_str()),Qt::MatchExactly, TASK_ID);
	if (!results.empty()) {
		itsModel->removeRows(itsModel->indexFromItem(results.first()).row(), 1);
	}
}

void SchedulerGUI::updateGraphicTask(unsigned task_id) {
    const Task *pTask(itsController->getTask(task_id));
    if (pTask) {
        itsGraphicResourceScene->updateTask(pTask);
    }
}

void SchedulerGUI::updateGraphicTasks(const scheduledTasksMap &scheduledTasks, const reservationsMap &reservations, const inActiveTasksMap & inactiveTasks) {
	itsGraphicResourceScene->updateTasks(scheduledTasks, reservations, inactiveTasks);
}

void SchedulerGUI::createTableDock(void) {
    // create table dock and its layout
    itsTableDock = new QDockWidget("Table schedule view", this);
	itsTableDockWidgetContents = new QWidget();
    itsTableDockMainLayout = new QGridLayout(itsTableDockWidgetContents);
    itsTableDockMainLayout->setMargin(5);
    // create the table view
	itsTableView = new TableView(itsTableDockWidgetContents);
	itsTableView->setWordWrap(false);
#if QT_VERSION >= 0x050000
    itsTableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    itsTableView->horizontalHeader()->setSectionsMovable(true);
#else
    itsTableView->verticalHeader()->setResizeMode(QHeaderView::Fixed);
    itsTableView->horizontalHeader()->setMovable(true);
#endif
	itsTableView->setDragEnabled(false);
	itsTableView->setDropIndicatorShown(true);
	itsTableView->setAcceptDrops(false);
	itsTableView->setAlternatingRowColors(true);
	itsTableView->sortByColumn(itsSortColumn, itsSortOrder);
    // create two table filtering checkboxes for observations and pipelines
    itsTableObsEnabled = new QCheckBox("Observations");
    itsTableObsEnabled->setToolTip("Hide/show Observations in table");
    itsTableObsEnabled->setChecked(true);
    itsTableDockMainLayout->addWidget(itsTableObsEnabled, 0, 0, 1, 1);
    itsTablePipesEnabled = new QCheckBox("Pipelines");
    itsTablePipesEnabled->setChecked(true);
    itsTablePipesEnabled->setToolTip("Hide/show Pipelines in table");
    itsTableDockMainLayout->addWidget(itsTablePipesEnabled, 0, 1, 1, 1);
    itsTableProjectFilter = new QComboBox();
    itsTableProjectFilter->setToolTip("Filter on specific project");
    itsTableProjectFilter->setMinimumWidth(175);  // Magix number
    itsTableDockMainLayout->addWidget(itsTableProjectFilter, 0, 2, 1, 1);
    itsTableSpacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    itsTableDockMainLayout->addItem(itsTableSpacerItem, 0, 3, 1, 1);
    itsTableDockMainLayout->addWidget(itsTableView, 1, 0, 1, 4);

	itsTableDock->setWidget(itsTableDockWidgetContents);
	itsTableDock->setAllowedAreas(Qt::LeftDockWidgetArea); // change all LeftDockWidgetArea in TopDockWidgetArea for vertical view

	addDockWidget(Qt::LeftDockWidgetArea, itsTableDock);
}

void SchedulerGUI::createGraphicDock(void) {
	// create docking widget for graphical area
    itsGraphicDock = new QDockWidget(tr("Graphic schedule view"), this);
    itsGraphicDockWidgetContents = new QWidget();
    itsGraphicDockMainLayout = new QVBoxLayout(itsGraphicDockWidgetContents);
	itsGraphicDockMainLayout->setMargin(0);
    itsGraphicDock->setAllowedAreas(Qt::LeftDockWidgetArea);
	itsGraphicResourceScene = new GraphicResourceScene(itsController);
	itsGraphicView = new QGraphicsView(itsGraphicResourceScene);
	itsGraphicView->setDragMode(QGraphicsView::RubberBandDrag);
	itsGraphicResourceScene->setViewPort(itsGraphicView);
	itsGraphicViewToolbar = new QToolBar(itsGraphicDockWidgetContents);
	// zoom in/zoom out buttons
	itsZoomInAction = new QAction(QIcon(":/icons/zoomin.png"), tr("Zoom in"), this);
	itsZoomOutAction = new QAction(QIcon(":/icons/zoomout.png"), tr("Zoom out"), this);
	itsGraphicViewToolbar->addAction(itsZoomInAction);
	itsGraphicViewToolbar->addAction(itsZoomOutAction);
	// Now button
	itsCenterOnNow = new QAction(QIcon(":/icons/clock.png"), tr("Now"), this);
	itsGraphicViewToolbar->addAction(itsCenterOnNow);
	// Color mode button
	itsTaskTypeColorModeAction = new QAction(QIcon(":/icons/taskcolormode.png"), tr("Toggle task color mode"), this);
	itsTaskTypeColorModeAction->setIconText("Status color mode");
	itsGraphicViewToolbar->addAction(itsTaskTypeColorModeAction);

	itsGraphicViewToolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
//	itsZoomInAction->setMaximumSize(20,20);
//	itsZoomOutAction->setMaximumSize(20,20);
//	itsGraphicViewToolbar->addAction(itsZoomOutButton);
//	itsGraphicViewToolbar->addWidget(itsZoomInButton);
	itsGraphicDockMainLayout->addWidget(itsGraphicViewToolbar);
	itsGraphicDockMainLayout->addWidget(itsGraphicView);
	itsGraphicDock->setWidget(itsGraphicDockWidgetContents);
	itsGraphicView->centerOn(0,0);
	itsGraphicView->show();
	addDockWidget(Qt::LeftDockWidgetArea, itsGraphicDock);
}

void SchedulerGUI::updateProjectsFilter(const campaignMap &campaigns) {
    bool projectFilterApplied(itsTableProjectFilter->currentIndex() != 0);
    QString curSelProject(itsTableProjectFilter->currentText());
    QStringList projects;
    projects.append("All projects");
    for (campaignMap::const_iterator it = campaigns.begin(); it != campaigns.end(); ++it) {
        if (itsActiveProjects.contains(it->second.name.c_str())) { // only show projects that are in the table
            projects << it->second.name.c_str();
        }
    }
    itsTableProjectFilter->blockSignals(true);
    itsTableProjectFilter->clear();
    itsTableProjectFilter->addItems(projects);
    itsTableProjectFilter->blockSignals(false);

    // re-select the previously selected project if any
    if (projectFilterApplied) {
        int cnt(itsTableProjectFilter->count());
        for (int i = 0; i < cnt; ++i) {
            if (itsTableProjectFilter->itemText(i) == curSelProject) {
                itsTableProjectFilter->setCurrentIndex(i);
                break;
            }
        }
    }
}



void SchedulerGUI::updateTimeLinePosAfterScroll(void) {
	itsGraphicResourceScene->updateTimeLineYPos();
}

void SchedulerGUI::updateStationTimeLinesAfterScroll(void) {
	itsGraphicResourceScene->updateStationNamesXPos();
}

void SchedulerGUI::createStatusBar(void) {
	itsStatusBar = new QStatusBar(this);
	this->setStatusBar(itsStatusBar);
    itsStatusBarStatusText = new QLabel(tr("Ready"), itsStatusBar);
	itsStatusBarStatusText->setMinimumWidth(300);
	itsStatusBar->addWidget(itsStatusBarStatusText);

    itsStatusBarTaskStatus = new QLabel("#S:0, #US:0, #INACT:0, #RES:0, #ERR:0", itsStatusBar);
    itsStatusBar->addWidget(itsStatusBarTaskStatus, 1);

    itsStatusBarUsedDatabase = new QLabel("SAS DB:" + itsController->getSasDatabaseName(), itsStatusBar);
    itsStatusBarUsedDatabase->setMinimumWidth(150);
    itsStatusBar->addWidget(itsStatusBarUsedDatabase);

    itsStatusBarProgress = new QProgressBar();
    itsStatusBarProgress->setMaximumHeight(15);
    itsStatusBarProgress->setMinimumWidth(150);
    itsStatusBarProgress->setMaximumWidth(150);
    itsStatusBar->addPermanentWidget(itsStatusBarProgress);

	itsStatusBar->show();
}

void SchedulerGUI::updateSasDatabaseName(void) {
    itsStatusBarUsedDatabase->setText("SAS DB:" + itsController->getSasDatabaseName());
}

void SchedulerGUI::updateProgressBar(int progress) {
	itsStatusBarProgress->setValue(progress);
}

void SchedulerGUI::setProgressBarMaximum(int maxValue) {
	itsStatusBarProgress->setEnabled(true);
	itsStatusBarProgress->setMaximum(maxValue);
}

void SchedulerGUI::updateStatusBar(unsigned nr_scheduled, unsigned nr_unscheduled, unsigned nr_inactive, unsigned nr_reservations, unsigned nr_errortasks, unsigned nr_pipelines, int progress) {
	QString statusStr = "#S:" + QString::number(nr_scheduled) + ", #US:" + QString::number(nr_unscheduled) +
	", #INACT:" + QString::number(nr_inactive) + ", #RES:" + QString::number(nr_reservations) +
	", #ERR:" + QString::number(nr_errortasks) + ", #PL:" + QString::number(nr_pipelines);
	itsStatusBarTaskStatus->setText(statusStr);
	itsStatusBarProgress->setValue(progress);
}

void SchedulerGUI::disableProgressBar(void) {
	itsStatusBarProgress->setValue(0);
	itsStatusBarProgress->setEnabled(false);
}

void SchedulerGUI::createSearchBar(void) {
	itsSearchBar = new QToolBar(this);
    addToolBar(Qt::BottomToolBarArea, itsSearchBar);
	itsSearchBar->setAccessibleName(QString("Search"));
	itsSearchBar->setWindowTitle(tr("Search"));
	itsSearchBar->setAllowedAreas(Qt::BottomToolBarArea|Qt::TopToolBarArea);

	//showSearchBarAct = new QAction(this);
//	this->addAction(showSearchBarAct);

	// create search widgets

	// close button
	closeSearchBarAct = new QAction(this);
	closeSearchBarAct->setObjectName(QString::fromUtf8("close"));
    closeIcon = new QIcon;
//#ifdef Q_OS_UNIX
    closeIcon->addPixmap(QPixmap(QString::fromUtf8(":/icons/close16x16.png")), QIcon::Normal, QIcon::Off);
//#elif defined(Q_OS_WIN)
//    closeIcon->addPixmap(QPixmap(QString::fromUtf8(":/icons\\close16x16.png")), QIcon::Normal, QIcon::Off);
//#endif
    closeSearchBarAct->setIcon(*closeIcon);
	itsSearchBar->addAction(closeSearchBarAct);

	// search line edit
	itsSearchLineEdit = new QLineEdit(this);
	itsSearchLineEdit->setObjectName(QString::fromUtf8("itsSearchLineEdit"));

	// previous button
	itsSearchPreviousButton = new QPushButton(itsSearchBar);
	itsSearchPreviousButton->setText(tr("&Previous"));
	itsSearchPreviousAct = new QAction(this);
	itsSearchPreviousAct->setObjectName(QString::fromUtf8("searchPreviousAct"));
	itsSearchPreviousButton->addAction(itsSearchPreviousAct);

	// next button
	itsSearchNextButton = new QPushButton(itsSearchBar);
	itsSearchNextButton->setText(tr("&Next"));
	itsSearchNextAct = new QAction(this);
	itsSearchNextAct->setObjectName(QString::fromUtf8("searchNextAct"));
	itsSearchNextButton->addAction(itsSearchNextAct);

	//current column
	itsSearchColumnOnlyCheckBox = new QCheckBox(itsSearchBar);
//	itsSearchColumnOnlyCheckBox->setChecked(true);
	itsSearchColumnOnlyCheckBox->setText(tr("Search current &column"));
	itsSearchColumnOnlyCheckBoxAct = new QAction(this);
	itsSearchColumnOnlyCheckBoxAct->setObjectName(QString::fromUtf8("columnOnlyAct"));
	itsSearchColumnOnlyCheckBox->addAction(itsSearchColumnOnlyCheckBoxAct);

//	//current row
//	itsSearchRowOnlyCheckBox = new QCheckBox(itsSearchBar);
//	itsSearchRowOnlyCheckBox->setText(tr("Search &row"));

	//current row

//	// match case checkbox
//	itsSearchMatchCaseCheckBox = new QCheckBox(itsSearchBar);
//	itsSearchMatchCaseCheckBox->setText(tr("&Match case"));

	// add widgets to search bar
	itsSearchBar->addWidget(itsSearchLineEdit);
	itsSearchBar->addWidget(itsSearchPreviousButton);
	itsSearchBar->addWidget(itsSearchNextButton);
	itsSearchBar->addWidget(itsSearchColumnOnlyCheckBox);
//	itsSearchBar->addWidget(itsSearchRowOnlyCheckBox);
//	itsSearchBar->addWidget(itsSearchMatchCaseCheckBox);

	itsSearchBar->hide();

}

/*
void SchedulerGUI::keyPressEvent ( QKeyEvent * event ) {
	std::cout << "keypressed event" << std::endl;
	//if (event->key() == Qt::Key_Escape)
}
*/

void SchedulerGUI::doSearch(const QString& term) {
	search(term);
}

void SchedulerGUI::searchAgain(void) {
	itsSearchLineEdit->setFocus(); // directly set focus to the line edit box again after user checks the column only checkbox
	itsSearchLineEdit->selectAll();//	scheduler.createStartSchedule();
	//	gui->updateGraphicTasks(data.getScheduledTasks());

	QString searchTerm = itsSearchLineEdit->text(); // fetch the changed search term directly from the line edit box
	if (!searchTerm.isEmpty()) {
		search(searchTerm);
	}
}

void SchedulerGUI::search(const QString& term) {
	clearSearch();
	if (itsModel && !term.isEmpty()) {
		if (itsSearchColumnOnlyCheckBox->isChecked()) {
			columnSearched = itsTableView->currentIndex().column();
			searchResult = itsModel->findItems(term,Qt::MatchContains, columnSearched);
		}
		else {
			for (unsigned short col = 0; col < NR_DATA_HEADERS; ++col) {
				searchResult += itsModel->findItems(term,Qt::MatchContains, col);
			}
		}

		if (!searchResult.isEmpty()) {
//			QColor color("white");
			QPalette palet = ( itsSearchLineEdit->palette() );
			palet.setColor( QPalette::Base, Qt::white );
			itsSearchLineEdit->setPalette(palet);
			search_iterator = searchResult.begin();

			itsTableView->setCurrentIndex(itsModel->indexFromItem(*search_iterator));
//			itsSearchPreviousButton->setEnabled(false);
//			if (searchResult.size() == 1) {
//				itsSearchNextButton->setEnabled(false);
//			}
//			else if (searchResult.size() > 1) {
//				itsSearchNextButton->setEnabled(true);
//			}
		}
		else {
//			QColor color("red");// = QColorDialog::getColor( Qt::red, this);
			QPalette palet = ( itsSearchLineEdit->palette() );
			palet.setColor( QPalette::Base, Qt::red );
			itsSearchLineEdit->setPalette(palet);
//			itsSearchPreviousButton->setEnabled(false);
//			itsSearchNextButton->setEnabled(false);
		}
	}
}

void SchedulerGUI::searchPrevious() {
	if (!searchResult.isEmpty()) {
		if (search_iterator != searchResult.begin()) {
//			itsSearchNextButton->setEnabled(true);//	scheduler.createStartSchedule();
			//	gui->updateGraphicTasks(data.getScheduledTasks());

			--search_iterator;
		}
		else if (searchResult.size() > 1) {
			if (QMessageBox::question(0, tr("Beginning of search results"),
					tr("This is the first search result?\n Do you want to jump to the last?"),
					QMessageBox::Ok | QMessageBox::Default,
					QMessageBox::Cancel) == QMessageBox::Ok) {
				search_iterator = searchResult.end()-1;
			}
			else return;
		}
		itsTableView->setCurrentIndex(itsModel->indexFromItem(*search_iterator));
//		if (search_iterator == searchResult.begin()) {
//			itsSearchPreviousButton->setEnabled(false);
//		}
	}
	else {
		doSearch(itsSearchLineEdit->text());
	}
}

void SchedulerGUI::searchNext() {
	if (!searchResult.isEmpty()) {
		if (search_iterator != searchResult.end()-1) {
//			itsSearchPreviousButton->setEnabled(true);
			++search_iterator;
		}
		else if (searchResult.size() > 1) {
			if (QMessageBox::question(0, tr("End of search results"),
					tr("This is the last search result?\n Do you want to jump to the first?"),
					QMessageBox::Ok | QMessageBox::Default,
					QMessageBox::Cancel) == QMessageBox::Ok) {
				search_iterator = searchResult.begin();
			}
			else return;
		}
		itsTableView->setCurrentIndex(itsModel->indexFromItem(*search_iterator));
	}
	else {
		doSearch(itsSearchLineEdit->text());
	}
}

void SchedulerGUI::newTable(SchedulerData const &data) {
	delete itsModel;
	itsModel = 0;
	itsModel = new QLofarDataModel(data.getNrTasks(), NR_DATA_HEADERS, 0);

	QStringList headerList;
	for (std::vector<std::string>::const_iterator it=data.getDataHeaders().begin(); it != data.getDataHeaders().end(); ++it) {
		headerList << it->c_str();
	}
	itsModel->setHorizontalHeaderLabels(headerList);

    itsModel->setSortRole(Qt::UserRole); // to get proper sorting make sure the userRole has the correct data (that sorts correctly) for every column

	itsTableView->setModel(itsModel);
	itsTableView->setItemDelegate(&itsDelegate);
	itsTableView->horizontalHeader()->setStretchLastSection(true);
#if QT_VERSION >= 0x050000
    itsTableView->horizontalHeader()->setSectionsClickable(true);
#else
    itsTableView->horizontalHeader()->setClickable(true);
#endif
	itsTableView->horizontalHeader()->setSortIndicatorShown(true);
	writeTableData(data);
}
/*
void SchedulerGUI::loadNewTable(SchedulerData const &data) {
	newTable(data);
	size_t startRow(0);
	std::vector<Task *> tasks;
	// write scheduled tasks to table
	tasks = data.getScheduledTasksVector();
	writeTasksToTable(tasks,0);
	startRow += tasks.size();
	// write the unscheduled tasks to the table
	tasks = data.getUnScheduledTasksVector();
	writeTasksToTable(tasks, startRow);
	startRow += tasks.size();
	// write the reservations to the table
	tasks = data.getReservationsVector();
	writeTasksToTable(tasks, startRow);
	startRow += tasks.size();
	// maintenance
	tasks = data.getMaintenanceVector();
	writeTasksToTable(tasks, startRow);
	startRow += tasks.size();
	// write the inactive tasks to the table
	tasks = data.getInactiveTaskVector();
	writeTasksToTable(tasks, startRow);
	startRow += tasks.size();
	// write pipeline tasks to table
	tasks = data.getPipelinesVector();
	writeTasksToTable(tasks,startRow);

	itsTableView->sortByColumn(itsSortColumn, itsSortOrder);
	setDefaultColumnWidths();
	itsTableView->resizeRowsToContents();
}
*/

void SchedulerGUI::setDefaultColumnWidths(void) {
	itsTableView->resizeColumnsToContents();
	itsTableView->setColumnWidth(PROJECT_ID,150); //
	itsTableView->setColumnWidth(TASK_NAME,150); //
	itsTableView->setColumnWidth(CONTACT_NAME,100); //
	itsTableView->setColumnWidth(CONTACT_PHONE,100); //
	itsTableView->setColumnWidth(CONTACT_EMAIL,100); //
	itsTableView->setColumnWidth(STATION_ID,200); //
	itsTableView->setColumnWidth(FILTER_TYPE,150); // Antenna mode
	itsTableView->setColumnWidth(ANTENNA_MODE,205); // Antenna mode
//	itsTableView->setColumnWidth(SOURCES,100); // sources
//	itsTableView->setColumnWidth(RIGHT_ASCENSION,100); // RA
	itsTableView->setColumnWidth(PLANNED_START,155); // declination
	itsTableView->setColumnWidth(PLANNED_END,155); // declination
	itsTableView->setColumnWidth(TASK_STATUS,100);
	itsTableView->setColumnWidth(UNSCHEDULED_REASON,280); // unscheduled reason
	itsTableView->setColumnWidth(FIXED_DAY,50);
	itsTableView->setColumnWidth(FIXED_TIME,50);
	itsTableView->setColumnWidth(PRIORITY,50);
    itsTableView->setColumnWidth(CLUSTER_NAME,50);
}

void SchedulerGUI::writeTableData(SchedulerData const &data) {
	// clear table search results to avoid referencing altered items in table
	if (itsModel) {
		clearSearch();
		itsModel->removeRows(0, itsModel->rowCount());
		itsModel->insertRows(0, data.getNrTasks());
	}
	if (data.getNrTasks() > 0) {
		size_t startRow(0);
		std::vector<Task *> tasks;
		// write scheduled tasks to table
		tasks = data.getScheduledTasksVector();
		writeTasksToTable(tasks,0);
		startRow += tasks.size();
		// write the unscheduled tasks to the table
		tasks = data.getUnScheduledTasksVector();
		writeTasksToTable(tasks, startRow);
		startRow += tasks.size();
		// write the reservations to the table
		tasks = data.getReservationsVector();
		writeTasksToTable(tasks, startRow);
		startRow += tasks.size();
		// maintenance
		tasks = data.getMaintenanceVector();
		writeTasksToTable(tasks, startRow);
		startRow += tasks.size();
		// write the inactive tasks to the table
		tasks = data.getInactiveTaskVector();
		writeTasksToTable(tasks, startRow);
		startRow += tasks.size();
		// write pipeline tasks to table
		tasks = data.getPipelinesVector();
		writeTasksToTable(tasks,startRow);

		itsTableView->sortByColumn(itsSortColumn, itsSortOrder);
		//	writeTasksToTable(data.getUnScheduledTasksSortFirstDate(),data.getNrScheduled());
		itsTableView->resizeRowsToContents();
		setDefaultColumnWidths();
		setErrorCells(data.getErrorTasks());
		itsTableView->repaint();
	}
}

void SchedulerGUI::updateTableTasksScheduleTimes(const std::vector<const Task *> &tasks) {
	int row;
	for (std::vector<const Task *>::const_iterator it = tasks.begin(); it != tasks.end(); ++it) {
		row = itsModel->findTaskRow((*it)->getID());
		if (row != -1) {
			itsModel->setData(itsModel->index(row, PLANNED_START), (*it)->getScheduledStart().toQDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            itsModel->setData(itsModel->index(row, PLANNED_START), (*it)->getScheduledStart().toQDateTime(), Qt::UserRole); // used for sorting
			itsModel->setData(itsModel->index(row, PLANNED_END), (*it)->getScheduledEnd().toQDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            itsModel->setData(itsModel->index(row, PLANNED_END), (*it)->getScheduledEnd().toQDateTime(), Qt::UserRole); // used for sorting
        }
	}
	itsTableView->repaint();
}

void SchedulerGUI::updateTableTaskScheduleTimes(const Task &task) {
	int row(itsModel->findTaskRow(task.getID()));
	if (row != -1) {
		itsModel->setData(itsModel->index(row, PLANNED_START), task.getScheduledStart().toQDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        itsModel->setData(itsModel->index(row, PLANNED_START), task.getScheduledStart().toQDateTime(), Qt::UserRole); // used for sorting
		itsModel->setData(itsModel->index(row, PLANNED_END), task.getScheduledEnd().toQDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        itsModel->setData(itsModel->index(row, PLANNED_END), task.getScheduledEnd().toQDateTime(), Qt::UserRole); // used for sorting
		itsTableView->repaint();
	}
}

void SchedulerGUI::updateTask(const Task *pTask) {
    updateTableTask(pTask);
    if (pTask->isStationTask()) {
        itsGraphicResourceScene->updateTask(pTask);
	}
}

std::vector<unsigned> SchedulerGUI::getSelectedRowsTaskIDs(void) const {
	std::vector<unsigned> taskIDs;
	QList<int> list(itsTableView->selectedRows().toList());
	qSort(list);

	for (QList<int>::const_iterator rit = list.begin(); rit != list.end(); ++rit) {
		taskIDs.push_back(itsModel->data(itsModel->index(*rit, (int)TASK_ID)).toUInt());
	}
	return taskIDs;
}

std::vector<unsigned> SchedulerGUI::getShownTaskIDs(void) const {
	std::vector<unsigned> taskIDs;

	for (int row = 0; row < itsModel->rowCount(); ++row) {
		taskIDs.push_back(itsModel->data(itsModel->index(row, (int)TASK_ID)).toUInt());
	}
	return taskIDs;
}

int SchedulerGUI::findTableTaskByID(unsigned ID, id_type IDtype) const {
	int nrRows = itsModel->rowCount();
	int col;
	if (IDtype == ID_SCHEDULER) col = TASK_ID;
	else if (IDtype == ID_SAS) col = SAS_ID;
	else col = MOM_ID;

	for (int row = 0; row <= nrRows; ++row) {
		if (itsModel->data(itsModel->index(row, col), Qt::UserRole).toUInt() == ID) return row; // if this is the row with the right task ID
	}
	return -1;
}

void SchedulerGUI::updateTableTask(const Task *pTask, int row) {
    if (pTask) {
        if (row == -1) {
            row = findTableTaskByID(pTask->getID(), ID_SCHEDULER);
            if (row == -1) { // task not found in table, add it
                unsigned rows = itsModel->rowCount() + 1;
                itsModel->setRowCount(rows);
                row = rows-1;
            }
        }
        // store the task type (int) at index (row,0) to be used to determine if a row needs to be treated specially
        // i.e. some cells need to be read only for task type RESERVATION and MAINTENANCE
        itsModel->setData(itsModel->index(row, TASK_TYPE), QVariant(static_cast<int>(pTask->getType())), USERDATA_ROLE);
        // store task status also as userdata
        itsModel->setData(itsModel->index(row, TASK_STATUS), QVariant(static_cast<int>(pTask->getStatus())), USERDATA_ROLE);

        QModelIndex idx(itsModel->index(row, TASK_ID));
        itsModel->setData(idx, pTask->getID());
        itsModel->setData(idx, pTask->getID(), Qt::UserRole);
        idx = itsModel->index(row, SAS_ID);
        itsModel->setData(idx, pTask->getSASTreeID());
        itsModel->setData(idx, pTask->getSASTreeID(), Qt::UserRole);
        idx = itsModel->index(row, MOM_ID);
        itsModel->setData(idx, pTask->getMomID());
        itsModel->setData(idx, pTask->getMomID(), Qt::UserRole);
        itsModel->setData(itsModel->index(row, GROUP_ID), pTask->getGroupID());
        itsModel->setData(itsModel->index(row, GROUP_ID), pTask->getGroupID(), Qt::UserRole); // for sorting
        itsModel->setData(itsModel->index(row, PROJECT_ID), pTask->getProjectID());
        itsModel->setData(itsModel->index(row, PROJECT_ID), pTask->getProjectID(), Qt::UserRole); // for sorting
        itsActiveProjects.insert(pTask->getProjectID());
        itsModel->setData(itsModel->index(row, TASK_NAME), pTask->getTaskName());
        itsModel->setData(itsModel->index(row, TASK_NAME), pTask->getTaskName(), Qt::UserRole); // for sorting
        itsModel->setData(itsModel->index(row, TASK_DESCRIPTION), pTask->SASTree().description().c_str());
        itsModel->setData(itsModel->index(row, TASK_DESCRIPTION), pTask->SASTree().description().c_str(), Qt::UserRole); // for sorting
        itsModel->setData(itsModel->index(row, CONTACT_NAME), pTask->getContactName());
        itsModel->setData(itsModel->index(row, CONTACT_NAME), pTask->getContactName(), Qt::UserRole); // for sorting
        itsModel->setData(itsModel->index(row, CONTACT_PHONE), pTask->getContactPhone());
        itsModel->setData(itsModel->index(row, CONTACT_PHONE), pTask->getContactPhone(), Qt::UserRole); // for sorting
        itsModel->setData(itsModel->index(row, CONTACT_EMAIL), pTask->getContactEmail());
        itsModel->setData(itsModel->index(row, CONTACT_EMAIL), pTask->getContactEmail(), Qt::UserRole); // for sorting
        itsModel->setData(itsModel->index(row, TASK_TYPE), pTask->getTypeStr());
        itsModel->setData(itsModel->index(row, TASK_TYPE), pTask->getTypeStr(), Qt::UserRole); // for sorting
        itsModel->setData(itsModel->index(row, CLUSTER_NAME), pTask->getOutputDataproductCluster());
        itsModel->setData(itsModel->index(row, CLUSTER_NAME), pTask->getOutputDataproductCluster(), Qt::UserRole); // for sorting

        const StationTask *pStationTask = dynamic_cast<const StationTask *>(pTask);
        if (pStationTask) { // is this a stationTask?
            // stations
            QStringList stationList;
            std::map<std::string, unsigned int> const &stations = pStationTask->getStations();
            for (std::map<std::string, unsigned int>::const_iterator stit = stations.begin(); stit != stations.end(); ++stit) {
                stationList.append(QString(stit->first.c_str()));
            }
            itsModel->setData(itsModel->index(row, STATION_ID), stationList);
            itsModel->setData(itsModel->index(row, STATION_ID), stationList, Qt::UserRole);
            // mode
            itsModel->setData(itsModel->index(row, ANTENNA_MODE), pStationTask->getAntennaModeStr());
            itsModel->setData(itsModel->index(row, ANTENNA_MODE), pStationTask->getAntennaModeStr(), Qt::UserRole);
            // filter type
            itsModel->setData(itsModel->index(row, FILTER_TYPE), pStationTask->getFilterTypeStr());
            itsModel->setData(itsModel->index(row, FILTER_TYPE), pStationTask->getFilterTypeStr(), Qt::UserRole);
            // clock frequency
            itsModel->setData(itsModel->index(row, CLOCK_FREQUENCY), pStationTask->getStationClockStr());
            itsModel->setData(itsModel->index(row, CLOCK_FREQUENCY), pStationTask->getStationClockStr(), Qt::UserRole);

            const Observation *pObs = dynamic_cast<const Observation *>(pTask);
            if (pObs) { // is this an observation?
                itsModel->setData(itsModel->index(row, RESERVATION_NAME), itsController->getReservationName(pObs->getReservation()));
                itsModel->setData(itsModel->index(row, RESERVATION_NAME), itsController->getReservationName(pObs->getReservation()), Qt::UserRole); // for sorting
                // Nr of subbands
                itsModel->setData(itsModel->index(row, NR_OF_SUBBANDS), pObs->getNrOfSubbands());
                itsModel->setData(itsModel->index(row, NR_OF_SUBBANDS), pObs->getNrOfSubbands(), Qt::UserRole); // for sorting
                // Night time weight
                itsModel->setData(itsModel->index(row, NIGHT_TIME_WEIGHT_FACTOR), pObs->getNightTimeWeightFactor());
                itsModel->setData(itsModel->index(row, NIGHT_TIME_WEIGHT_FACTOR), pObs->getNightTimeWeightFactor(), Qt::UserRole); // for sorting
                // for observations show real end time which could differ from scheduled end time
                const AstroDateTime &realEnd(pTask->getRealEnd());
                Task::task_status state(pTask->getStatus());
                if (state >= Task::FINISHED && realEnd.isSet()) {
                    if (realEnd <= pTask->getScheduledEnd().subtractMinutes(1)) {
                        string afs = state == Task::ABORTED ? "Aborted early at:" : "Finished early at:";
                        afs += realEnd.toString();
                        itsModel->setData(itsModel->index(row, PLANNED_END), afs.c_str(), Qt::ToolTipRole);
                        itsModel->setData(itsModel->index(row, PLANNED_END), (int)Qt::darkGreen, Qt::UserRole+1);
                    }
                    else if (realEnd >= pTask->getScheduledEnd().addMinutes(50)) {
                        string afs = state == Task::ABORTED ? "Aborted late at:" : "Finish late at:";
                        afs += realEnd.toString() + "\nCheck for missing MetaData!";
                        itsModel->setData(itsModel->index(row, PLANNED_END), afs.c_str(), Qt::ToolTipRole);
                        itsModel->setData(itsModel->index(row, PLANNED_END), (int)Qt::red, Qt::UserRole+1);
                    }
                }

            }

            // check if observations are currently hidden in the table
            if (!itsTableObsEnabled->isChecked()) {
                itsTableView->hideRow(row);
            }
        }
        else if (pTask->isPipeline() && !itsTablePipesEnabled->isChecked()) { // check if pipelines are currently hidden in the table
            itsTableView->hideRow(row);
        }

        if (itsTableProjectFilter->currentIndex() != 0) { // project Filter enabled?
            if (pTask->getProjectID() != itsTableProjectFilter->currentText()) {
                itsTableView->hideRow(row);
            }
        }

        if (pTask->hasPredecessors()) {
            // predecessor min and max time difference
            itsModel->setData(itsModel->index(row, PREDECESSORS), pTask->getPredecessorsString());
            itsModel->setData(itsModel->index(row, PREDECESSORS), pTask->getPredecessorsString(), Qt::UserRole); // for sorting
            itsModel->setData(itsModel->index(row, PRED_MIN_TIME_DIF),  QDateTime::fromString(QString(pTask->getPredecessorMinTimeDif().toString().c_str()),"yyyy-MM-dd hh:mm:ss"));
            itsModel->setData(itsModel->index(row, PRED_MIN_TIME_DIF),  pTask->getPredecessorMinTimeDif().toQTime(), Qt::UserRole); // for sorting
            itsModel->setData(itsModel->index(row, PRED_MAX_TIME_DIF), QDateTime::fromString(QString(pTask->getPredecessorMaxTimeDif().toString().c_str()),"yyyy-MM-dd hh:mm:ss"));
            itsModel->setData(itsModel->index(row, PRED_MAX_TIME_DIF), pTask->getPredecessorMaxTimeDif().toQTime(), Qt::UserRole); // for sorting
        }

        // priority
        itsModel->setData(itsModel->index(row, PRIORITY), pTask->getPriority());
        itsModel->setData(itsModel->index(row, PRIORITY), pTask->getPriority(), Qt::UserRole);
        // duration
        itsModel->setData(itsModel->index(row, TASK_DURATION), pTask->getDuration().toString().c_str());
        itsModel->setData(itsModel->index(row, TASK_DURATION), pTask->getDuration().totalSeconds(), Qt::UserRole);
        // planned start
        itsModel->setData(itsModel->index(row, PLANNED_START), QDateTime::fromString(QString(pTask->getScheduledStart().toString().c_str()),"yyyy-MM-dd hh:mm:ss"));
        itsModel->setData(itsModel->index(row, PLANNED_START), pTask->getScheduledStart().toQDateTime(), Qt::UserRole);
        // planned end
        itsModel->setData(itsModel->index(row, PLANNED_END), QDateTime::fromString(QString(pTask->getScheduledEnd().toString().c_str()),"yyyy-MM-dd hh:mm:ss"));
        itsModel->setData(itsModel->index(row, PLANNED_END), pTask->getScheduledEnd().toQDateTime(), Qt::UserRole);
        // fixed day
        itsModel->setData(itsModel->index(row, FIXED_DAY), static_cast<unsigned short>(pTask->getFixedDay()));
        itsModel->setData(itsModel->index(row, FIXED_DAY), static_cast<unsigned short>(pTask->getFixedDay()), Qt::UserRole); // for sorting
        // fixed time
        itsModel->setData(itsModel->index(row, FIXED_TIME), static_cast<unsigned short>(pTask->getFixedTime()));
        itsModel->setData(itsModel->index(row, FIXED_TIME), static_cast<unsigned short>(pTask->getFixedTime()), Qt::UserRole); // for sorting
        // task status
        itsModel->setData(itsModel->index(row, TASK_STATUS), pTask->getStatusStr());
        itsModel->setData(itsModel->index(row, TASK_STATUS), pTask->getStatusStr(), Qt::UserRole); // for sorting
        // Reason if unscheduled
        QString reason(pTask->getReason().c_str());
        reason = reason.replace('$','\n');
        itsModel->setData(itsModel->index(row, UNSCHEDULED_REASON), reason);
        itsModel->setData(itsModel->index(row, UNSCHEDULED_REASON), reason, Qt::UserRole); // for sorting
        itsModel->setData(itsModel->index(row, UNSCHEDULED_REASON), reason, Qt::ToolTipRole);

        const TaskStorage *task_storage(pTask->storage());
        if (task_storage) {
            itsModel->setData(itsModel->index(row, STORAGE_SIZE), humanReadableUnits((long double)task_storage->getTotalStoragekBytes(), SIZE_UNITS).c_str());
            itsModel->setData(itsModel->index(row, STORAGE_SIZE), task_storage->getTotalStoragekBytes(), Qt::UserRole); // for sorting
        }
    }
}

void SchedulerGUI::writeTasksToTable(const std::vector<Task *> &tasks, unsigned int startRow) {
	for (std::vector<Task *>::const_iterator it = tasks.begin(); it != tasks.end(); ++it) {
        updateTableTask(*it, startRow++);
	}
}


void SchedulerGUI::showSearchBar() {
	itsSearchBar->show();
	itsSearchLineEdit->selectAll();
	itsSearchLineEdit->setFocus();
}

void SchedulerGUI::cleanup(void) {
	updateStatusBar(0,0,0,0,0,0,0);
	columnSearched = 0;
	itsSortColumn = PLANNED_START;
	itsSortOrder = Qt::DescendingOrder;
	clearSearch();
	delete itsModel;
	itsModel = 0;
	ui.action_Undo->setEnabled(false);
	ui.action_Redo->setEnabled(false);
}

void SchedulerGUI::clearTasks(void) {
	itsGraphicResourceScene->clearTasks();
}

void SchedulerGUI::connectInternalSignals(void)
{
	connect(ui.action_About, SIGNAL(triggered()), this, SLOT(about()));
	connect(ui.action_Find, SIGNAL(triggered()), this, SLOT(showSearchBar()));
	connect(closeSearchBarAct, SIGNAL(triggered()), this, SLOT(closeSearchBar()));
	connect(itsSearchLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(doSearch(const QString&)) );
	connect(itsSearchNextButton, SIGNAL(clicked()), this, SLOT(searchNext()));
	connect(itsSearchPreviousButton, SIGNAL(clicked()), this, SLOT(searchPrevious()));
	connect(itsSearchColumnOnlyCheckBox, SIGNAL(clicked()), this, SLOT(searchAgain()));
	connect(itsSearchLineEdit, SIGNAL(returnPressed()), this, SLOT(searchNext()));
	connect(itsTableView->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(toggleTableSort(int)));

	connect(ui.action_Move_task, SIGNAL(triggered()), this, SLOT(openMoveTasksDialog()));

	connect(ui.actionHorizontal_view, SIGNAL(triggered()), this, SLOT(horizontalView()));
	connect(ui.actionVertical_view, SIGNAL(triggered()), this, SLOT(verticalView()));
	connect(ui.actionGraphicalViewShow, SIGNAL(toggled(bool)), this, SLOT(graphicalViewEnable(bool)));
	connect(ui.actionTableViewShow, SIGNAL(toggled(bool)), this, SLOT(tableViewEnable(bool)));
	connect(ui.actionSelect_colums_to_view, SIGNAL(triggered()), this, SLOT(viewColumns()));
	connect(ui.actionConnect_to_Data_monitor, SIGNAL(triggered()), this, SLOT(connectToDataMonitor()));

	// Graphic scene signals
	connect(itsController, SIGNAL(schedulerSettingsChanged()), this, SLOT(applyGraphicSceneChanges()));
	connect(itsZoomInAction, SIGNAL(triggered()), this, SLOT(zoomIn()));
	connect(itsZoomOutAction, SIGNAL(triggered()), this, SLOT(zoomOut()));
	connect(itsTaskTypeColorModeAction, SIGNAL(triggered()), this, SLOT(toggleTaskColorMode()));
	connect(itsCenterOnNow, SIGNAL(triggered()), this, SLOT(scrollToNow()));
	connect(itsGraphicResourceScene, SIGNAL(setEnableZoomIn(bool)), this, SLOT(setZoomInEnable(bool)));
	connect(itsGraphicResourceScene, SIGNAL(setEnableZoomOut(bool)), this, SLOT(setZoomOutEnable(bool)));
	connect(itsGraphicResourceScene, SIGNAL(clickedGraphicTask(unsigned)), this, SLOT(handleGraphicViewClick(unsigned)));
	connect(itsGraphicResourceScene, SIGNAL(showTaskDialog(const Task *)), this, SLOT(showTaskDialog(const Task *)));
	connect(itsTableView, SIGNAL(mouseClick(const QModelIndex &, QMouseEvent *)), this, SLOT(handleTableClick(const QModelIndex &, QMouseEvent *)));
	connect(itsTableView, SIGNAL(tableContextMenuRequest(const QModelIndex &, QContextMenuEvent *)), this, SLOT(showTableContextMenu(const QModelIndex &, QContextMenuEvent *)));
	connect(itsTableView->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(handleTableHorHeaderClick(int)));
    connect(itsTableObsEnabled, SIGNAL(stateChanged(int)), this, SLOT(filterTable(void)));
    connect(itsTablePipesEnabled, SIGNAL(stateChanged(int)), this, SLOT(filterTable(void)));
    connect(itsTableProjectFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(filterTable(void)));

	connect(ui.actionPublish_schedule, SIGNAL(triggered()), this, SLOT(openPublishDialog()));
    connect(&itsPublishDialog, SIGNAL(goPublish(const scheduleWeekVector &)), this, SLOT(doPublish(const scheduleWeekVector &)));
	connect(&itsViewColumnsDialog, SIGNAL(viewColumns(const std::vector<unsigned int> &)), this, SLOT(setViewColumns(const std::vector<unsigned int> &)));
	connect(itsGraphicView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateTimeLinePosAfterScroll(void))); // signal to repaint time line at the top of the view
	connect(itsGraphicView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateStationTimeLinesAfterScroll(void))); // signal to keep station names visible
}

void SchedulerGUI::connectToDataMonitor(void) {
	if (itsController->isDataMonitorConnected()) {
		itsController->disconnectDataMonitor();
	}
	else {
		if (!itsController->connectToDataMonitor()) { // Controller will update the connection icon
			QMessageBox::critical(0, tr("No connection to Data Monitor"),
					tr("Could not connect to the Data Monitor.\n Please check Data Monitor connection settings."));
		}
	}
}

void SchedulerGUI::openMoveTasksDialog(void) {
	ShiftTasksDialog shiftDlg(this, itsController);
}

void SchedulerGUI::viewColumns(void) {
	itsViewColumnsDialog.show();
}

void SchedulerGUI::setViewColumns(const std::vector<unsigned int> &columns) {
	for (int i = 0; i < itsModel->columnCount(); ++i) {
		itsTableView->hideColumn(i);
	}
	for (std::vector<unsigned int>::const_iterator it = columns.begin(); it != columns.end(); ++it) {
		itsTableView->showColumn(*it);
	}
	itsTableView->resizeColumnsToContents();
//	setDefaultColumnWidths();
}

void SchedulerGUI::zoomIn(void) const {
	itsGraphicResourceScene->zoomIn();
}

void SchedulerGUI::zoomOut(void) const {
	itsGraphicResourceScene->zoomOut();
}

void SchedulerGUI::setTaskColorMode(Task::task_color_mode color_mode) const {
	itsGraphicResourceScene->setTaskColorMode(color_mode);
}

void SchedulerGUI::toggleTaskColorMode(void) const {
	if (itsGraphicResourceScene->getTaskColorMode() == Task::TASK_STATUS_COLOR_MODE) {
		itsGraphicResourceScene->setTaskColorMode(Task::TASK_TYPE_COLOR_MODE);
		itsTaskTypeColorModeAction->setIconText("Type color mode");

		}
	else {
		itsGraphicResourceScene->setTaskColorMode(Task::TASK_STATUS_COLOR_MODE);
		itsTaskTypeColorModeAction->setIconText("Status color mode");
	}

}

void SchedulerGUI::scrollToNow(void) const {
	itsGraphicResourceScene->scrollToNow();
}

void SchedulerGUI::setEnableScheduleMenuItems(bool enabled) {
	//ui.action_Save_schedule->setEnabled(enabled);
	ui.action_Save_schedule_as->setEnabled(enabled);
	ui.action_Save_task_list->setEnabled(enabled);
	ui.action_Close_schedule->setEnabled(enabled);
	ui.action_Optimize_schedule->setEnabled(enabled);
	ui.action_Create_initial_schedule->setEnabled(enabled);
	ui.action_Balance_schedule->setEnabled(enabled);
	ui.action_Add_task->setEnabled(enabled);
	ui.action_Delete_task->setEnabled(enabled);
	ui.actionPublish_schedule->setEnabled(itsEnablePublish);
}

void SchedulerGUI::setEnableTaskListMenuItems(bool enabled) {
	ui.action_Close_schedule->setEnabled(enabled);
	ui.action_Save_task_list->setEnabled(enabled);
	ui.action_Optimize_schedule->setEnabled(enabled);
	ui.action_Create_initial_schedule->setEnabled(enabled);
	ui.action_Balance_schedule->setEnabled(enabled);
	ui.action_Find->setEnabled(enabled);
}

// focusOnTaskInTable is called when user clicks on a task in the graphic view
void SchedulerGUI::handleGraphicViewClick(unsigned taskID) {
	QList<QStandardItem *> items = itsModel->findItems(QString::number(taskID), Qt::MatchExactly, TASK_ID);
	if (!items.empty()) {
		QModelIndex idx = itsModel->indexFromItem(items.first());
		int h = itsTableView->horizontalScrollBar()->value();
		itsTableView->scrollTo(idx);
		itsTableView->horizontalScrollBar()->setValue(h);
		itsTableView->selectRow(idx.row());
	}
}

// focusOnTaskInGraphicView is called when user clicks on a task in the table
void SchedulerGUI::handleTableClick(const QModelIndex &index, QMouseEvent *event) { // const Qt::KeyboardModifiers &modifier_keys
	this->blockSignals(true);
	itsTableView->blockSignals(true);
	if (event->button() == Qt::LeftButton) {
		Qt::KeyboardModifiers modifier_keys = event->modifiers();
		QSet<int> selectedRows(itsTableView->selectedRows());

		// reset search if user selected different column and checkbox this column only is checked
		if (itsSearchColumnOnlyCheckBox->isChecked() && (columnSearched != itsTableView->currentIndex().column())) {
			searchResult.clear();
		}

		if ((modifier_keys == Qt::ControlModifier) || (modifier_keys == Qt::ShiftModifier)) {
			itsTableView->setSelectionMode(QAbstractItemView::MultiSelection);
			itsSelectedTableTasks.clear();
			int row;
			foreach (row, selectedRows) {
				itsSelectedTableTasks.push_back(itsModel->index(row, TASK_ID).data().toUInt());
			}
			itsController->setSelectedTasks(itsSelectedTableTasks);

			itsGraphicResourceScene->deselectAllGraphicTasks();

			for (std::vector<unsigned>::const_iterator it = itsSelectedTableTasks.begin(); it != itsSelectedTableTasks.end(); ++it) {
				itsGraphicResourceScene->selectGraphicTask(*it);
			}
		}
		else { // single selection
			itsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
			itsSelectedTaskID = itsModel->index(index.row(), TASK_ID).data().toUInt();
			itsGraphicResourceScene->deselectAllGraphicTasks();
			itsGraphicResourceScene->selectGraphicTask(itsSelectedTaskID);
			itsController->setSelectedTask(itsSelectedTaskID);
//			itsController->selectTask(itsSelectedTaskID, true, false);
		}

		itsGraphicResourceScene->ensureTaskIsVisible(itsModel->data(itsModel->index(index.row(), 0), TASK_ID).toUInt());
	}
	else if (event->button() == Qt::RightButton) {
		itsSelectedTaskID = itsModel->index(index.row(), TASK_ID).data().toUInt();
		if (!itsController->isSelected(itsSelectedTaskID)) {
			itsSelectedTaskID = itsModel->index(index.row(), TASK_ID).data().toUInt();
			itsGraphicResourceScene->deselectAllGraphicTasks();
			itsGraphicResourceScene->selectGraphicTask(itsSelectedTaskID);
			itsController->setSelectedTask(itsSelectedTaskID);
			itsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
		}
	}
	itsTableView->blockSignals(false);
	this->blockSignals(false);
}

void SchedulerGUI::showTableContextMenu(const QModelIndex &index, QContextMenuEvent *event) {
	itsSelectedTaskID = itsModel->data(itsModel->index(index.row(), TASK_ID)).toUInt();
	if (itsController->multipleSelected()) {
		QMenu menu,submenu("Select tasks within these groups");
		QAction *action = menu.addAction("Unschedule multiple tasks");
		connect(action, SIGNAL(triggered()), this, SLOT(unscheduleSelectedTasks(void)));
		action = menu.addAction("set PRESCHEDULED");
		connect(action, SIGNAL(triggered()), this, SLOT(preScheduleSelectedTasks()));
		action = menu.addAction("set SCHEDULED");
		connect(action, SIGNAL(triggered()), this, SLOT(scheduleSelectedTasks()));
		action = menu.addAction("Put on hold now!");
		connect(action, SIGNAL(triggered()), this, SLOT(setSelectedTasksOnHold(void)));
		action = menu.addAction("Move/Redistribute tasks");
		connect(action, SIGNAL(triggered()), this, SLOT(moveSelectedTasks(void)));
		submenu.setTitle("Select tasks within these groups");
		menu.addMenu(&submenu);
		action = submenu.addAction("All tasks");
		connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentTaskGroupsAll(void)));
		action = submenu.addAction("Observations");
		connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentTaskGroupsObservation(void)));
		action = submenu.addAction("Calibration Pipelines");
		connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentTaskGroupsCalibrator(void)));
		action = submenu.addAction("Target Pipelines");
		connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentTaskGroupsTarget(void)));
		action = submenu.addAction("Pre-processing Pipelines");
		connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentTaskGroupsPreProcessing(void)));
        action = submenu.addAction("Long-Baseline Pipelines");
        connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentTaskGroupsLongBaseline(void)));
        action = submenu.addAction("Imaging Pipelines");
		connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentTaskGroupsImaging(void)));
        action = submenu.addAction("Pulsar Pipelines");
        connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentTaskGroupsPulsar(void)));
        action = menu.addAction("Copy tasks");
		connect(action, SIGNAL(triggered()), this, SLOT(copySelectedTasks(void)));
		action = menu.addAction("Delete tasks");
		connect(action, SIGNAL(triggered()), this, SLOT(deleteSelectedTasks(void)));
		action = menu.addAction("multi-edit");
		connect(action, SIGNAL(triggered()), this, SLOT(showSelectedTasksProperties(void)));
		menu.exec(event->globalPos());
	}
	else {
		const Task *pTask(itsController->getTask(itsSelectedTaskID));

		if (pTask) {
			Task::task_status status = pTask->getStatus();
			QMenu menu, submenu;
			QAction *action(0);
			if ((status == Task::PRESCHEDULED) || (status == Task::SCHEDULED)) {
				action = menu.addAction("Unschedule task");
				connect(action, SIGNAL(triggered()), this, SLOT(unscheduleTask(void)));
			}
			if ((status < Task::PRESCHEDULED) || (status == Task::SCHEDULED)) {
				action = menu.addAction("set PRESCHEDULED");
				connect(action, SIGNAL(triggered()), this, SLOT(preScheduleTask()));
			}
			if (status == Task::PRESCHEDULED) {
				action = menu.addAction("set SCHEDULED");
				connect(action, SIGNAL(triggered()), this, SLOT(scheduleTask()));
			}
			if (status <= Task::SCHEDULED) {
					if (status != Task::ON_HOLD) {
						action = menu.addAction("Put on hold now!");
						connect(action, SIGNAL(triggered()), this, SLOT(setTaskOnHold(void)));
					}
					action = menu.addAction("Move task");
					connect(action, SIGNAL(triggered()), this, SLOT(moveSelectedTasks(void)));
			}
			if (pTask->getGroupID() != 0) {
				submenu.setTitle("Select tasks within this group");
				menu.addMenu(&submenu);
				action = submenu.addAction("All tasks");
				connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentTaskGroupsAll(void)));
				action = submenu.addAction("Observations");
				connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentTaskGroupsObservation(void)));
				action = submenu.addAction("Calibration Pipelines");
				connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentTaskGroupsCalibrator(void)));
				action = submenu.addAction("Target Pipelines");
				connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentTaskGroupsTarget(void)));
				action = submenu.addAction("Pre-processing Pipelines");
				connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentTaskGroupsPreProcessing(void)));
                action = submenu.addAction("Long-Baseline Pipelines");
                connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentTaskGroupsLongBaseline(void)));
                action = submenu.addAction("Imaging Pipelines");
				connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentTaskGroupsImaging(void)));
                action = submenu.addAction("Pulsar Pipelines");
                connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentTaskGroupsPulsar(void)));
            }

			action = menu.addAction("Copy task");
			connect(action, SIGNAL(triggered()), this, SLOT(copySelectedTask(void)));
			if (status == Task::ABORTED) {
				action = menu.addAction("Reschedule aborted task");
				connect(action, SIGNAL(triggered()), this, SLOT(rescheduleAbortedTask(void)));
			}
			action = menu.addAction("Delete task");
			connect(action, SIGNAL(triggered()), this, SLOT(deleteSelectedTasks(void)));

			action = menu.addAction("Properties");
			connect(action, SIGNAL(triggered()), this, SLOT(showTaskProperties(void)));

			action = menu.addAction("SAS state history");
			connect(action, SIGNAL(triggered()), this, SLOT(showStateHistory(void)));

			int sasTreeID(pTask->getSASTreeID());
			if (sasTreeID) {
				action = menu.addAction("SAS tree viewer");
				connect(action, SIGNAL(triggered()), this, SLOT(openSASTreeViewer(void)));
                action = menu.addAction("View meta-data");
                connect(action, SIGNAL(triggered()), this, SLOT(openMetaDataViewer(void)));
            }

			menu.exec(event->globalPos());
		}
	}
}

void SchedulerGUI::filterTable(void) {
    int nrRows(itsModel->rowCount());
    bool obsEnabled(itsTableObsEnabled->isChecked());
    bool pipesEnabled(itsTablePipesEnabled->isChecked());
    bool projectFilterEnabled(itsTableProjectFilter->currentIndex() != 0);
    for (int row = 0; row < nrRows; ++row) {
        if (obsEnabled && pipesEnabled) {
            if (projectFilterEnabled) {
                if (itsTableProjectFilter->currentText() == itsModel->data(itsModel->index(row, PROJECT_ID)).toString()) {
                    itsTableView->showRow(row);
                }
                else itsTableView->hideRow(row);
            }
            else itsTableView->showRow(row);
        }
        else {
            Task::task_type type = static_cast<Task::task_type>(itsModel->data(itsModel->index(row, TASK_TYPE), USERDATA_ROLE).toInt());
            if (type == Task::OBSERVATION || type == Task::RESERVATION || type == Task::MAINTENANCE) {
                if (obsEnabled) {
                    if (projectFilterEnabled) {
                        if (itsTableProjectFilter->currentText() == itsModel->data(itsModel->index(row, PROJECT_ID)).toString()) {
                            itsTableView->showRow(row);
                        }
                        else itsTableView->hideRow(row);
                    }
                    else itsTableView->showRow(row);
                }
                else itsTableView->hideRow(row);
            }
            else if (type == Task::PIPELINE) {
                if (pipesEnabled) {
                    if (projectFilterEnabled) {
                        if (itsTableProjectFilter->currentText() == itsModel->data(itsModel->index(row, PROJECT_ID)).toString()) {
                            itsTableView->showRow(row);
                        }
                        else itsTableView->hideRow(row);
                    }
                    else itsTableView->showRow(row);
                }
                else itsTableView->hideRow(row);
            }
        }
    }
}

void SchedulerGUI::unscheduleTask(void) {
	itsController->unscheduleTask(itsSelectedTaskID);
}

void SchedulerGUI::unscheduleSelectedTasks(void) {
	itsController->unscheduleSelectedTasks();
}

void SchedulerGUI::preScheduleTask(void) {
	itsController->scheduleTask(itsSelectedTaskID, Task::PRESCHEDULED);
}

void SchedulerGUI::scheduleTask(void) {
	itsController->scheduleTask(itsSelectedTaskID, Task::SCHEDULED);
}

void SchedulerGUI::preScheduleSelectedTasks(void) {
	itsController->scheduleSelectedTasks(Task::PRESCHEDULED);
}

void SchedulerGUI::scheduleSelectedTasks(void) {
	itsController->scheduleSelectedTasks(Task::SCHEDULED);
}

void SchedulerGUI::setTaskOnHold(void) {
	itsController->setTaskOnHold(itsSelectedTaskID);
}

void SchedulerGUI::selectCurrentTaskGroupsAll(void) {
	itsController->selectCurrentTaskGroups();
}

void SchedulerGUI::selectCurrentTaskGroupsObservation(void) {
    itsController->selectCurrentTaskGroups(SEL_OBSERVATIONS);
}

void SchedulerGUI::selectCurrentTaskGroupsCalibrator(void) {
    itsController->selectCurrentTaskGroups(SEL_CALIBRATOR_PIPELINES);
}

void SchedulerGUI::selectCurrentTaskGroupsTarget(void) {
    itsController->selectCurrentTaskGroups(SEL_TARGET_PIPELINES);
}

void SchedulerGUI::selectCurrentTaskGroupsImaging(void) {
    itsController->selectCurrentTaskGroups(SEL_IMAGING_PIPELINES);
}

void SchedulerGUI::selectCurrentTaskGroupsPreProcessing(void) {
    itsController->selectCurrentTaskGroups(SEL_PREPROCESSING_PIPELINES);
}

void SchedulerGUI::selectCurrentTaskGroupsLongBaseline(void) {
    itsController->selectCurrentTaskGroups(SEL_LONGBASELINE_PIPELINES);
}

void SchedulerGUI::selectCurrentTaskGroupsPulsar(void) {
    itsController->selectCurrentTaskGroups(SEL_PULSAR_PIPELINES);
}

void SchedulerGUI::copySelectedTask(void) {
	itsController->copyTask(itsSelectedTaskID);
}

void SchedulerGUI::setSelectedTasksOnHold(void) {
	itsController->setSelectedTasksOnHold();
}

void SchedulerGUI::rescheduleAbortedTask(void) {
	itsController->rescheduleAbortedTask(itsSelectedTaskID);
}

void SchedulerGUI::deleteSelectedTasks(void) {
	itsController->deleteSelectedTasks();
}

void SchedulerGUI::moveSelectedTasks(void) {
	itsController->openMoveTasksDialog();
}

void SchedulerGUI::copySelectedTasks(void) {
	itsController->copySelectedTasks();
}

void SchedulerGUI::showTaskProperties(void) {
	const Task *pTask = itsController->getTask(itsSelectedTaskID);
	if (pTask) {
		showTaskDialog(pTask);
	}
}

void SchedulerGUI::showSelectedTasksProperties(void) {
	itsController->multiEditSelectedTasks();
}

void SchedulerGUI::showStateHistory(void) {
	itsController->showTaskStateHistory(itsSelectedTaskID);
}

void SchedulerGUI::openSASTreeViewer(void) const {
	const Task *pTask(itsController->getTask(itsSelectedTaskID));
	if (pTask) {
        itsController->openSASTreeViewer(pTask->getSASTreeID());
	}
}

void SchedulerGUI::openMetaDataViewer(void) const {
    const Task *pTask(itsController->getTask(itsSelectedTaskID));
    if (pTask) {
        itsController->openMetaDataViewer(pTask->getSASTreeID());
    }
}

void SchedulerGUI::handleTableHorHeaderClick(int row) {
//	itsTableView->setSelectionMode(QAbstractItemView::SelectRows);
	QModelIndex idx = itsModel->index(row, TASK_ID);
	unsigned int taskID = itsModel->data(idx).toUInt();
	if (itsController->isSelected(taskID)) { // task is currently selected
		itsController->deselectTask(taskID, false, true);
	}
	else { // task is not currently selected
		itsController->selectTask(taskID, false, true);
	}
}

void SchedulerGUI::selectRows(const std::vector<unsigned> &selectedTasks) {
	itsTableView->clearSelection();
	for (std::vector<unsigned>::const_iterator it = selectedTasks.begin(); it != selectedTasks.end(); ++it) {
		QList<QStandardItem *> items = itsModel->findItems(QString::number(*it), Qt::MatchExactly, TASK_ID);
		if (!items.isEmpty()) {
			itsTableView->selectRow(itsModel->indexFromItem(items.first()).row());
		}
	}
}

void SchedulerGUI::clearSelection(void) {
	itsSelectedTableTasks.clear();
	itsTableView->clearSelection();
	itsGraphicResourceScene->deselectAllGraphicTasks();
}

void SchedulerGUI::selectTableTask(unsigned taskID, bool tableclicked) {
	std::vector<unsigned>::const_iterator it = std::find(itsSelectedTableTasks.begin(), itsSelectedTableTasks.end(), taskID);
	if (it == itsSelectedTableTasks.end()) { // if not yet selected
		itsSelectedTableTasks.push_back(taskID);
		itsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
		if (tableclicked) { // when table was clicked we have to re-select all selected rows otherwise the clicked row will not be selected
			itsTableView->clearSelection();
			for (int row = 0; row < itsModel->rowCount(); ++row) {
				if (std::find(itsSelectedTableTasks.begin(), itsSelectedTableTasks.end(), itsModel->data(itsModel->index(row, TASK_ID)).toUInt()) != itsSelectedTableTasks.end()) {
					itsTableView->selectRow(row);
				}
			}
		}
		else { // this function call was not caused by a table click, in this case we only have to select the row of the newly selected task without having to re-select all other selected tasks
			for (int row = 0; row < itsModel->rowCount(); ++row) {
				if (itsModel->data(itsModel->index(row, TASK_ID)).toUInt() == taskID) {
					itsTableView->selectRow(row);
					break;
				}
			}
		}
		itsTableView->setSelectionBehavior(QAbstractItemView::SelectItems);
	}
}


void SchedulerGUI::deselectTableTask(unsigned taskID) {
	std::vector<unsigned>::iterator it = std::find(itsSelectedTableTasks.begin(), itsSelectedTableTasks.end(), taskID);
	if (it != itsSelectedTableTasks.end()) {
		itsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
		itsSelectedTableTasks.erase(it);
		itsTableView->clearSelection();
		for (std::vector<unsigned>::const_iterator it = itsSelectedTableTasks.begin(); it != itsSelectedTableTasks.end(); ++it) {
			QList<QStandardItem *> items = itsModel->findItems(QString::number(*it), Qt::MatchExactly, TASK_ID);
			if (!items.isEmpty()) {
				itsTableView->selectRow(items.first()->row()); // select/deselect the row depending on its current selection state
			}
		}
		itsTableView->setSelectionBehavior(QAbstractItemView::SelectItems);
	}
}


void SchedulerGUI::selectTasks(const std::vector<unsigned> &tasks) {
	this->blockSignals(true);
	QAbstractItemView::SelectionBehavior prevBehaviour(itsTableView->selectionBehavior());
	itsGraphicResourceScene->deselectAllGraphicTasks();
	itsTableView->clearSelection();
	itsSelectedTableTasks.clear();
	if (tasks.size() > 1) {
		itsTableView->setSelectionMode(QAbstractItemView::MultiSelection);
	}
	else {
		itsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
	}
	itsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	for (std::vector<unsigned>::const_iterator it = tasks.begin(); it != tasks.end(); ++it) {
		itsGraphicResourceScene->selectGraphicTask(*it);
		itsSelectedTableTasks.push_back(*it);
		QList<QStandardItem *> items = itsModel->findItems(QString::number(*it), Qt::MatchExactly, TASK_ID);
		if (!items.isEmpty()) {
			itsTableView->selectRow(itsModel->indexFromItem(items.first()).row());
		}
	}
	itsTableView->setSelectionBehavior(prevBehaviour);
	this->blockSignals(false);
}

void SchedulerGUI::selectTask(unsigned taskID, bool singleSelection, bool selectRows, bool tableClick) { // should be used by the controller only
	if (singleSelection) {
		itsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
		itsGraphicResourceScene->deselectAllGraphicTasks();
		itsSelectedTableTasks.clear();
	}
	else {
		itsTableView->setSelectionMode(QAbstractItemView::MultiSelection);
	}
	itsGraphicResourceScene->selectGraphicTask(taskID);
	if (selectRows) {
		selectTableTask(taskID, tableClick);
	}
	else {
		std::vector<unsigned>::iterator it = std::find(itsSelectedTableTasks.begin(), itsSelectedTableTasks.end(), taskID);
		if (it == itsSelectedTableTasks.end()) {
			itsSelectedTableTasks.push_back(taskID);
		}
	}
}

void SchedulerGUI::deselectTask(unsigned taskID, bool singleSelection, bool selectRows) { // should be used by the controller only
	if (singleSelection) {
		itsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
	}
	else {
		itsTableView->setSelectionMode(QAbstractItemView::MultiSelection);
	}
	itsGraphicResourceScene->deselectGraphicTask(taskID);
	if (selectRows) {
		deselectTableTask(taskID);
	}
	else {
		std::vector<unsigned>::iterator it = std::find(itsSelectedTableTasks.begin(), itsSelectedTableTasks.end(), taskID);
		if (it == itsSelectedTableTasks.end()) {
			itsSelectedTableTasks.erase(it);
		}
	}
}

bool SchedulerGUI::checkPublishSettings(void) {
	QString path;
	if (Controller::theSchedulerSettings.publishLocal()) {
		path = Controller::theSchedulerSettings.getLocalPublishPath();
		if (path.isEmpty()) {
			// publish path not set
			QMessageBox::warning(this, tr("Local publish path not set"),
					tr("The local publish path is not defined. Please set the path.\n"
							"menu Settings -> Schedule settings -> Publishing"),
							QMessageBox::Ok, QMessageBox::Ok);
			setStatusText("Publishing aborted");
			return false;
		}
#ifdef Q_OS_UNIX
		if (path.right(1).compare("/") != 0) {
			path += '/';
			Controller::theSchedulerSettings.setLocalPublishPath(path);
		}
#elif defined(Q_OS_WIN)
		if (path.right(1).compare("\\") != 0) {
			path += "\\";
			Controller::theSchedulerSettings.setLocalPublishPath(path);
		}
#endif
		QFileInfo publishPath(path);
		if (!publishPath.isDir()) {
			itsPublishDialog.hide();
			QMessageBox::warning(this, tr("Publish path not valid"),
					tr("The Publish path does not point to an existing directory.\nPlease enter a correct directory path.\n"
							"menu Settings -> Schedule settings -> Publishing"),
							QMessageBox::Ok, QMessageBox::Ok);
			setStatusText("Publishing aborted");
			return false;
		}
	}
	else { // REMOTE PUBLISHING
		if (Controller::theSchedulerSettings.getSchedulerAccountName().isEmpty()) {
			QMessageBox::warning(this, tr("Scheduler account name not set"),
					tr("The scheduler account name is not set.\n Please specify the account used for uploading the schedule to the web server.\n"
							"menu Settings -> Schedule settings -> Publishing"),
							QMessageBox::Ok, QMessageBox::Ok);
			setStatusText("Publishing aborted");
			return false;
		}
#ifdef Q_OS_WIN
		if (Controller::theSchedulerSettings.getPrivateKeyFile().isEmpty()) {
			QMessageBox::warning(this, tr("Private key filename not specified"),
					tr("The private key filename is not set.\n Please specify the private key file for automated logon to the web server.\n"
							"menu Settings -> Schedule settings -> Publishing"),
							QMessageBox::Ok, QMessageBox::Ok);
			setStatusText("Publishing aborted");
			return false;
		}
		else if (!QFileInfo(Controller::theSchedulerSettings.getPrivateKeyFile()).exists()) {
			QMessageBox::warning(this, tr("Private key file not found"),
					tr("The specified private key file could not be found.\n Please specify the private key file for automated logon to the web server.\n"
							"menu Settings -> Schedule settings -> Publishing"),
							QMessageBox::Ok, QMessageBox::Ok);
			setStatusText("Publishing aborted");
			return false;
		}
#endif
		if (Controller::theSchedulerSettings.getWebServerName().isEmpty()) {
			QMessageBox::warning(this, tr("Web server name not set"),
					tr("The web server name is not set.\n Please specify the name of the web server.\n"
							"menu Settings -> Schedule settings -> Publishing"),
							QMessageBox::Ok, QMessageBox::Ok);
			setStatusText("Publishing aborted");
			return false;
		}
		path = Controller::theSchedulerSettings.getWebServerPublishPath();
		if (path.isEmpty()) { // publish path not set
			QMessageBox::warning(this, tr("Web server publish path not set"),
					tr("The web server publish path is not defined. Please set the path.\n"
							"menu Settings -> Schedule settings -> Publishing"),
							QMessageBox::Ok, QMessageBox::Ok);
			setStatusText("Publishing aborted");
			return false;
		}
	}
	return true;
}



void SchedulerGUI::openPublishDialog(void) {
//	bool publish_local = Controller::theSchedulerSettings.getLocalPublishPath();
	if (checkPublishSettings()) {
		itsPublishDialog.show();
	}
}

void SchedulerGUI::doPublish(const scheduleWeekVector &publish_weeks) {
    if (publish(publish_weeks)) {
        QStringList wkl;
        for (scheduleWeekVector::const_iterator it = publish_weeks.begin(); it != publish_weeks.end(); ++it) {
            wkl.append(QString::number(it->first));
        }
        QMessageBox::information(this, tr("Publishing successful"),
                              tr("Publishing finished successfully.\nThe following week numbers have been uploaded to the web-schedule:\n") +
                                 wkl.join(QChar(',')));
    }
    else {
        QMessageBox::critical(this, tr("Publishing finished with errors"),
                              tr("Publishing finished with errors.\nThe schedule could not correctly be uploaded to the webserver \n"));
    }
}

bool SchedulerGUI::publish(const scheduleWeekVector &publish_weeks) {
	setStatusText("Publishing in progress...");
	bool publish_local = Controller::theSchedulerSettings.publishLocal();
	QString local_path, web_server_name, account_name, server_path, privateKeyFileName, working_dir, systemCmd;
	QDir localPublishDir;
	working_dir = QDir::toNativeSeparators(QDir::currentPath());
	local_path = QDir::toNativeSeparators(Controller::theSchedulerSettings.getLocalPublishPath());
	localPublishDir.setPath(local_path);

    if (!publish_local) {
	// delete tmp directory if it exists already
		if (localPublishDir.exists("publish_tmp")) {
			FileUtils filemanip;
			localPublishDir.cd("publish_tmp");
			filemanip.removeDir(localPublishDir.canonicalPath());
			localPublishDir.cdUp();
		}
		// recreate the tmp dir in local publish directory, because we are going to delete this local directory structure after upload to web server
		localPublishDir.mkdir("publish_tmp");
		localPublishDir.cd("publish_tmp");
//		local_path = QDir::toNativeSeparators(localPublishDir.absolutePath());
		account_name = Controller::theSchedulerSettings.getSchedulerAccountName();
		web_server_name = Controller::theSchedulerSettings.getWebServerName();
		server_path = Controller::theSchedulerSettings.getWebServerPublishPath();
		privateKeyFileName = Controller::theSchedulerSettings.getPrivateKeyFile();
	}

    // remember current graphic view zoom factor and position
    zoom_level prevZoom = itsGraphicResourceScene->getZoomLevel();
    int xPrevScrollPos = itsGraphicView->horizontalScrollBar()->value();
    int yPrevScrollPos = itsGraphicView->verticalScrollBar()->value();

	setProgressBarMaximum(publish_weeks.size());
    itsGraphicResourceScene->setZoomLevel(ZM_DAY);
	Task::task_color_mode prevColorMode = itsGraphicResourceScene->getTaskColorMode();
	itsGraphicResourceScene->setTaskColorMode(Task::TASK_TYPE_COLOR_MODE);
	itsGraphicResourceScene->hideCurrentTimeLine();
	itsController->deselectAllTasks();
	itsGraphicView->repaint();

	int startPos, endPos;
	int height = static_cast<int>(itsGraphicResourceScene->sceneRect().height());
	std::string stations;
	int progress(0), imgWidth;
	QString weekName, weekNrStr, prevWeek, nextWeek, htmlFileName; // stationsPicStr
	QDateTime date = QDateTime::currentDateTime().toUTC();
	QLocale lc(QLocale::English, QLocale::UnitedStates);
	string dateStr = lc.toString(date,"dddd dd MMM yyyy hh:mm").toStdString();

	AstroDate startDate, endDate, nextWeekMonday;

	QString fullPath, yearStr, stationImgPath, stationsPicStr, weekImg;
	std::vector<QString> yearDirsToPublish;

	// generate the station picture
	stationImgPath = localPublishDir.canonicalPath() + QDir::toNativeSeparators("/stations.png");

	itsGraphicView->horizontalScrollBar()->setValue(0);
	itsGraphicView->verticalScrollBar()->setValue(0);

	QImage *img = new QImage(QSize(70, height), QImage::Format_ARGB32_Premultiplied);
	img->fill(0);
	QPainter *paint = new QPainter(img);
	itsGraphicResourceScene->render(paint, QRectF(), QRectF(0, 0, 70, height));
	paint->end();
	img->save(stationImgPath, "png"); //    "/2009/stations_week_52.png"
	delete paint;
	delete img;

	QFile stationImgFile(stationImgPath); // necessary later on

	const campaignMap &projects(itsController->getCampaignList());
	campaignMap::const_iterator plocit;

	// *** loop through the published weeks and upload the weeks
	for (scheduleWeekVector::const_iterator it = publish_weeks.begin(); it != publish_weeks.end(); ++it) {
		// step 1 generate the week schedule pictures
		startDate = it->second;
		endDate = it->second.addDays(6);
		nextWeekMonday = it->second.addDays(7);
		yearStr = QString::number(startDate.addDays(3).getYear());
        if (!localPublishDir.exists(yearStr)) { // create the year directory for the current week if it doesn't exist yet
            localPublishDir.mkdir(yearStr);
            // create remote directory even if it already exists
            // we have to do this with separate script because if the remote directory already exists then this script will return with error
            // and thus abort the uploading of the actual publish files
#if defined(Q_OS_WIN)
            QFile scriptFile("create_dir.txt");
            scriptFile.remove();
            if (!scriptFile.open(QIODevice::ReadWrite | QIODevice::Text)) {
                setStatusText("Publishing failed");
                disableProgressBar();
                return false;
            }
            QString scriptText = "option batch abort\noption confirm off\nopen " + account_name + "@" + web_server_name + "\n";
            scriptText += "mkdir " + server_path + "/" + yearStr + "\n";
            scriptText += "exit\n";
            scriptFile.write(scriptText.toStdString().c_str());
            scriptFile.close();
            systemCmd = working_dir + "\\WinSCP\\WinSCP.com /privatekey=\"" + privateKeyFileName + "\" /script=" + working_dir + "\\create_dir.txt";
            std::system(systemCmd.toStdString().c_str()); // creates the remote 'year' directory
            yearDirsToPublish.push_back(yearStr);
#endif
#ifdef Q_OS_UNIX
        systemCmd = "ssh " + account_name + "@" + web_server_name + " mkdir " + server_path + "/" + yearStr;
		std::system(systemCmd.toStdString().c_str()); // creates the remote 'year' directory
		yearDirsToPublish.push_back(yearStr);
#endif
		}

		fullPath = localPublishDir.absolutePath() + QDir::toNativeSeparators("/" + yearStr + "/");

		weekNrStr = QString::number(it->first);
		weekName = "week_" + weekNrStr; //     "week_52"
		stationsPicStr = "stations_" + weekName + ".png";
		weekImg = fullPath + weekName + ".png";

		// copy the station picture to the year directory with a week specific name (to prevent overwriting other weeks on the webserver)
		stationImgFile.copy(fullPath + stationsPicStr);

		// generate the week picture
		startPos = itsGraphicResourceScene->time2xPos(startDate);
		endPos = itsGraphicResourceScene->time2xPos(nextWeekMonday) + 1;
		imgWidth = endPos - startPos;
		img = new QImage(QSize(imgWidth, height), QImage::Format_ARGB32_Premultiplied);
		img->fill(0);
		paint = new QPainter(img);
		itsGraphicResourceScene->render(paint, QRectF(), QRectF(startPos, 0, imgWidth, height));
		paint->end();
		img->save(weekImg, "png");
		delete paint;
		delete img;

		prevWeek = QString("week_") + QString::number(startDate.subtractDays(7).getWeek()) + ".html";
		nextWeek = QString("week_") + QString::number(nextWeekMonday.getWeek()) + ".html";

		// step 2 generate the HTML pages
		const std::map<std::string, std::vector<Task *> > tasks = itsController->getPublishTasks(startDate, nextWeekMonday);
		ofstream file;
		htmlFileName = fullPath + weekName + ".html"; //    "/2009/week_52.html"
		file.open(htmlFileName.toStdString().c_str(), std::ios::out);
		if (file.is_open()) {
			file << " <!-- " << endl << "created by the LOFAR Scheduler" << endl
			<< "created on: " << dateStr << "<BR>" << endl << endl
			<< "LOFAR Scheduler version: " << SCHEDULER_VERSION << endl << "-->" << endl << endl
			<< "<SUB>Change date (UTC): " << dateStr << "</SUB><BR>" << endl
			<< "<TABLE BORDER='0' WIDTH=1100><TR><TD>" << endl
			<< "<IMG SRC='" << yearStr.toStdString() << "/" << stationsPicStr.toStdString() << "' WIDTH=70 HEIGHT=" << int2String(height)
			<< " STYLE='float: left; padding-right: 0px; border-style: none;' BORDER='0' ALT='LOFAR Schedule of week " << weekNrStr.toStdString() << "' TITLE='LOFAR Schedule of week " << weekNrStr.toStdString() << "'>" << endl
			<< "<IMG SRC='" << yearStr.toStdString() << "/" << weekName.toStdString() << ".png' WIDTH=" << imgWidth << " HEIGHT=" << int2String(height)
			<< " STYLE='border-style: none;' ALT='LOFAR Schedule of week " << weekNrStr.toStdString() << "' BORDER='0'";
			if (!tasks.empty()) {
				file << " USEMAP='#tasks'" << endl;
			}
			file << "></TD></TR></TABLE><BR>" << endl
			<< "<TABLE id='report'><TR class='fixed'><TD colspan='10'>LOFAR schedule tasks for week " << weekNrStr.toStdString() << ", " << yearStr.toStdString() << "   (click on a project to see its tasks)</TD></TR>\n"
			<< "<TR class='fixed'><TD WIDTH='50px'>ID</TD><TD WIDTH='200px'>Task</TD><TD WIDTH='100px'>Type</TD><TD WIDTH='100px'>Mode</TD><TD WIDTH='100px'>Stations</TD><TD WIDTH='150px'>Start (UTC)</TD><TD WIDTH='150px'>Stop (UTC)</TD><TD WIDTH='50px'>Duration</TD><TD WIDTH='200px'>Description</TD><td width='16px'></TR>" << endl;

			// insert task properties into project table
			//const std::vector<Task *> tasks = itsController->getScheduledTasksWithinPeriod(startDate, nextWeekMonday);

			for (std::map<std::string, std::vector<Task *> >::const_iterator pit = tasks.begin(); pit != tasks.end(); ++pit) {
				plocit = projects.find(pit->first);
				file << "<TR class='project'><TD colspan='9'><strong>" << pit->first;
				if (plocit != projects.end()) {
					file << " - " << plocit->second.title;
				}
				file << "</strong></TD><td><div class='arrow'></div></TR>" << endl;
				for (std::vector<Task *>::const_iterator tit = pit->second.begin(); tit != pit->second.end(); ++tit) {
                    if ((*tit)->isStationTask()) {
                        stations = static_cast<StationTask *>(*tit)->getStationNamesStr(',', 5, "<br>"); // <br> ipv \n werkt ook met ddrivetip
                    }
					file << "<TR><TD>" << (*tit)->getSASTreeID()
							 << "</TD><TD>" << (*tit)->getTaskName()
							 << "</TD><TD>"	<< (*tit)->getTypeStr()
							 << "</TD><TD>"	<< (*tit)->getProcessSubtypeStr()
							 << "</TD><TD onmouseover=\"ddrivetip('" << stations << "', 'yellow', 260)\" onmouseout=\"hideddrivetip()\">" << stations.substr(0,17);
					if (stations.length() > 17) {
						file << "...";
					}
					file << "</TD><TD>"
							<< (*tit)->getScheduledStart().toString() << "</TD><TD>"
							<< (*tit)->getScheduledEnd().toString() << "</TD><TD>"
							<< (*tit)->getDuration().toString(3) << "</TD><TD colspan='2'>"
							<< (*tit)->SASTree().description() << "</TD></TR>" << endl;
				}
			}

			file << "</TABLE>" << endl;

			// create the image maps
			if (!tasks.empty()) {
				file << "<MAP NAME='tasks'>\n";
				const tasksMap &graphicTasks = itsGraphicResourceScene->getGraphicTasks();
				unsigned int taskID;
				tasksMap::const_iterator gtit;
				for (std::map<std::string, std::vector<Task *> >::const_iterator pit = tasks.begin(); pit != tasks.end(); ++pit) {
					for (std::vector<Task *>::const_iterator tit = pit->second.begin(); tit != pit->second.end(); ++tit) {
						taskID = (*tit)->getID();
						gtit = graphicTasks.find(taskID);
						if (gtit != graphicTasks.end()) {
							for (std::vector<GraphicTask *>::const_iterator git = gtit->second.begin(); git != gtit->second.end(); ++git) {
								// COORDS are left, top, right, bottom
								file << "<AREA SHAPE='rect' COORDS='" << (*git)->left() - startPos << "," << (*git)->top() << ","
										<< (*git)->right() - startPos << "," << (*git)->bottom()
										<< "' NOHREF onMouseOver=\"ddrivetip('" << (*git)->toolTipHTML() << "', 'yellow', 250)\"; onMouseout=\"hideddrivetip()\">\n";
							}
						}
					}
				}
				file << "</MAP>\n";
			}

			file.close();
		}

		// update the progress bar after each created week
		updateProgressBar(progress++);
	}

	// we're done rendering now restore previous view settings
	itsGraphicResourceScene->setZoomLevel(prevZoom);
	itsGraphicResourceScene->setTaskColorMode(prevColorMode);
	itsGraphicResourceScene->showCurrentTimeLine();
	itsGraphicView->horizontalScrollBar()->setValue(xPrevScrollPos);
	itsGraphicView->verticalScrollBar()->setValue(yPrevScrollPos);
	itsGraphicView->repaint();

	// delete original station picture that is in the root publish directory
	QFile::remove(stationImgPath);

	// step 3 (optional publish to web) transfer files to web server and delete local copy
	if (!publish_local) {

		setStatusText((std::string("Uploading files to web server ") + web_server_name.toStdString()).c_str());
		// scp YYYY lofarsched@dop40:/www/astron.nl/lofar-schedule/    (where YYYY is the four digit year number)
#ifdef Q_OS_UNIX
		systemCmd = "cd " + localPublishDir.canonicalPath() + "; scp -r " + yearStr + " " + account_name + "@" + web_server_name + ":" + server_path;
#endif

        if (std::system(systemCmd.toStdString().c_str()) != 0) {
            disableProgressBar();
            setStatusText("Publishing failed");
            return false;
        }
	}

    setStatusText("Publishing successfull");
	disableProgressBar();
    return true;
}

void SchedulerGUI::tableViewEnable(bool checked) {
	if (checked) {
		itsTableView->show();
		itsTableDock->show();
	}
	else {
		itsTableView->hide();
		itsTableDock->hide();
	}
}

void SchedulerGUI::graphicalViewEnable(bool checked) {
	if (checked) {
		itsGraphicView->show();
		itsGraphicDock->show();
	}
	else {
		itsGraphicView->hide();
		itsGraphicDock->hide();
	}
}

void SchedulerGUI::applyGraphicSceneChanges(void) {
	itsGraphicResourceScene->applyNewScheduleTimeSpan();
//	itsGraphicView->repaint();
}

void SchedulerGUI::toggleTableSort(int column) {
	itsSortColumn = static_cast<data_headers>(column);
	if (itsSortOrder == Qt::AscendingOrder)
		itsSortOrder = Qt::DescendingOrder;
	else
		itsSortOrder = Qt::AscendingOrder;
	itsTableView->sortByColumn(itsSortColumn, itsSortOrder);
	emit tableSortChanged();
}

void SchedulerGUI::closeEvent(QCloseEvent *event)
{
	event->ignore();
	emit quitApp();
}

QString SchedulerGUI::openTaskFileDialog()
{
	return fileDialog(tr("Open Task list file"), "csv", tr("Task list files (*.csv)"));
}

QString SchedulerGUI::saveTaskFileDialog()
{
	return fileDialog(tr("Save Task list file"), "csv", tr("Task list files (*.csv)"), 1);
}

QString SchedulerGUI::openProjectDialog()
{
	return fileDialog(tr("Open Schedule project file"), "pro", tr("Scheduling project files (*.pro)"));
}

QString SchedulerGUI::saveProjectDialog()
{
	return fileDialog(tr("Save Schedule project file"), "pro", tr("Scheduling project files (*.pro)"), 1);
}

QString SchedulerGUI::fileDialog(const QString &title, const QString &def_suffix, const QString &filter, int save_load) {
	QFileDialog dialog;
	QFileInfo fi;
	QString path="";
    dialog.setNameFilters(filter.split('\n'));
	dialog.setWindowTitle(title);


	if (lastPath == "") {
		char *home = getenv("HOME");
		if (home) lastPath = home;
		else lastPath = ".";
	} else if (QFileInfo(lastPath).isDir()) {
		lastPath = QFileInfo(lastPath).canonicalPath();
	} else {
		lastPath = QFileInfo(lastPath).absoluteDir().canonicalPath();
	}

	dialog.setDirectory(lastPath);

	if (save_load == 0)
		dialog.setAcceptMode(QFileDialog::AcceptOpen);
	else
		dialog.setAcceptMode(QFileDialog::AcceptSave);

	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setConfirmOverwrite(false); // we'll do that

	dialog.setDefaultSuffix(def_suffix);

	bool good = false;

	while (!good) {

		path = "";

		if (!dialog.exec()) break;

		QStringList files = dialog.selectedFiles();
		if (files.empty()) break;
		path = *files.begin();
//		std::cout << "path:" << path.toStdString() << std::endl;

		fi = path;

		if (fi.isDir()) {
			QMessageBox::critical(0, tr("Directory selected"),
					tr("File \"%1\" is a directory").arg(path));
			continue;
		}
		if (save_load == 1) {
			if (fi.exists()) {
				if (QMessageBox::question(0, tr("File exists"),
						tr("The file \"%1\" already exists.\nDo you want to overwrite it?").arg(path),
						QMessageBox::Ok,
						QMessageBox::Cancel) != QMessageBox::Ok) {
					continue;
				}
			}
		}
		good = true;
	}
	lastPath = path;
	return path;
}


void SchedulerGUI::addUndo(const QString &undo_action) {
	itsUndoList.append(undo_action);
	setUndoText(QString("Undo ") + undo_action);
	ui.action_Undo->setEnabled(true);
}

void SchedulerGUI::removeUndo(const QString &undo_info) {
	for (QStringList::iterator it = itsUndoList.begin(); it != itsUndoList.end(); ++it) {
		if (undo_info.compare(*it) == 0) {
			itsUndoList.erase(it);
			if (!itsUndoList.isEmpty()) {
				setUndoText(QString("Undo ") + itsUndoList.back());
			}
			else {
				ui.action_Undo->setEnabled(false);
				setUndoText("Undo");
			}
			return;
		}
	}
}

void SchedulerGUI::undo(bool store_redo) {
	if (!itsUndoList.isEmpty()) {
		if (store_redo) {
			itsRedoList.append(itsUndoList.back());
			ui.action_Redo->setEnabled(true);
			setRedoText(QString("Redo ") + itsRedoList.back());
		}
		itsUndoList.pop_back();
		if (!itsUndoList.isEmpty()) { // still undo's left ?
			setUndoText(QString("Undo ") + itsUndoList.back());
		}
		else {
			setUndoText("Undo");
			ui.action_Undo->setEnabled(false);
		}
	}
}

void SchedulerGUI::redo(void) {
	if (!itsRedoList.isEmpty()) {
		itsUndoList.append(itsRedoList.back());
		ui.action_Undo->setEnabled(true);
		setUndoText(QString("Undo ") + itsUndoList.back());
		itsRedoList.pop_back();
		if (!itsRedoList.isEmpty()) { // still redo's left ?
			setRedoText(QString("Redo ") + itsRedoList.back());
		}
		else {
			setRedoText("Redo");
			ui.action_Redo->setEnabled(false);
		}
	}
}


void SchedulerGUI::removeLastUndo(void) {
	if (!itsUndoList.isEmpty()) {
		itsUndoList.pop_back();
		if (!itsUndoList.isEmpty()) { // still undo's left ?
			setUndoText(QString("Undo ") + itsUndoList.back());
		}
		else {
			setUndoText("Undo");
			ui.action_Undo->setEnabled(false);
		}
	}
}


void SchedulerGUI::removeTaskFromScene(unsigned taskID) {
	itsGraphicResourceScene->removeTaskFromScene(taskID);
}


void SchedulerGUI::parsetTreeView(const QString &parset, const OTDBtree &otdb_tree) {
	itsTreeViewer->view(parset, otdb_tree);
}

void SchedulerGUI::about()
{
	QString msg = "<b>LOFAR Scheduler</b><br>Version:";
	msg += SCHEDULER_VERSION;
	msg += "<br>Copyright: ASTRON 2009";

   QMessageBox::about(this, "About LOFAR Scheduler", msg);
}

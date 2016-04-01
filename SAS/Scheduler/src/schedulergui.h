/*
 * schedulergui.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Feb 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/schedulergui.h $
 *
 */

#ifndef SCHEDULERGUI_H
#define SCHEDULERGUI_H

#include <string>
#include <vector>
#include <map>
#include <QMainWindow>
#include <QFileInfo>
#include <QToolBar>
#include <QGraphicsView>
#include <QScrollBar>
#include "lofar_scheduler.h"
#include "ui_schedulergui.h"
#include "taskdialog.h"
#include "publishdialog.h"
#include "tablecolumnselectdialog.h"
#include "tableview.h"
#include "scheduletabledelegate.h"
#include "schedulerdatablock.h"
#include "qlofardatamodel.h"

class GraphicResourceScene;
class GraphicStorageScene;
class QBoxLayout;
class SchedulerData;
class QPushButton;
class QCheckBox;
class QTableWidgetItem;
//class QGraphicsView;
class QWidget;
class QDockWidget;
class QStatusBar;
class QProgressBar;
class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QSpacerItem;
class QAction;
class Task;
class Controller;
class ParsetTreeViewer;
class QLCDNumber;

class SchedulerGUI : public QMainWindow
{
    Q_OBJECT

public:
    SchedulerGUI(Controller *controller);
    ~SchedulerGUI();

    void connectInternalSignals(void); // for handling of internal signals
    const Ui::SchedulerGUIClass &getSchedulerGUIClass(void) const {return ui;}

	QString openTaskFileDialog(); // opens a file open dialog to select a task file to open
	QString saveTaskFileDialog(); // opens a save file dialog to save the task file
	QString openProjectDialog();
	QString saveProjectDialog();

	// handle to the scene for the controller to be able to connect signals
	GraphicResourceScene * scene(void) const {return itsGraphicResourceScene;}

	// Scheduler table methods
	void newTable(SchedulerData const & data);
//	void loadNewTable(SchedulerData const &data);
	void writeTableData(SchedulerData const &data); // updates the table data (to be used after the data has been changed)
	void updateTableTaskScheduleTimes(const Task &task);
	void updateTableTasksScheduleTimes(const std::vector<const Task *> &tasks);
    void updateGraphicTask(unsigned task_id);
    void updateTableTask(const Task *, int row=-1);
    void updateTask(const Task *); // updates a task in the GUI
//	void addRow();
//	void deleteRow();
    const ScheduleTableDelegate *getTableDelegate(void) const {return &itsDelegate;}
    void setErrorCells(const errorTasksMap & errorTasks) {itsModel->setErrorCells(errorTasks); }
    void repaintTable(void) {itsTableView->repaint();}
    bool isErrorIndex(const QModelIndex &index) const {return itsModel->isErrorIndex(index);}
    void clearErrorIndex(const QModelIndex &index) {itsModel->clearErrorIndex(index);}
    void addErrorIndex(const QModelIndex &index) {itsModel->addErrorIndex(index);}
    void clearErrorCell(unsigned int taskID, data_headers column) {itsModel->clearErrorCell(taskID, column);}
	void cleanup(void); // clean up gui and its data to initial settings
    void setSortCol(data_headers sortCol) {itsSortColumn = sortCol;}
    data_headers sortCol(void) const {return itsSortColumn;}
    void setSortOrd(Qt::SortOrder sort_order) {itsSortOrder = sort_order;}
    Qt::SortOrder sortOrd(void) const {return itsSortOrder;}
    QString fileDialog (const QString &title, const QString &def_suffix, const QString &filter, int save_load = 0);
    std::vector<unsigned> getSelectedRowsTaskIDs(void) const;
    std::vector<unsigned> getShownTaskIDs(void) const;

    void updateGraphicStations(); // updates all stations in the graphical view
    void updateGraphicTasks(const scheduledTasksMap &scheduledTasks, const reservationsMap &reservations, const inActiveTasksMap & inactiveTasks);
    void updateSceneTimeSpan(void);
    void clearTasks(void);
    void setProgressBarMaximum(int maxValue);
    void setStatusText(const char *status) {itsStatusBarStatusText->setText(status);}
    void clearStatusText(void) {itsStatusBarStatusText->clear();}
    void setStatusText(const QString &status) {setStatusText(status.toStdString().c_str());}
    void updateStatusBar(unsigned nr_scheduled, unsigned nr_unscheduled, unsigned nr_inactive, unsigned nr_reservations, unsigned nr_errortasks, unsigned nr_pipelines, int progress = 0);
    void updateProgressBar(int progress);// {itsStatusBarProgress->setValue(progress);}

    void disableProgressBar(void);

    //enable/disable menu items
//    void setSaveTaskListEnable(bool);
    void setDataMonitorConnectionButton(bool enable) {ui.actionConnect_to_Data_monitor->setChecked(enable);}
    void setEnableTaskListMenuItems(bool); // call after task list was loaded
    void setEnableScheduleMenuItems(bool); // call after a schedule was created
    void addUndo(const QString &undo_action);
    void removeUndo(const QString &undo_info);
    void removeLastUndo(void);
    void undo(bool store_redo = true);
    void redo(void);
    void clearUndo(void) { itsUndoList.clear(); ui.action_Undo->setText("Undo"); ui.action_Undo->setEnabled(false); }
    void clearRedo(void) { itsRedoList.clear(); ui.action_Redo->setText("Redo"); ui.action_Redo->setEnabled(false); }
    void clearUndoRedo(void) { clearUndo(); clearRedo();}
    void setUndoMenuItemEnabled(bool enabled) {ui.action_Undo->setEnabled(enabled);}
    void setRedoMenuItemEnabled(bool enabled) {ui.action_Redo->setEnabled(enabled);}
	TaskDialog *taskDialog(void) const {return itsTaskDialog;}
	void updateTaskDialogStations(void);
	void updateTaskDialog(const Task *task=0) {itsTaskDialog->update(task);}
	void addTaskDialog(unsigned int taskID) {itsTaskDialog->addTask(taskID);}
//	void addReservationDialog(unsigned int taskID) {itsTaskDialog->addReservation(taskID);}
    void addTask(const Task *pTask);
	void deleteTaskFromGUI(unsigned taskID, Task::task_type type = Task::OBSERVATION);

	void selectRows(const std::vector<unsigned> &selectedTasks);
	void selectTask(unsigned taskID, bool singleSelection, bool selectRows = true, bool tableClick = false);  // should be used by the controller only
	void selectTasks(const std::vector<unsigned> &tasks); // clears current selection and selects the supplied tasks
	void deselectTask(unsigned taskID, bool singleSelection, bool selectRows = true); // should be used by the controller only
//	void selectRow(int row);
//	void deselectRow(int row);
	void clearSelection(void);
	void multiEditTasks(std::vector<Task *> &tasks);
	// put the correct week numbers and dates in the publish week selection dialog
	void initPublishDialog(void) {itsPublishDialog.initPublishDialog();}
	void setSaveRequired(bool save_required);
	void setTestMode(bool enable_test_mode) {itsIsTestMode = enable_test_mode; updateWindowTitle(); }
	void setApplicationName(const QString &title) {itsWindowTitle = title; updateWindowTitle(); }
	void setCurrentFileName(const QString &fileName) {itsCurrentFileName = fileName; updateWindowTitle();}
	void clearCurrentFileName(void) {itsCurrentFileName = "no name"; updateWindowTitle();}
	void updateWindowTitle(void);
	void setTaskColorMode(Task::task_color_mode color_mode) const;
//	Task::task_color_mode getTaskColorMode(void) const {return itsGraphicResourceScene->getTaskColorMode();}
	void setEmptyThrashIcon(void) {ui.action_Thrashcan->setIcon(QIcon(tr(":/icons/empty_trash.png")));}
	void setFullThrashIcon(void) {ui.action_Thrashcan->setIcon(QIcon(tr(":/icons/full_trash.png")));}
	void updateDataMonitorConnectionStatus(void); // updates the data monitor connection icon according to current connection status
	void setExistingProjects(const campaignMap &projects);
	void removeTaskFromScene(unsigned taskID);
    void parsetTreeView(const QString &parset, const OTDBtree &otdb_tree = OTDBtree());
    void sortTable(void) {itsTableView->sortByColumn(itsSortColumn, itsSortOrder);}
    void loadProcessTypes(void) {itsTaskDialog->loadProcessTypes();}
    void updateSasDatabaseName(void);
    bool publish(const scheduleWeekVector &);
    void updateProjectsFilter(const campaignMap &);

private:
	void writeTasksToTable(const std::vector<Task *> &tasks, unsigned int startRow);
	inline void setUndoText(const QString &txt) {ui.action_Undo->setText(txt); ui.action_Undo->setToolTip(txt);}
	inline void setRedoText(const QString &txt) {ui.action_Redo->setText(txt); ui.action_Redo->setToolTip(txt);}

signals:
	void quitApp(void) const;
	void tableSortChanged(void) const;

private:
	int findTableTaskByID(unsigned ID, id_type IDtype) const;
    void closeEvent(QCloseEvent *event); // when user closes the main window
    void setShortcutKeys(void);
    void createSearchBar(void);
    void createStatusBar(void);
    void createTableDock(void);
    void createGraphicDock(void);
    void search(const QString& term);
    void clearSearch(void) {searchResult.clear(); search_iterator = 0;}
    void setDefaultColumnWidths(void);
    bool checkPublishSettings(void);
    void createMainToolbar(void);
    void selectTableTask(unsigned taskID, bool tableclicked = false);
    void deselectTableTask(unsigned taskID);

public slots:
    void doPublish(const scheduleWeekVector &);

private slots:
	void applyGraphicSceneChanges(void);
	void about(void); // shows about dialog
    void closeSearchBar(void) {itsSearchBar->hide();} // closes the search bar
	void showSearchBar(void); // shows the search bar
	void doSearch(const QString& term); // do a search on term
    void searchNext(void);
    void searchPrevious(void);
    void searchAgain(void);
    void toggleTableSort(int column);
    void horizontalView(void);
    void verticalView(void);
    void zoomIn(void) const;
    void zoomOut(void) const;
    void setZoomInEnable(bool enable) {itsZoomInAction->setEnabled(enable);}
    void setZoomOutEnable(bool enable) {itsZoomOutAction->setEnabled(enable);}
	void toggleTaskColorMode(void) const;
    void scrollToNow(void) const;

    void handleGraphicViewClick(unsigned taskID);
    void handleTableHorHeaderClick(int);
    void handleTableClick(const QModelIndex &index, QMouseEvent *event);
    void showTableContextMenu(const QModelIndex &index, QContextMenuEvent *event);
    void tableViewEnable(bool);
    void graphicalViewEnable(bool);
    void openPublishDialog(void);
    void viewColumns(void);
    void setViewColumns(const std::vector<unsigned int> &columns);
    void filterTable(void);
    void connectToDataMonitor(void);

    void updateTimeLinePosAfterScroll(void);
    void updateStationTimeLinesAfterScroll(void);

	void unscheduleTask(void);
	void unscheduleSelectedTasks(void);
	void preScheduleTask(void);
	void scheduleTask(void);
	void preScheduleSelectedTasks(void);
	void scheduleSelectedTasks(void);
	void setTaskOnHold(void);
	void copySelectedTask(void);
	void selectCurrentTaskGroupsAll(void);
	void selectCurrentTaskGroupsObservation(void);
	void selectCurrentTaskGroupsCalibrator(void);
	void selectCurrentTaskGroupsTarget(void);
    void selectCurrentTaskGroupsPulsar(void);
	void selectCurrentTaskGroupsPreProcessing(void);
    void selectCurrentTaskGroupsLongBaseline(void);
	void selectCurrentTaskGroupsImaging(void);
	void setSelectedTasksOnHold(void);
	void rescheduleAbortedTask(void);
	void deleteSelectedTasks(void);
	void moveSelectedTasks(void);
	void copySelectedTasks(void);
	void showTaskProperties(void);
	void showSelectedTasksProperties(void);
	void showStateHistory(void);
    void openSASTreeViewer(void) const;
    void openMetaDataViewer(void) const;

public slots:
	void openMoveTasksDialog(void);
	void showTaskDialog(const Task *task, tabIndex tab = TAB_SCHEDULE);

private slots:
    void updateCurrentTime(void);

private:
	data_headers itsSortColumn;
	Qt::SortOrder itsSortOrder;
	QLofarDataModel * itsModel;
	ScheduleTableDelegate itsDelegate;
	QToolBar * itsMainToolBar;
    QLCDNumber *itsLCDtimer;
	QToolBar * itsSearchBar;
	QLineEdit * itsSearchLineEdit;
	QPushButton * itsSearchNextButton;
	QPushButton * itsSearchPreviousButton;
	QCheckBox * itsSearchColumnOnlyCheckBox;
    // timer for displaying current UTC
    QTimer *itsTimer;
	// search result
	QList<QStandardItem *>::const_iterator search_iterator;
	QList<QStandardItem *> searchResult;
	int columnSearched;
	bool itsIsTestMode, itsIsSaveRequired, itsEnablePublish;
    QSet<QString> itsActiveProjects;

	// the GUI
    Ui::SchedulerGUIClass ui;

    QStringList itsUndoList, itsRedoList;
    QString itsWindowTitle, itsCurrentFileName;

    // graphic schedule view dock
    QDockWidget *itsGraphicDock;
    QWidget *itsGraphicDockWidgetContents;
    QToolBar *itsGraphicViewToolbar;
    QAction *itsZoomInAction;
    QAction *itsZoomOutAction;
    QAction *itsTaskTypeColorModeAction;
    QAction *itsCenterOnNow;
    QGraphicsView *itsGraphicView;
    GraphicResourceScene *itsGraphicResourceScene;
    QVBoxLayout *itsGraphicDockMainLayout;

    // graphic storage resource use view dock
    QDockWidget *itsGraphicStorageDock;
    QWidget *itsStorageDockWidgetContents;
    QVBoxLayout *itsStorageDockLayout;
    GraphicStorageScene *itsGraphicStorageScene;

    // table view dock
    QDockWidget *itsTableDock;
    QWidget *itsTableDockWidgetContents;
    QGridLayout *itsTableDockMainLayout;
    QSpacerItem *itsTableSpacerItem;
    TableView *itsTableView;
    QCheckBox *itsTableObsEnabled, *itsTablePipesEnabled;
    QComboBox *itsTableProjectFilter;
    QPushButton *pb_Cancel, *pb_Apply;
    std::vector<unsigned> itsSelectedTableTasks;
    unsigned int itsSelectedTaskID;

    //status bar
    QStatusBar *itsStatusBar;
    QLabel *itsStatusBarStatusText;
    QLabel *itsStatusBarTaskStatus;
    QLabel *itsStatusBarUsedDatabase;
    QProgressBar *itsStatusBarProgress;

    // controller
    Controller *itsController;

    // task dialog
	TaskDialog *itsTaskDialog;

	// Parset Tree View Dialog;
	ParsetTreeViewer *itsTreeViewer;

	// publish dialog
	PublishDialog itsPublishDialog;

	//column view dialog
	tableColumnSelectDialog itsViewColumnsDialog;

// Actions
    QAction *showSearchBarAct;
    QAction *closeSearchBarAct;
    QAction *itsSearchPreviousAct;
    QAction *itsSearchNextAct;
    QAction *itsSearchColumnOnlyCheckBoxAct;

    QString lastPath;

// Icons
    QIcon *closeIcon;
};

#endif // SCHEDULERGUI_H

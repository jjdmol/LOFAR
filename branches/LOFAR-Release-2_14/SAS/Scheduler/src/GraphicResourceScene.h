/*
 * GraphicResourceScene.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date    : $Date$
 * First creation : Oct 14, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/GraphicResourceScene.h $
 *
 */

#ifndef GRAPHICRESOURCESCENE_H_
#define GRAPHICRESOURCESCENE_H_

enum zoom_level {
	ZM_MIN,
	ZM_HOUR,
	ZM_DAY,
	ZM_WEEK,
	ZM_MONTH
//	ZM_YEAR
};

#include "astrodatetime.h"
#include "GraphicTimeLine.h"
#include "GraphicCurrentTimeLine.h"
#include "schedulerdatablock.h"
#include "taskdialog.h"
#include "Controller.h"
#include <map>
#include <vector>
#include <QGraphicsScene>

class QGraphicsView;
class GraphicStationTaskLine;
class GraphicTask;
class QGraphicsSceneMouseEvent;
class QGraphicsSimpleTextItem;
class Station;
class QTimer;

// key = stationID, second.first = StationLineYPos, second.second = graphic station time line
typedef std::map<unsigned, std::pair<int, GraphicStationTaskLine * > > stationTimeLineMap;
// key = taskID, vector contains the GraphicTask objects (one for every station time line)
typedef std::map<unsigned, std::vector<GraphicTask *> > tasksMap;

class GraphicResourceScene : public QGraphicsScene {

	Q_OBJECT

public:
	GraphicResourceScene(Controller *controller);
	virtual ~GraphicResourceScene();

	zoom_level getZoomLevel(void) const {return itsZoomLevel;}
	void setZoomLevel(zoom_level zoom);// {itsZoomLevel = zoom; update();}

	void setViewPort(QGraphicsView *view) {itsParentView = view;}

	const stationTimeLineMap &getStationTimeLines(void) const {return itsStationTimeLines;}
	qreal getTimeLineWidth(void) {return itsTimeLine->getWidth();}
	// convert a AstroDateTime to a position on the station time line
	// where 0 corresponds to the position of the start of the station time line
	int time2xPos(const AstroDateTime &date) const;
	AstroDateTime xPos2Time(int xpos);
	void updateTimeLineYPos(void);
    void updateCurrentTime(const QDateTime &UTC) {itsCurrentTimeLine->updateTime(UTC);}
	void updateStationNamesXPos(void);
	int getHorizontalScrollPos(void);

	const AstroDate &getStartDay(void) {return itsStartDay;}
	const AstroDate &getEndDay(void) {return itsEndDay;}
	qreal getTimeLineZeroPos(void) const {return itsTimeLineZeroPos;}

	const Controller *controller(void) {return itsController;}

	void applyNewScheduleTimeSpan(bool centerOnSelectedTask = false);
	void updateStationTimeLines();
    void updateTask(const Task *task);
	void updateTasks(const scheduledTasksMap &scheduledTasks, const reservationsMap &reservations, const inActiveTasksMap & inactiveTasks);
	void zoomIn(void);
	void zoomOut(void);
	void setTaskColorMode(Task::task_color_mode color_mode);
	Task::task_color_mode getTaskColorMode(void) const {return itsTaskColorMode;}
	void setNewTimeSpan(void);
	void showCurrentTimeLine(void) { itsCurrentTimeLine->show(); }
	void hideCurrentTimeLine(void) { itsCurrentTimeLine->hide(); }
	void scrollToNow(void);// {itsParentView->centerOn(itsCurrentTimeLine)/*->ensureVisible(0,0,10,10)*/; }
	void ensureTaskIsVisible(unsigned int taskID);

	void clearTasks(void);
	const std::vector<GraphicTask *> findTask(unsigned taskID) const;

	void startMoveTask(int x_start_pos) {lastXPosition = x_start_pos;}
	void moveTask(unsigned task_id, QGraphicsSceneMouseEvent *event);
	void moveSelectedTasks(QGraphicsSceneMouseEvent *event);
	void requestTaskMove(unsigned task_id, const AstroDateTime &new_start_time);
	void openTaskDialog(const Task *task);
    void addTask(const Task *); // add a task to the scene
	void removeTaskFromScene(unsigned task_id); // delete a task from the scene
	const Task *getTask(unsigned task_id) const {return itsController->getTask(task_id);}
	const Task *getScheduledTask(unsigned task_id) const {return itsController->getScheduledTask(task_id);}
	const Task *getUnscheduledTask(unsigned task_id) const {return itsController->getUnscheduledTask(task_id);}
	const Task *getInactiveTask(unsigned task_id) const {return itsController->getInactiveTask(task_id);}
	const tasksMap &getGraphicTasks(void) const { return itsTasks; }

	// used by graphicTask context menu
	void unscheduleTask(unsigned taskID);
	void unscheduleSelectedTasks(void);
	void scheduleTask(unsigned taskID, Task::task_status status);
	void preScheduleSelectedTasks(void);
	void scheduleSelectedTasks(void);
	void setTaskOnHold(unsigned taskID);
	void setSelectedTasksOnHold(void);
	void copyTask(unsigned taskID);
	void copySelectedTasks(void);
	void rescheduleTask(unsigned taskID);
	void deleteTask(unsigned taskID);
	void deleteSelectedTasks(void);
	void showTaskProperties(unsigned taskID);
	void showSelectedTasksProperties(void);
	void showTaskStateHistory(unsigned taskID);
	void clearSelection(void);
	void selectTask(unsigned taskID, bool singleSelection);
	void deselectTask(unsigned taskID, bool singleSelection);
	void openMoveTasksDialog(void);

private:
	GraphicTask *findGraphicTaskByStation(unsigned task_id, unsigned station_id) const;
	void addGraphicTask(GraphicStationTaskLine *stationLine, unsigned task_id, unsigned station_id, Task::task_color_mode color_mode/*, bool inActive*/);
	void selectGraphicTask(unsigned taskID, bool centerOnTask = false);
	void deselectGraphicTask(unsigned taskID);
	void deselectAllGraphicTasks(void);

	friend class SchedulerGUI; // schedulerGUI may access my private members

signals:
	void showTaskDialog(const Task *task);
	void updateTaskDialogWithTask(const Task *task);
//	void graphicSceneChanged(void);
	void setEnableZoomIn(bool);
	void setEnableZoomOut(bool);
	void clickedGraphicTask(unsigned taskID);
	void taskRescheduleRequest(unsigned, const AstroDateTime &, bool forced_unschedule = false);

private slots:
	void handleRubberBandSelection(void);


private:
	QGraphicsView *itsParentView;

	// controller
    Controller *itsController;

	zoom_level itsZoomLevel;
	int itsTimeLineZeroPos; // corresponds to time line zero position
	AstroDate itsStartDay, itsEndDay;

	//graphic objects
	Task::task_color_mode itsTaskColorMode;
	GraphicTimeLine *itsTimeLine;
	GraphicCurrentTimeLine *itsCurrentTimeLine;
	stationTimeLineMap itsStationTimeLines;
    std::map<unsigned, std::pair<QGraphicsSimpleTextItem *, QGraphicsRectItem *> > itsStationNameLabels;
	tasksMap itsTasks;
	int lastXPosition;
};

#endif /* GRAPHICRESOURCESCENE_H_ */

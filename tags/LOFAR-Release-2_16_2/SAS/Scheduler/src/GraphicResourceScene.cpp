/*
 * GraphicResourceScene.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date    : $Date$
 * First creation : Oct 14, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/GraphicResourceScene.cpp $
 *
 */

#include <algorithm>
#include <QPaintEngine>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QScrollBar>
#include <QMessageBox>
#include <QTimer>
#include "GraphicResourceScene.h"
#include "GraphicStationTaskLine.h"
#include "GraphicTask.h"
#include "Controller.h"
#include "station.h"

GraphicResourceScene::GraphicResourceScene(Controller *controller)
	: itsParentView(0), itsController(controller), itsZoomLevel(ZM_DAY),
	itsTimeLineZeroPos(70), itsTaskColorMode(Task::TASK_STATUS_COLOR_MODE), lastXPosition(0)
{
	setNewTimeSpan();
	itsTimeLine = new GraphicTimeLine(this);
	this->addItem(itsTimeLine);
	itsCurrentTimeLine = new GraphicCurrentTimeLine(this, 200);
	this->addItem(itsCurrentTimeLine);
	setSceneRect(0, 0, itsTimeLine->getWidth() + 150, 200);
    connect(this, SIGNAL(selectionChanged()), this, SLOT(handleRubberBandSelection()));
}

GraphicResourceScene::~GraphicResourceScene() {
	delete itsTimeLine;
	for (stationTimeLineMap::iterator it = itsStationTimeLines.begin(); it != itsStationTimeLines.end(); ++it) {
        delete it->second.second;
    }
    for (std::map<unsigned, std::pair<QGraphicsSimpleTextItem *, QGraphicsRectItem *> >::iterator nit = itsStationNameLabels.begin(); nit != itsStationNameLabels.end(); ++ nit) {
        delete nit->second.first;
        delete nit->second.second;
    }
    delete itsCurrentTimeLine;
}

int GraphicResourceScene::time2xPos(const AstroDateTime &date) const {
	double difDays = date.toJulian() - itsStartDay.toJulian();
	if (itsZoomLevel == ZM_MONTH) {
		return static_cast<int>(difDays * 5) + itsTimeLineZeroPos; // week = 35 pixels, day = 5 pixels
	}
	else if (itsZoomLevel == ZM_WEEK) {
		return static_cast<int>(difDays * 48) + itsTimeLineZeroPos; // day = 48 pixels, hour = 2 pixels
	}
	else if (itsZoomLevel == ZM_DAY) {
		return static_cast<int>(difDays * 120) + itsTimeLineZeroPos; // day = 120 pixels, hour = 5 pixels
	}
	else if (itsZoomLevel == ZM_HOUR) {
		return static_cast<int>(difDays * 480) + itsTimeLineZeroPos; // day = 480 pixels, hour = 20 pixels
	}
	else if (itsZoomLevel == ZM_MIN) {
		return static_cast<int>(difDays * 1440) + itsTimeLineZeroPos; // day = 1440 pixels, hour = 60 pixels
	}
	return 0;
}


AstroDateTime GraphicResourceScene::xPos2Time(int xpos) {
	int pos = xpos - itsTimeLineZeroPos;
	AstroDateTime time(itsStartDay);
	switch (itsZoomLevel) {
	case ZM_MONTH: // week = 35 pixels, day = 5 pixels
		if (pos >= 5) {
			time = time.addDays(pos / 5);
		}
		else if (pos <= -5) {
			time = time.subtractDays(pos / 5);
		}
		break;
	case ZM_WEEK: // day = 48 pixels, hour = 2 pixels
		if (pos >= 2) {
			time = time.addHours(pos / 2);
		}
		else if (pos <= -2) {
			time = time.subtractHours(-pos / 2);
		}
		break;
	case ZM_DAY: // day = 120 pixels, hour = 5 pixels
		if (pos >= 5) {
			time = time.addHours(pos / 5);
		}
		else if (pos <= -5) {
			time = time.subtractHours(-pos / 5);
		}
		break;
	case ZM_HOUR: // day = 480 pixels, hour = 20 pixels, 15 minutes = 5 pixels
		if (pos >= 5) {
			time = time.addMinutes(15 * pos / 5);
		}
		else if (pos <= -5) {
			time = time.subtractMinutes(-15 * pos / 5);
		}
		break;
	case ZM_MIN: // day = 1440 pixels, hour = 60 pixels, 1 minute = 1 pixel
		if (pos >= 1) {
			time = time.addMinutes(pos);
		}
		else if (pos <= -1) {
			time = time.subtractMinutes(-pos);
		}
		break;
	}
	return time;
}

void GraphicResourceScene::applyNewScheduleTimeSpan(bool centerOnSelectedTask) {
	setNewTimeSpan();
	itsTimeLine->updateWidth();
	for (stationTimeLineMap::iterator it = itsStationTimeLines.begin(); it != itsStationTimeLines.end(); ++it) {
        it->second.second->updateWidth();
	}
	for (tasksMap::iterator it = itsTasks.begin(); it != itsTasks.end(); ++it) {
		for (std::vector<GraphicTask *>::iterator git = it->second.begin(); git != it->second.end(); ++git) {
			(*git)->updatePosition();
		}
	}

	setSceneRect(0, 0, itsTimeLine->getWidth() + 150.0, itsStationTimeLines.size() * 20 + 60);
	itsCurrentTimeLine->updateHeight(itsStationTimeLines.size() * 20 + 15);
    itsCurrentTimeLine->redrawTime();

	if (centerOnSelectedTask) {
		if (!itsController->selectedTasks().empty()) {
			tasksMap::iterator tit = itsTasks.find(itsController->lastSelectedTask());
			if (tit != itsTasks.end()) {
				itsParentView->centerOn(tit->second.front());
				//tit->second.front()->ensureVisible();
			}
		}
	}

	update();
}

void GraphicResourceScene::setNewTimeSpan(void) {
	itsStartDay = Controller::theSchedulerSettings.getEarliestSchedulingDay();
	itsEndDay = Controller::theSchedulerSettings.getLatestSchedulingDay();

	if (itsZoomLevel == ZM_MONTH) {
		// set end date to the first day of the next week
		unsigned week = itsEndDay.getWeek();
		while (week == itsEndDay.getWeek()) {itsEndDay = itsEndDay.addDays(1);}
		// set currentDate to the first day of the current week
		week = itsStartDay.getWeek();
		while (week == itsStartDay.getWeek()) {itsStartDay = itsStartDay.subtractDays(1);}
		itsStartDay = itsStartDay.addDays(1);
	}
	else if (itsZoomLevel == ZM_WEEK) {
	}
	else if (itsZoomLevel == ZM_DAY) {
	}
	else if (itsZoomLevel == ZM_HOUR) {
	}
	else if (itsZoomLevel == ZM_MIN) {
	}

}

void GraphicResourceScene::scrollToNow(void) {
	itsParentView->centerOn(itsCurrentTimeLine);
	itsParentView->verticalScrollBar()->setValue(0);
}

void GraphicResourceScene::ensureTaskIsVisible(unsigned int taskID) {
	tasksMap::const_iterator it = itsTasks.find(taskID);
	if (it != itsTasks.end()) {
		if (!(it->second.empty())) {
			it->second.front()->ensureVisible();
		}
	}
}

int GraphicResourceScene::getHorizontalScrollPos(void) {
	return itsParentView->horizontalScrollBar()->value();
}

void GraphicResourceScene::updateTimeLineYPos(void) {
	int moveY(itsParentView->verticalScrollBar()->value() - static_cast<int>(itsTimeLine->y()));
	itsTimeLine->moveBy(0,moveY);
	itsCurrentTimeLine->moveBy(0,moveY);
	QCoreApplication::processEvents();
}

void GraphicResourceScene::updateStationNamesXPos(void) {
    int labelXpos(getHorizontalScrollPos());
    for (stationTimeLineMap::iterator it = itsStationTimeLines.begin(); it != itsStationTimeLines.end(); ++it) {
        it->second.second->update();
        itsStationNameLabels[it->first].first->setX(labelXpos+30); // text
        itsStationNameLabels[it->first].second->setX(labelXpos); // rectangle around text
    }
    QCoreApplication::processEvents();
}

void GraphicResourceScene::updateStationTimeLines() {
	const stationNameIDMapping &stations = Controller::theSchedulerSettings.getStations();
	for (stationTimeLineMap::iterator it = itsStationTimeLines.begin(); it != itsStationTimeLines.end(); ++it) {
        // CAUTION!: ALSO REMOVES THE CHILD GRAPHICS TASK OBJECTS WHILE THESE ARE NOT REMOVED FROM itsTasks tasksMap
        delete it->second.second;
	}
    itsStationTimeLines.clear();
    for (std::map<unsigned, std::pair<QGraphicsSimpleTextItem *, QGraphicsRectItem *> >::iterator nit = itsStationNameLabels.begin(); nit != itsStationNameLabels.end(); ++ nit) {
        delete nit->second.first;
        delete nit->second.second;
    }
    itsStationNameLabels.clear();
    int StationLineYPos, labelXpos(itsTimeLineZeroPos - 40);
    for (stationNameIDMapping::const_iterator it = stations.begin(); it != stations.end(); ++ it) {
        // create station time line
        StationLineYPos = itsStationTimeLines.size() * 20 + 40;
        GraphicStationTaskLine *stationTimeLine = new GraphicStationTaskLine(this, it->second, StationLineYPos);
        addItem(stationTimeLine);
        // create station label
        QGraphicsSimpleTextItem *stationName = new QGraphicsSimpleTextItem(it->first.c_str(), 0);
        stationName->setPos(labelXpos, StationLineYPos-2);
        stationName->setFont(QFont("Liberation Sans", 9, QFont::Bold));
        stationName->setZValue(10);
        const QPointF &sp(stationName->pos());
        QRectF r(sp.x()-2, sp.y()-2, 9*(it->first.length()-1)+4, 13);
        QGraphicsRectItem * rect = new QGraphicsRectItem(r, 0);
        rect->setZValue(9);
        rect->setPen(QPen(Qt::NoPen));
        rect->setBrush(QColor(255,255,255,160));
        itsStationNameLabels[it->second] = std::pair<QGraphicsSimpleTextItem *, QGraphicsRectItem *>(stationName, rect);
        itsStationTimeLines[it->second] = std::pair<int, GraphicStationTaskLine *>(StationLineYPos, stationTimeLine);
    }
    setSceneRect(0, 0, itsTimeLine->getWidth() + 150.0, itsStationTimeLines.size() * 20 + 60);
    itsCurrentTimeLine->updateHeight(itsStationTimeLines.size() * 20 + 15);
    itsCurrentTimeLine->redrawTime();
    updateStationNamesXPos();
}

const std::vector<GraphicTask *> GraphicResourceScene::findTask(unsigned taskID) const {
	tasksMap::const_iterator it = itsTasks.find(taskID);
	if (it != itsTasks.end()) {
		return it->second;
	}
	else {
		std::vector<GraphicTask *> graphic_tasks;
		return graphic_tasks;
	}
}


void GraphicResourceScene::clearTasks(void) {
	this->blockSignals(true);
	for (tasksMap::iterator it = itsTasks.begin(); it != itsTasks.end(); ++it) {
		for (std::vector<GraphicTask *>::iterator git = it->second.begin(); git != it->second.end(); ++git) {
			removeItem(*git);
			delete *git;
			*git = 0;
		}
		it->second.clear();
	}
	itsTasks.clear();
	this->blockSignals(false);
}

GraphicTask *GraphicResourceScene::findGraphicTaskByStation(unsigned task_id, unsigned station_id) const {
	tasksMap::const_iterator it = itsTasks.find(task_id);
	if (it != itsTasks.end()) {
		for (std::vector<GraphicTask *>::const_iterator sit = it->second.begin(); sit != it->second.end(); ++sit) {
			if ((*sit)->stationID() == station_id) {
				return *sit;
			}
		}
		return 0;
	}
	else {
		return 0;
	}
}

void GraphicResourceScene::addTask(const Task *task) {
    const StationTask *pTask = dynamic_cast<const StationTask *>(task);
    if (pTask) {
        Task::task_status status = pTask->getStatus();
        if ((status >= Task::PRESCHEDULED) && (pTask->getScheduledStart().isSet())) { // only insert in graphic view if task status is above or equal to scheduled state
            const std::map<std::string, unsigned> &stations = pTask->getStations();
            for (std::map<std::string, unsigned>::const_iterator sit = stations.begin(); sit != stations.end(); ++sit) {
                stationTimeLineMap::iterator ssit = itsStationTimeLines.find(sit->second);
                if (ssit != itsStationTimeLines.end()) {
                    if (status >= Task::FINISHED) {
                        addGraphicTask(ssit->second.second, pTask->getID(), sit->second, itsTaskColorMode);
                    }
                    else {
                        addGraphicTask(ssit->second.second, pTask->getID(), sit->second, itsTaskColorMode);
                    }
                }
            }
        }
    }
    else {
        debugWarn("si","GraphicResourceScene::addTask: not a StationTask or task not found, ID:", pTask->getID());
    }
}

void GraphicResourceScene::addGraphicTask(GraphicStationTaskLine *stationLine, unsigned task_id, unsigned station_id, Task::task_color_mode color_mode/*, bool inactive*/) {
	GraphicTask *graphicTask = new GraphicTask(stationLine, this, task_id, station_id, color_mode/*, inactive*/); // implicitly adds the item to the scene
	itsTasks[task_id].push_back(graphicTask);
}

void GraphicResourceScene::removeTaskFromScene(unsigned task_id) {
	tasksMap::iterator it = itsTasks.find(task_id);
	blockSignals(true); // we have to block signals from the scene otherwise it will send signals (rubberbandselection changed) and cause SIGTRAPS
	if (it != itsTasks.end()) {
		for (std::vector<GraphicTask *>::iterator git = it->second.begin(); git != it->second.end(); ++git) {
			removeItem(*git);
			delete *git; // deletes the graphicTask objects and implicitly removes them from the scene
		}
		it->second.clear();
	}
	itsTasks.erase(task_id);
	blockSignals(false);
}

void GraphicResourceScene::updateTask(const Task *task) {
    const StationTask *pTask = dynamic_cast<const StationTask *>(task);
    if (pTask) {
        unsigned task_id(pTask->getID());
        if (pTask->getStatus() >= Task::PRESCHEDULED && pTask->getScheduledStart().isSet()) { // it the task is still in the schedule (i.e. if it should be displayed)
            const std::map<std::string, unsigned> &stations = pTask->getStations();
            for (std::map<std::string, unsigned>::const_iterator sit = stations.begin(); sit != stations.end(); ++sit) {
                stationTimeLineMap::iterator ssit = itsStationTimeLines.find(sit->second);
                if (ssit != itsStationTimeLines.end()) {
                    //search for an existing graphictask with this station id
                    GraphicTask *graphicTask = findGraphicTaskByStation(task_id, sit->second);
                    this->blockSignals(true);
                    if (!graphicTask) { // this station was added to the task or this task has just been added
                        addGraphicTask(ssit->second.second, task_id, sit->second, itsTaskColorMode);
                    }
                    else {
                        graphicTask->updateTask();
                    }
                    this->blockSignals(false);
                }
            }
            //remove graphictasks from removed stations for this task
            tasksMap::iterator it = itsTasks.find(task_id);
            if (it != itsTasks.end()) {
                bool changed(false);
                std::vector<GraphicTask *> newVec;
                this->blockSignals(true);
                for (std::vector<GraphicTask *>::iterator git = it->second.begin(); git != it->second.end(); ++git) {
                    if (stations.find(Controller::theSchedulerSettings.getStationName((*git)->stationID())) == stations.end()) {
                        removeItem(*git);
                        delete *git; // implicitly removes the item from the scene
                    }
                    else {
                        newVec.push_back(*git);
                        changed = true;
                    }
                }
                this->blockSignals(false);
                if (changed) it->second = newVec;
            }
        }
        else { // task is not ACTIVE anymore, remove the graphictask objects from the scene
            removeTaskFromScene(task_id);
        }
        this->update();
    }
}

void GraphicResourceScene::updateTasks(const scheduledTasksMap &scheduledTasks, const reservationsMap &reservations, const inActiveTasksMap & inactiveTasks) {
	unsigned taskID;
	clearTasks();

	for (scheduledTasksMap::const_iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
		taskID = it->second->getID();
		const std::map<std::string, unsigned> &stations = it->second->getStations();
		for (std::map<std::string, unsigned>::const_iterator sit = stations.begin(); sit != stations.end(); ++sit) {
			stationTimeLineMap::iterator ssit = itsStationTimeLines.find(sit->second);
			if (ssit != itsStationTimeLines.end()) {
                addGraphicTask(ssit->second.second, taskID, sit->second, itsTaskColorMode);
			}
		}
	}
	for (reservationsMap::const_iterator it = reservations.begin(); it != reservations.end(); ++it) {
		if (it->second->getStatus() == Task::PRESCHEDULED) {
			taskID = it->second->getID();
			const std::map<std::string, unsigned> &stations = it->second->getStations();
			for (std::map<std::string, unsigned>::const_iterator sit = stations.begin(); sit != stations.end(); ++sit) {
				stationTimeLineMap::iterator ssit = itsStationTimeLines.find(sit->second);
				if (ssit != itsStationTimeLines.end()) {
                    addGraphicTask(ssit->second.second, taskID, sit->second, itsTaskColorMode);
				}
			}
		}
	}
    for (inActiveTasksMap::const_iterator it = inactiveTasks.begin(); it != inactiveTasks.end(); ++it) {
        taskID = it->second->getID();
        if (it->second->isStationTask()) {
            const StationTask *pTask(static_cast<const StationTask *>(it->second));
            if (pTask->getScheduledStart().isSet()) {
                const std::map<std::string, unsigned> &stations = pTask->getStations();
                for (std::map<std::string, unsigned>::const_iterator sit = stations.begin(); sit != stations.end(); ++sit) {
                    stationTimeLineMap::iterator ssit = itsStationTimeLines.find(sit->second);
                    if (ssit != itsStationTimeLines.end()) {
                        addGraphicTask(ssit->second.second, taskID, sit->second, itsTaskColorMode);
                    }
                }
            }
        }
	}

	// see if the previously selected task is still displayed (is still scheduled) and select it again
	if (!itsController->selectedTasks().empty()) {
		tasksMap::iterator it;
		const std::vector<unsigned> &selectedTasks = itsController->selectedTasks();
		for (std::vector<unsigned>::const_iterator vit = selectedTasks.begin(); vit != selectedTasks.end(); ++vit) {
			if ((it = itsTasks.find(*vit)) != itsTasks.end()) {
				for (std::vector<GraphicTask *>::iterator git = it->second.begin(); git != it->second.end(); ++git) {
					(*git)->setSelected(true);
				}
			}
		}
		if (Controller::theSchedulerSettings.getFocusTaskAtClick()) {
			emit clickedGraphicTask(itsController->lastSelectedTask());
		}
	}
}

void GraphicResourceScene::selectGraphicTask(unsigned taskID, bool centerOnTask) {
	tasksMap::iterator tit = itsTasks.find(taskID);
	if (tit != itsTasks.end()) {
		if (!tit->second.empty()) {
			for (std::vector<GraphicTask *>::iterator git = tit->second.begin(); git != tit->second.end(); ++git) {
				(*git)->setSelected(true);
				(*git)->update();
			}
			// set highlight on possible predecessors
			/*
			const IDvector &predecessors = tit->second.front()->getPredecessors();
			if (!predecessors.emtpy()) {
				for (IDvector::const_iterator pit = predecessors.begin(); pit != predecessors.end(); ++pit) {

					if ((tit = itsTasks.find(predecessor)) != itsTasks.end()) {
						for (std::vector<GraphicTask *>::iterator it = tit->second.begin(); it != tit->second.end(); ++it) {
							(*it)->setPredecessorHighLight(true);
							(*it)->update();
						}
					}
				}
			}
			*/
		}
	}
	if (centerOnTask) {
		tit = itsTasks.find(taskID);
		if (tit != itsTasks.end()) {
			tit->second.front()->ensureVisible();
		}
	}
}

void GraphicResourceScene::handleRubberBandSelection(void) { // called when user drags a rubberband over graphicTasks
	if (QApplication::keyboardModifiers() != Qt::ControlModifier) { // control key was held down while changing selection
		itsController->deselectAllTasks();
	}
	// add the rubberband bounded tasks to the selection
	unsigned taskID;//, predecessor;
	tasksMap::iterator tit;
	QList<QGraphicsItem *> selectedGraphicTasks = selectedItems();
	for (QList<QGraphicsItem *>::iterator it = selectedGraphicTasks.begin(); it != selectedGraphicTasks.end(); ++it) {
		taskID = static_cast<GraphicTask *>(*it)->getTaskID();
		itsController->selectTask(taskID,false,true);
	}
}

void GraphicResourceScene::selectTask(unsigned taskID, bool singleSelection) {
	itsController->selectTask(taskID, singleSelection, true);
}

void GraphicResourceScene::deselectTask(unsigned taskID, bool singleSelection) {
	itsController->deselectTask(taskID, singleSelection, true);
}

void GraphicResourceScene::deselectGraphicTask(unsigned taskID) {
	this->blockSignals(true);
	tasksMap::iterator tit;
	if ((tit = itsTasks.find(taskID)) != itsTasks.end()) {
		for (std::vector<GraphicTask *>::iterator it = tit->second.begin(); it != tit->second.end(); ++it) {
			(*it)->setSelected(false);
		}
		// remove highlight from possible predecessors
		/*
		predecessor = tit->second.front()->getPredecessor();
		if (predecessor) {
			if ((tit = itsTasks.find(predecessor)) != itsTasks.end()) {
				for (std::vector<GraphicTask *>::iterator it = tit->second.begin(); it != tit->second.end(); ++it) {
					(*it)->setPredecessorHighLight(false);
//					(*it)->update();
				}
			}
		}
		*/
	}
	this->blockSignals(false);
}

void GraphicResourceScene::deselectAllGraphicTasks(void) {
	this->blockSignals(true);
	for (tasksMap::iterator it = itsTasks.begin(); it != itsTasks.end(); ++it) {
		for (std::vector<GraphicTask *>::iterator git = it->second.begin(); git != it->second.end(); ++git) {
			(*git)->setSelected(false);
			(*git)->setPredecessorHighLight(false);
//			(*git)->update();
		}
	}
	this->blockSignals(false);
}

void GraphicResourceScene::clearSelection(void) {
	itsController->deselectAllTasks();
}

void GraphicResourceScene::moveTask(unsigned task_id, QGraphicsSceneMouseEvent *event) {
	tasksMap::iterator tit;
	if ((tit = itsTasks.find(task_id)) != itsTasks.end()) {
		for (std::vector<GraphicTask *>::iterator it = tit->second.begin(); it != tit->second.end(); ++it) {
			(*it)->moveBy(event->lastScreenPos().x() - lastXPosition, 0);
		}
		lastXPosition = event->lastScreenPos().x();
	}
}

void GraphicResourceScene::moveSelectedTasks(QGraphicsSceneMouseEvent *event) {
	int move(event->lastScreenPos().x() - lastXPosition);
	tasksMap::iterator tit;
	const std::vector<unsigned> &selectedTasks(itsController->selectedTasks());
	for (std::vector<unsigned>::const_iterator it = selectedTasks.begin(); it != selectedTasks.end(); ++it) {
		if ((tit = itsTasks.find(*it)) != itsTasks.end()) {
			for (std::vector<GraphicTask *>::iterator git = tit->second.begin(); git != tit->second.end(); ++git) {
				(*git)->moveBy(move, 0);
			}
		}
	}
	lastXPosition = event->lastScreenPos().x();
}


void GraphicResourceScene::setZoomLevel(zoom_level zoom) {
	zoom_level prev_zoom = itsZoomLevel;
	itsZoomLevel = zoom;
	applyNewScheduleTimeSpan(true);
	if (zoom != itsZoomLevel) {
		if ((prev_zoom == ZM_MIN) & (zoom > ZM_MIN))
			emit setEnableZoomIn(true);
		else if ((prev_zoom == ZM_MONTH) & (zoom < ZM_MONTH))
			emit setEnableZoomOut(true);
	}
}

void GraphicResourceScene::zoomIn(void) {
	if (itsZoomLevel == ZM_MONTH) {
		itsZoomLevel = ZM_WEEK;
		applyNewScheduleTimeSpan(true);
		emit setEnableZoomOut(true);
	}
	else if (itsZoomLevel == ZM_WEEK) {
		itsZoomLevel = ZM_DAY;
		applyNewScheduleTimeSpan(true);
	}
	else if (itsZoomLevel == ZM_DAY) {
		itsZoomLevel = ZM_HOUR;
		applyNewScheduleTimeSpan(true);
	}
	else if (itsZoomLevel == ZM_HOUR) {
		itsZoomLevel = ZM_MIN;
		applyNewScheduleTimeSpan(true);
		emit setEnableZoomIn(false);
	}
}

void GraphicResourceScene::zoomOut(void) {
	if (itsZoomLevel == ZM_MIN)  {
		itsZoomLevel = ZM_HOUR;
		applyNewScheduleTimeSpan(true);
		emit setEnableZoomIn(true);
	}
	else if (itsZoomLevel == ZM_HOUR)  {
		itsZoomLevel = ZM_DAY;
		applyNewScheduleTimeSpan(true);
	}
	else if (itsZoomLevel == ZM_DAY) {
		itsZoomLevel = ZM_WEEK;
		applyNewScheduleTimeSpan(true);
	}
	else if (itsZoomLevel == ZM_WEEK) {
		itsZoomLevel = ZM_MONTH;
		applyNewScheduleTimeSpan(true);
		emit setEnableZoomOut(false);
	}
}

void GraphicResourceScene::setTaskColorMode(Task::task_color_mode color_mode) {
	if (itsTaskColorMode != color_mode) {
		itsTaskColorMode = color_mode;
		for (tasksMap::iterator it = itsTasks.begin(); it != itsTasks.end(); ++it) {
			for (std::vector<GraphicTask *>::iterator git = it->second.begin(); git != it->second.end(); ++git) {
				(*git)->setTaskColorMode(itsTaskColorMode);
			}
		}
	}
}

void GraphicResourceScene::openMoveTasksDialog(void) {
	itsController->openMoveTasksDialog();
}

void GraphicResourceScene::requestTaskMove(unsigned task_id, const AstroDateTime &new_start_time) {
	itsController->moveSelectedTasksRequest(task_id, new_start_time);
}


void GraphicResourceScene::openTaskDialog(const Task *task) {
	emit showTaskDialog(task);
}
/*
void GraphicResourceScene::updateTaskDialog(const Task *task) {
	emit updateTaskDialogWithTask(task);
}
*/
/*
void GraphicResourceScene::abortTask(unsigned taskID) {
	itsController->abortTask(taskID);
}
*/
void GraphicResourceScene::unscheduleTask(unsigned taskID) {
	itsController->unscheduleTask(taskID);
//	update();
}

void GraphicResourceScene::unscheduleSelectedTasks(void) {
	itsController->unscheduleSelectedTasks();
}


void GraphicResourceScene::scheduleTask(unsigned taskID, Task::task_status status) {
	itsController->scheduleTask(taskID, status);
}

void GraphicResourceScene::preScheduleSelectedTasks(void) {
	itsController->scheduleSelectedTasks(Task::PRESCHEDULED);
}

void GraphicResourceScene::scheduleSelectedTasks(void) {
	itsController->scheduleSelectedTasks(Task::SCHEDULED);
}

void GraphicResourceScene::setTaskOnHold(unsigned taskID) {
	itsController->setTaskOnHold(taskID);
}

void GraphicResourceScene::setSelectedTasksOnHold(void) {
	itsController->setSelectedTasksOnHold();
}

void GraphicResourceScene::copyTask(unsigned taskID) {
	itsController->copyTask(taskID);
}

void GraphicResourceScene::copySelectedTasks(void) {
	itsController->copySelectedTasks();
}

void GraphicResourceScene::rescheduleTask(unsigned taskID){
	if (itsCurrentTimeLine->isWithinSchedule()) {
		itsController->rescheduleAbortedTask(taskID, itsCurrentTimeLine->currentDateTime() + Controller::theSchedulerSettings.getMinimumTimeBetweenTasks());
	}
	else {
		QMessageBox::warning(0, tr("Cannot reschedule task"),
				tr("The current time is outside of the defined scheduling range.\nAn aborted task can only be scheduled in the future."));
		QApplication::beep();
	}
}

void GraphicResourceScene::deleteTask(unsigned taskID) {
	itsController->deleteTask(taskID);
}

void GraphicResourceScene::deleteSelectedTasks(void) {
	itsController->deleteSelectedTasks();
}

void GraphicResourceScene::showTaskProperties(unsigned taskID) {
	const Task *pTask = itsController->getScheduledTask(taskID);
	if (pTask) { emit showTaskDialog(pTask); }
	else {
		pTask = itsController->getInactiveTask(taskID);
		if (pTask) { emit showTaskDialog(pTask); }
	}

}

void GraphicResourceScene::showSelectedTasksProperties(void) {
	itsController->multiEditSelectedTasks();
}

void GraphicResourceScene::showTaskStateHistory(unsigned taskID) {
	itsController->showTaskStateHistory(taskID);
}


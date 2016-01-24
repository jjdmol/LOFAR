/*
 * GraphicTask.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date    : $Date$
 * First creation : Oct 20, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/GraphicTask.cpp $
 *
 */

#include "GraphicTask.h"
#include "GraphicResourceScene.h"
#include "task.h"
#include "GraphicStationTaskLine.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QWhatsThis>
#include <QDrag>
#include <QMimeData>
#include <QBitmap>
#include <QToolTip>
#include <algorithm>
using std::max;
using std::min;
using std::string;

GraphicTask::GraphicTask(GraphicStationTaskLine *stationLine, GraphicResourceScene *scene, unsigned int task_id,
        unsigned station_id, Task::task_color_mode color_mode)
    : itsScene(scene), itsTaskID(task_id), itsStationID(station_id),
	itsHeight(TASK_RECT_HEIGHT), EndBeforeStart(false), itsColorMode(color_mode), itsSelectedHighLightOn(false),
	itsPredecessorHighLightOn(false), dontMove(false), hasBeenMoved(false)
{
	scene->addItem(this); // adds the item to the scene
	yZeroPosTimeLine = static_cast<int>(stationLine->pos().y());
    setZValue(3); // item is at z level 3 (background is 0)
	const Task *pTask = itsScene->getTask(itsTaskID);
	setPos(itsScene->time2xPos(pTask->getScheduledStart()), yZeroPosTimeLine);

    AstroDateTime endTime(pTask->getScheduledEnd());
    const AstroDateTime &realEnd(pTask->SASTree().stopTime());
    if (pTask->isObservation() && pTask->getStatus() >= Task::FINISHED && realEnd.isSet()) {
        if (realEnd <= pTask->getScheduledEnd().subtractMinutes(1)) {
            endTime = realEnd;
        }
    }
    itsWidth = itsScene->time2xPos(endTime) - static_cast<int>(pos().x());
	if (itsWidth < 0) {
		itsWidth = 5;
		EndBeforeStart = true;
	}
	else EndBeforeStart = false;
	itsBoundings = QRectF(-0.5, -0.5, itsWidth + 3, itsHeight + 3);
	updateTask(/*inactive*/); // updates the tooltip
	setAcceptedMouseButtons(Qt::LeftButton|Qt::RightButton);
	setFlags(QGraphicsItem::ItemIsSelectable);
}

GraphicTask::~GraphicTask() {
}

void GraphicTask::updatePosition(void) {
	const Task *pTask = itsScene->getTask(itsTaskID);
	setPos(itsScene->time2xPos(pTask->getScheduledStart()), yZeroPosTimeLine);
    AstroDateTime endTime(pTask->getScheduledEnd());
    const AstroDateTime &realEnd(pTask->SASTree().stopTime());
    if (pTask->isObservation() && pTask->getStatus() >= Task::FINISHED && realEnd.isSet()) {
        if (realEnd <= pTask->getScheduledEnd().subtractMinutes(1)) {
            endTime = realEnd;
        }
    }
    itsWidth = itsScene->time2xPos(endTime) - static_cast<int>(pos().x());
    if (itsWidth < 0) {
		itsWidth = 5;
		EndBeforeStart = true;
	}
	else EndBeforeStart = false;
	itsBoundings = QRectF(-0.5, -0.5, itsWidth + 3, itsHeight + 3);
	update();
}

void GraphicTask::updateTask() {
	const Task *pTask = itsScene->getTask(itsTaskID);
	Task::task_status status(pTask->getStatus());
	Task::task_type type(pTask->getType());
	if (status >= Task::SCHEDULED) dontMove = true;
	else dontMove = false;
	if (type == Task::RESERVATION) {
        setZValue(2); // reservation is at stacking level 2, regular task at 3
	}
	else {
        setZValue(3); // draw regular task always on top of reservations
	}
	setPos(itsScene->time2xPos(pTask->getScheduledStart()), yZeroPosTimeLine);
    AstroDateTime endTime(pTask->getScheduledEnd());
    string earlyStr;
    const AstroDateTime &realEnd(pTask->SASTree().stopTime());
    if (pTask->isObservation() && pTask->getStatus() >= Task::FINISHED && realEnd.isSet()) {
        if (realEnd <= pTask->getScheduledEnd().subtractMinutes(1)) {
            endTime = realEnd;
            earlyStr = " (EARLY)";
        }
    }
    itsWidth = itsScene->time2xPos(endTime) - static_cast<int>(pos().x());	if (itsWidth < 0) {
		itsWidth = 5;
		EndBeforeStart = true;
	}
	else EndBeforeStart = false;
	itsBoundings = QRectF(-0.5, -0.5, itsWidth + 3, itsHeight + 3);
	itsTooltip = "Task:" + int2String(pTask->getID()) + " " + pTask->getTypeStr() + " (" +
			int2String(pTask->getSASTreeID()) +
		")\nSubtype:" + pTask->getProcessSubtypeStr() +
		std::string("\nProject:") +
		pTask->getProjectName() + "\nTask:" +
		pTask->getTaskName() + "\nStatus:" +
        pTask->getStatusStr() + earlyStr + "\n" +
		pTask->getScheduledStart().getDate().toString() + " - " +
		pTask->getScheduledEnd().getDate().toString();
	if (EndBeforeStart) {
		itsTooltip += string("(!)");
	}
	if (pTask->getFixedDay()) {
		itsTooltip += " (Fixed)";
	}
	itsTooltip += "\n" + pTask->getScheduledStart().getTime().toString() + " - " +
		pTask->getScheduledEnd().getTime().toString();
	if (pTask->getFixedTime()) {
		itsTooltip += " (Fixed)";
	}
    itsTooltip += "\nDuration:" + pTask->getDuration().toString();
    if (pTask->hasStorage()) {
        itsTooltip += string("\nData size:") + humanReadableUnits((long double)pTask->storage()->getTotalStoragekBytes(), SIZE_UNITS).c_str();
    }
    if (pTask->isStationTask()) {
        itsTooltip += "\nStations:\n" + static_cast<const StationTask *>(pTask)->getStationNamesStr(',', 6);
    }

	setToolTip(QString(itsTooltip.c_str()));
	update(); // also do a redraw because task times changed
}

std::string GraphicTask::toolTipHTML(void) const {
	const Task *pTask = itsScene->getTask(itsTaskID);
	string br("<br>");
	string tooltip(string(pTask->getTypeStr()) + " (" + int2String(pTask->getSASTreeID()) + ")");

	if (pTask->hasLinkedTask()) {
		tooltip += " (linked task)";
	}
	tooltip += br + pTask->getProcessSubtypeStr() + br
	+ string("Project: ") + pTask->getProjectName() + br
	+ string("Task: ") + pTask->getTaskName() + br
	+ string("Start: ") + pTask->getScheduledStart().toString() + br
	+ string("Stop: ") + pTask->getScheduledEnd().toString() + br
	+ string("Duration: ") + pTask->getDuration().toString();
	if (pTask->isObservation()) {
        const Observation *pObs = static_cast<const Observation *>(pTask);
        tooltip += br + "Settings: " + pObs->getStationClockStr() + br + pObs->getAntennaModeStr() + " / " + pObs->getFilterTypeStr() + br;
	}
	return tooltip;
}

void GraphicTask::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/) {
	QBrush brush;
	QPen pen;
	QColor color;
	bool penSet(false);
    const Task *pTask(0);
	pTask = itsScene->getTask(itsTaskID);
	if (pTask) {
		Task::task_type type(pTask->getType());

		if (itsSelectedHighLightOn) {
			pen.setColor(Qt::red);
			pen.setWidth(3);
			penSet = true;
		}
		else if (itsPredecessorHighLightOn) {
			pen.setColor(QColor(0,255,255));
			pen.setWidth(3);
			penSet = true;
		}

		if (itsColorMode == Task::TASK_TYPE_COLOR_MODE) {
			if (type == Task::RESERVATION) {
				color = pTask->getTypeColor();
				if (!penSet) {
					pen.setColor(color);
					pen.setWidth(1);
				}
				brush.setColor(color);
				brush.setStyle(Qt::BDiagPattern);
			}
			else if (type == Task::MAINTENANCE) {
				color = pTask->getTypeColor();
				if (!penSet) {
					pen.setColor(color);
					pen.setWidth(1);
				}
				brush.setColor(color);
				brush.setStyle(Qt::DiagCrossPattern);
			}
			else {
				color = pTask->getTypeColor();
				if (!penSet) {
					pen.setColor(color);
					pen.setWidth(1);
				}
				brush.setColor(color);
				brush.setStyle(Qt::SolidPattern);
			}
		}
		else { // task status color mode
			if (type == Task::RESERVATION) {
				color = pTask->getTypeColor();
				brush.setStyle(Qt::BDiagPattern);
				if (!penSet) {
					pen.setWidth(1);
				}
			}
			else if (type == Task::MAINTENANCE) {
				color = pTask->getTypeColor();
				brush.setStyle(Qt::DiagCrossPattern);
				if (!penSet) {
					pen.setWidth(1);
				}
			}
			else {
				color = pTask->getStatusColor();
				brush.setStyle(Qt::SolidPattern);
				if (!penSet) {
					pen.setWidth(1);
				}
			}
			if (!penSet) {
				pen.setColor(color);
			}
			brush.setColor(color);
		}

    painter->setPen(pen);
    painter->setBrush(brush);

    int xpos = static_cast<int>(pos().x());
	if (xpos + itsWidth > 70) {
	if (xpos <= 70) {
		int h2 = itsHeight/2;
		itsBoundings.setX(- xpos - h2);
		QPointF points[3] = {
		     QPointF(70 - xpos, 0),
		     QPointF(70 - xpos - h2, h2),
		     QPointF(70 - xpos, itsHeight),
		 };

		painter->drawRect(QRectF(70 - xpos, 0, itsWidth - (70 - xpos), itsHeight));
		painter->drawPolygon(points, 3);
	}
	else {
		if (EndBeforeStart) {
			QPolygon polygon;
			polygon << QPoint(0, 0) << QPoint(5, 0) << QPoint(3, itsHeight/2) << QPoint(5,itsHeight/2) << QPoint(3,itsHeight) << QPoint(0,itsHeight);
			painter->drawPolygon(polygon);
		}
		else {
			painter->drawRect(QRectF(0, 0, itsWidth, itsHeight));
		}
	}
	}
	if (EndBeforeStart) {
		QPolygon polygon;
		polygon << QPoint(0, 0) << QPoint(5, 0) << QPoint(3, itsHeight/2) << QPoint(5,itsHeight/2) << QPoint(3,itsHeight) << QPoint(0,itsHeight);
		painter->drawPolygon(polygon);
	}
// for test purpose: draw boundings
//		QPen pen(Qt::black, 1);
//		painter->setBrush(Qt::NoBrush);
//		painter->setPen(pen);
//		painter->drawRect(itsBoundings);

}
}

void GraphicTask::mousePressEvent(QGraphicsSceneMouseEvent * event) {
	if (event->button() == Qt::LeftButton) {
		dragStartPosition = event->lastScreenPos();
		itsScene->startMoveTask(dragStartPosition.x());
		if (event->modifiers() == Qt::ControlModifier) { // control key was pressed
			if (itsSelectedHighLightOn) {
				itsSelectedHighLightOn = false;
				itsScene->deselectTask(itsTaskID, false);
			}
			else {
				itsSelectedHighLightOn = true;
				itsScene->selectTask(itsTaskID, false);
			}
		}
		else { // without control key
			if (!itsSelectedHighLightOn) { // if not already selected
				itsSelectedHighLightOn = true;
				itsScene->selectTask(itsTaskID, true);
			}
		}
		update();
	}
	else if (event->button() == Qt::RightButton) {
		if (!itsSelectedHighLightOn) {
			itsScene->selectTask(itsTaskID, true);
		}
	}
	event->accept();
}

void GraphicTask::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) {
	itsSelectedHighLightOn = true;
	itsScene->selectTask(itsTaskID, true);
	itsScene->showTaskProperties(itsTaskID);
	event->accept();
}

void GraphicTask::mouseReleaseEvent(QGraphicsSceneMouseEvent * event) {
	if (!hasBeenMoved) {
		if (!(event->modifiers() == Qt::ControlModifier)) { // no control key
			itsSelectedHighLightOn = true;
		}
	}
	setCursor(Qt::ArrowCursor);
	if ((event->button() == Qt::LeftButton) && (!dontMove)) {
		if (hasBeenMoved) {
			// get the new position and calculate the new time
			itsScene->requestTaskMove(itsTaskID, itsScene->xPos2Time(static_cast<int>(scenePos().x())));
			hasBeenMoved = false;
		}
	}
}

void GraphicTask::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if (!dontMove) {
		if (!(event->buttons() & Qt::LeftButton))
			return;
		if ((event->lastScreenPos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
			return;
		else setCursor(Qt::SizeHorCursor);
	}
	else { // cannot move an inactive task
		setCursor(Qt::ForbiddenCursor);
		event->ignore();
		return;
	}

	itsScene->moveSelectedTasks(event);
    hasBeenMoved = true;
}

void GraphicTask::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
	// first check if this should apply to multiple tasks
	if (itsScene->controller()->multipleSelected()) {
		QMenu menu;
		QAction *action = menu.addAction("Unschedule multiple tasks");
		connect(action, SIGNAL(triggered()), this, SLOT(unscheduleSelectedTasks()));
		action = menu.addAction("set PRESCHEDULED");
		connect(action, SIGNAL(triggered()), this, SLOT(preScheduleSelectedTasks()));
		action = menu.addAction("set SCHEDULED");
		connect(action, SIGNAL(triggered()), this, SLOT(scheduleSelectedTasks()));
        action = menu.addAction("Move/Redistribute tasks");
		connect(action, SIGNAL(triggered()), this, SLOT(showMoveTaskDialog()));
		action = menu.addAction("Copy tasks");
		connect(action, SIGNAL(triggered()), this, SLOT(copySelectedTasks()));
		action = menu.addAction("Delete tasks");
		connect(action, SIGNAL(triggered()), this, SLOT(deleteSelectedTasks()));
		action = menu.addAction("Put on hold now!");
		connect(action, SIGNAL(triggered()), this, SLOT(setSelectedTasksOnHold()));
		action = menu.addAction("Multi-edit");
		connect(action, SIGNAL(triggered()), this, SLOT(showSelectedTasksProperties()));
		menu.exec(event->screenPos());
	}
	else {
		const Task *pTask(itsScene->getTask(itsTaskID));
		if (pTask) {
			Task::task_status status = pTask->getStatus();
			QMenu menu;
			QAction *action;
			if ((pTask->isObservation()) && ((status == Task::PRESCHEDULED) || (status == Task::SCHEDULED))) {
				action = menu.addAction("Unschedule task");
				connect(action, SIGNAL(triggered()), this, SLOT(unscheduleTask()));
			}
			if ((pTask->isObservation()) && ((status < Task::PRESCHEDULED) || (status == Task::SCHEDULED))) {
				action = menu.addAction("set PRESCHEDULED");
				connect(action, SIGNAL(triggered()), this, SLOT(preScheduleTask()));
			}
			if ((pTask->isObservation()) && (status == Task::PRESCHEDULED)) {
				action = menu.addAction("set SCHEDULED");
				connect(action, SIGNAL(triggered()), this, SLOT(scheduleTask()));
			}
			action = menu.addAction("Move task");
			connect(action, SIGNAL(triggered()), this, SLOT(showMoveTaskDialog()));
			action = menu.addAction("Copy task");
			connect(action, SIGNAL(triggered()), this, SLOT(copyTask()));
			if (status == Task::ABORTED) {
				action = menu.addAction("Reschedule aborted task");
				connect(action, SIGNAL(triggered()), this, SLOT(rescheduleTask()));
			}
			action = menu.addAction("Delete task");
			connect(action, SIGNAL(triggered()), this, SLOT(deleteTask()));

			if ((status <= Task::SCHEDULED) & (status != Task::ON_HOLD)) {
				action = menu.addAction("Put on hold now!");
				connect(action, SIGNAL(triggered()), this, SLOT(setTaskOnHold()));
			}
			action = menu.addAction("Properties");
			connect(action, SIGNAL(triggered()), this, SLOT(showTaskProperties()));

			action = menu.addAction("SAS State history");
			connect(action, SIGNAL(triggered()), this, SLOT(showStateHistory()));

			int sasTreeID(pTask->getSASTreeID());
			if (sasTreeID) {
				action = menu.addAction("SAS tree viewer");
				connect(action, SIGNAL(triggered()), this, SLOT(openSASTreeViewer(void)));
                action = menu.addAction("View meta-data");
                connect(action, SIGNAL(triggered()), this, SLOT(openMetaDataViewer(void)));
			}

			menu.exec(event->screenPos());
		}
	}
}

void GraphicTask::unscheduleTask(void) {
	itsScene->unscheduleTask(itsTaskID);
}

void GraphicTask::unscheduleSelectedTasks(void) {
	itsScene->unscheduleSelectedTasks();
}

void GraphicTask::preScheduleTask(void) {
	itsScene->scheduleTask(itsTaskID, Task::PRESCHEDULED);
}

void GraphicTask::scheduleTask(void) {
	itsScene->scheduleTask(itsTaskID, Task::SCHEDULED);
}

void GraphicTask::preScheduleSelectedTasks(void) {
	itsScene->preScheduleSelectedTasks();
}

void GraphicTask::scheduleSelectedTasks(void) {
	itsScene->scheduleSelectedTasks();
}

void GraphicTask::setTaskOnHold(void) {
	itsScene->setTaskOnHold(itsTaskID);
}

void GraphicTask::setSelectedTasksOnHold(void) {
	itsScene->setSelectedTasksOnHold();
}

void GraphicTask::copyTask(void) {
	itsScene->copyTask(itsTaskID);
}

void GraphicTask::copySelectedTasks(void) {
	itsScene->copySelectedTasks();
}

void GraphicTask::rescheduleTask(void) {
	itsScene->rescheduleTask(itsTaskID);
}

void GraphicTask::deleteTask(void) {
	itsScene->deleteTask(itsTaskID);
}

void GraphicTask::deleteSelectedTasks(void) {
	itsScene->deleteSelectedTasks();
}

void GraphicTask::showTaskProperties(void) {
	itsScene->showTaskProperties(itsTaskID);
}

void GraphicTask::showMoveTaskDialog(void) {
	itsScene->openMoveTasksDialog();
}

void GraphicTask::showSelectedTasksProperties(void) {
	itsScene->showSelectedTasksProperties();
}

void GraphicTask::showStateHistory(void) {
	itsScene->showTaskStateHistory(itsTaskID);
}

void GraphicTask::openSASTreeViewer(void) const {
	const Task *pTask(itsScene->controller()->getTask(itsTaskID));
	if (pTask) {
		itsScene->controller()->openSASTreeViewer(pTask->getSASTreeID());
	}
}

void GraphicTask::openMetaDataViewer(void) const {
    const Task *pTask(itsScene->controller()->getTask(itsTaskID));
    if (pTask) {
        itsScene->controller()->openMetaDataViewer(pTask->getSASTreeID());
    }
}


void GraphicTask::setSelected(bool is_selected) {
	if (is_selected) {
		if (!itsSelectedHighLightOn) {
			itsSelectedHighLightOn = true;
			update(itsBoundings);
		}
	}
	else if (itsSelectedHighLightOn) {
		itsSelectedHighLightOn = false;
		update(itsBoundings);
	}
}

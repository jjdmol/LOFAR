/*
 * GraphicTask.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date    : $Date$
 * First creation : Oct 20, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/GraphicTask.h $
 *
 */

#ifndef GRAPHICTASK_H_
#define GRAPHICTASK_H_

#define TASK_RECT_HEIGHT 10

#include <QGraphicsItem>
#include <QRectF>
#include <QPoint>
#include <QObject>
#include "astrodatetime.h"
#include "task.h"
#include <string>

class GraphicResourceScene;
class QDragMoveEvent;
class Task;
class GraphicStationTaskLine;

class GraphicTask : /*public QObject,*/ public QGraphicsObject {

	Q_OBJECT

public:
	GraphicTask(GraphicStationTaskLine *stationLine, GraphicResourceScene *scene, unsigned int task_id,
			unsigned station_id, Task::task_color_mode color_mode/*, bool inactive*/);
	virtual ~GraphicTask();

    QRectF boundingRect() const {return itsBoundings;}
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    unsigned getTaskID(void) const {return itsTaskID;}

    const std::string &toolTip(void) const { return itsTooltip; }
    std::string toolTipHTML(void) const; // the tooltip but with all the '\n' replaced by <br>

    void setTaskColorMode(Task::task_color_mode color_mode) { if (itsColorMode != color_mode) {itsColorMode = color_mode; update(itsBoundings);} }
//    void setNormalColorMode(void) {itsTaskTypeColorOn = false;}
//    void hoverEnterEvent(QGraphicsSceneMouseEvent * event);
//    void hoverLeaveEvent(QGraphicsSceneMouseEvent * event);

    void setSelected(bool is_selected);
    void setPredecessorHighLight(bool onoff) {itsPredecessorHighLightOn = onoff; update(itsBoundings);}
//    void fixSchedule(bool fix_schedule) {fixedSchedule = fix_schedule;}
//    void setStatus(task_status) { itsStatus = task_status;}
//    void setInactive(bool inactive) {isInactive = inactive;}
//    void setHovered(bool is_hovered);

    unsigned stationID(void) const {return itsStationID;}
//    const IDvector &getPredecessors(void) const;

    void updateTask(/*bool inActive*/);
	void updatePosition(void);

	int xPos(void) const {return static_cast<int>(pos().x());}
	int yPos(void) const {return yZeroPosTimeLine;}
	int left(void) const {return static_cast<int>(pos().x());}
	int right(void) const {return static_cast<int>(pos().x()) + itsWidth;}
	int top(void) const {return yZeroPosTimeLine;}
	int bottom(void) const {return yZeroPosTimeLine + itsHeight;}

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent * event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );

private slots:
//	void abortTask(void);
	void unscheduleTask(void);
	void unscheduleSelectedTasks(void);
	void preScheduleTask(void);
	void scheduleTask(void);
	void preScheduleSelectedTasks(void);
	void scheduleSelectedTasks(void);
	void setTaskOnHold(void);
	void setSelectedTasksOnHold(void);
	void copyTask(void);
	void copySelectedTasks(void);
	void rescheduleTask(void);
	void deleteTask(void);
	void deleteSelectedTasks(void);
	void showTaskProperties(void);
	void showSelectedTasksProperties(void);
	void showStateHistory(void);
    void openSASTreeViewer(void) const;
    void openMetaDataViewer(void) const;
	void showMoveTaskDialog(void);


protected:
	void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
	GraphicResourceScene *itsScene;
	unsigned itsTaskID;
	unsigned itsStationID;
	int yZeroPosTimeLine;
	int itsWidth, itsHeight;
	QRectF itsBoundings;
	bool EndBeforeStart;
	std::string itsTooltip;
	Task::task_color_mode itsColorMode;
	bool itsSelectedHighLightOn, itsPredecessorHighLightOn; //itsHoverHighLightOn
	QPoint dragStartPosition;
	bool dontMove, hasBeenMoved;
};

#endif /* GRAPHICTASK_H_ */

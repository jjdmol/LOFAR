/*
 * GraphicCurrentTimeLine.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 15-apr-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/GraphicCurrentTimeLine.cpp $
 *
 */

#include "GraphicCurrentTimeLine.h"
#include "GraphicResourceScene.h"
#include "lofar_scheduler.h"
#include "astrodatetime.h"
#include <QPainter>
#include "Controller.h"

GraphicCurrentTimeLine::GraphicCurrentTimeLine(GraphicResourceScene *scene, int height)
: itsScene(scene), itsHeight(height), itsLinePen(Qt::red ,2 , Qt::DashLine), isInSchedule(false), isHidden(false) {
//	updateTime();
    setZValue(4); // time line is on top of everything, tasks are at z level 1, background at 0
	itsTimeBackgroundBrush = QBrush(Qt::white, Qt::SolidPattern);
	setY(20);
}

GraphicCurrentTimeLine::~GraphicCurrentTimeLine() {
	// TODO Auto-generated destructor stub
}

void GraphicCurrentTimeLine::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/) {
	if (isInSchedule & (!isHidden)) {
		painter->setPen(itsLinePen);
		painter->drawLine(0,0,0,itsHeight);
        QString UTtimeStr(itsCurrentTime.toString("hh:mm:ss") + " UT");
		painter->setBrush(itsTimeBackgroundBrush);
		painter->setPen(Qt::blue);
		painter->setBrush(QColor(255, 255, 255, 185));
		painter->setFont(QFont("Helvetica",9,QFont::Normal));
		// upper time display
		painter->drawRect(4,6,78,23);
		painter->drawText(7,18,UTtimeStr);
		painter->drawText(7,28,QString(itsLST.toString(1).c_str()) + " LST");
		// lower time display
//		painter->drawRect(4,itsHeight-1,52,12);
//		painter->drawText(7,itsHeight + 10,timeStr);
		// for test purpose: draw boundings
//		QPen pen(Qt::cyan, 1);
//		painter->setBrush(Qt::NoBrush);
//		painter->setPen(pen);
//		painter->drawRect(itsBoundings);
	}
}

void GraphicCurrentTimeLine::updateTime(const QDateTime &UTC) {
    itsCurrentTime = UTC;
    int JD = itsCurrentTime.date().toJulianDay() - J2000_EPOCH;
    if ((JD >= Controller::theSchedulerSettings.getEarliestSchedulingDay().toJulian()) & (JD <=  Controller::theSchedulerSettings.getLatestSchedulingDay().toJulian())) {
        isInSchedule = true;
        itsLST = AstroDateTime(itsCurrentTime).toLST();
        setPos(itsScene->time2xPos(AstroDateTime(itsCurrentTime.date().day(), itsCurrentTime.date().month(), itsCurrentTime.date().year(),
                itsCurrentTime.time().hour(), itsCurrentTime.time().minute(), itsCurrentTime.time().second()) ), pos().y() );
        itsBoundings = QRectF(-1, -1, 85, itsHeight + 14);
        update(itsBoundings);
    }
    else isInSchedule = false;
}

AstroDateTime GraphicCurrentTimeLine::currentDateTime(void) const {
	AstroDateTime now(itsCurrentTime.date().day(), itsCurrentTime.date().month(), itsCurrentTime.date().year(),
			itsCurrentTime.time().hour(), itsCurrentTime.time().minute(), itsCurrentTime.time().second());
	return now;
}

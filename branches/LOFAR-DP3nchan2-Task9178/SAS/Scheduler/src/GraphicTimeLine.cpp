/*
 * GraphicTimeLine.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date    : $Date$
 * First creation : Oct 14, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/GraphicTimeLine.cpp $
 *
 */

#include "lofar_scheduler.h"
#include "lofar_utils.h"
#include "GraphicTimeLine.h"
#include "GraphicResourceScene.h"
#include "astrodate.h"
#include <QPainter>

GraphicTimeLine::GraphicTimeLine(GraphicResourceScene *parent)
	: itsHeight(27), itsScene(parent), itsScenePos(70)
{
	setPos(70,0);
    setZValue(4); // set stacking level
	updateWidth();
    itsTimeLineDayImage1 = new QImage(":/images/timeline_day.png");
    itsTimeLineDayImage2 = new QImage(":/images/timeline_day_long.png");

}

GraphicTimeLine::~GraphicTimeLine() {
    delete itsTimeLineDayImage1;
    delete itsTimeLineDayImage2;
}

void GraphicTimeLine::updateWidth(void) {
	AstroDate lastDay = itsScene->getEndDay();
	lastDay = lastDay.addDays(1);
	itsWidth = itsScene->time2xPos(lastDay) - itsScenePos;
	update();
}

QRectF GraphicTimeLine::boundingRect() const {
    return QRectF(-65.5, 0, itsWidth + 36, itsHeight + 10);
}

void GraphicTimeLine::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/) {

	// draw a non-opaque white rectangle for the background
	painter->setPen(Qt::NoPen);
	painter->setBrush(QColor(255, 255, 255, 200));
	painter->drawRect(boundingRect());

	QFont normalFont("Liberation Sans",9,QFont::Normal);
	QPen thickPen(Qt::black, 2);
	QPen thinPen(Qt::black, 1);
	AstroDate currentDate = itsScene->getStartDay();
	AstroDate endDate = itsScene->getEndDay();

	painter->setPen(thickPen);
	painter->setFont(normalFont);

	zoom_level zl = itsScene->getZoomLevel();

	if (zl == ZM_MONTH) {
		unsigned int month;
		int xMonthPos, xWeekPos;
		//draw the first week tick
		QString weekStr = QString::number(currentDate.getWeek());
		painter->drawLine(0, 15, 0, 17);
		painter->drawText(-35, 27, "Week: ");
		painter->drawText(16 - (weekStr.length()-1)*5, 27, weekStr);

		// draw the base time line
		painter->setPen(thinPen);
		painter->drawLine(0, 15, itsWidth, 15);

		unsigned int lastMonth(currentDate.getMonth());
		currentDate = currentDate.addDays(7);
		QString monthStr;
		while (currentDate <= endDate) {
			month = currentDate.getMonth();
			if (month != lastMonth) { // draw month inset
				xMonthPos = itsScene->time2xPos(AstroDate(1,month,currentDate.getYear())) - itsScenePos;
				painter->setPen(thickPen);
				painter->drawLine(xMonthPos, 5, xMonthPos, 17);
				painter->setPen(thinPen);
				monthStr = QString(MONTH_NAMES_ENG[month-1]) + " (" + QString::number(currentDate.getYear()) + ")";
				painter->drawText(QPoint(xMonthPos + 50 - monthStr.length()/2, 11), monthStr);
				lastMonth = month;
			}
			xWeekPos = itsScene->time2xPos(currentDate) - itsScenePos;
			painter->drawLine(xWeekPos, 15, xWeekPos, 17); // week ticks
			weekStr = QString::number(currentDate.getWeek());
			painter->drawText(QPoint(xWeekPos + 16 - (weekStr.length()-1)*5, 27), weekStr);
			currentDate = currentDate.addDays(7);
		}
		xWeekPos = itsScene->time2xPos(currentDate) - itsScenePos;
		painter->drawLine(xWeekPos, 15, xWeekPos, 17); // last week thick
	}
	else if (zl == ZM_WEEK) {
		unsigned int week;
		int xDayPos, xWeekPos;
		//draw the first day tick
		QString dayStr = QString::number(currentDate.getDay());
		QString weekStr;
		painter->drawLine(0, 15, 0, 17);
		painter->drawText(-35, 27, "Day:");
		painter->drawText(20 - (dayStr.length()-1)*5, 27, dayStr);

		// draw the base time line
		painter->setPen(thinPen);
		painter->drawLine(0, 15, itsWidth, 15);

		unsigned int lastWeek(currentDate.getWeek());
		currentDate = currentDate.addDays(1);
		while (currentDate <= endDate) {
			week = currentDate.getWeek();
			if (week != lastWeek) { // draw week inset
				xWeekPos = itsScene->time2xPos(currentDate) - itsScenePos;
				painter->setPen(thickPen);
				painter->drawLine(xWeekPos, 5, xWeekPos, 17);
				painter->setPen(thinPen);
				weekStr = "week " + QString::number(currentDate.getWeek()) + " ("
					+ MONTH_SHORT_NAMES_ENG[currentDate.getMonth()-1] + " " + QString::number(currentDate.getYear()) + ")";
				painter->drawText(QPoint(xWeekPos + 120 /*- weekStr.length()*4*/, 11), weekStr);
				lastWeek = week;
			}
			xDayPos = itsScene->time2xPos(currentDate) - itsScenePos;
			painter->drawLine(xDayPos, 15, xDayPos, 17); // day ticks
			dayStr = QString::number(currentDate.getDay());
			painter->drawText(QPoint(xDayPos + 20 - (dayStr.length()-1)*5, 27), dayStr);
			currentDate = currentDate.addDays(1);
		}
		xDayPos = itsScene->time2xPos(currentDate) - itsScenePos;
		painter->drawLine(xDayPos, 15, xDayPos, 17); // last day thick
	}
	else if (zl == ZM_DAY) {
        painter->drawText(-65, 27, "Hour (UTC)");
		// draw the base time line
		painter->setPen(thinPen);
		painter->drawLine(0, 15, itsWidth, 15);
		QString dayStr;
		int xDayPos;
		while (currentDate <= endDate) {
			xDayPos = itsScene->time2xPos(currentDate) - itsScenePos;
			dayStr = QString(DAY_SHORT_NAMES_ENG[currentDate.getDayOfTheWeek()]) + " "
				+ QString::number(currentDate.getDay()) + " "
				+ MONTH_SHORT_NAMES_ENG[currentDate.getMonth()-1] + ", "
				+ QString::number(currentDate.getYear());
			painter->drawText(xDayPos + 15, 13, dayStr);
			painter->drawLine(xDayPos, 5, xDayPos, 19);
			for (unsigned int hour = 0; hour < 24; ++hour) {
				painter->drawLine(xDayPos + hour * 5, 15, xDayPos + hour * 5, 17);
			}
			painter->drawLine(xDayPos + 60, 15, xDayPos + 60, 19); // 12:00 hour longer tick
			currentDate = currentDate.addDays(1);
		}
		xDayPos = itsScene->time2xPos(currentDate) - itsScenePos;
		painter->drawLine(xDayPos, 5, xDayPos, 19); // last day thick
	}
	else if (zl == ZM_HOUR) {
        painter->drawText(-65, 27, "Hour (UTC)");
		// draw the base time line
		painter->setPen(thinPen);
		QString dayStr;
		int xDayPos;
		while (currentDate <= endDate) {
			xDayPos = itsScene->time2xPos(currentDate) - itsScenePos;
			dayStr = QString(DAY_NAMES_ENG[currentDate.getDayOfTheWeek()]) + " "
					+ QString::number(currentDate.getDay()) + " "
					+ MONTH_NAMES_ENG[currentDate.getMonth()-1] + ", "
					+ QString::number(currentDate.getYear());
			painter->drawText(xDayPos + 180, 13, dayStr);
            painter->drawImage(xDayPos -1, 3, *itsTimeLineDayImage1);
			currentDate = currentDate.addDays(1);
		}
		xDayPos = itsScene->time2xPos(currentDate) - itsScenePos;
	}
	else if (zl == ZM_MIN) {
        painter->drawText(-65, 27, "Hour (UTC)");
		// draw the base time line
        QString dayStr;
		int xDayPos;
		painter->setPen(thinPen);
		while (currentDate <= endDate) {
			xDayPos = itsScene->time2xPos(currentDate) - itsScenePos;
			dayStr = QString(DAY_NAMES_ENG[currentDate.getDayOfTheWeek()]) + " "
					+ QString::number(currentDate.getDay()) + " "
					+ MONTH_NAMES_ENG[currentDate.getMonth()-1] + ", "
					+ QString::number(currentDate.getYear());
			painter->drawText(xDayPos + 180, 13, dayStr);
            painter->drawImage(xDayPos -7, 0, *itsTimeLineDayImage2);
			currentDate = currentDate.addDays(1);
		}
		xDayPos = itsScene->time2xPos(currentDate) - itsScenePos;
	}

//	painter->setPen(QPen(Qt::magenta, 0.5));
//	painter->drawRect(boundingRect());
	// for test purpose: draw boundings
//	painter->setPen(QPen(Qt::magenta, 1.0));
//	painter->setBrush(Qt::NoBrush);
//	painter->drawRect(boundingRect());

}

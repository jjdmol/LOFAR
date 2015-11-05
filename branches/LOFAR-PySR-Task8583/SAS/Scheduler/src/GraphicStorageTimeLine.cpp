/*
 * GraphicStorageTimeLine.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date    : $Date$
 * First creation : Nov 16, 2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/GraphicStorageTimeLine.cpp $
 *
 */

#include "GraphicStorageTimeLine.h"
#include "graphicstoragescene.h"
#include "Controller.h"
#include <QPainter>
#include <QRect>
#include <QGraphicsSceneMouseEvent>

using std::max;
using std::min;

GraphicStorageTimeLine::GraphicStorageTimeLine(GraphicStorageScene *parent, int yPos)
	: itsScene(parent), itsHeight(13)
	{
	setZValue(0); // background level
	setPos(parent->getTimeLineZeroPos(), yPos -1);
	updateWidth();
}

GraphicStorageTimeLine::~GraphicStorageTimeLine() {
	itsScene = 0;
}

QRectF GraphicStorageTimeLine::boundingRect() const {
    return QRectF(-40.5, -0.5, itsWidth + 1, itsHeight + 2);
}

void GraphicStorageTimeLine::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/) {
	QPen thickPen(Qt::black, 1.5);
	QPen thinPen(Qt::black, 0.5);

	// draw the station name
//	painter->drawText(-40, itsHeight - 2, itsStationName.c_str());

	// draw day and night times
//	QRect nightRect;
//	QPen nightPen(Qt::lightGray, 0.5);
//	QBrush nightBrush(Qt::lightGray, Qt::Dense4Pattern);
//	painter->setPen(nightPen);
//	painter->setBrush(nightBrush);
//	nightRect.setBottom(itsHeight);
//		const std::vector<std::pair<double, double> > &stationSunVector = Controller::theSchedulerSettings.getStationSunVector(itsStationID);
//		if (!stationSunVector.empty()) {
//			int firstDayInSunVector = Controller::theSchedulerSettings.getEarliestSchedulingDay().toJulian();
//			int startDayIdx = itsScene->getStartDay().toJulian() - firstDayInSunVector;
//			int endDayIdx = itsScene->getEndDay().toJulian() - firstDayInSunVector;
//			// take care not to go out of sun vector bounds
//			startDayIdx = max(startDayIdx, 1);
//			endDayIdx = min(endDayIdx, static_cast<int>(stationSunVector.size()));
//			for (int dayIdx = startDayIdx; dayIdx < endDayIdx; ++dayIdx) {
//				nightRect.setLeft(itsScene->time2xPos(stationSunVector.at(dayIdx-1).second) - pos().x());
//				nightRect.setRight(itsScene->time2xPos(stationSunVector.at(dayIdx).first) - pos().x());
//				painter->drawRect(nightRect);
//			}
//		}

	// draw the station base line
	painter->setPen(QPen(Qt::black, 1.0));
	painter->drawLine(0, itsHeight - 2, itsWidth - 41, itsHeight - 2);
	// for test purpose: draw boundings
//	painter->setPen(QPen(Qt::blue, 1));
//	painter->setBrush(Qt::NoBrush);
//	painter->drawRect(boundingRect());
}

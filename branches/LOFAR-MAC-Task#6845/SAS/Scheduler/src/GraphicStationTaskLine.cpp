/*
 * GraphicStationTaskLine.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date    : $Date$
 * First creation : Oct 15, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/GraphicStationTaskLine.cpp $
 *
 */

#include "GraphicStationTaskLine.h"
#include "GraphicResourceScene.h"
#include "station.h"
#include "Controller.h"
#include <algorithm>
#include <QPainter>
#include <QRect>
#include <QGraphicsSceneMouseEvent>

using std::max;
using std::min;

GraphicStationTaskLine::GraphicStationTaskLine(GraphicResourceScene *parent, const Station &station, int yPos)
    : itsScene(parent), itsStationID(station.getStationID()), itsHeight(13)
	{
    setZValue(0); // stacking level
    setPos(parent->getTimeLineZeroPos(), yPos -1);
	updateWidth();
}

GraphicStationTaskLine::GraphicStationTaskLine(GraphicResourceScene *parent, unsigned int stationID, int yPos)
    : itsScene(parent), itsStationID(stationID), itsHeight(13)
{
    setZValue(0); // background level
	setPos(parent->getTimeLineZeroPos(), yPos -1);
    updateWidth();
}


GraphicStationTaskLine::~GraphicStationTaskLine() {
	itsScene = 0;
}

QRectF GraphicStationTaskLine::boundingRect() const {
    return QRectF(-40.5, -0.5, itsWidth + 1, itsHeight + 2);
}

void GraphicStationTaskLine::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/) {
	// draw day and night times
	QRect nightRect;
	QPen nightPen(Qt::lightGray, 0.5);
	QBrush nightBrush(Qt::lightGray, Qt::Dense4Pattern);
    painter->setPen(nightPen);
    painter->setBrush(nightBrush);
    nightRect.setBottom(itsHeight);
    const std::vector<std::pair<double, double> > &stationSunVector = Controller::theSchedulerSettings.getStationSunVector(itsStationID);
    if (!stationSunVector.empty()) {
        int firstDayInSunVector = Controller::theSchedulerSettings.getEarliestSchedulingDay().toJulian();
        int startDayIdx = itsScene->getStartDay().toJulian() - firstDayInSunVector;
        int endDayIdx = itsScene->getEndDay().toJulian() - firstDayInSunVector;
        // take care not to go out of sun vector bounds
        startDayIdx = max(startDayIdx, 1);
        endDayIdx = min(endDayIdx, static_cast<int>(stationSunVector.size()));
        for (int dayIdx = startDayIdx; dayIdx < endDayIdx; ++dayIdx) {
            nightRect.setLeft(itsScene->time2xPos(stationSunVector.at(dayIdx-1).second) - static_cast<int>(pos().x()));
            nightRect.setRight(itsScene->time2xPos(stationSunVector.at(dayIdx).first) - static_cast<int>(pos().x()));
            painter->drawRect(nightRect);
        }
    }

    // draw the station base line
    painter->setPen(QPen(Qt::black, 1.0));
    painter->drawLine(0, itsHeight - 2, itsWidth - 41, itsHeight - 2);
}

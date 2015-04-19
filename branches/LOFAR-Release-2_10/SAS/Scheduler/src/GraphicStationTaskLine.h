/*
 * GraphicStationTaskLine.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date    : $Date$
 * First creation : Oct 15, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/GraphicStationTaskLine.h $
 *
 */

#ifndef GRAPHICSTATIONTASKLINE_H_
#define GRAPHICSTATIONTASKLINE_H_

#include <QGraphicsItem>
#include "GraphicResourceScene.h"

class Station;

class GraphicStationTaskLine : public QGraphicsItem {
public:
	GraphicStationTaskLine(GraphicResourceScene *parent, const Station &station, int yPos);
    GraphicStationTaskLine(GraphicResourceScene *parent, unsigned int stationID, int yPos);
	virtual ~GraphicStationTaskLine();

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    unsigned int stationID(void) const {return itsStationID;}
	inline void updateWidth(void) {itsWidth = static_cast<int>(itsScene->getTimeLineWidth()) + 41; update();}

private:
    GraphicResourceScene *itsScene;
	unsigned int itsStationID, itsCurrentHighlightTask;
	int itsWidth, itsHeight;
};

#endif /* GRAPHICSTATIONTASKLINE_H_ */

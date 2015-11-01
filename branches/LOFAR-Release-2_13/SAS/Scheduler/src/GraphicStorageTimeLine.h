/*
 * GraphicStorageTimeLine.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date    : $Date$
 * First creation : Nov 16, 2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/GraphicStorageTimeLine.h $
 *
 */

#ifndef GRAPHICSTORAGETIMELINE_H_
#define GRAPHICSTORAGETIMELINE_H_

#include <QGraphicsItem>
#include "graphicstoragescene.h"

class GraphicStorageTimeLine : public QGraphicsItem {
public:
	GraphicStorageTimeLine(GraphicStorageScene *parent, int yPos);
	virtual ~GraphicStorageTimeLine();

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

//    void mousePressEvent(QGraphicsSceneMouseEvent * event);

	inline void updateWidth(void) {itsWidth = itsScene->getTimeLineWidth() + 41; update();}

private:
	GraphicStorageScene *itsScene;
	int itsWidth, itsHeight;
};

#endif /* GRAPHICSTORAGETIMELINE_H_ */

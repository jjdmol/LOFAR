/*
 * GraphicTimeLine.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date    : $Date$
 * First creation : Oct 14, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/GraphicTimeLine.h $
 *
 */

#ifndef GRAPHICTIMELINE_H_
#define GRAPHICTIMELINE_H_

#include <QGraphicsItem>
#include "astrodatetime.h"
class GraphicResourceScene;

class GraphicTimeLine : public QGraphicsItem {
public:
	GraphicTimeLine(GraphicResourceScene *parent);
	virtual ~GraphicTimeLine();

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    int getWidth(void) {return itsWidth;}
    int time2xPos(const AstroDate &date) const;

    void updateWidth(void);

private:
	int itsWidth, itsHeight;
    GraphicResourceScene *itsScene;
	int itsScenePos;
    QImage *itsTimeLineDayImage1, *itsTimeLineDayImage2;
};

#endif /* GRAPHICTIMELINE_H_ */

/*
 * GraphicCurrentTimeLine.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 15-apr-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/GraphicCurrentTimeLine.h $
 *
 */

#ifndef GRAPHICCURRENTTIMELINE_H_
#define GRAPHICCURRENTTIMELINE_H_

#include <QGraphicsItem>
#include <QDateTime>
#include <QPen>
#include <QBrush>
#include "astrodatetime.h"
class GraphicResourceScene;


class GraphicCurrentTimeLine : public QGraphicsItem {
public:
	GraphicCurrentTimeLine(GraphicResourceScene *scene, int height);
	virtual ~GraphicCurrentTimeLine();

    QRectF boundingRect() const {return itsBoundings;}
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    inline void updateHeight(int height) {itsHeight = height;}
    void updateTime(const QDateTime &UTC);
    void redrawTime(void) {updateTime(itsCurrentTime);}
    bool isWithinSchedule(void) {return isInSchedule;}
    AstroDateTime currentDateTime(void) const;
    void show(void) {isHidden = false;}
    void hide(void) {isHidden = true;}

private:
	GraphicResourceScene *itsScene;
	QRectF itsBoundings;
	QDateTime itsCurrentTime;
	AstroTime itsLST;
	int itsHeight;
	QPen itsLinePen;
	QBrush itsTimeBackgroundBrush;
	bool isInSchedule, isHidden;
};

#endif /* GRAPHICCURRENTTIMELINE_H_ */

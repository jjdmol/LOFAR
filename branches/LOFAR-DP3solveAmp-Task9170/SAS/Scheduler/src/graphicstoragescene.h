/*
 * GraphicResourceScene.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date    : $Date$
 * First creation : Nov 12, 2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/graphicstoragescene.h $
 *
 */

#ifndef GRAPHICSTORAGESCENE_H
#define GRAPHICSTORAGESCENE_H

#include <QtGui/QWidget>
#include "ui_graphicstoragescene.h"
#include <QGraphicsScene>
#include "GraphicTimeLine.h"

class Controller;

class GraphicStorageScene : public QGraphicsScene
{
    Q_OBJECT

public:
	GraphicStorageScene(Controller *controller);
    ~GraphicStorageScene();

	int getTimeLineWidth(void) {return static_cast<int>(itsTimeLine->getWidth());}
	int getTimeLineZeroPos(void) const {return itsTimeLineZeroPos;}

private:
    Ui::GraphicStorageSceneClass ui;
	QGraphicsView *itsParentView;
    Controller *itsController;
	GraphicTimeLine *itsTimeLine;
	int itsTimeLineZeroPos; // corresponds to time line zero position
};

#endif // GRAPHICSTORAGESCENE_H

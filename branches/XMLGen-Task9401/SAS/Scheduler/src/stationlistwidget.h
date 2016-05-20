/*
 * stationlistwidget.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Apr 6, 2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/stationlistwidget.h $
 *
 */

#ifndef STATIONLISTWIDGET_H
#define STATIONLISTWIDGET_H

#include <QListWidget>
#include "ui_stationlistwidget.h"

class QMouseEvents;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;


// class used as list of available stations
class StationListWidget : public QListWidget
{
    Q_OBJECT

public:
    StationListWidget(QWidget *parent = 0);
    ~StationListWidget();

    void addStations(const QStringList &stations);
    void removeStation(const QString &station);

protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent * event);
	bool dropMimeData(int index, const QMimeData *data, Qt::DropAction action);
	void keyPressEvent ( QKeyEvent * event );
	void mousePressEvent ( QMouseEvent * event );
	QMimeData *mimeData(const QList<QListWidgetItem *> items) const;

signals:
//	void updateUsedStations(void) const;
	void stationsRemoved(void) const;
	void addStationsToUse(const QStringList &) const;

private:
    Ui::StationListWidgetClass ui;
    QPoint startPos;
};

#endif // STATIONLISTWIDGET_H

/*
 * stationtreewidget.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Apr 6, 2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/stationtreewidget.h $
 *
 */

#ifndef STATIONTREEWIDGET_H
#define STATIONTREEWIDGET_H

#include <QTreeWidget>
#include "ui_stationtreewidget.h"

// class used as tree of used stations
class StationTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    StationTreeWidget(QWidget *parent = 0);
    ~StationTreeWidget();

    void addStations(const QStringList &stations);
    QTreeWidgetItem * addSuperStation(void);
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent * event);
	void dragLeaveEvent(QDragLeaveEvent * event);
	bool dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action);
	QMimeData *mimeData(const QList<QTreeWidgetItem *> items) const;
	QStringList mimeTypes() const;
	Qt::DropActions supportedDropActions () const;
	void keyPressEvent (QKeyEvent * event);
    void addSuperStationAndChilds(void);
    void clear(void) {QTreeWidget::clear(); itsNrOfSuperStations = 0; itsHasChanged = false; itsIsMixed = false; itsNrStationsToRemove = 0;}
    unsigned getNrStationsRemoved(void) const {return itsNrStationsToRemove;}
    void resetHasChangedFlag(void) {itsHasChanged = false;}
    bool hasChanged(void) const {return itsHasChanged;}
    bool isMixed(void) const {return itsIsMixed;}
    void setMixed(void);

protected:
	void dropEvent(QDropEvent * event);

private slots:
	void updateNrStationsToRemove(unsigned nrStations) {itsNrStationsToRemove = nrStations;}

signals:
	void stationsDeleted(const QStringList &) const;
	void checkForStationChanges(void) const;
	void prepareRemoveStations(unsigned) const;

private:
    Ui::StationTreeWidgetClass ui;
    unsigned itsNrStationsToRemove;
    QPoint startPos;
    int itsNrOfSuperStations;
    bool itsHasChanged;
    bool itsIsMixed;
};

#endif // STATIONTREEWIDGET_H

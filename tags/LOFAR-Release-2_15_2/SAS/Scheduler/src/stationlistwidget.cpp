/*
 * stationlistwidget.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Apr 6, 2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/stationlistwidget.cpp $
 *
 */

#include "stationlistwidget.h"
#include <QMimeData>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

#include <iostream>

StationListWidget::StationListWidget(QWidget *parent)
    : QListWidget(parent)
{
	ui.setupUi(this);
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
}

StationListWidget::~StationListWidget()
{

}

void StationListWidget::addStations(const QStringList &stations) {
	for (QStringList::const_iterator it = stations.begin(); it != stations.end(); ++it) {
		if (findItems(*it, Qt::MatchExactly).isEmpty()) {
			addItem(*it);
		}
	}
	sortItems();
}

void StationListWidget::removeStation(const QString &station) {
	QList<QListWidgetItem *> list = findItems(station, Qt::MatchExactly);
	if (!list.isEmpty()) {
		QModelIndex idx = indexFromItem(list.first());
		delete takeItem(idx.row());
	}
}

void StationListWidget::dragEnterEvent(QDragEnterEvent *event) {
    if (event->source() != this)
		event->accept();
}

void StationListWidget::dragMoveEvent(QDragMoveEvent * event) {
	if (event->source())
		event->accept();
}

void StationListWidget::mousePressEvent(QMouseEvent * event) {
	// when space key was pressed the multiselection mode (keyboard selection) was set, now go back to extendedselection for mouse selection
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	QListWidget::mousePressEvent(event);
}

void StationListWidget::keyPressEvent(QKeyEvent * event) {
	QStringList stationList;
	QList<QListWidgetItem*> items;
	switch (event->key()) {
	case Qt::Key_Space:
		setSelectionMode(QAbstractItemView::MultiSelection);
		QListWidget::keyPressEvent(event);
		break;
	case Qt::Key_Right:
	case Qt::Key_Plus:
	case Qt::Key_Return:
	case Qt::Key_Insert:
		items = this->selectedItems();
		for (QList<QListWidgetItem *>::const_iterator it = items.begin(); it != items.end(); ++it) {
			stationList.append((*it)->text());
			delete *it;
		}
		emit addStationsToUse(stationList);

		break;
	default:
		QListWidget::keyPressEvent(event);
		break;
	}
}

bool StationListWidget::dropMimeData(int /*index*/, const QMimeData *data, Qt::DropAction /*action*/)
{
//	bool possibleChange(false);
	QStringList listOfStations = data->text().split("|");
	if (!listOfStations.isEmpty()) {
		for (QStringList::const_iterator it = listOfStations.begin(); it != listOfStations.end(); ++it) { // add top-level item(s)
			QListWidgetItem *stationItem = new QListWidgetItem(*it, this);
			stationItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsDragEnabled);
//			possibleChange = true;
		}
		sortItems();
	}
	if (!listOfStations.isEmpty()) emit stationsRemoved();
	return true;

}


QMimeData *StationListWidget::mimeData(const QList<QListWidgetItem *> items) const {
		QMimeData *mimeData = new QMimeData;
		QString stationList;
		for (QList<QListWidgetItem *>::const_iterator it = items.begin(); it != items.end()-1; ++it) {
			stationList += (*it)->text() + "|";
		}
		stationList += items.back()->text();

		mimeData->setText(stationList);
		return mimeData;
}

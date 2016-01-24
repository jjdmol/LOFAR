/*
 * stationtreewidget.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Apr 6, 2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/stationtreewidget.cpp $
 *
 */

#include "lofar_scheduler.h"
#include "stationtreewidget.h"
#include <QMimeData>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <iostream>

StationTreeWidget::StationTreeWidget(QWidget *parent)
    : QTreeWidget(parent), itsNrOfSuperStations(0), itsHasChanged(false), itsIsMixed(false)
{
	ui.setupUi(this);
    setAcceptDrops(true);
//    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);
    setDefaultDropAction(Qt::MoveAction);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setRootIsDecorated(true);
//    setSortingEnabled(true);
    setAnimated(true);
    setHeaderHidden(true);
    connect(this, SIGNAL(prepareRemoveStations(unsigned)), this, SLOT(updateNrStationsToRemove(unsigned)));
}

StationTreeWidget::~StationTreeWidget()
{

}

void StationTreeWidget::addStations(const QStringList &stations) {
	if (itsIsMixed) {
		clear(); // remove MIXED
		itsIsMixed = false;
	}
	if (!stations.isEmpty()) {
		for (QStringList::const_iterator it = stations.begin(); it != stations.end(); ++it) {
			QTreeWidgetItem * stationItem = new QTreeWidgetItem(this, QStringList(*it));
			stationItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsDragEnabled);
		}
		itsHasChanged = true;
	}
	sortItems(1,Qt::AscendingOrder);
	sortItems(0,Qt::AscendingOrder);
}

QTreeWidgetItem * StationTreeWidget::addSuperStation(void) {
	QTreeWidgetItem *superStation = new QTreeWidgetItem(this, QStringList(QString(">Super station ") + QString::number(++itsNrOfSuperStations)));
	superStation->setFlags(Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsDropEnabled);
	itsHasChanged = true;
	return superStation;
}

void StationTreeWidget::addSuperStationAndChilds(void) {
	QTreeWidgetItem *superStation = addSuperStation();
	QList<QTreeWidgetItem *> selectedStations = selectedItems();
	if (!selectedStations.isEmpty()) {
		QList<QTreeWidgetItem *> deleteChildren, deleteSuperStations;
		for (QList<QTreeWidgetItem *>::iterator it = selectedStations.begin(); it != selectedStations.end(); ++it) {
			if ((*it)->text(0).startsWith(">")) {
				for (int i = 0; i < (*it)->childCount(); ++i) {
					QTreeWidgetItem *copy = new QTreeWidgetItem(*(*it)->child(i));
					superStation->addChild(copy);
				}
				deleteSuperStations.append(*it);
			}
			else {
				QTreeWidgetItem *copy = new QTreeWidgetItem(**it);
				superStation->addChild(copy);
				deleteChildren.append(*it);
			}
		}
		this->addTopLevelItem(superStation);
		for (QList<QTreeWidgetItem *>::iterator it = deleteChildren.begin(); it != deleteChildren.end(); ++it) {
			delete *it;
		}
		for (QList<QTreeWidgetItem *>::iterator it = deleteSuperStations.begin(); it != deleteSuperStations.end(); ++it) {
			delete *it;
		}
		sortItems(1,Qt::AscendingOrder);
		sortItems(0,Qt::AscendingOrder);
		itsHasChanged = true;
	}
}

void StationTreeWidget::setMixed(void) {
	clear();
	new QTreeWidgetItem(this, QStringList("MIXED"));
	itsIsMixed = true;
}

void StationTreeWidget::dragEnterEvent(QDragEnterEvent *event) {
	if (event->source()) {
		if (itsIsMixed) {
			clear(); // remove the '(MIXED)' item
			itsIsMixed = false;
		}
		//		event->setDropAction(Qt::MoveAction);
		event->accept();
		itsHasChanged = true;
	}
}

void StationTreeWidget::dragMoveEvent(QDragMoveEvent * event) {
	if (event->source()) {
		event->accept();
	}
}

void StationTreeWidget::dragLeaveEvent ( QDragLeaveEvent * event ) {
//	if (event->source()) {
		event->accept();
		itsHasChanged = true;
//	}
}

void StationTreeWidget::dropEvent(QDropEvent * event) {
	QTreeWidget::dropEvent(event);
	sortItems(1,Qt::AscendingOrder);
	sortItems(0,Qt::AscendingOrder);
	emit checkForStationChanges();
}

// which mime types can be dropped on this StationTreeWidget
QStringList StationTreeWidget::mimeTypes () const
{
    QStringList qstrList;
    // list of accepted mime types for drop
    qstrList.append("text/uri-list");
    return qstrList;
}

// which actions are supported by this StationTreeWidget class
Qt::DropActions StationTreeWidget::supportedDropActions () const
{
    // returns what actions are supported when dropping
    return Qt::MoveAction;
}

// mimeData: function that creates the dragged data
QMimeData *StationTreeWidget::mimeData(const QList<QTreeWidgetItem *> items) const
{
	unsigned nrStationsToRemove(0);
	QMimeData *mimeData = new QMimeData;
	QString stationList;
	for (QList<QTreeWidgetItem *>::const_iterator it = items.begin(); it != items.end()-1; ++it) {
		stationList += (*it)->text(0) + "|";
		if (!((*it)->parent())) { // if this station does not come from a super station
			++nrStationsToRemove; // count the number of stations that will be removed
		}
	}
	QString lastItemStr(items.back()->text(0));
	if (lastItemStr.compare(MULTIPLE_VALUE_TEXT) != 0) { // if the user is not dragging '(MIXED)'
		stationList += items.back()->text(0);
		if (!(items.back()->parent())) {
			++nrStationsToRemove;
		}
		mimeData->setText(stationList);
		emit prepareRemoveStations(nrStationsToRemove);
	}

	return mimeData;
}

// dropMimeData: function that adds the data dropped on the tree widget
bool StationTreeWidget::dropMimeData(QTreeWidgetItem *parent, int /*index*/, const QMimeData *data, Qt::DropAction /*action*/)
{
	bool possibleChange(false);
	QStringList listOfStations = data->text().split("|");
	if (parent == 0) {
		for (QStringList::const_iterator it = listOfStations.begin(); it != listOfStations.end(); ++it) { // add top-level item(s)
			QStringList stationItemText(*it);
			QTreeWidgetItem *stationItem = new QTreeWidgetItem(this, stationItemText);
			stationItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsDragEnabled);
			possibleChange = true;
		}
	}
	else {
		if (parent->text(0).startsWith(">")) { // add to existing super station parent
			for (QStringList::const_iterator it = listOfStations.begin(); it != listOfStations.end(); ++it) { // add stations to a super-station
				QStringList stationItemText(*it);
				QTreeWidgetItem *stationItem = new QTreeWidgetItem(parent, stationItemText);
				stationItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsDragEnabled); // these stations should not have children
				possibleChange = true;
			}
		}
	}
	sortItems(1,Qt::AscendingOrder);
	sortItems(0,Qt::AscendingOrder);
	if (possibleChange) {
		itsHasChanged = true;
		emit checkForStationChanges();
	}
	return true;
}

void StationTreeWidget::keyPressEvent (QKeyEvent * event) {
	int key(event->key());
	if ((key == Qt::Key_Delete) | (key == Qt::Key_Left) | (key == Qt::Key_Minus)) {
		QStringList deletedStations;
		QList<QTreeWidgetItem *> deleteChildren, deleteSuperStations;
		QList<QTreeWidgetItem *> selectedStations = selectedItems();
		for (QList<QTreeWidgetItem *>::iterator it = selectedStations.begin(); it != selectedStations.end(); ++it) {
			if ((*it)->text(0).startsWith(">")) {
				for (int i = 0; i < (*it)->childCount(); ++i) {
					deletedStations.append((*it)->child(i)->text(0));
				}
				deleteSuperStations.append(*it);
			}
			else {
				deletedStations.append((*it)->text(0));
				deleteChildren.append(*it);
			}
		}
		if (!deleteChildren.isEmpty()) {
			// now delete the items from the tree
			for (QList<QTreeWidgetItem *>::iterator it = deleteChildren.begin(); it != deleteChildren.end(); ++it) {
				delete *it;
			}
			itsHasChanged = true;
		}
		if (!deleteSuperStations.isEmpty()) {
			for (QList<QTreeWidgetItem *>::iterator it = deleteSuperStations.begin(); it != deleteSuperStations.end(); ++it) {
				delete *it;
			}
			itsHasChanged = true;
		}
		emit stationsDeleted(deletedStations); // emit signal to start adding the stations to the list of available stations
		emit checkForStationChanges();
	}
	else if ((event->key() == Qt::Key_A) && (event->modifiers() == Qt::AltModifier)) { // select All
		selectAll();
	}
	else if (event->key() == Qt::Key_S) {
		sortItems(1,Qt::AscendingOrder);
		sortItems(0,Qt::AscendingOrder);
	}
	else {
		QTreeWidget::keyPressEvent(event);
	}
}

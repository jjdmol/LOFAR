#include "tableview.h"
#include <QMouseEvent>
#include <iostream>

TableView::TableView(QWidget *parent)
    : QTableView(parent)
{
}

TableView::~TableView()
{

}

void TableView::mousePressEvent(QMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		if (event->modifiers() == Qt::ControlModifier) { // control or shift key is held down
			if (selectionMode() != QAbstractItemView::MultiSelection) {
				QModelIndexList list(this->selectedIndexes());
				setSelectionMode(QAbstractItemView::MultiSelection);
				setSelectionBehavior(QAbstractItemView::SelectRows);
				for (QModelIndexList::iterator lit = list.begin(); lit != list.end(); ++lit) {
					QModelIndex index = this->model()->index(lit->row(), lit->column());
					this->selectionModel()->select(index, QItemSelectionModel::Deselect);
					for (int col = 0; col < this->model()->columnCount(); ++col) {
						QModelIndex index = this->model()->index(lit->row(), col);
						this->selectionModel()->select(index, QItemSelectionModel::Select);
					}
				}
			}
			else { 
			// make sure that the clicked row selection is toggled.
			// user clicked on it while multiselection was already on, 
			// meaning the clicked row was either selected or deselected
//				int row(this->indexAt(event->pos()).row());
//				for (int col = 0; col < this->model()->columnCount(); ++col) {
//					QModelIndex index = this->model()->index(row, col);
//					this->selectionModel()->select(index, QItemSelectionModel::Toggle);
//				}
				selectRow(this->indexAt(event->pos()).row());
				setCurrentIndex(this->indexAt(event->pos()));
			}
		}
		else if (event->modifiers() == Qt::ShiftModifier) {
			if (selectionMode() != QAbstractItemView::ContiguousSelection) {
				setSelectionMode(QAbstractItemView::ContiguousSelection);
				setSelectionBehavior(QAbstractItemView::SelectRows);
				clearSelection();
				selectRow(this->indexAt(event->pos()).row());
			}
		}

		else if (selectionMode() != QAbstractItemView::SingleSelection) {
			setSelectionMode(QAbstractItemView::SingleSelection);
			clearSelection();
			setSelectionBehavior(QAbstractItemView::SelectItems);
		}
	}

	QModelIndex index = this->indexAt(event->pos());
	if (index.row() != -1) {
		QTableView::mousePressEvent(event);
		emit mouseClick(index, event);
	}
}

void TableView::contextMenuEvent(QContextMenuEvent *event) {
	// first check if this should apply to multiple tasks
	QModelIndex index(indexAt(event->pos()));
	if (index.row() != -1) {
		QTableView::contextMenuEvent(event);
		emit tableContextMenuRequest(index, event);
	}
}

QSet<int> TableView::selectedRows(void) const {
	QSet<int> set;
	QModelIndexList indexes(selectedIndexes());
	QModelIndex index;
	foreach(index, indexes) {
		set.insert(index.row());
	}
	return set;
}

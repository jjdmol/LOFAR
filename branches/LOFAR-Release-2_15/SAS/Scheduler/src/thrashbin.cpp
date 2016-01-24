/*
 * thrashbin.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 18-March-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/thrashbin.cpp $
 *
 */

#include "thrashbin.h"
#include <QStringList>

Thrashbin::Thrashbin(QWidget *parent)
    : QDialog(parent), itsSortColumn(0), itsSortOrder(Qt::AscendingOrder)
{
	ui.setupUi(this);
	setupThrashBin();
}

Thrashbin::~Thrashbin()
{

}

void Thrashbin::emptyThrashBin(void) {
	itsTasks.clear();
	ui.tableWidgetThrash->clearContents();
	ui.tableWidgetThrash->setRowCount(0);
}

void Thrashbin::setupThrashBin(void) {
	QStringList labels;
	labels << "ID" << tr("Task name") << tr("Project name") << tr("Task type") << tr("Status");
	ui.tableWidgetThrash->setColumnCount(5);
	ui.tableWidgetThrash->horizontalHeader()->setDefaultSectionSize(75);
	ui.tableWidgetThrash->verticalHeader()->setDefaultSectionSize(16);
	ui.tableWidgetThrash->setHorizontalHeaderLabels(labels);
	ui.tableWidgetThrash->setColumnWidth(0, 40); // task ID
	ui.tableWidgetThrash->setColumnWidth(1, 150); // task name
	ui.tableWidgetThrash->setColumnWidth(2, 150); // project name
	ui.tableWidgetThrash->setColumnWidth(3, 100); // task type
	ui.tableWidgetThrash->setColumnWidth(4, 80); // task status
//	ui.tableWidgetThrash->verticalHeader()->setDefaultSectionSize(16);
	ui.tableWidgetThrash->setSelectionMode(QAbstractItemView::MultiSelection);
	ui.tableWidgetThrash->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidgetThrash->horizontalHeader()->setSortIndicatorShown(true);
//	ui.tableWidgetThrash->setSortingEnabled(true);
	connect(ui.tableWidgetThrash->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortTable(int)));
}

void Thrashbin::sortTable(int column) {
	itsSortColumn = column;
	if (itsSortOrder == Qt::AscendingOrder)
		itsSortOrder = Qt::DescendingOrder;
	else
		itsSortOrder = Qt::AscendingOrder;
	ui.tableWidgetThrash->sortByColumn(itsSortColumn, itsSortOrder);
}


void Thrashbin::addTasks(const std::vector<Task *> &tasks) {
	unsigned taskID;
    for (std::vector<Task *>::const_iterator it = tasks.begin(); it != tasks.end(); ++it) {
        taskID = (*it)->getID();
        std::pair<std::map<unsigned, Task *>::iterator, bool> ret =
            itsTasks.insert(std::map<unsigned, Task *>::value_type (taskID, *it));
		if (ret.second) {
			int row = ui.tableWidgetThrash->rowCount();
			ui.tableWidgetThrash->insertRow(row);
			//task ID
			QTableWidgetItem *item = new QTableWidgetItem();
			item->setData(0,taskID); // we have to use setData to get correct number sorting for this column
			item->setData(100, taskID);
			ui.tableWidgetThrash->setItem(row, 0, item);
			// task name
            item = new QTableWidgetItem((*it)->getTaskName());
			item->setData(100, taskID);
			ui.tableWidgetThrash->setItem(row, 1, item);
			// project name
            item = new QTableWidgetItem((*it)->getProjectName());
			item->setData(100, taskID);
			ui.tableWidgetThrash->setItem(row, 2, item);
			// task type
            item = new QTableWidgetItem((*it)->getTypeStr());
			item->setData(100, taskID);
			ui.tableWidgetThrash->setItem(row, 3, item);
			// task status
            item = new QTableWidgetItem((*it)->getStatusStr());
			item->setData(100, taskID);
			ui.tableWidgetThrash->setItem(row, 4, item);
		}
	}
	ui.tableWidgetThrash->sortByColumn(itsSortColumn, itsSortOrder);
	if (!itsTasks.empty())
		emit thrashBinContainsItems();
}

void Thrashbin::removeRestoredTasks(const std::vector<unsigned> &restoredTasks) {
	for (std::vector<unsigned>::const_iterator it = restoredTasks.begin(); it != restoredTasks.end(); ++it) {
		itsTasks.erase(*it);
	}
	updateThrashBin();
}

void Thrashbin::updateThrashBin(void) {
	ui.tableWidgetThrash->clearContents();
	ui.tableWidgetThrash->setRowCount(itsTasks.size());
	int row(0);
	QTableWidgetItem *item;
    for (std::map<unsigned, Task *>::const_iterator it = itsTasks.begin(); it != itsTasks.end(); ++it) {
		unsigned taskID = it->first;
		//task ID
		item = new QTableWidgetItem();
		item->setData(0,taskID); // we have to use setData to get correct number sorting for this column
		item->setData(100, taskID);
		ui.tableWidgetThrash->setItem(row, 0, item);
		// task name
        item = new QTableWidgetItem(it->second->getTaskName());
		item->setData(100, taskID);
		ui.tableWidgetThrash->setItem(row, 1, item);
		// project name
        item = new QTableWidgetItem(it->second->getProjectName());
		item->setData(100, taskID);
		ui.tableWidgetThrash->setItem(row, 2, item);
		// task type
        item = new QTableWidgetItem(it->second->getTypeStr());
		item->setData(100, taskID);
		ui.tableWidgetThrash->setItem(row, 3, item);
		// task status
        item = new QTableWidgetItem(it->second->getStatusStr());
		item->setData(100, taskID);
		ui.tableWidgetThrash->setItem(row, 4, item);
	    ++row;
	}
	if (itsTasks.empty()) {
		ui.pushButtonDelete->setEnabled(false);
		ui.pushButtonRestore->setEnabled(false);
		emit thrashBinIsEmpty();
	}
	else {
		ui.tableWidgetThrash->sortByColumn(itsSortColumn, itsSortOrder);
		emit thrashBinContainsItems();
	}
}

void Thrashbin::checkSelection(void) {
	if (ui.tableWidgetThrash->selectedItems().isEmpty()) {
		ui.pushButtonDelete->setEnabled(false);
		ui.pushButtonRestore->setEnabled(false);
	}
	else {
		ui.pushButtonDelete->setEnabled(true);
		ui.pushButtonRestore->setEnabled(true);
	}
}

void Thrashbin::restoreTasks(void) const {
	std::vector<unsigned> tasks;
	unsigned taskID;
	QList<QTableWidgetItem *> list = ui.tableWidgetThrash->selectedItems();
	for (QList<QTableWidgetItem *>::const_iterator it = list.begin(); it != list.end(); ++it) {
		if ((*it)->column() == 0) {
			taskID = (*it)->data(100).toUInt();
			tasks.push_back(taskID);
		}
	}
	emit restoreTasksRequest(tasks);
}

void Thrashbin::deleteTasks(void) {
	unsigned taskID;
	std::vector<unsigned> tasksToDestroy;
    std::map<unsigned, Task *>::iterator tit;
	QList<QTableWidgetItem *> list = ui.tableWidgetThrash->selectedItems();
	for (QList<QTableWidgetItem *>::const_iterator it = list.begin(); it != list.end(); ++it) {
		taskID = (*it)->data(100).toUInt();
		if ((tit = itsTasks.find(taskID)) != itsTasks.end()) {
			itsTasks.erase(tit);
			tasksToDestroy.push_back(taskID);
		}
	}
	updateThrashBin();
	emit destroyTasks(tasksToDestroy);
}

void Thrashbin::reject(void) {
	ui.tableWidgetThrash->clearSelection();
	hide();
}

/*
 * conflictdialog.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 2-sep-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/conflictdialog.cpp $
 *
 */

#include "conflictdialog.h"
#include "Controller.h"

ConflictDialog::ConflictDialog(const Controller *controller)
    : itsController(controller)
{
	ui.setupUi(this);
	// setup conflicts tree
	ui.treeWidgetConflicts->setColumnCount(5);
	QStringList header;
	header << "Task" << "Conflict" << "System" << "Start time" << "End time";
	ui.treeWidgetConflicts->setHeaderLabels(header);
	ui.treeWidgetConflicts->header()->resizeSection(0, 70);
	ui.treeWidgetConflicts->header()->resizeSection(1, 150);
	ui.treeWidgetConflicts->header()->resizeSection(2, 55);
	ui.treeWidgetConflicts->header()->resizeSection(3, 55);
	ui.treeWidgetConflicts->header()->resizeSection(4, 55);
#if QT_VERSION >= 0x050000
    ui.treeWidgetConflicts->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    ui.treeWidgetConflicts->header()->setResizeMode(QHeaderView::ResizeToContents);
#endif
}

ConflictDialog::~ConflictDialog()
{

}

void ConflictDialog::addConflict(const Task *task, task_conflict conflict) {
	 QStringList itemcolums;
	 QString conflictStr;
	 // task id and name
	 itemcolums << "(" + QString::number(task->getID()) + ") " + task->getTaskName();
	 // conflict description
	 if ((conflict >= CONFLICT_NO_CONFLICT) && (conflict < _END_CONFLICTS_)) {
		 conflictStr = TASK_CONFLICTS[conflict];
	 }
	 else {
		 conflictStr = tr("Unknown conflict");
	 }
	 itemcolums << conflictStr;
	 // scheduled start and end time
	 itemcolums << "";
	 itemcolums << task->getScheduledStart().toString().c_str();
	 itemcolums << task->getScheduledEnd().toString().c_str();

	 QTreeWidgetItem *item = new QTreeWidgetItem(ui.treeWidgetConflicts, itemcolums);
	 item->setData(0, Qt::UserRole,task->getID());
	 item->setData(1, Qt::UserRole, TAB_SCHEDULE);

//	 itsConflictTreeItems[task->getID()].append(item);
}

void ConflictDialog::addStorageConflict(const Task *task, dataProductTypes dataProduct, const std::vector<storageResult> &results) {
	for (std::vector<storageResult>::const_iterator it = results.begin(); it != results.end(); ++it) {
		addStorageConflict(task, dataProduct, it->conflict, it->storageNodeID, it->storageRaidID);
	}
}

void ConflictDialog::addStorageConflict(const Task *task, dataProductTypes dataProduct, task_conflict conflict, int node_id, int raid_id) {
	 QStringList itemcolums;
	 QString conflictStr;

	 // task id and name
	 itemcolums << "(" + QString::number(task->getID()) + ") " + task->getTaskName();
	 // conflict description
	 if ((conflict >= CONFLICT_NO_CONFLICT) && (conflict < _END_CONFLICTS_)) {
		 conflictStr = QString(DATA_PRODUCTS[dataProduct]) + ":" + TASK_CONFLICTS[conflict];
	 }
	 else {
		 conflictStr = QString(DATA_PRODUCTS[dataProduct]) + ":" + tr("Unknown conflict");
	 }
	 itemcolums << conflictStr;

	 // Storage node (system)
	 QString strStorageNode;
	 if (node_id != 0) {
		 strStorageNode = itsController->getStorageNodeName(node_id).c_str();
		 if (raid_id != 0) {
			 strStorageNode += itsController->getStorageRaidName(node_id, raid_id).c_str();
		 }
		 itemcolums << strStorageNode;
	 }
	 else {
		 itemcolums << "";
	 }

	 // scheduled start and end time
	 itemcolums.append(task->getScheduledStart().toString().c_str());
	 itemcolums.append(task->getScheduledEnd().toString().c_str());

	 QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, itemcolums);
	 item->setData(0, Qt::UserRole,task->getID());
	 item->setData(1, Qt::UserRole, TAB_STORAGE);

	 // get the details of the storage conflict from the task
     if (task->hasStorage()) {
         const std::vector<storageResult> &taskConflicts = task->storage()->getStorageCheckResult();

         for (std::vector<storageResult>::const_iterator it = taskConflicts.begin(); it != taskConflicts.end(); ++it) {
             strStorageNode = itsController->getStorageNodeName(it->storageNodeID).c_str();
             itemcolums.clear();
             itemcolums << "";
             itemcolums << QString(DATA_PRODUCTS[dataProduct]) + ":" + TASK_CONFLICTS[it->conflict];
             itemcolums << strStorageNode + itsController->getStorageRaidName(it->storageNodeID, it->storageRaidID).c_str();
             QTreeWidgetItem *subitem = new QTreeWidgetItem(item, itemcolums);
             subitem->setData(0, Qt::UserRole,task->getID());
             subitem->setData(1, Qt::UserRole, TAB_STORAGE);

         }
     }

     ui.treeWidgetConflicts->insertTopLevelItem(0,item);

//	 itsConflictTreeItems[task->getID()].append(item);
}


void ConflictDialog::addDataSlotConflict(const Task *task, task_conflict conflict, const std::string &station_name, unsigned otherTask_id) {
	 QStringList itemcolums;
	 // task id and name
	 itemcolums << "(" + QString::number(task->getID()) + ") " + task->getTaskName();
	 // conflict description
	 if (conflict == CONFLICT_BITMODE) {
		 itemcolums << "Bit mode differs from task " + QString::number(otherTask_id) + " running on same station";
	 }
	 else if (conflict == CONFLICT_OUT_OF_DATASLOTS) {
		 itemcolums << "All data slots are occupied on station";
	 }
	 else {
		 itemcolums << tr("Unknown conflict");
	 }

	 // Station (system)
	 itemcolums << station_name.c_str();

	 // scheduled start and end time
	 itemcolums << task->getScheduledStart().toString().c_str() << task->getScheduledEnd().toString().c_str();

	 // add the task
	 QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, itemcolums);
	 item->setData(0, Qt::UserRole, task->getID());
	 item->setData(1, Qt::UserRole, TAB_SCHEDULE);

	 // add the other conflicting task as a child item
	 const Task *otherTask = itsController->getTask(otherTask_id);
	 if (otherTask) {
		 itemcolums.clear();
		 itemcolums << "(" + QString::number(otherTask->getID()) + ") " + otherTask->getTaskName()
		            << "" << station_name.c_str()
		            << otherTask->getScheduledStart().toString().c_str()
		            << otherTask->getScheduledEnd().toString().c_str();
		 QTreeWidgetItem *subitem = new QTreeWidgetItem(item, itemcolums);
		 subitem->setData(0, Qt::UserRole,task->getID());
		 subitem->setData(1, Qt::UserRole, TAB_STORAGE);
	 }

	 ui.treeWidgetConflicts->insertTopLevelItem(0,item);


//	 itsConflictTreeItems[task->getID()].append(item);
}

void ConflictDialog::manualSolveConflict(void) const {
	QList<QTreeWidgetItem*> selectItems(ui.treeWidgetConflicts->selectedItems());
	if (!selectItems.isEmpty()) {
		itsController->showTaskDialog(selectItems.first()->data(0, Qt::UserRole).toUInt(), static_cast<tabIndex>(selectItems.first()->data(1, Qt::UserRole).toUInt()));
	}
}


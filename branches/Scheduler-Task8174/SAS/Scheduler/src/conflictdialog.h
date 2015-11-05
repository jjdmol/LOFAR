/*
 * conflictdialog.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 2-sep-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/conflictdialog.h $
 *
 */

#ifndef CONFLICTDIALOG_H
#define CONFLICTDIALOG_H

#include <QDialog>
#include "ui_conflictdialog.h"
#include <map>
#include "taskstorage.h"

class Controller;

// conflictTreeMap: key = task ID, value = list of column items for conflict tree widget
typedef std::map<unsigned, QList<QTreeWidgetItem *> > conflictTreeMap;

class ConflictDialog : public QDialog
{
    Q_OBJECT

public:
    ConflictDialog(const Controller *controller);
    ~ConflictDialog();

    void addConflict(const Task *task, task_conflict conflict);
    void addStorageConflict(const Task *task, dataProductTypes dataProduct, task_conflict conflict, int node_id = 0, int raid_id = 0);
    void addStorageConflict(const Task *task, dataProductTypes dataProduct, const std::vector<storageResult> &results);
    void addDataSlotConflict(const Task *task, task_conflict conflict, const std::string &station_name, unsigned otherTask_id = 0);
    void clearAllConflicts(void) {/*itsConflictTreeItems.clear();*/ui.treeWidgetConflicts->clear();}

private:
	void clearConflict();

private slots:
	void manualSolveConflict(void) const;

private:
    Ui::ConflictDialogClass ui;
    const Controller *itsController;
//    conflictTreeMap itsConflictTreeItems;
};

#endif // CONFLICTDIALOG_H

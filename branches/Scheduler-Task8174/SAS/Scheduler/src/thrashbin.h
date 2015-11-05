/*
 * thrashbin.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 18-March-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/thrashbin.h $
 *
 */

#ifndef THRASHBIN_H
#define THRASHBIN_H

#include <QDialog>
#include "ui_thrashbin.h"
#include <map>
#include <vector>
#include "task.h"

class Thrashbin : public QDialog
{
    Q_OBJECT

public:
    Thrashbin(QWidget *parent = 0);
    ~Thrashbin();

    void addTasks(const std::vector<Task *> &tasks);
    void emptyThrashBin(void);// {itsTasks.clear();ui.tableWidgetThrash->clear();}
	void removeRestoredTasks(const std::vector<unsigned> &tasks);

private:
	void setupThrashBin(void);
	void updateThrashBin(void);

private slots:
	void deleteTasks(void);
	void restoreTasks(void) const;
	void reject(void);
	void checkSelection(void);
	void sortTable(int column);

signals:
	void restoreTasksRequest(const std::vector<unsigned> &tasks) const;
	void thrashBinIsEmpty(void) const;
	void thrashBinContainsItems(void) const;
	void destroyTasks(std::vector<unsigned>) const;

private:
    Ui::ThrashbinClass ui;
    std::map<unsigned, Task *> itsTasks;
    int itsSortColumn;
    Qt::SortOrder itsSortOrder;
};

#endif // THRASHBIN_H

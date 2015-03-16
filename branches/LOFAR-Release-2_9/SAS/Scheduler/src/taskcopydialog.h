#ifndef TASKCOPYDIALOG_H
#define TASKCOPYDIALOG_H

#include <QtGui/QDialog>
#include "ui_taskcopydialog.h"
#include "astrodatetime.h"
#include "astrotime.h"
#include "task.h"

class TaskCopyDialog : public QDialog
{
    Q_OBJECT

public:
    TaskCopyDialog(QWidget *parent = 0);
    ~TaskCopyDialog();

    const AstroDateTime &getStartTime(void) const {return itsStartTime;}
    const AstroTime &getTimeStep(void) const {return itsTimeStep;}
    const Task::task_status &getTaskState(void) const {return itsTaskState;}
    void setStartTime(const AstroDateTime &start_time);
    void setTimeStep(const AstroTime &time_step);

private slots:
	void okClicked(void);
    void cancelClicked(void);
    void nrCopiesChanged(int nrCopies);

private:
    Ui::TaskCopyDialogClass ui;
    AstroDateTime itsStartTime;
    AstroTime itsTimeStep;
    Task::task_status itsTaskState;
};

#endif // TASKCOPYDIALOG_H

/*
 * redistributetasksdialog.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 26-oct-2012
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/redistributetasksdialog.h $
 *
 */

#ifndef REDISTRIBUTETASKSDIALOG_H
#define REDISTRIBUTETASKSDIALOG_H

#include <QDialog>
#include "ui_redistributetasksdialog.h"

class redistributeTasksDialog : public QDialog
{
    Q_OBJECT

public:
    redistributeTasksDialog(QWidget *parent = 0);
    ~redistributeTasksDialog();

    void setStartTime(const QDateTime &start_time) {ui.dateTimeEditStartTime->setDateTime(start_time);}
    void setTimeStep1(const QTime &gap1) {ui.timeEditTimeStep1->setTime(gap1);}
    void setTimeStep2(const QTime &gap2) {ui.timeEditTimeStep2->setTime(gap2);}

    QDateTime getStartTime(void) const {return ui.dateTimeEditStartTime->dateTime();}
    QTime getTimeStep1(void) const {return ui.timeEditTimeStep1->time();}
    QTime getTimeStep2(void) const {return ui.timeEditTimeStep2->time();}
    int getNrParallelTasks(void) const {return ui.spinBoxNrParallelTasks->value();}

    void setRedistributeMode(void);
    void setAfterPredecessorMode(void);
    void setMoveToLSTType(void) {ui.labelDateTimeEditStart->setText("Center LST time:");}
    void setMoveToStartType(void) {ui.labelDateTimeEditStart->setText("Start time:");}

private:
    Ui::redistributeTasksDialogClass ui;
};

#endif // REDISTRIBUTETASKSDIALOG_H

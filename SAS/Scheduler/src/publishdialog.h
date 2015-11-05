/*
 * publishdialog.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date    : $Date$
 * First creation : Dec 8, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/publishdialog.h $
 *
 */

#ifndef PUBLISHDIALOG_H
#define PUBLISHDIALOG_H

#include <QDialog>
#include <QFileInfo>
#include "ui_publishdialog.h"
#include "schedulersettings.h"

class PublishDialog : public QDialog
{
    Q_OBJECT

public:
    PublishDialog(QWidget *parent = 0);
    ~PublishDialog();

    // put the correct week numbers and dates in the publish week selection dialog
    void initPublishDialog(void);
    void show(void);

private:
	void accept(void);

signals:
    void goPublish(const scheduleWeekVector &);

private slots:
	void toggleSelection(void);

private:
    Ui::PublishDialogClass ui;
    scheduleWeekVector itsPublishWeeks;
    bool select;
};

#endif // PUBLISHDIALOG_H

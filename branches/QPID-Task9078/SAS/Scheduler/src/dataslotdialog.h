/*
 * dataslotdialog.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : June 2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/dataslotdialog.h $
 *
 */

#ifndef DATASLOTDIALOG_H
#define DATASLOTDIALOG_H

#include <QDialog>
#include "ui_dataslotdialog.h"
#include "task.h"
#include "stationtask.h"

class DataSlotDialog : public QDialog
{
    Q_OBJECT

public:
    DataSlotDialog(QWidget *parent = 0);
    ~DataSlotDialog();

    void clear();
    void loadData(const dataSlotMap &dataSlots);

private:
    Ui::DataSlotDialogClass ui;
};

#endif // DATASLOTDIALOG_H

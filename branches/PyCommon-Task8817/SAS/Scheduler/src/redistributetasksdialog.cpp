/*
 * redistributetasksdialog.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 26-oct-2012
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/redistributetasksdialog.cpp $
 *
 */

#include "redistributetasksdialog.h"

redistributeTasksDialog::redistributeTasksDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
}

redistributeTasksDialog::~redistributeTasksDialog()
{

}

void redistributeTasksDialog::setRedistributeMode(void) {
	ui.dateTimeEditStartTime->show();
	ui.labelDateTimeEditStart->show();
	ui.labelNrTasksInParallel->show();
	ui.spinBoxNrParallelTasks->show();
	ui.labelTimeStep1->setText("Time gap 1:");
	ui.labelTimeStep2->show();
	ui.timeEditTimeStep2->show();
}

void redistributeTasksDialog::setAfterPredecessorMode(void) {
	ui.dateTimeEditStartTime->hide();
	ui.labelDateTimeEditStart->hide();
	ui.labelNrTasksInParallel->hide();
	ui.spinBoxNrParallelTasks->hide();
	ui.labelTimeStep1->setText("Distance to predecessor:");
	ui.labelTimeStep2->hide();
	ui.timeEditTimeStep2->hide();
}

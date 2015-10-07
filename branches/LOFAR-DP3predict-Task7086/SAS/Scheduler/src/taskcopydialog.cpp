#include "taskcopydialog.h"
#include "Controller.h"

TaskCopyDialog::TaskCopyDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.pushButton_Ok, SIGNAL(clicked()), this, SLOT(okClicked(void)));
	connect(ui.pushButton_Cancel, SIGNAL(clicked()), this, SLOT(cancelClicked(void)));
	QStringList states;
	states << task_states_str[Task::UNSCHEDULED] << task_states_str[Task::PRESCHEDULED];
	ui.comboBoxNewStatus->addItems(states);
	ui.comboBoxNewStatus->setCurrentIndex(1); // set to PRESCHEDULED by default
	ui.pushButton_Ok->setDefault(true);
	connect(ui.spinBoxNrCopies, SIGNAL(valueChanged(int)), this, SLOT(nrCopiesChanged(int)));
	// set the default start time to 10 minutes from now
	QDateTime startTime = QDateTime::currentDateTime().toUTC();
	QDateTime minTime = startTime;
	startTime = startTime.addSecs(630); // add the task 10.5 minutes from now and round to start at complete minutes
	startTime.setTime(QTime(startTime.time().hour(), startTime.time().minute()));
	ui.dateTimeEditStartTime->setMinimumDate(minTime.date());
	ui.dateTimeEditStartTime->setDateTime(startTime);
	ui.lineEditTimeStep->setText(Controller::theSchedulerSettings.getMinimumTimeBetweenTasks().toString().c_str());
}

TaskCopyDialog::~TaskCopyDialog()
{

}

void TaskCopyDialog::okClicked(void) {
	QDateTime start(ui.dateTimeEditStartTime->dateTime());
	itsStartTime = AstroDateTime(start.date().day(), start.date().month(), start.date().year(),
			start.time().hour(), start.time().minute(), start.time().second());
	itsTimeStep = ui.lineEditTimeStep->text();
	itsTaskState = taskStatusFromString(ui.comboBoxNewStatus->currentText().toStdString());
	done (ui.spinBoxNrCopies->value());
}

void TaskCopyDialog::cancelClicked(void) {
	done(0);
}

void TaskCopyDialog::nrCopiesChanged(int nrCopies) {
	if (nrCopies == 1) {
		ui.lineEditTimeStep->setEnabled(false);
		ui.labelTimeStep->setEnabled(false);
	}
	else {
		ui.lineEditTimeStep->setEnabled(true);
		ui.labelTimeStep->setEnabled(true);
	}
}

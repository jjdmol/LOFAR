#include "sasuploaddialog.h"
#include "Controller.h"
#include <QMessageBox>

SASUploadDialog::SASUploadDialog(QWidget *parent, Controller *controller)
    : QDialog(parent), itsController(controller), nrOfNewSchedulerTasks(0), nrOfDeletedSchedulerTasks(0), nrOfNewSASTasks(0),
    nrOfDeletedSASTasks(0), nrOfChangedTasks(0), nrOfUnchangedTasks(0)
{
	ui.setupUi(this);
	setupUploadDialog();
	ui.tableWidget_NewAndDeletedScheduleTasks->setSortingEnabled(true);
	ui.tableWidget_NewAndDeletedSASTasks->setSortingEnabled(true);
	ui.tableWidget_ChangedTasks->setSortingEnabled(true);
	ui.tableWidget_UnchangedTasks->setSortingEnabled(true);
	// set default size of columns
	ui.tableWidget_NewAndDeletedScheduleTasks->setColumnWidth(0,22); // +/-
	ui.tableWidget_NewAndDeletedScheduleTasks->setColumnWidth(1,40); // task id
	ui.tableWidget_NewAndDeletedScheduleTasks->setColumnWidth(2,50); // tree id
	ui.tableWidget_NewAndDeletedScheduleTasks->setColumnWidth(3,100); // task name
	ui.tableWidget_NewAndDeletedScheduleTasks->setColumnWidth(4,100); // project name
	ui.tableWidget_NewAndDeletedScheduleTasks->setColumnWidth(5,80); // status
	ui.tableWidget_NewAndDeletedScheduleTasks->setColumnWidth(6,110); // scheduled start
	ui.tableWidget_NewAndDeletedScheduleTasks->setColumnWidth(7,110); // scheduled end

	ui.tableWidget_NewAndDeletedSASTasks->setColumnWidth(0,22); // +/-
	ui.tableWidget_NewAndDeletedSASTasks->setColumnWidth(1,40); // task id
	ui.tableWidget_NewAndDeletedSASTasks->setColumnWidth(2,50); // tree id
	ui.tableWidget_NewAndDeletedSASTasks->setColumnWidth(3,100); // task name
	ui.tableWidget_NewAndDeletedSASTasks->setColumnWidth(4,100); // project name
	ui.tableWidget_NewAndDeletedSASTasks->setColumnWidth(5,85); // status
	ui.tableWidget_NewAndDeletedSASTasks->setColumnWidth(6,110); // scheduled start
	ui.tableWidget_NewAndDeletedSASTasks->setColumnWidth(7,110); // scheduled end

	ui.tableWidget_ChangedTasks->setColumnWidth(0,40); // task id
	ui.tableWidget_ChangedTasks->setColumnWidth(1,50); // tree id
	ui.tableWidget_ChangedTasks->setColumnWidth(2,100); // task name
	ui.tableWidget_ChangedTasks->setColumnWidth(3,100); // project name
	ui.tableWidget_ChangedTasks->setColumnWidth(4,85); // status
	ui.tableWidget_ChangedTasks->setColumnWidth(5,300); // changes
	ui.tableWidget_ChangedTasks->horizontalHeader()->setStretchLastSection(true);

	ui.tableWidget_UnchangedTasks->setColumnWidth(0,40); // task id
	ui.tableWidget_UnchangedTasks->setColumnWidth(1,50); // tree id
	ui.tableWidget_UnchangedTasks->setColumnWidth(2,100); // task name
	ui.tableWidget_UnchangedTasks->setColumnWidth(3,100); // project name
	ui.tableWidget_UnchangedTasks->setColumnWidth(4,85); // status
	ui.tableWidget_UnchangedTasks->setColumnWidth(5,110); // scheduled start
	ui.tableWidget_UnchangedTasks->setColumnWidth(6,110); // scheduled end

    if (itsController->autoPublishAllowed()) {
        ui.checkBoxAutoPublish->setEnabled(true);
        ui.checkBoxAutoPublish->setChecked(true);
        connect(ui.checkBoxAutoPublish,SIGNAL(clicked()),this,SLOT(setAutoPublish(void)));
    }
    else {
        ui.checkBoxAutoPublish->setEnabled(false);
        ui.checkBoxAutoPublish->setChecked(false);
        ui.checkBoxAutoPublish->setToolTip("Auto publishing is only allowed for user lofarsys");
    }
}

SASUploadDialog::~SASUploadDialog()
{

}

void SASUploadDialog::setAutoPublish(void) {
    itsController->setAutoPublish(ui.checkBoxAutoPublish->isChecked());
}

void SASUploadDialog::show(void) {
	updateSchedulerTasksLabel();
	updateSASTasksLabel();
	updateChangedTasksLabel();
	updateUnChangedTasksLabel();
	if ((nrOfChangedTasks == 0) && (nrOfDeletedSchedulerTasks == 0) && (nrOfNewSchedulerTasks == 0)) {
		showNormal();
		if (nrOfDeletedSASTasks == 0) {
			QMessageBox::information(0, QObject::tr("No schedule changes"), QObject::tr("There are no schedule changes that need uploading"));
			ui.pushButton_Cancel->hide();
			ui.pushButton_Commit->setText(QObject::tr("Close"));
			itsController->clearStatusText();
		}
		else {
			QMessageBox::warning(0, QObject::tr("Some tasks deleted"), QObject::tr("Some tasks were deleted externally.\nThey will also be deleted from the scheduler"));
			ui.pushButton_Cancel->show();
			ui.pushButton_Commit->setText(QObject::tr("Commit"));
		}
	}
	else {
		ui.pushButton_Cancel->show();
		ui.pushButton_Commit->setText(QObject::tr("Commit"));
		showNormal();
	}
}

void SASUploadDialog::cancelUpload(void) {
    if (ui.pushButton_Commit->text() == "Commit") {
        itsController->setStatusText("Upload to SAS canceled by user");
	}
	this->reject();
}

void SASUploadDialog::clear(void) {
	ui.tableWidget_NewAndDeletedScheduleTasks->clearContents();
	ui.tableWidget_NewAndDeletedScheduleTasks->setRowCount(0);
	ui.tableWidget_NewAndDeletedSASTasks->clearContents();
	ui.tableWidget_NewAndDeletedSASTasks->setRowCount(0);
	ui.tableWidget_ChangedTasks->clearContents();
	ui.tableWidget_ChangedTasks->setRowCount(0);
	ui.tableWidget_UnchangedTasks->clearContents();
	ui.tableWidget_UnchangedTasks->setRowCount(0);
	nrOfNewSchedulerTasks = 0;
	nrOfDeletedSchedulerTasks = 0;
	nrOfNewSASTasks = 0;
	nrOfDeletedSASTasks = 0;
	nrOfChangedTasks = 0;
	nrOfUnchangedTasks = 0;
	ui.label_SchedulerTasks->setText("Schedule (empty)");
	ui.label_SASTasks->setText("SAS database (empty)");
}

void SASUploadDialog::setupUploadDialog(void) {


	// add headers to the different tables
	QStringList labels1, labels2, labels3, labels4;

	// New and deleted schedule tasks table
	labels1 << "+/-" << "ID" << "tree ID" << QObject::tr("Task name") << QObject::tr("Project name") << QObject::tr("Status")
		<< QObject::tr("Start") << QObject::tr("End");
	ui.tableWidget_NewAndDeletedScheduleTasks->setColumnCount(8);
	ui.tableWidget_NewAndDeletedScheduleTasks->horizontalHeader()->setDefaultSectionSize(75);
	ui.tableWidget_NewAndDeletedScheduleTasks->setHorizontalHeaderLabels(labels1);
//	ui.tableWidget_NewAndDeletedScheduleTasks->setColumnWidth(0, 25);
	ui.tableWidget_NewAndDeletedScheduleTasks->verticalHeader()->setDefaultSectionSize(16);
	ui.tableWidget_NewAndDeletedScheduleTasks->setSelectionMode(QAbstractItemView::NoSelection);
	ui.tableWidget_NewAndDeletedScheduleTasks->setEditTriggers(QAbstractItemView::NoEditTriggers);


	// New and deleted SAS tasks table
	labels2 << "+/-" << "ID" << "tree ID" << QObject::tr("Task name") << QObject::tr("Project name") << QObject::tr("Status")
		<< QObject::tr("Start") << QObject::tr("End");
	ui.tableWidget_NewAndDeletedSASTasks->setColumnCount(8);
	ui.tableWidget_NewAndDeletedSASTasks->horizontalHeader()->setDefaultSectionSize(75);
	ui.tableWidget_NewAndDeletedSASTasks->setHorizontalHeaderLabels(labels2);
//	ui.tableWidget_NewAndDeletedSASTasks->setColumnWidth(0, 25);
	ui.tableWidget_NewAndDeletedSASTasks->verticalHeader()->setDefaultSectionSize(16);

	// Changed tasks table
	labels3 << "ID" << "tree ID" << QObject::tr("Task name") << QObject::tr("Project name") << QObject::tr("Status")
		<< QObject::tr("Changes");
	ui.tableWidget_ChangedTasks->setColumnCount(6);
	ui.tableWidget_ChangedTasks->horizontalHeader()->setDefaultSectionSize(75);
	ui.tableWidget_ChangedTasks->setHorizontalHeaderLabels(labels3);
//	ui.tableWidget_ChangedTasks->setColumnWidth(3, 200);
	ui.tableWidget_ChangedTasks->verticalHeader()->setDefaultSectionSize(16);

	// Unchanged tasks table
	labels4 << "ID" << "tree ID" << QObject::tr("Task name") << QObject::tr("Project name") << QObject::tr("Status")
		<< QObject::tr("Start") << QObject::tr("End");
	ui.tableWidget_UnchangedTasks->setColumnCount(7);
	ui.tableWidget_UnchangedTasks->horizontalHeader()->setDefaultSectionSize(75);
	ui.tableWidget_UnchangedTasks->setHorizontalHeaderLabels(labels4);
	ui.tableWidget_UnchangedTasks->verticalHeader()->setDefaultSectionSize(16);

	connect(ui.pushButton_Commit, SIGNAL(clicked()), this, SLOT(commitScheduleToSAS()));
	connect(ui.pushButton_Cancel, SIGNAL(clicked()), this, SLOT(cancelUpload()));
	//	tableWidget_NewAndDeletedScheduleTasks
//	tableWidget_NewAndDeletedSASTasks
//	tableWidget_ChangedTasks
//	tableWidget_UnchangedTasks
}

void SASUploadDialog::commitScheduleToSAS(void) {
	if ((nrOfChangedTasks == 0) && (nrOfDeletedSchedulerTasks == 0) && (nrOfNewSchedulerTasks == 0) && (nrOfDeletedSASTasks == 0)) {
		this->hide();
	} else {
		itsController->commitScheduleToSAS();
	}
}

void SASUploadDialog::addNewSchedulerTask(const Task &task) {
	int id = task.getID();
	int row = ui.tableWidget_NewAndDeletedScheduleTasks->rowCount();
	++nrOfNewSchedulerTasks;
//	updateSchedulerTasksLabel();
	ui.tableWidget_NewAndDeletedScheduleTasks->insertRow(row);
	QTableWidgetItem *item = new QTableWidgetItem("+");
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedScheduleTasks->setItem(row, 0, item);
	item = new QTableWidgetItem();
	item->setData(0,id); // we have to use setData to get correct number sorting for this column
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedScheduleTasks->setItem(row, 1, item);
	item = new QTableWidgetItem(task.getTaskName());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedScheduleTasks->setItem(row, 3, item); // column 2 is skipped (treeID not set for new tasks)
	item = new QTableWidgetItem(task.getProjectName());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedScheduleTasks->setItem(row, 4, item);
	item = new QTableWidgetItem(task.getStatusStr());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedScheduleTasks->setItem(row, 5, item);
	item = new QTableWidgetItem(task.getScheduledStart().toString().c_str());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedScheduleTasks->setItem(row, 6, item);
	item = new QTableWidgetItem(task.getScheduledEnd().toString().c_str());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedScheduleTasks->setItem(row, 7, item);
}

void SASUploadDialog::addDeletedSchedulerTask(const Task *task) {
    unsigned id = task->getID();
	int row = ui.tableWidget_NewAndDeletedScheduleTasks->rowCount();
	++nrOfDeletedSchedulerTasks;
	ui.tableWidget_NewAndDeletedScheduleTasks->insertRow(row);
	QTableWidgetItem *item = new QTableWidgetItem("-");
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedScheduleTasks->setItem(row, 0, item);
	item = new QTableWidgetItem();
	item->setData(0,id); // we have to use setData to get correct number sorting for this column
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedScheduleTasks->setItem(row, 1, item);
	item = new QTableWidgetItem();
    item->setData(0,task->getSASTreeID());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedScheduleTasks->setItem(row, 2, item);
    item = new QTableWidgetItem(task->getTaskName());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedScheduleTasks->setItem(row, 3, item);
    item = new QTableWidgetItem(task->getProjectName());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedScheduleTasks->setItem(row, 4, item);
    item = new QTableWidgetItem(task->getStatusStr());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedScheduleTasks->setItem(row, 5, item);
    item = new QTableWidgetItem(task->getScheduledStart().toString().c_str());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedScheduleTasks->setItem(row, 6, item);
    item = new QTableWidgetItem(task->getScheduledEnd().toString().c_str());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedScheduleTasks->setItem(row, 7, item);
}

void SASUploadDialog::addNewSASTask(const Task *task) {
    int id = task->getID();
	int row = ui.tableWidget_NewAndDeletedSASTasks->rowCount();
	++nrOfNewSASTasks;
	ui.tableWidget_NewAndDeletedSASTasks->insertRow(row);
	QTableWidgetItem *item = new QTableWidgetItem("+");
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedSASTasks->setItem(row, 0, item);
	item = new QTableWidgetItem();
	item->setData(0,id); // we have to use setData to get correct number sorting for this column
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedSASTasks->setItem(row, 1, item);
	item = new QTableWidgetItem();
    item->setData(0,task->getSASTreeID());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedSASTasks->setItem(row, 2, item);
    item = new QTableWidgetItem(task->getTaskName());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedSASTasks->setItem(row, 3, item);
    item = new QTableWidgetItem(task->getProjectName());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedSASTasks->setItem(row, 4, item);
    item = new QTableWidgetItem(task->getStatusStr());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedSASTasks->setItem(row, 5, item);
    item = new QTableWidgetItem(task->getScheduledStart().toString().c_str());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedSASTasks->setItem(row, 6, item);
    item = new QTableWidgetItem(task->getScheduledEnd().toString().c_str());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedSASTasks->setItem(row, 7, item);
}

void SASUploadDialog::addDeletedSASTask(const Task *task) {
    int id = task->getID();
	int row = ui.tableWidget_NewAndDeletedSASTasks->rowCount();
	++nrOfDeletedSASTasks;
//	updateSASTasksLabel();
	ui.tableWidget_NewAndDeletedSASTasks->insertRow(row);
	QTableWidgetItem *item = new QTableWidgetItem("-");
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedSASTasks->setItem(row, 0, item);
	item = new QTableWidgetItem();
	item->setData(0,id); // we have to use setData to get correct number sorting for this column
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedSASTasks->setItem(row, 1, item);
	item = new QTableWidgetItem();
    item->setData(0,task->getSASTreeID());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedSASTasks->setItem(row, 2, item);
    item = new QTableWidgetItem(task->getTaskName());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedSASTasks->setItem(row, 3, item);
    item = new QTableWidgetItem(task->getProjectName());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedSASTasks->setItem(row, 4, item);
    item = new QTableWidgetItem(task->getStatusStr());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedSASTasks->setItem(row, 5, item);
    item = new QTableWidgetItem(task->getScheduledStart().toString().c_str());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedSASTasks->setItem(row, 6, item);
    item = new QTableWidgetItem(task->getScheduledEnd().toString().c_str());
	item->setData(100, id);
	ui.tableWidget_NewAndDeletedSASTasks->setItem(row, 7, item);
}

void SASUploadDialog::addChangedTask(const Task &task, const QString &diffStr, bool conflict) {
	int id = task.getID();
	int row = ui.tableWidget_ChangedTasks->rowCount();
	++nrOfChangedTasks;
	ui.tableWidget_ChangedTasks->insertRow(row);

	QColor textColor;

	QTableWidgetItem *item = new QTableWidgetItem();

	textColor = conflict ? Qt::red : Qt::black;
	item->setData(0,id); // we have to use setData to get correct number sorting for this column
	item->setData(100, id);
	item->setTextColor(textColor);
	ui.tableWidget_ChangedTasks->setItem(row, 0, item);
	item = new QTableWidgetItem(QString::number(task.getSASTreeID()));
	item->setData(100, id);
	item->setTextColor(textColor);
	ui.tableWidget_ChangedTasks->setItem(row, 1, item);
	item = new QTableWidgetItem(task.getTaskName());
	item->setData(100, id);
	item->setTextColor(textColor);
	ui.tableWidget_ChangedTasks->setItem(row, 2, item);
	item = new QTableWidgetItem(task.getProjectName());
	item->setData(100, id);
	item->setTextColor(textColor);
	ui.tableWidget_ChangedTasks->setItem(row, 3, item);
	item = new QTableWidgetItem(task.getStatusStr());
	item->setData(100, id);
	item->setTextColor(textColor);
	ui.tableWidget_ChangedTasks->setItem(row, 4, item);
    item = new QTableWidgetItem(diffStr);
    item->setToolTip(QString(diffStr).replace(",","\n"));
	item->setData(100, id);
	item->setTextColor(textColor);
	ui.tableWidget_ChangedTasks->setItem(row, 5, item);
}

void SASUploadDialog::addUnchangedTask(const Task &task) {
	int id = task.getID();
	int row = ui.tableWidget_UnchangedTasks->rowCount();
	++nrOfUnchangedTasks;
//	updateUnChangedTasksLabel();
	ui.tableWidget_UnchangedTasks->insertRow(row);
	QTableWidgetItem *item = new QTableWidgetItem();
	item->setData(0,id); // we have to use setData to get correct number sorting for this column
	item->setData(100, id);
	ui.tableWidget_UnchangedTasks->setItem(row, 0, item);
	item = new QTableWidgetItem();
	item->setData(0,task.getSASTreeID());
	item->setData(100, id);
	ui.tableWidget_UnchangedTasks->setItem(row, 1, item);
	item = new QTableWidgetItem(task.getTaskName());
	item->setData(100, id);
	ui.tableWidget_UnchangedTasks->setItem(row, 2, item);
	item = new QTableWidgetItem(task.getProjectName());
	item->setData(100, id);
	ui.tableWidget_UnchangedTasks->setItem(row, 3, item);
	item = new QTableWidgetItem(task.getStatusStr());
	item->setData(100, id);
	ui.tableWidget_UnchangedTasks->setItem(row, 4, item);
	item = new QTableWidgetItem(task.getScheduledStart().toString().c_str());
	item->setData(100, id);
	ui.tableWidget_UnchangedTasks->setItem(row, 5, item);
	item = new QTableWidgetItem(task.getScheduledEnd().toString().c_str());
	item->setData(100, id);
	ui.tableWidget_UnchangedTasks->setItem(row, 6, item);
}

void SASUploadDialog::updateSchedulerTasksLabel(void) {
	QString str("Schedule (+");
	str += QString::number(nrOfNewSchedulerTasks);
	str += "/-";
	str += QString::number(nrOfDeletedSchedulerTasks);
	str += ")";
	ui.label_SchedulerTasks->setText(str);
}

void SASUploadDialog::updateSASTasksLabel(void) {
	QString str("SAS database (+");
	str += QString::number(nrOfNewSASTasks);
	str += "/-";
	str += QString::number(nrOfDeletedSASTasks);
	str += ")";
	ui.label_SASTasks->setText(str);
}

void SASUploadDialog::updateChangedTasksLabel(void) {
	QString str("Changed tasks (");
	if (nrOfChangedTasks) {
		str += QString::number(nrOfChangedTasks);
	}
	else {
		str += "No changed tasks";
	}
	str += ")";
	ui.groupBox_ChangedTasks->setTitle(str);
}

void SASUploadDialog::updateUnChangedTasksLabel(void) {
	QString str("Unchanged tasks (");
	if (nrOfUnchangedTasks) {
		str += QString::number(nrOfUnchangedTasks);
	}
	else {
		str += "No unchanged tasks";
	}
	str += ")";
	ui.groupBox_UnchangedTasks->setTitle(str);
}

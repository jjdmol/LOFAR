#ifndef SASUPLOADDIALOG_H
#define SASUPLOADDIALOG_H

#include <QDialog>
#include "ui_sasuploaddialog.h"
#include "task.h"

class Controller;

class SASUploadDialog : public QDialog
{
    Q_OBJECT

public:
    SASUploadDialog(QWidget *parent = 0, Controller *controller = 0);
    ~SASUploadDialog();

    void clear(void);
    void show(void);

    void addNewSchedulerTask(const Task &task);
    void addDeletedSASTask(const Task *task);
    void addDeletedSchedulerTask(const Task *task);
    void addNewSASTask(const Task *task);
    void addChangedTask(const Task &task, const QString &diffStr, bool conflict = false);
    void addUnchangedTask(const Task &task);
    void setAutoPublishEnabled(bool enable) {ui.checkBoxAutoPublish->setChecked(enable);}
    bool autoPublish(void) const {return ui.checkBoxAutoPublish->isChecked();}

private:
	void setupUploadDialog(void);
	void updateSchedulerTasksLabel(void);
	void updateSASTasksLabel(void);
	void updateChangedTasksLabel(void);
	void updateUnChangedTasksLabel(void);

signals:
	void SASUploadCanceled(void) const;

private slots:
	void cancelUpload(void);
	void commitScheduleToSAS(void);
    void setAutoPublish(void);

private:
    Ui::SASUploadDialogClass ui;
    Controller *itsController;
    unsigned nrOfNewSchedulerTasks, nrOfDeletedSchedulerTasks, nrOfNewSASTasks,
    nrOfDeletedSASTasks, nrOfChangedTasks, nrOfUnchangedTasks;
};

#endif // SASUPLOADDIALOG_H

#ifndef SASPROGRESSDIALOG_H
#define SASPROGRESSDIALOG_H

#include <QtGui/QDialog>
#include "ui_sasprogressdialog.h"

class SASProgressDialog : public QDialog
{
    Q_OBJECT

public:
    SASProgressDialog(QWidget *parent = 0);
    ~SASProgressDialog();

	void addText(const QString &text); // force an immediate redraw
    void addError(const QString &text);
    void setProgressPercentage(int value) {ui.progressBar->setValue(value);}
    void clear(void) {ui.listWidget_Progress->clear();ui.progressBar->setValue(0); disableClose();}
    void disableClose(void) {ui.pushButton_Close->setText("Please wait"); ui.pushButton_Close->setEnabled(false);}
    void enableClose(void) {ui.pushButton_Close->setText("Close"); ui.pushButton_Close->setEnabled(true);}

private:
    Ui::SASProgressDialogClass ui;
};

#endif // SASPROGRESSDIALOG_H

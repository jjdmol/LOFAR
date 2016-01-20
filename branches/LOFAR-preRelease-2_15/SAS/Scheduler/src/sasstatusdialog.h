#ifndef SASSTATUSDIALOG_H
#define SASSTATUSDIALOG_H

#include <QtGui/QDialog>
#include "ui_sasstatusdialog.h"

class SASStatusDialog : public QDialog
{
    Q_OBJECT

public:
    SASStatusDialog(QWidget *parent = 0);
    ~SASStatusDialog();

    void addText(const QString &text) {
    	QListWidgetItem *item = new QListWidgetItem(text, ui.listWidget_Status);
    	ui.listWidget_Status->addItem(item);
    	ui.listWidget_Status->scrollToItem(item);
    }

    void addError(const QString &text) {
    	QListWidgetItem *item = new QListWidgetItem(text);
    	item->setForeground(QBrush(Qt::red));
    	ui.listWidget_Status->addItem(item);
    	ui.listWidget_Status->scrollToItem(item);
    }

    void clear(void) {ui.listWidget_Status->clear();}

private:
    Ui::SASStatusDialogClass ui;
};

#endif // SASSTATUSDIALOG_H

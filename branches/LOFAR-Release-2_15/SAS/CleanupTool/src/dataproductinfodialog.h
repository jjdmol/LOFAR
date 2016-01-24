#ifndef DATAPRODUCTINFODIALOG_H
#define DATAPRODUCTINFODIALOG_H

#include <QtGui/QWidget>
#include "ui_dataproductinfodialog.h"

class DataProductInfoDialog : public QWidget
{
    Q_OBJECT

public:
    DataProductInfoDialog(QWidget *parent = 0);
    ~DataProductInfoDialog();

    friend class CEPCleanMainWindow;

private slots:
    void checkBoxDeletedReset(void);
    void checkBoxExpiredReset(void);

private:
    Ui::DataProductInfoDialogClass ui;
};

#endif // DATAPRODUCTINFODIALOG_H

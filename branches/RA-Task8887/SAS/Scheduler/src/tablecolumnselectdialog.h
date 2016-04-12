#ifndef TABLECOLUMNSELECTDIALOG_H
#define TABLECOLUMNSELECTDIALOG_H

#include <QDialog>
#include "ui_tablecolumnselectdialog.h"

class tableColumnSelectDialog : public QDialog
{
    Q_OBJECT

public:
    tableColumnSelectDialog(QWidget *parent = 0);
    ~tableColumnSelectDialog();

    void show(void);

private:
	void initDialog(void);

private:
	void accept(void);

private slots:
	void toggleSelection(void);

signals:
	void viewColumns(const std::vector<unsigned int> &) const;

private:
    Ui::tableColumnSelectDialogClass ui;
    std::vector<unsigned int> itsSelectedColumns;
    bool selectAll;
};

#endif // TABLECOLUMNSELECTDIALOG_H

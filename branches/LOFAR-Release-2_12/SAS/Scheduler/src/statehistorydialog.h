#ifndef STATEHISTORYDIALOG_H
#define STATEHISTORYDIALOG_H

#include <QtGui/QDialog>
#include "ui_statehistorydialog.h"
#include <QDateTime>

class StateHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    StateHistoryDialog(QWidget *parent = 0);
    ~StateHistoryDialog();

    void addStateInfo(const QString &treeID, const QString &momID,
    		const QString &state, const QString &username, const QDateTime &modtime);
    void clear(void) {ui.tableWidgetStateInfo->clearContents(); ui.tableWidgetStateInfo->setRowCount(0);}

private:
    Ui::StateHistoryDialogClass ui;
};

#endif // STATEHISTORYDIALOG_H

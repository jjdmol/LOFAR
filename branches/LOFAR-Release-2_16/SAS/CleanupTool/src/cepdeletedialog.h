/*
 * cepdeletedialog.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : Jan 5, 2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/cepdeletedialog.h $
 *
 */

#ifndef CEPDELETEDIALOG_H
#define CEPDELETEDIALOG_H

#include <QtGui/QDialog>
#include "ui_cepdeletedialog.h"
#include "lofar_scheduler.h"
#include "cepcleanmainwindow.h"

class Controller;
class QProcess;

class CEPdeleteDialog : public QDialog
{
    Q_OBJECT

    friend class CEPCleanMainWindow;

public:
    CEPdeleteDialog(QWidget *parent);
    ~CEPdeleteDialog();

    void addText(const QString &text) {ui.textEditDelete->append(text);}

    void setNodesCommandsInfo(const std::map<QString, QString> &nci);
    void setVICtreesToDelete(const deleteVICmap &vics);
    void addTask(const QStringList &);
    inline void addMarkedDeleted(const QString &dbName, int sasID, dataProductTypes dpType) {
    	itsTreesToMark[dbName].push_back(std::pair<int, dataProductTypes>(sasID, dpType));
    }

private:
    enum runState {
    	NOT_STARTED,
    	RUNNING,
    	RETRYING,
    	FAILED,
    	SUCCESS
    };

    void cleanFinished(void);
    bool isVICmarkedForDeletion(int treeID) const;
    bool deleteVicTrees(void);
    void retryDelete(void);

private slots:
    void deleteConfirmed(void);
    void cancelClicked(void);
    void okClicked(void);
    void nodeCleanFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    Ui::CEPdeleteDialogClass ui;
    int itsRow, itsExitCode, itsRetryCount;
    QProcess *fp;
    QString itsConnectCmd;
    QString itsCEP4ConnectCmd;

    runState itsState;
    std::map<QString, QString> itsNodeCommands;
    std::map<QString, QString>::const_iterator itsNodeit;
    std::map<QString, std::pair<runState, QProcess *> > itsCleanProcesses;
    deletedDataMap itsTreesToMark;
    deleteVICmap itsVICtreesToDelete;
    CEPCleanMainWindow *itsParentCleanupDialog;
};

#endif // CEPDELETEDIALOG_H

/*
 * schedulesettingsdialog.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11825 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-09-25 11:35:16 +0000 (Thu, 25 Sep 2014) $
 * First creation : 5-march-2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/schedulesettingsdialog.h $
 *
 */

#ifndef SCHEDULESETTINGSDIALOG_H
#define SCHEDULESETTINGSDIALOG_H

#include <QtGui/QDialog>
#include "ui_schedulesettingsdialog.h"
#include "lofar_scheduler.h"
#include "DataMonitorConnection.h"
class QString;
class Controller;

class ScheduleSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    ScheduleSettingsDialog(Controller *controller);
    ~ScheduleSettingsDialog();

    bool getLoadDefaultSettingsOnStartUp(void) const {return itsLoadDefaultSettings;}
	const QString &getSASDatabase(void) const {return itsSASDatabase;}
	const QString &getSASHostName(void) const {return itsSASHostName;}
	const QString &getSASUserName(void) const {return itsSASUserName;}
	const QString &getSASPassword(void) const {return itsSASPassword;}
	const QString &getDMDatabase(void) const {return itsDMDatabase;}
	const QString &getDMHostName(void) const {return itsDMHostName;}
	const QString &getDMUserName(void) const {return itsDMUserName;}
	const QString &getDMPassword(void) const {return itsDMPassword;}
    const QMap<QString, QPair<bool, QString> > &getExcludeStrings(void) const {return itsExcludeStrings;}

	//update the storage nodes capacity info box
	void updateStorageNodeInfoTree(const storageHostsMap &nodes, const statesMap &states,	const hostPartitionsMap &hostPartitions);
	void updatePreferredStorageLists(const storageHostsMap &nodes);

	void setLoadDefaultSettingsOnStartUp(bool loadDefault) {itsLoadDefaultSettings = loadDefault;}
	void setSASUserName(const std::string &SAS_username) {itsSASUserName = SAS_username.c_str();}
	void setSASPassword(const std::string &SAS_password) {itsSASPassword = SAS_password.c_str();}
	void setSASDatabase(const std::string &SAS_database) {itsSASDatabase = SAS_database.c_str();}
	void setSASHostName(const std::string &SAS_hostname) {itsSASHostName = SAS_hostname.c_str();}
	void setDMUserName(const std::string &DM_username) {itsDMUserName = DM_username.c_str();}
	void setDMPassword(const std::string &DM_password) {itsDMPassword = DM_password.c_str();}
	void setDMDatabase(const std::string &DM_database) {itsDMDatabase = DM_database.c_str();}
	void setDMHostName(const std::string &DM_hostname) {itsDMHostName = DM_hostname.c_str();}
    void setExcludeStrings(const QMap<QString, QPair<bool, QString> > &strings) {itsExcludeStrings = strings;}

	void stopStorageWaitCursor(void) {
		ui.pb_RefreshStorageNodesInfo->setEnabled(true);
		QApplication::restoreOverrideCursor();
	}

private:
    void keyPressEvent(QKeyEvent *event);
	void createActions(void);
	bool checkSASsettings(void);
	int checkSASconnection(void);
	void updateSASConnectionSettings(void); // gets the sas connection settings from the dialog

signals:
	void actionSaveSettings(void) const;

public slots:
	void show(void);

private slots:
	void okClicked(void);
	void cancelClicked(void);
	int testSASconnection(bool quietWhenOk = false);
	void doRefreshStorageNodesInfo(void);
    void checkExcludeString(QTreeWidgetItem *, int);

private:
	Ui::ScheduleSettingsDialogClass ui;
	Controller *itsController;
    bool itsLoadDefaultSettings;
	QString itsSASDatabase, itsSASHostName, itsSASUserName, itsSASPassword;
	QString itsDMDatabase, itsDMHostName, itsDMUserName, itsDMPassword;
    QMap<QString, QPair<bool, QString> > itsExcludeStrings;
};

#endif // SCHEDULESETTINGSDIALOG_H

/*
 * SASConnectDialog.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11424 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 13:31:31 +0000 (Mon, 06 Jan 2014) $
 * First creation : Aug 23, 2012
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/sasconnectdialog.h $
 *
 */

#ifndef SASCONNECTDIALOG_H
#define SASCONNECTDIALOG_H

#include <QtGui/QDialog>
#include "ui_sasconnectdialog.h"
#include <QString>
#include <QStringList>

class SASConnectDialog : public QDialog
{
    Q_OBJECT

public:
    SASConnectDialog(QWidget *parent, const QString &username = "paulus", const QString &password = "boskabouter", const QStringList &DBName = QStringList(QString("LOFAR_1;LOFAR_2;LOFAR_3;LOFAR_4")), const QString &hostname = "sas003.control.lofar");
    ~SASConnectDialog();
    void setData(const QString &username, const QString &password, const QStringList &DBName, const QString &hostname);
	const QString & getUserName(void) {return itsUserName;}
	const QString & getPassword(void) {return itsPassword;}
	const QStringList & getDBName(void) {return itsDBName;}
	const QString & getHostName(void) {return itsHostName;}
    void setUserName(const QString &user_name) {itsUserName = user_name; ui.lineEditUserName->setText(user_name);}
    void setHostName(const QString &host_name) {itsHostName = host_name; ui.lineEditHostName->setText(host_name);}
    void setDatabase(const QStringList &database_names) {itsDBName = database_names; ui.lineEditDatabaseName->setText(database_names.join(QChar(';')));}
    void setPassword(const QString &password) {itsPassword = password; ui.lineEditPassword->setText(password);}

private slots:
	void accept(void);

private:
    Ui::SASConnectDialogClass ui;
    QStringList itsDBName;
    QString itsHostName, itsUserName, itsPassword;
};

#endif // SASCONNECTDIALOG_H

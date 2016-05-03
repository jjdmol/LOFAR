/*
 * SASConnectDialog.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Aug 23, 2012
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/sasconnectdialog.h $
 *
 */

#ifndef SASCONNECTDIALOG_H
#define SASCONNECTDIALOG_H

#include <QDialog>
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

private slots:
	void accept(void);

private:
    Ui::SASConnectDialogClass ui;
    QStringList itsDBName;
    QString itsHostName, itsUserName, itsPassword;
};

#endif // SASCONNECTDIALOG_H

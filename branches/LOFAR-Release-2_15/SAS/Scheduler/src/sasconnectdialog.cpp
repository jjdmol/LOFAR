/*
 * sasconnectdialog.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Aug 23, 2012
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/sasconnectdialog.cpp $
 *
 */

#include "sasconnectdialog.h"

SASConnectDialog::SASConnectDialog(QWidget *parent, const QString &username, const QString &password, const QStringList &DBName, const QString &hostname)
    : QDialog(parent), itsDBName(DBName), itsHostName(hostname), itsUserName(username), itsPassword(password)
{
	ui.setupUi(this);
	ui.lineEditHostName->setText(hostname);
	ui.lineEditDatabaseName->setText(DBName.join(";"));
	ui.lineEditUserName->setText(username);
	ui.lineEditPassword->setText(password);
	connect(ui.pushButtonOk, SIGNAL(clicked()), this, SLOT(accept(void)));
	connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject(void)));
}

SASConnectDialog::~SASConnectDialog()
{

}

void SASConnectDialog::setData(const QString &username, const QString &password, const QStringList &DBName, const QString &hostname) {
	itsUserName = username;
	itsHostName = hostname;
	itsDBName = DBName;
	itsPassword = password;
}

void SASConnectDialog::accept(void) {
	itsUserName = ui.lineEditUserName->text();
	itsPassword = ui.lineEditPassword->text();
	itsHostName = ui.lineEditHostName->text();
	itsDBName = ui.lineEditDatabaseName->text().split(";");

	done(1);
}

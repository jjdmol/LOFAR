//# Copyright (C) 2012-2014  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$
#include <lofar_config.h>

#include <iostream>
#include <QtCore/QVariant>
#include <QSqlError>
#include "sasdatabaseconnection.h"



using namespace std;

SASDatabaseConnection::SASDatabaseConnection(const QString &aUsername,
                   const QString &aPassword,
                   const QString &aHostname,
                   const QString &aDBName,

                   const QString &aDBId,
                   const QString &aDBType,
                   const QString &postgresUsername,
                   const QString &postgresPassword)
    :
      itsSASUserName(aUsername),
      itsSASPassword(aPassword),
      itsHostname(aHostname),
      itsDBName(aDBName),
      itsDBId(aDBId),
      itsDBType(aDBType),
      itsPostgresUsername(postgresUsername),
      itsPostgresPassword(postgresPassword)
{

    QSqlDatabase tempDB = QSqlDatabase::addDatabase(itsDBType, itsDBId);


    tempDB.setHostName(itsHostname);
    tempDB.setDatabaseName(itsDBName);
    tempDB.setUserName(itsPostgresUsername);
    tempDB.setPassword(itsPostgresPassword);

    sasDB = new QSqlDatabase(QSqlDatabase::database(itsDBId));
    testAuthentication();
}

SASDatabaseConnection::~SASDatabaseConnection()
{
    disconnect();
}

int SASDatabaseConnection::testAuthentication()
{
    if (!sasDB->open())
        return -1; // could not connect to SAS database

    // Call helper function on query containing class
    QSqlQuery query = sasQueries.doOTDBlogin(*sasDB, itsSASUserName, itsSASPassword);

    // If query returned any feedback
    if (!query.next())
        return -3;

    itsAuthToken = query.value(0).toString();

    // check authentication token (should not be zero)
    if (itsAuthToken.isEmpty())
        return -2; // no write permissions to SAS DB

    return 0;
}

void SASDatabaseConnection::disconnect()
{
    sasDB->close();
    delete sasDB; // Forcefully destruct the connection
    QSqlDatabase::removeDatabase(itsDBId);
}

QString SASDatabaseConnection::lastError()
{
    return sasDB->lastError().text();
}

QSqlQuery SASDatabaseConnection::treeidFROMgettreelist(
         QString tree)
{
    return sasQueries.treeidFROMgettreelist(*sasDB, tree);
}

QSqlQuery SASDatabaseConnection::now()
{
    return sasQueries.now(*sasDB);
}

QSqlQuery SASDatabaseConnection::getTreesInPeriod(
        QString start_date, QString end_date, int treetype)
{
    return sasQueries.getTreesInPeriod(*sasDB,
                       start_date, end_date, treetype);
}

QSqlQuery SASDatabaseConnection::limitsFromGetVHitemList(QString vicTreeId)
{
    return sasQueries.limitsFromGetVHitemList(*sasDB,
                       vicTreeId);
}


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
    QSqlQuery query = doOTDBlogin(itsSASUserName, itsSASPassword);

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
    delete sasDB; // Forcefully destruct the connection
    sasDB = 0;
    QSqlDatabase::removeDatabase(itsDBId);
}

QString SASDatabaseConnection::lastError()
{
    return sasDB->lastError().text();
}

QSqlQuery SASDatabaseConnection::doOTDBlogin(QString sasUserName,
                                     QString sasPassword)
{
    std::vector<QString> argumentList;
    argumentList.push_back(sasUserName);
    argumentList.push_back(sasPassword);
    return  sasQueries.doQuery(*sasDB, "otdblogin", argumentList);
}

QSqlQuery SASDatabaseConnection::treeidFROMgettreelist(
         QString tree)
{
    std::vector<QString> argumentList;
    argumentList.push_back(tree);
    return sasQueries.doQuery(*sasDB, "gettreelist",  argumentList);
}

QSqlQuery SASDatabaseConnection::now()
{
    std::vector<QString> argumentList;
    return sasQueries.doQuery(*sasDB, "now",  argumentList);
}

QSqlQuery SASDatabaseConnection::getTreesInPeriod(
        QString start_date, QString end_date, int treetype)
{
    std::vector<QString> argumentList;
    argumentList.push_back(QString::number(treetype));
    argumentList.push_back(start_date);
    argumentList.push_back(end_date);

    return sasQueries.doQuery(*sasDB, "getTreesInPeriod",  argumentList);
}

QSqlQuery SASDatabaseConnection::limitsFromGetVHitemList(QString vicTreeId)
{
    std::vector<QString> argumentList;
    argumentList.push_back(vicTreeId);
    return sasQueries.doQuery(*sasDB, "limitsfromgetVHitemList",  argumentList);
}


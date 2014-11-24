#ifndef SASDATABASECONNECTION_H
#define SASDATABASECONNECTION_H

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

#include "qsqlquery.h"
#include "qstring.h"
#include "qsqldatabase.h"
#include "sassqlqueries.h"

// Class collecting all querys used by SASConnection in a single object
class SASDatabaseConnection
{
public:
    // Create the db connection, resulting in an object with a valid state.
    SASDatabaseConnection(const QString &username,
                          const QString &password,
                          const QString &hostname,
                          const QString &DBName,
                          const QString &DBId  = "SASDB",
                          const QString &DBType = "QPSQL",
                          const QString &postgresUsername = "postgres",
                          const QString &postgresPassword = "");

    // Test the authentication of the current dbconnection
    // return 0 if no issues found.
    // return -1 if SAS database connection is not up
    // return -2 if OTDBlogin select statement returned with incorrect information
    // return -3 of the query failed
    int testAuthentication();

    // Close the internal database connection and remove the symbolic link
    // from the qt database container
    void disconnect();

    // Get the last error of the internal QSql database connection
    QString lastError();

    // Return the open status of the internal connection
    bool open(){return sasDB.open();}


    QString getAuthToken(){return itsAuthToken;}

    QSqlDatabase getSasDB(){return sasDB;}

private:
    QSqlDatabase sasDB;
    SASSqlQueries sasQueries;

    QString itsSASUserName;
    QString itsSASPassword;
    QString itsHostname;
    QString itsDBName;
    QString itsDBId;
    QString itsDBType;
    QString itsPostgresUsername;
    QString itsPostgresPassword;

    QString itsAuthToken;

};

#endif

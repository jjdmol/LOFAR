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
    sasDB = QSqlDatabase::addDatabase(itsDBType,itsDBId);
    sasDB.setHostName(itsHostname);
    sasDB.setDatabaseName(itsDBName);
    sasDB.setUserName(itsPostgresUsername);
    sasDB.setPassword(itsPostgresPassword);

    sasDB = QSqlDatabase::database(itsDBId);

}

int SASDatabaseConnection::testAuthentication()
{
    if (!sasDB.open())
        return -1; // could not connect to SAS database

    // I Have seen this functionality before (almost)
    QSqlQuery query(sasDB);
    query.exec("SELECT OTDBlogin('" + itsSASUserName
               + "','" + itsSASPassword + "')");

    if (!query.next())
        return -3;

    // check authentication token (should not be zero)
    if (query.value(0).toUInt() == 0)
            return -2; // no write permissions to SAS DB

    return 0;
}

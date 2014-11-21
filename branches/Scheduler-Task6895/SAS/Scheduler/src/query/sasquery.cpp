#include <lofar_config.h>

# include "sasquery.h"


SASQuery::SASQuery()/*const QString &username,
                   const QString &password,
                   const QString &DBName,
                   const QString &hostname)*/
{

    // DO nothing yet
}


/*
 *
 * int SASConnection::testConnect(const QString &username,
       const QString &password, const QString &DBname, const QString &hostname)
{
    disconnect();
    QSqlDatabase sasDB = QSqlDatabase::addDatabase("QPSQL","SASDB");
    // TODO: hardcode username and password
    sasDB.setUserName("postgres");
    sasDB.setPassword("");
    sasDB.setHostName(hostname);
    sasDB.setDatabaseName(DBname);

    if (!sasDB.open())
        return -1; // could not connect to SAS database

    // I Have seen this functionality before (almost)
    QSqlQuery query(sasDB);
    query.exec("SELECT OTDBlogin('" + username + "','" + password + "')");

    if (query.next())
    {
        if (query.value(0).toUInt() == 0)
        { // check authentication token (should not be zero)
            return -2; // no write permissions to SAS DB
        }
    }
    else
        return -3; // could not execute query on database

    query.finish(); //

    return 0;
}*/

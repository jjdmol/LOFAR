#include <lofar_config.h>

#include <QtTest/QtTest>

#include "../../../src/databaseconnections/sasdatabaseconnection.h"
#include <utility>
#include <vector>
#include <QString>
#include <iostream>

using namespace std;

class testsasdatabaseconnection: public QObject
{
        Q_OBJECT
private slots:
    void testConstructor();
    void testDoQuery();

};

void testsasdatabaseconnection::testConstructor()
{
    SASDatabaseConnection connection(QString("paulus"),
                                     QString("boskabouter"),
                                     QString("sas099.control.lofar"),
                                     QString("LOFAR-preRelease-2_5"));

    QCOMPARE(connection.testAuthentication(), 0);
}

void testsasdatabaseconnection::testDoQuery()
{
    SASDatabaseConnection connection(QString("paulus"),
                                     QString("boskabouter"),
                                     QString("sas099.control.lofar"),
                                     QString("LOFAR-preRelease-2_5"));

    QString queryId = "OTDBlogin";
    std::vector<QString> argumentList;
    argumentList.push_back("paulus");
    argumentList.push_back("boskabouter");


    QSqlQuery query =  connection.doQuery(queryId, argumentList);
    QCOMPARE(query.next(), true);
}


 
QTEST_MAIN(testsasdatabaseconnection)
#include "testsasdatabaseconnection.moc"


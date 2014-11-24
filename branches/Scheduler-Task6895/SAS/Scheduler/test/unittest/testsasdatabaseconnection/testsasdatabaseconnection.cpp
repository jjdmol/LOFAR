#include <lofar_config.h>

#include <QtTest/QtTest>

#include "../../../src/databaseconnections/sasdatabaseconnection.h"

class testsasdatabaseconnection: public QObject
{
        Q_OBJECT
private slots:
    void  testConstructor();
};
 
void testsasdatabaseconnection::testConstructor()
{


    SASDatabaseConnection connection(QString("paulus"),
                                     QString("boskabouter"),
                                     QString("sas099.control.lofar"),
                                     QString("LOFAR-preRelease-2_5"));
    QCOMPARE(connection.testAuthentication(), 0);

}
 
QTEST_MAIN(testsasdatabaseconnection)
#include "testsasdatabaseconnection.moc"


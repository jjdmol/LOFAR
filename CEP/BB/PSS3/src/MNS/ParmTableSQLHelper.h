#ifndef PARMTABLESQLHELPER_H
#define PARMTABLESQLHELPER_H

#include <Common/lofar_string.h>
#include <MNS/MeqDomain.h>
#include <MNS/MeqParmHolder.h>
#include <MNS/MeqPolc.h>

namespace LOFAR {

class ParmTableSQLHelper
{
public:
  // get query strings
  static   string getUpdateQuery(MeqParmHolder MPH, string tableName);
  static   string getInsertQuery(MeqParmHolder MPH, string tableName);
  static   string getDefInsertQuery(MeqParmHolder MPH, string tableName);
  static   string getGetPolcsQuery(const string& parmName, const MeqDomain& domain, string tableName);
  static   string getGetInitCoeffQuery(const string& parmName, string tableName);
  static   string getFindQuery(const string& parmName, const MeqDomain& domain, string tableName);
  static   string getSourcesQuery(string tableName);

  // read query results
  static   MeqPolc readMeqPolc(char** resRow);
  static   MeqPolc readDefMeqPolc(char** resRow);
  static   MeqParmHolder readMeqParmHolder(char** resRow);

private:
  static string quote (unsigned char* src, int length);
  static int unquote (unsigned char* src, char* dst, int length);
  static int seven2eightbits (unsigned char* dest, unsigned char* src, int length);
  static int eight2sevenbits (unsigned char* dest, unsigned char* src, int length);

  static   MeqMatrix getMeqMatrix(char** resRow, int column);
  static   double getDouble(char** resRow, int column);
  static   MeqDomain getDomain(char** resRow, int column);
  static   string getPolcNoDomainColumns();
  static   MeqPolc readPolcNoDomainQRes(char** resRow, int column);
  static   string getDomainColumns();
  static   MeqDomain readDomainQRes(char** resRow, int column);
  static   string getMeqParmNoPolcColumns();
  static   MeqParmHolder readMeqParmNoPolcQRes(char** resRow, int column);
  static   string MeqMat2string(const MeqMatrix &MM);
};

}

#endif

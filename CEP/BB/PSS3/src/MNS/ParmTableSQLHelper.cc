#include <Common/BlobIBufChar.h>
#include <Common/BlobOBufChar.h>
#include <Common/BlobIBufString.h>
#include <Common/BlobOBufString.h>
#include <Common/BlobIStream.h>
#include <Common/BlobOStream.h>
#include <Common/BlobString.h>
#include "ParmTableSQLHelper.h"
#include <Common/Debug.h>
#include <casa/BasicMath/Math.h>
#include <stdlib.h>
#include <sstream>
#include <Common/lofar_vector.h>


namespace LOFAR {

#define PRECISION 20

  string ParmTableSQLHelper::getGetPolcsQuery(const string& parmName, const MeqDomain& domain, string tableName)
  {
    string s = "SELECT " + getPolcNoDomainColumns() + ", " + getDomainColumns() + " FROM " + tableName + " WHERE name = '" + parmName +"'";
    return s;
  }

  string ParmTableSQLHelper::getGetInitCoeffQuery(const string& parmName, string tableName)
  {
    string s =  "SELECT " + getPolcNoDomainColumns () + 
      " FROM " + tableName + "def WHERE name = '" + parmName + "'";
    return s;
  }
  string ParmTableSQLHelper::getFindQuery(const string& parmName, const MeqDomain& domain, string tableName)
  {
    std::ostringstream s;
    s.precision(PRECISION);
    s << "SELECT " << getMeqParmNoPolcColumns() << ", " << getPolcNoDomainColumns() << ", " << getDomainColumns() <<
      " FROM " << tableName << " WHERE name = '" << parmName << "' AND tb < " << domain.endX() <<
      " AND te > " << domain.startX() << " AND fb < " << domain.endY() << " AND fe > " << domain.startY();
    return s.str();
  }
  string ParmTableSQLHelper::getSourcesQuery(string tableName)
  {
    std::ostringstream qs;
    qs << "SELECT name FROM " << tableName << " WHERE name LIKE 'RA.%' " <<
      "UNION SELECT name FROM " << tableName << "def WHERE name LIKE 'RA.%';";
    return qs.str();
  }

  MeqPolc ParmTableSQLHelper::readMeqPolc(char** resRow)
  {
    MeqPolc MP = readPolcNoDomainQRes(resRow, 0);
    MP.setDomain(readDomainQRes(resRow, 8));
    return MP;
  }

  MeqPolc ParmTableSQLHelper::readDefMeqPolc(char** resRow)
  {
    MeqPolc MP = readPolcNoDomainQRes( resRow, 0);
    return MP;
  }
  
  MeqParmHolder ParmTableSQLHelper::readMeqParmHolder(char** resRow)
  {
    MeqParmHolder MPH = readMeqParmNoPolcQRes(resRow, 0);
    MeqPolc MP = readPolcNoDomainQRes (resRow, 3);
    MP.setDomain( readDomainQRes (resRow, 11));
    MPH.setPolc( MP); 
 
    return MPH;
  }

  MeqMatrix ParmTableSQLHelper::getMeqMatrix(char** resRow, int column)
  {
    MeqMatrix MM;
    char str7[strlen(resRow[column])];
    int nrOfChars = unquote(str7, ( char*)resRow[column], strlen(resRow[column]));
    LOFAR::BlobIBufChar bb(str7, nrOfChars);
    LOFAR::BlobIStream bs(bb);
    bs >> MM;
    return MM;
  };
  double ParmTableSQLHelper::getDouble(char** resRow, int column)
  {
    return strtod(resRow[column], NULL);
  };
  MeqDomain ParmTableSQLHelper::getDomain(char** resRow, int column)
  {
    MeqDomain MD( getDouble(resRow, column), getDouble(resRow,column+1), getDouble(resRow,column+2), getDouble(resRow,column+3));
    return MD;
  };




  // This is what needs to be in the table:

  // MeqPolc:
  // itsCoeff (MM), itsSimCoeff(MM), itsPertSimCoeff(MM), domain (4doubles)
  // itsPertValue(double), itsIsRelPert(bool), itsX0(double), itsY0(double), normalized (bool)

  // MeqParmDefHolder:
  // MeqPolc (- domain) + name (string) , srcnr (int) , statnr (int)

  // MeqParmHolder:
  // MeqParmDefHolder (incl domain)

  // so the select query if we just need names is
  // SELECT name FROM ... WHERE ...
  // so the select query if we just need Polc
  // SELECT Coeff, SimCoeff, PertSimCoeff, tb, te, fb, fe, pertvalue, isrelpert, t0, f0, normalized  FROM ... WHERE ...
  // so the select query if we just need MeqParmDefHolder
  // SELECT Coeff, SimCoeff, PertSimCoeff, pertvalue, isrelpert, t0, f0, normalized, name, scrnr, statnr FROM ... WHERE ...
  // so the select query if we just need MeqParmHolder
  // SELECT Coeff, SimCoeff, PertSimCoeff, tb, te, fb, fe, pertvalue, isrelpert, t0, f0, normalized, name, scrnr, statnr FROM ... WHERE ...


  // The query strings are grouped with the query readers, because the table names are hard coded right now.
  // The way the columns should be interpreted is determined by the query.

  // update query
  // we need the following columns here
  // Coeff, SimCoeff, PertSimCoeff, pertvalue, isrelpert, t0, f0, normalized, tb, te, fb, fe, name, srcnr, statnr
  string ParmTableSQLHelper::getUpdateQuery(MeqParmHolder MPH, string tableName)
  {
    std::ostringstream qs;
    qs.precision(PRECISION);
    qs << "UPDATE " << tableName 
       << " SET Coeff = '" << MeqMat2string(MPH.getPolc().getCoeff())
       << "', SimCoeff = '" << MeqMat2string(MPH.getPolc().getSimCoeff())
       << "', PertSimCoeff = '" << MeqMat2string(MPH.getPolc().getPertSimCoeff())
       << "', pertvalue = " << MPH.getPolc().getPerturbation()
       << ", isrelpert = " << MPH.getPolc().isRelativePerturbation() 
       << ", t0 = " << MPH.getPolc().getX0() 
       << ", f0 = " << MPH.getPolc().getY0() 
       << ", normalized = " << MPH.getPolc().isNormalized()
       << ", tb = " << MPH.getPolc().domain().startX() 
       << ", te = " << MPH.getPolc().domain().endX()
       << ", fb = " << MPH.getPolc().domain().startY() 
       << ", fe = " << MPH.getPolc().domain().endY()
       << ", srcnr = " << MPH.getSourceNr() 
       << ", statnr = " << MPH.getStation()
       << " WHERE "
       << " name = '" << MPH.getName() 
       << "' AND tb < " << MPH.getPolc().domain().endX() 
       << " AND te > " << MPH.getPolc().domain().startX() 
       << " AND fb < " << MPH.getPolc().domain().endY() 
       << " AND fe > " << MPH.getPolc().domain().startY();
    string s=qs.str();
    return s;
  };

  // insert query
  // we need the following columns here
  // Coeff, SimCoeff, PertSimCoeff, pertvalue, isrelpert, t0, f0, normalized, tb, te, fb, fe, name, srcnr, statnr
  string ParmTableSQLHelper::getInsertQuery(MeqParmHolder MPH, string tableName)
  {
    std::ostringstream qs;
    qs.precision(PRECISION);
    qs << "INSERT INTO " << tableName << " ("
       << " Coeff, SimCoeff, PertSimCoeff, pertvalue, isrelpert, t0, f0, normalized, tb, te, fb, fe, name, srcnr, statnr"
       << ") VALUES ('"
       << MeqMat2string(MPH.getPolc().getCoeff()) << "', '"
       << MeqMat2string(MPH.getPolc().getSimCoeff()) << "', '"
       << MeqMat2string(MPH.getPolc().getPertSimCoeff()) << "', "
       << MPH.getPolc().getPerturbation() << ", "
       << MPH.getPolc().isRelativePerturbation()  << ", "
       << MPH.getPolc().getX0()  << ", "
       << MPH.getPolc().getY0()  << ", "
       << MPH.getPolc().isNormalized() << ", "
       << MPH.getPolc().domain().startX()  << ", "
       << MPH.getPolc().domain().endX() << ", "
       << MPH.getPolc().domain().startY()  << ", "
       << MPH.getPolc().domain().endY() << ", '"
       << MPH.getName()  << "', "
       << MPH.getSourceNr()  << ", "
       << MPH.getStation()
       << ")";
    string s=qs.str();
    //  cout<<"query: "<<s<<endl;
    return s;
  }

  // insert query
  // we need the following columns here
  // Coeff, SimCoeff, PertSimCoeff, pertvalue, isrelpert, t0, f0, normalized, tb, te, fb, fe, name, srcnr, statnr
  string ParmTableSQLHelper::getDefInsertQuery(MeqParmHolder MPH, string tableName)
  {
    std::ostringstream qs;
    qs.precision(PRECISION);
    qs << "INSERT INTO " << tableName << "def ("
       << " Coeff, SimCoeff, PertSimCoeff, pertvalue, isrelpert, t0, f0, normalized, name, srcnr, statnr"
       << ") VALUES ('"
       << MeqMat2string(MPH.getPolc().getCoeff()) << "', '"
       << MeqMat2string(MPH.getPolc().getSimCoeff()) << "', '"
       << MeqMat2string(MPH.getPolc().getPertSimCoeff()) << "', "
       << MPH.getPolc().getPerturbation() << ", "
       << MPH.getPolc().isRelativePerturbation()  << ", "
       << MPH.getPolc().getX0()  << ", "
       << MPH.getPolc().getY0()  << ", "
       << MPH.getPolc().isNormalized() << ", '"
       << MPH.getName()  << "', "
       << MPH.getSourceNr()  << ", "
       << MPH.getStation()
       << ")";
    string s=qs.str();
    return s;
  }

  // Query for reading polcs without domain
  string ParmTableSQLHelper::getPolcNoDomainColumns()
  {
    return "Coeff, SimCoeff, PertSimCoeff, pertvalue, isrelpert, t0, f0, normalized";
    //       0        1           2           3         4         5  6    7       
  };
  MeqPolc ParmTableSQLHelper::readPolcNoDomainQRes(char** resRow, int column)
  {
    MeqPolc MP;

    MP.setCoeff( getMeqMatrix (resRow, column++));
    MP.setSimCoeff( getMeqMatrix (resRow, column++));
    MP.setPertSimCoeff( getMeqMatrix (resRow, column++));
    MP.setPerturbation ( getDouble( resRow, column), (atoi(resRow[column+1]) == 1));
    column += 2;
    MP.setX0 ( getDouble ( resRow, column++) );
    MP.setY0 ( getDouble ( resRow, column++) );
    MP.setNormalize (atoi(resRow[column++]) == 1);

    return MP;
  };
  // Query for reading domain
  string ParmTableSQLHelper::getDomainColumns()
  {
    return "tb, te, fb, fe";
    //       0  1   2   3
  };
  MeqDomain ParmTableSQLHelper::readDomainQRes(char** resRow, int column)
  {
    return getDomain( resRow, column); // domain is 4 columns, so next column to be read is column column+4;
  };
  // Query for reading MeqParmDefHolder
  string ParmTableSQLHelper::getMeqParmNoPolcColumns()
  {
    return "name, srcnr, statnr";
    //       0      1      2
  };
  MeqParmHolder ParmTableSQLHelper::readMeqParmNoPolcQRes(char** resRow, int column)
  {
    MeqParmHolder MPH;
    MPH.setName(resRow[column]);
    MPH.setSourceNr (atoi(resRow[column++]));
    MPH.setStation (atoi(resRow[column++]));

    return MPH;
  };

  string ParmTableSQLHelper::MeqMat2string(const MeqMatrix &MM)
  {
    char str[512];
    BlobOBufChar bb(str, 1024, 0);
    BlobOStream bs(bb);
    bs << MM;
    return quote(str, bb.size());
  };

  string ParmTableSQLHelper::quote ( char* src, int length)
  {
    // quote \0 and ' etc
    char escapedChars[] = {'\0', 'w', '\'', '\"', '\b', '\n', '\r', '\t', '\\' };
    char escapers[]     = { '0', '1',  '2',  '3',  '4',  '5',  '6',  '7', '8'  }; // the array index should be equal to the value because of the way unquote works
    bool escaped;
  
    string c;
    c.reserve(2*length);
    int srci = 0;
    while (srci<length)
      {
	escaped = false;
	for ( int i=0; i<9; i++) {
	  if (*(src+srci) == escapedChars[i]) {
	    c.append("w");	  
	    c.append(1, escapers[i]);
	    escaped = true;
	    break;
	  }
	}
	//      cout<<"char :>"<<*(src+srci)<<"< number :"<<(int)*(src+srci)<<endl;
	if (!escaped) c.append(1,*(src+srci));
	srci++;
      }
    return c;
  }

  int ParmTableSQLHelper::unquote ( char* dest,  char* src, int length)
  {
    char escapedChars[] = {'\0', 'w', '\'', '\"', '\b', '\n', '\r', '\t', '\\' };
    char escapers[]     = { '0', '1',  '2',  '3',  '4',  '5',  '6',  '7', '8'  }; // the array index should be equal to the value because of the way unquote works

    int srci = 0;
    int desti = 0;
    while (srci<length) {
      if (*(src+srci) == 'w') {
	srci++;
	for ( int i=0; i<9; i++) {
	  if (*(src+srci) == escapers[i]) {
	    dest[desti] = escapedChars[i];
	  }
	}
      } else {
	dest[desti]=*(src+srci);
      }
      //    cout<<"char :>"<<dest[desti]<<"< number :"<<(int)dest[desti]<<endl;
      srci++;
      desti++;
    }
    return desti;
  }
}

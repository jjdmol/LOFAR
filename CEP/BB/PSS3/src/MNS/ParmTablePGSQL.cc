//# ParmTablePGSQL.cc: Object to hold parameters in a database table.
//#
//# Copyright (C) 2002
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <MNS/ParmTablePGSQL.h>
#include <MNS/MeqDomain.h>
#include <Common/Debug.h>
#include <casa/BasicMath/Math.h>
#include <stdlib.h>
#include <sstream>
#include <Common/BlobIBufChar.h>
#include <Common/BlobOBufChar.h>
#include <Common/BlobIBufString.h>
#include <Common/BlobOBufString.h>
#include <Common/BlobIStream.h>
#include <Common/BlobOStream.h>
#include <Common/BlobString.h>

using namespace LOFAR;

#define PRECISION 20

ParmTablePGSQL::ParmTablePGSQL (const string& hostName, const string& userName, const string& tableName) : itsTableName (tableName)
{
  string connectionString = "host=" + hostName+" user=postgres dbname=" + userName;
  itsDBConnection = PQconnectdb(connectionString.c_str());
  AssertMsg ( (PQstatus( itsDBConnection ) == CONNECTION_OK), "Cannot connect to database with connectionstring =" + connectionString);
}

ParmTablePGSQL::~ParmTablePGSQL()
{

  PQfinish(itsDBConnection);
  itsDBConnection = 0;

}

vector<MeqPolc> ParmTablePGSQL::getPolcs (const string& parmName,
				       int, int,
				       const MeqDomain& domain)
{
  vector<MeqPolc> result;

  string query = "SELECT " + getPolcNoDomainColumns() + ", " + getDomainColumns() + " FROM " + itsTableName + " WHERE name = '" + parmName +"'";

  PGresult* queryResult = PQexec ( itsDBConnection, query.c_str());
  
  AssertMsg(PQresultStatus(queryResult) == PGRES_TUPLES_OK, "no Polcs found");
  for (int row=0; row<PQntuples(queryResult); row++)
  {
    MeqPolc MP = readPolcNoDomainQRes(queryResult, row, 0);
    MP.setDomain( readDomainQRes(queryResult, row, 8));
    result.push_back (MP);
  }

  PQclear (queryResult);

  return result;
}

MeqPolc ParmTablePGSQL::getInitCoeff (const string& parmName,
				   int srcnr, int statnr)
{
  // Try to find the default initial values in the InitialValues subtable.
  // The parameter name consists of parts (separated by dots), so the
  // parameters are categorised in that way.
  // An initial value can be defined for the full name or for a higher
  // category.
  // So look up until found or until no more parts are left.
  MeqPolc result;
  PGresult* queryResult;

  string name = parmName;
  while (true) {
    string query = "SELECT " + getPolcNoDomainColumns () + 
                   " FROM " + itsTableName + "Def WHERE name = '" + name + "'";

    queryResult = PQexec ( itsDBConnection, query.c_str());

    AssertMsg (PQresultStatus(queryResult) == PGRES_TUPLES_OK ,"query for initcoeff went wrong");
    if (PQntuples(queryResult)>0)
    {
      AssertMsg (PQntuples(queryResult)== 1,"too many matches for default value");
      result = readPolcNoDomainQRes( queryResult, 0, 0);
      break;
    } else 
    {
      string::size_type idx = name.rfind ('.');
      // Exit loop if no more name parts.
      if (idx == string::npos) 
      {
	//cout<<"no match for default value"<<endl;
	break;
      }
      // Remove last part and try again.
      name = name.substr (0, idx);
    }
    PQclear (queryResult); // clear query before next query
  }
  PQclear (queryResult); // clear query after break;

  return result;
}
				    
void ParmTablePGSQL::putCoeff (const string& parmName,
			    int srcnr, int statnr,
			    const MeqPolc& polc)
{
  const MeqDomain& domain = polc.domain();
  VMParm set = find (parmName, domain);
  if (! set.empty()) {
    AssertMsg (set.size()==1, "Parameter " << parmName <<
		 " has multiple entries for time "
		 << domain.startX() << ':' << domain.endX() << " and freq "
		 << domain.startY() << ':' << domain.endY());
    MeqParmHolder& parm = set[0];
    const MeqDomain& pdomain = parm.getPolc().domain();

    AssertMsg (near(domain.startX(), pdomain.startX())  &&
	       near(domain.endX(), pdomain.endX())  &&
	       near(domain.startY(), pdomain.startY())  &&
	       near(domain.endY(), pdomain.endY()),
	       "Parameter " << parmName <<
	       " has a partially instead of fully matching entry for time "
		 << domain.startX() << ':' << domain.endX() << " and freq "
		 << domain.startY() << ':' << domain.endY());
    MeqPolc newPolc = parm.getPolc();
    newPolc.setCoeff (polc.getCoeff());
    parm.setPolc (newPolc);
    string query = getUpdateQuery(parm);
    PGresult* queryResult = PQexec ( itsDBConnection, query.c_str());
    PQclear (queryResult);
  } else {
    putNewCoeff (parmName, srcnr, statnr, polc);
  }
}

void ParmTablePGSQL::putNewCoeff (const string& parmName,
			    int srcnr, int statnr,
			    const MeqPolc& polc)
{
  MeqParmHolder parm(parmName, srcnr, statnr, polc);
  string query = getInsertQuery(parm);
  PGresult* queryResult = PQexec ( itsDBConnection, query.c_str());
  PQclear (queryResult);
}

void ParmTablePGSQL::putNewDefCoeff (const string& parmName,
			    int srcnr, int statnr,
			    const MeqPolc& polc)
{
  MeqParmHolder parm(parmName, srcnr, statnr, polc);
  string query = getDefInsertQuery(parm);
  PGresult* queryResult = PQexec ( itsDBConnection, query.c_str());
  PQclear (queryResult);
}

VMParm ParmTablePGSQL::find (const string& parmName,
			    const MeqDomain& domain)
{
  VMParm set;

  stringstream query;
  query.precision(PRECISION);
  query << "SELECT " << getMeqParmNoPolcColumns() << ", " << getPolcNoDomainColumns() << ", " << getDomainColumns() <<
                 " FROM " << itsTableName << " WHERE name = '" << parmName << "' AND tb < " << domain.endX() <<
                 " AND te > " << domain.startX() << " AND fb < " << domain.endY() << " AND fe > " << domain.startY();
  PGresult* queryResult = PQexec ( itsDBConnection, query.str().c_str());
  
  if (PQresultStatus(queryResult) == PGRES_TUPLES_OK)
  {
    for (int row=0; row<PQntuples(queryResult); row++)
    {
      MeqParmHolder MPH = readMeqParmNoPolcQRes(queryResult, row, 0);
      MeqPolc MP = readPolcNoDomainQRes (queryResult, row, 3);
      MP.setDomain( readDomainQRes (queryResult, row, 11));
      MPH.setPolc( MP); 
      set.push_back( MPH );
    }
  };

  PQclear (queryResult);

  return set;
}

vector<string> ParmTablePGSQL::getSources()
{
  vector<string> nams;
  stringstream qs;
  qs.precision(PRECISION);
  qs << "SELECT name FROM " << itsTableName << " WHERE name LIKE 'RA.%' " <<
    "UNION SELECT name FROM " << itsTableName << "Def WHERE name LIKE 'RA.%'";
  PGresult* queryResult = PQexec ( itsDBConnection, qs.str().c_str());
  
  if (PQresultStatus(queryResult) == PGRES_TUPLES_OK)
  {
    for (int row=0; row<PQntuples(queryResult); row++)
    {
      string s(PQgetvalue(queryResult,
                          row,
                          0));
      nams.push_back (s);
    }
  };

  PQclear (queryResult);

  return nams;
}

void ParmTablePGSQL::unlock()
  {};


inline MeqMatrix ParmTablePGSQL::getMeqMatrix(PGresult* queryResult, int row, int column)
{
  MeqMatrix MM;
  char* chbuf = PQgetvalue ( queryResult, row, column);
  char binbuf[strlen(chbuf)];
  int nrOfChars = decode(chbuf, binbuf, strlen(chbuf));
  LOFAR::BlobIBufChar bb(binbuf, nrOfChars);
  LOFAR::BlobIStream bs(bb);
  bs >> MM;
  return MM;
};
inline double ParmTablePGSQL::getDouble(PGresult* queryResult, int row, int column)
{
  return strtod(PQgetvalue ( queryResult, row, column), NULL);
};
inline bool ParmTablePGSQL::getBool(PGresult* queryResult, int row, int column)
{
  return (getInt(queryResult, row, column) == 1);
};
inline string ParmTablePGSQL::getString(PGresult* queryResult, int row, int column)
{
  string s( PQgetvalue ( queryResult, row, column));
  return s;
};
inline int ParmTablePGSQL::getInt(PGresult* queryResult, int row, int column)
{
  return atoi( PQgetvalue ( queryResult, row, column));
};
inline MeqDomain ParmTablePGSQL::getDomain(PGresult* queryResult, int row, int column)
{
  MeqDomain MD(getDouble(queryResult, row, column), getDouble(queryResult, row, column+1), getDouble(queryResult, row, column+2), getDouble(queryResult, row, column+3));
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
inline string ParmTablePGSQL::getUpdateQuery(MeqParmHolder MPH)
{
  stringstream qs;
  qs.precision(PRECISION);
  qs << "UPDATE " << itsTableName 
     << " Coeff = '" << MeqMat2string(MPH.getPolc().getCoeff())
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
  //  cout<<"query: "<<s<<endl;
  return s;
};

// insert query
// we need the following columns here
// Coeff, SimCoeff, PertSimCoeff, pertvalue, isrelpert, t0, f0, normalized, tb, te, fb, fe, name, srcnr, statnr
inline string ParmTablePGSQL::getInsertQuery(MeqParmHolder MPH)
{
  stringstream qs;
  qs.precision(PRECISION);
  qs << "INSERT INTO " << itsTableName << " ("
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
inline string ParmTablePGSQL::getDefInsertQuery(MeqParmHolder MPH)
{
  stringstream qs;
  qs.precision(PRECISION);
  qs << "INSERT INTO " << itsTableName << " ("
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
  //  cout<<"query: "<<s<<endl;
  return s;
}

// Query for reading polcs without domain
inline string ParmTablePGSQL::getPolcNoDomainColumns()
{
  return "Coeff, SimCoeff, PertSimCoeff, pertvalue, isrelpert, t0, f0, normalized";
  //       0        1           2           3         4         5  6    7       
};
inline MeqPolc ParmTablePGSQL::readPolcNoDomainQRes(PGresult* queryResult, int row, int column)
{
  MeqPolc MP;

  MP.setCoeff( getMeqMatrix ( queryResult, row, column++) );
  MP.setSimCoeff( getMeqMatrix ( queryResult, row, column++) );
  MP.setPertSimCoeff( getMeqMatrix ( queryResult, row, column++) );
  MP.setPerturbation ( getDouble( queryResult, row, column), getBool ( queryResult, row, column+1));
  column += 2;
  MP.setX0 ( getDouble ( queryResult, row, column++) );
  MP.setY0 ( getDouble ( queryResult, row, column++) );
  MP.setNormalize ( getBool ( queryResult, row, column++));

  return MP;
};
// Query for reading domain
inline string ParmTablePGSQL::getDomainColumns()
{
  return "tb, te, fb, fe";
  //       0  1   2   3
};
inline MeqDomain ParmTablePGSQL::readDomainQRes(PGresult* queryResult, int row, int column)
{
  return getDomain( queryResult, row, column); // domain is 4 columns, so next column to be read is column column+4;
};
// Query for reading MeqParmDefHolder
inline string ParmTablePGSQL::getMeqParmNoPolcColumns()
{
  return "name, srcnr, statnr";
  //       0      1      2
};
inline MeqParmHolder ParmTablePGSQL::readMeqParmNoPolcQRes(PGresult* queryResult, int row, int column)
{
  MeqParmHolder MPH;
  MPH.setName( getString( queryResult, row, column++));
  MPH.setSourceNr ( getInt( queryResult, row, column++));
  MPH.setStation ( getInt( queryResult, row, column++));

  return MPH;
};


inline string ParmTablePGSQL::MeqMat2string(const MeqMatrix &MM)
{
  char str[1024];
  BlobOBufChar bb(str, 1024, 0);
  BlobOStream bs(bb);
  bs << MM;
  string dstr;
  return encode(str, bb.size());
};

string ParmTablePGSQL::encode (char* b, int length)
{
  string c;
  c.reserve(2*length);
  int bi = 0;
  while (bi<length)
    {
      if (*(b+bi) == '\0')
	{
	  c.append("w");
	  c.append("0");
	}      
      else if (*(b+bi) == 'w')
	{
	  c.append("w");
	  c.append("1");
	}
      else if (*(b+bi) == '\'')
	{
	  c.append("w");
	  c.append("2");
	}
      else
	{
	  c.append(1,*(b+bi));
	}
      bi++;
    }
  return c;
}

int ParmTablePGSQL::decode (char* c, char* b, int length)
{
  int bi = 0;
  int ci = 0;
  while (ci<length)
    {
      if (*(c+ci) == 'w')
	{
	  if (*(c+ci+1) == '1')
	    {
	      b[bi]='w';
	      ci++;
	    }
	  else if (*(c+ci+1) == '0')
	    {
	      b[bi]='\0';
	      ci++;
	    }
	  else if (*(c+ci+1) == '2')
	    {
	      b[bi]='\'';
	      ci++;
	    }
	}      
      else
	{
	  b[bi]=*(c+ci);
	}
      ci++;
      bi++;
    }
  return bi;
}
  



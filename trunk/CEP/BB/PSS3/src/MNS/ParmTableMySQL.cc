//# ParmTableMySQL.cc: Object to hold parameters in a mysql database table.
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

#include <MNS/ParmTableMySQL.h>
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

ParmTableMySQL::ParmTableMySQL (const string& hostName, const string& userName, const string& tableName) : itsTableName (tableName)
{
  mysql_init(&itsDB);
  //MYSQL *mysql_real_connect(MYSQL *mysql, const char *host, const char *user, const char *passwd, const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag) 
  if (mysql_real_connect(           &itsDB,          "dop50",          "mysql",               NULL, userName.c_str(),               0,                    NULL,                         0)==NULL)
  {
    AssertMsg(false, "no connection to database");
  }
  //cout<<"connected to database"<<endl;
}

ParmTableMySQL::~ParmTableMySQL()
{
  mysql_close(&itsDB);
}

vector<MeqPolc> ParmTableMySQL::getPolcs (const string& parmName,
				       int, int,
				       const MeqDomain& domain)
{
  //cout<<"retreiving polynomial coefficients"<<endl;
  string query = "SELECT " + getPolcNoDomainColumns() + ", " + getDomainColumns() + " FROM " + itsTableName + " WHERE name = '" + parmName +"'";
  //cout<<"query: "<<query<<endl;
  vector<MeqPolc> result;

  if (mysql_query(&itsDB, query.c_str()))
  {
    AssertMsg(false, "Couldn't find polynomial coefficients");
  } else
  {
    MYSQL_RES* queryResult = mysql_store_result (&itsDB);

    for (unsigned int row=0; row<mysql_num_rows(queryResult); row++)
    {
      MYSQL_ROW resRow = mysql_fetch_row(queryResult);
      MeqPolc MP = readPolcNoDomainQRes(&resRow, 0);
      MP.setDomain( readDomainQRes(&resRow, 8));
      result.push_back (MP);
    }
    mysql_free_result(queryResult);
  }

  //cout<<"finished retreiving polc: "<<result.size()<<" polcs found."<<endl;
  return result;
}

MeqPolc ParmTableMySQL::getInitCoeff (const string& parmName,
				   int srcnr, int statnr)
{
  // Try to find the default initial values in the InitialValues subtable.
  // The parameter name consists of parts (separated by dots), so the
  // parameters are categorised in that way.
  // An initial value can be defined for the full name or for a higher
  // category.
  // So look up until found or until no more parts are left.
  //cout<<"retreiving intital coefficients for "<<parmName<<endl;
  MeqPolc result;

  string name = parmName;
  while (true) {
    string query = "SELECT " + getPolcNoDomainColumns () + 
                   " FROM " + itsTableName + "def WHERE name = '" + name + "'";
    //cout<<"query: "<<query<<endl;

    if (! mysql_query(&itsDB, query.c_str()))
    {
      //cout<<"query succeeded"<<endl;
      MYSQL_RES* queryResult;
      queryResult = mysql_store_result (&itsDB);
      AssertMsg(queryResult != NULL, "Initial coeff could not be retreived from the database")
      //cout<<"results stored"<<endl;

      if (mysql_num_rows(queryResult)>0)
      {
	AssertMsg (mysql_num_rows(queryResult)== 1,"too many matches for default value");
	MYSQL_ROW resRow = mysql_fetch_row(queryResult);
	//cout<<"reading columns"<<endl;
	result = readPolcNoDomainQRes( &resRow, 0);
	//cout<<"columns read"<<endl;
	mysql_free_result (queryResult); // clear query before break;
	break;
      } else 
      {
	string::size_type idx = name.rfind ('.');
	// Exit loop if no more name parts.
	if (idx == string::npos) 
	{
	  //cout<<"no match for default value"<<endl;
	  mysql_free_result (queryResult); // clear query before break;
	  break;
	}
	// Remove last part and try again.
	name = name.substr (0, idx);
      }
      mysql_free_result (queryResult); // clear query before next query
    } else {
      AssertMsg(false, "Couldn't execute query for initial coefficients");
      break;
    }
  }

  //cout<<"finished retreiving intital coefficients"<<endl;
  return result;
}
				    
void ParmTableMySQL::putCoeff (const string& parmName,
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
    mysql_query(&itsDB, query.c_str());
  } else {
    putNewCoeff (parmName, srcnr, statnr, polc);
  }
}

void ParmTableMySQL::putNewCoeff (const string& parmName,
			    int srcnr, int statnr,
			    const MeqPolc& polc)
{
  MeqParmHolder parm(parmName, srcnr, statnr, polc);
  string query = getInsertQuery(parm);
  mysql_query(&itsDB, query.c_str());
}

void ParmTableMySQL::putNewDefCoeff (const string& parmName,
			    int srcnr, int statnr,
			    const MeqPolc& polc)
{
  MeqParmHolder parm(parmName, srcnr, statnr, polc);
  string query = getDefInsertQuery(parm);
  mysql_query(&itsDB, query.c_str());
}

VMParm ParmTableMySQL::find (const string& parmName,
			    const MeqDomain& domain)
{
  VMParm set;

  stringstream query;
  query.precision(PRECISION);
  query << "SELECT " << getMeqParmNoPolcColumns() << ", " << getPolcNoDomainColumns() << ", " << getDomainColumns() <<
                 " FROM " << itsTableName << " WHERE name = '" << parmName << "' AND tb < " << domain.endX() <<
                 " AND te > " << domain.startX() << " AND fb < " << domain.endY() << " AND fe > " << domain.startY();

  if (! mysql_query ( &itsDB, query.str().c_str()))
  {
    MYSQL_RES* queryResult = mysql_store_result(&itsDB);
  
    for (unsigned int row=0; row<mysql_num_rows(queryResult); row++)
    {
      MYSQL_ROW resRow = mysql_fetch_row(queryResult);
      MeqParmHolder MPH = readMeqParmNoPolcQRes(&resRow, 0);
      MeqPolc MP = readPolcNoDomainQRes (&resRow, 3);
      MP.setDomain( readDomainQRes (&resRow, 12));
      MPH.setPolc( MP); 
      set.push_back( MPH );
    }
    mysql_free_result (queryResult);
  };

  return set;
}

vector<string> ParmTableMySQL::getSources()
{
  //cout<<"retreiving sources"<<endl;
  vector<string> nams;
  stringstream qs;
  qs.precision(PRECISION);
  qs << "SELECT name FROM " << itsTableName << " WHERE name LIKE 'RA.%' " <<
        "UNION SELECT name FROM " << itsTableName << "def WHERE name LIKE 'RA.%';";
  //cout << "Query: "<<qs.str().c_str()<<endl;
  if (mysql_query ( &itsDB, qs.str().c_str()))
  {
    //cout<<"Could not retreive sources from database: "<<mysql_error(&itsDB)<<endl;
    //cout<<"MySQL status: "<<mysql_stat(&itsDB)<<endl;
    AssertMsg(false, "database error, quitting");
  } else
  {
    MYSQL_RES* queryResult = mysql_store_result(&itsDB);  
    AssertMsg(queryResult != NULL, "names of sources could not be retreived from the database");
    AssertMsg(mysql_num_rows(queryResult)>0,"no sources found in database");
    for (unsigned int row=0; row<mysql_num_rows(queryResult); row++)
    {
      MYSQL_ROW resRow = mysql_fetch_row(queryResult);
      string s(resRow[0]);
      nams.push_back (s);
    }
    mysql_free_result (queryResult);
  }
  //cout<<"finished retreiving "<<nams.size()<<" sources from "<<itsTableName<<endl;
  return nams;
}

void ParmTableMySQL::unlock()
  {};


inline MeqMatrix ParmTableMySQL::getMeqMatrix(MYSQL_ROW* resRow, int column)
{
  MeqMatrix MM;
  char* chbuf = (*resRow)[column];
  char binbuf[strlen(chbuf)];
  int nrOfChars = decode(chbuf, binbuf, strlen(chbuf));
  LOFAR::BlobIBufChar bb(binbuf, nrOfChars);
  LOFAR::BlobIStream bs(bb);
  bs >> MM;
  return MM;
};
inline double ParmTableMySQL::getDouble(MYSQL_ROW* resRow, int column)
{
  return strtod((*resRow)[column], NULL);
};
inline MeqDomain ParmTableMySQL::getDomain(MYSQL_ROW* resRow, int column)
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
inline string ParmTableMySQL::getUpdateQuery(MeqParmHolder MPH)
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
  return s;
};

// insert query
// we need the following columns here
// Coeff, SimCoeff, PertSimCoeff, pertvalue, isrelpert, t0, f0, normalized, tb, te, fb, fe, name, srcnr, statnr
inline string ParmTableMySQL::getInsertQuery(MeqParmHolder MPH)
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
  return s;
}

// insert query
// we need the following columns here
// Coeff, SimCoeff, PertSimCoeff, pertvalue, isrelpert, t0, f0, normalized, tb, te, fb, fe, name, srcnr, statnr
inline string ParmTableMySQL::getDefInsertQuery(MeqParmHolder MPH)
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
  return s;
}

// Query for reading polcs without domain
inline string ParmTableMySQL::getPolcNoDomainColumns()
{
  return "Coeff, SimCoeff, PertSimCoeff, pertvalue, isrelpert, t0, f0, normalized";
  //       0        1           2           3         4         5  6    7       
};
inline MeqPolc ParmTableMySQL::readPolcNoDomainQRes(MYSQL_ROW* resRow, int column)
{
  MeqPolc MP;

  MP.setCoeff( getMeqMatrix (resRow, column++));
  MP.setSimCoeff( getMeqMatrix (resRow, column++));
  MP.setPertSimCoeff( getMeqMatrix (resRow, column++));
  MP.setPerturbation ( getDouble( resRow, column), (atoi((*resRow)[column+1]) == 1));
  column += 2;
  MP.setX0 ( getDouble ( resRow, column++) );
  MP.setY0 ( getDouble ( resRow, column++) );
  MP.setNormalize (atoi((*resRow)[column++]) == 1);

  return MP;
};
// Query for reading domain
inline string ParmTableMySQL::getDomainColumns()
{
  return "tb, te, fb, fe";
  //       0  1   2   3
};
inline MeqDomain ParmTableMySQL::readDomainQRes(MYSQL_ROW* resRow, int column)
{
  return getDomain( resRow, column); // domain is 4 columns, so next column to be read is column column+4;
};
// Query for reading MeqParmDefHolder
inline string ParmTableMySQL::getMeqParmNoPolcColumns()
{
  return "name, srcnr, statnr";
  //       0      1      2
};
inline MeqParmHolder ParmTableMySQL::readMeqParmNoPolcQRes(MYSQL_ROW* resRow, int column)
{
  MeqParmHolder MPH;
  MPH.setName((*resRow)[column]);
  MPH.setSourceNr (atoi((*resRow)[column++]));
  MPH.setStation (atoi((*resRow)[column++]));

  return MPH;
};


inline string ParmTableMySQL::MeqMat2string(const MeqMatrix &MM)
{
  char str[1024];
  BlobOBufChar bb(str, 1024, 0);
  BlobOStream bs(bb);
  bs << MM;
  string dstr;
  return encode(str, bb.size());
};

string ParmTableMySQL::encode (char* b, int length)
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

int ParmTableMySQL::decode (char* c, char* b, int length)
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
  

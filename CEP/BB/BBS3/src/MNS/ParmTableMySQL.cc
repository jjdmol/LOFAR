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
#include <MNS/ParmTableSQLHelper.h>
#include <MNS/MeqDomain.h>
#include <Common/LofarLogger.h>
#include <casa/BasicMath/Math.h>
#include <stdlib.h>
#include <sstream>

using namespace casa;

namespace LOFAR {

ParmTableMySQL::ParmTableMySQL (const string& hostName, const string& userName, const string& tableName) : itsTableName (tableName)
{
  mysql_init(&itsDB);
  //MYSQL *mysql_real_connect(MYSQL *mysql, const char *host, const char *user, const char *passwd, const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag) 
  if (mysql_real_connect(           &itsDB, hostName.c_str(),          "mysql",               NULL, userName.c_str(),               0,                    NULL,                         0)==NULL)
  {
    ASSERTSTR(false, "no connection to database");
  }
  LOG_TRACE_STAT("connected to database");
}

ParmTableMySQL::~ParmTableMySQL()
{
  mysql_close(&itsDB);
}

vector<MeqPolc> ParmTableMySQL::getPolcs (const string& parmName,
				       int, int,
				       const MeqDomain& domain)
{
  LOG_TRACE_STAT("retreiving polynomial coefficients");
#if 0
  string query = ParmTableSQLHelper::getGetPolcsQuery(parmName, domain, itsTableName);
  LOG_TRACE_VAR_STR("query: "<<query);
  vector<MeqPolc> result;

  if (mysql_query(&itsDB, query.c_str()))
  {
    ASSERTSTR(false, "Couldn't find polynomial coefficients");
  } else
  {
    MYSQL_RES* queryResult = mysql_store_result (&itsDB);

    for (unsigned int row=0; row<mysql_num_rows(queryResult); row++)
    {
      MYSQL_ROW resRow = mysql_fetch_row(queryResult);
      result.push_back ( ParmTableSQLHelper::readMeqPolc(resRow));
    }
    mysql_free_result(queryResult);
  }
#else
  vector<MeqParmHolder> MPH = find(parmName, domain);
  vector<MeqPolc> result;
  for (int i=0; i<MPH.size(); i++) {
    result.push_back(MPH[i].getPolc());
  }
  
#endif

  LOG_TRACE_STAT_STR("finished retreiving polc: "<<result.size()<<" polcs found.");
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
  LOG_TRACE_STAT_STR("retreiving intital coefficients for "<<parmName);
  MeqPolc result;

  string name = parmName;
  while (true) {
    string query = ParmTableSQLHelper::getGetInitCoeffQuery(name, itsTableName);
    LOG_TRACE_VAR_STR("query: "<<query);
    if (! mysql_query(&itsDB, query.c_str()))
    {
      LOG_TRACE_VAR("query succeeded");
      MYSQL_RES* queryResult;
      queryResult = mysql_store_result (&itsDB);
      ASSERTSTR(queryResult != NULL, "Initial coeff could not be retreived from the database")
      LOG_TRACE_VAR("results stored");

      if (mysql_num_rows(queryResult)>0)
      {
	ASSERTSTR (mysql_num_rows(queryResult)== 1,"too many matches for default value");
	MYSQL_ROW resRow = mysql_fetch_row(queryResult);
	LOG_TRACE_VAR("reading columns");
	result = ParmTableSQLHelper::readDefMeqPolc(resRow);
	LOG_TRACE_VAR("columns read");
	mysql_free_result (queryResult); // clear query before break;
	break;
      } else 
      {
	string::size_type idx = name.rfind ('.');
	// Exit loop if no more name parts.
	if (idx == string::npos) 
	{
	  LOG_TRACE_VAR("no match for default value");
	  mysql_free_result (queryResult); // clear query before break;
	  break;
	}
	// Remove last part and try again.
	name = name.substr (0, idx);
      }
      mysql_free_result (queryResult); // clear query before next query
    } else {
      ASSERTSTR(false, "Couldn't execute query for initial coefficients");
      break;
    }
  }

  LOG_TRACE_STAT("finished retreiving intital coefficients");
  return result;
}
				    
void ParmTableMySQL::putCoeff (const string& parmName,
			    int srcnr, int statnr,
			    const MeqPolc& polc)
{
  const MeqDomain& domain = polc.domain();
  vector<MeqParmHolder> set = find (parmName, domain);
  if (! set.empty()) {
    ASSERTSTR (set.size()==1, "Parameter " << parmName <<
		 " has multiple entries for time "
		 << domain.startX() << ':' << domain.endX() << " and freq "
		 << domain.startY() << ':' << domain.endY());
    MeqParmHolder& parm = set[0];
    const MeqDomain& pdomain = parm.getPolc().domain();

    ASSERTSTR (near(domain.startX(), pdomain.startX())  &&
	       near(domain.endX(), pdomain.endX())  &&
	       near(domain.startY(), pdomain.startY())  &&
	       near(domain.endY(), pdomain.endY()),
	       "Parameter " << parmName <<
	       " has a partially instead of fully matching entry for time "
	       << domain.startX() << ':' << domain.endX() << " and freq "
	       << domain.startY() << ':' << domain.endY() << endl
	       << "(" << pdomain.startX() << ":" << pdomain.endX()
	       << ", " << pdomain.startY() << ":" << pdomain.endY()) ;
    MeqPolc newPolc = parm.getPolc();
    newPolc.setCoeff (polc.getCoeff());
    parm.setPolc (newPolc);
    string query = ParmTableSQLHelper::getUpdateQuery(parm, itsTableName);
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
  string query = ParmTableSQLHelper::getInsertQuery(parm, itsTableName);
  mysql_query(&itsDB, query.c_str());
}

void ParmTableMySQL::putNewDefCoeff (const string& parmName,
			    int srcnr, int statnr,
			    const MeqPolc& polc)
{
  MeqParmHolder parm(parmName, srcnr, statnr, polc);
  string query = ParmTableSQLHelper::getDefInsertQuery(parm, itsTableName);
  mysql_query(&itsDB, query.c_str());
}

vector<MeqParmHolder> ParmTableMySQL::find (const string& parmName,
			    const MeqDomain& domain)
{
  LOG_TRACE_STAT("searching for MParms");
  vector<MeqParmHolder> set;

  string query;
  query = ParmTableSQLHelper::getFindQuery(parmName, domain, itsTableName);
  LOG_TRACE_VAR_STR("query: "<<query);

  if (! mysql_query ( &itsDB, query.c_str()))
  {
    MYSQL_RES* queryResult = mysql_store_result(&itsDB);
  
    for (unsigned int row=0; row<mysql_num_rows(queryResult); row++)
    {
      MYSQL_ROW resRow = mysql_fetch_row(queryResult);
      set.push_back( ParmTableSQLHelper::readMeqParmHolder(resRow) );
    }
    mysql_free_result (queryResult);
  };

  return set;
}

vector<string> ParmTableMySQL::getSources()
{
  LOG_TRACE_STAT("retreiving sources");
  vector<string> nams;
  string query = ParmTableSQLHelper::getSourcesQuery(itsTableName);
  LOG_TRACE_VAR_STR("Query: "<<query.c_str());
  if (mysql_query ( &itsDB, query.c_str()))
  {
    LOG_TRACE_VAR_STR("Could not retreive sources from database: "<<mysql_error(&itsDB));
    LOG_TRACE_VAR_STR("MySQL status: "<<mysql_stat(&itsDB));
    ASSERTSTR(false, "database error, quitting");
  } else
  {
    MYSQL_RES* queryResult = mysql_store_result(&itsDB);  
    ASSERTSTR(queryResult != NULL, "names of sources could not be retreived from the database");
    ASSERTSTR(mysql_num_rows(queryResult)>0,"no sources found in database");
    for (unsigned int row=0; row<mysql_num_rows(queryResult); row++)
    {
      MYSQL_ROW resRow = mysql_fetch_row(queryResult);
      string s(resRow[0]);
      nams.push_back (s);
    }
    mysql_free_result (queryResult);
  };
  LOG_TRACE_STAT_STR("finished retreiving "<<nams.size()<<" sources from "<<itsTableName);
  return nams;
}

void ParmTableMySQL::unlock()
  {};

}

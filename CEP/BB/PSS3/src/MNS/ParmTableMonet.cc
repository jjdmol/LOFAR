//# ParmTableMonet.cc: Object to hold parameters in a mysql database table.
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

#include <MNS/ParmTableMonet.h>
#include <MNS/ParmTableSQLHelper.h>
#include <MNS/MeqDomain.h>
#include <Common/LofarLogger.h>
#include <casa/BasicMath/Math.h>
#include <stdlib.h>
#include <sstream>

namespace LOFAR {

ParmTableMonet::ParmTableMonet (const string& hostName, const string& userName, const string& tableName, bool autocommit) : itsTableName (tableName)
{
  //                     host    , port , user, pass?,  query language
  itsDB = mapi_connect(hostName.c_str(), 45123, userName.c_str(), "monetdb", "sql");
  if (mapi_error(itsDB))
  {
    LOG_FATAL_STR("no connection to database"<<"error: "<<mapi_error_str(itsDB));
    exit(1);
  }
  // set autocommit off (0)
  if (autocommit) {
    if (mapi_setAutocommit(itsDB, 1) == MERROR)
      LOG_WARN("error setting autocommit"); 
  } else {
    if (mapi_setAutocommit(itsDB, 0) == MERROR)
      LOG_WARN("error unsetting autocommit"); 
  };
  LOG_TRACE_STAT_STR("connected to monet database on "<<hostName<<" with user "<<userName);
}

ParmTableMonet::~ParmTableMonet()
{
  MapiHdl hdl = mapi_query(itsDB, "commit;");
  mapi_close_handle(hdl);
  mapi_destroy(itsDB);
}

vector<MeqPolc> ParmTableMonet::getPolcs (const string& parmName,
				       int, int,
				       const MeqDomain& domain)
{
  LOG_TRACE_STAT_STR("retreiving polynomial coefficients "<<parmName<<" from "<<itsTableName);
  string query = ParmTableSQLHelper::getGetPolcsQuery(parmName, domain, itsTableName);
  LOG_TRACE_VAR_STR("query: "<<query);
  vector<MeqPolc> result;

  MapiHdl hdl;
  if ((hdl = mapi_query(itsDB, query.c_str()))==NULL)
  {
    ASSERTSTR(false, "Couldn't find polynomial coefficients");
  } else
  {
    mapi_fetch_all_rows(hdl);

    while (mapi_fetch_row(hdl))
    {
      char** resRow = mapi_fetch_field_array(hdl);
      result.push_back ( ParmTableSQLHelper::readMeqPolc(resRow));
    }
    mapi_close_handle(hdl);
  }

  LOG_TRACE_STAT_STR("finished retreiving polc: "<<result.size()<<" polcs found.");
  return result;
}

MeqPolc ParmTableMonet::getInitCoeff (const string& parmName,
				   int srcnr, int statnr)
{
  // Try to find the default initial values in the InitialValues subtable.
  // The parameter name consists of parts (separated by dots), so the
  // parameters are categorised in that way.
  // An initial value can be defined for the full name or for a higher
  // category.
  // So look up until found or until no more parts are left.
  LOG_TRACE_STAT_STR("retreiving intital coefficients for "<<parmName<<" from "<<itsTableName);
  MeqPolc result;
  MapiHdl hdl;

  string name = parmName;
  while (true) {
    string query = ParmTableSQLHelper::getGetInitCoeffQuery(name, itsTableName);
    LOG_TRACE_VAR_STR("query: "<<query);

    if ((hdl =  mapi_query(itsDB, query.c_str())) != NULL)
    {
      LOG_TRACE_VAR_STR("query succeeded");
      int num_rows = mapi_fetch_all_rows(hdl);

      if (num_rows>0)
      {
	mapi_fetch_row(hdl);
	ASSERTSTR (num_rows== 1,"too many matches for default value");
	char** resRow = mapi_fetch_field_array(hdl);
	LOG_TRACE_VAR_STR("reading columns");
	result = ParmTableSQLHelper::readDefMeqPolc(resRow);
	LOG_TRACE_VAR_STR("columns read");
	mapi_close_handle (hdl); // clear query before break;
	break;
      } else 
      {
	string::size_type idx = name.rfind ('.');
	// Exit loop if no more name parts.
	if (idx == string::npos) 
	{
	  LOG_TRACE_VAR_STR("no match for default value");
	  mapi_close_handle (hdl); // clear query before break;
	  break;
	}
	// Remove last part and try again.
	name = name.substr (0, idx);
      }
      mapi_close_handle (hdl); // clear query before next query
    } else {
      ASSERTSTR(false, "Couldn't execute query for initial coefficients");
      break;
    }
  }

  LOG_TRACE_STAT_STR("finished retreiving intital coefficients");
  return result;
}
				    
void ParmTableMonet::putCoeff (const string& parmName,
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
		 << domain.startY() << ':' << domain.endY());
    MeqPolc newPolc = parm.getPolc();
    newPolc.setCoeff (polc.getCoeff());
    parm.setPolc (newPolc);
    string query = ParmTableSQLHelper::getUpdateQuery(parm, itsTableName);
    MapiHdl hdl = mapi_query(itsDB, query.c_str());
    mapi_close_handle (hdl);
  } else {
    putNewCoeff (parmName, srcnr, statnr, polc);
  }
}

void ParmTableMonet::putNewCoeff (const string& parmName,
			    int srcnr, int statnr,
			    const MeqPolc& polc)
{
  MeqParmHolder parm(parmName, srcnr, statnr, polc);
  string query = ParmTableSQLHelper::getInsertQuery(parm, itsTableName);
  MapiHdl hdl = mapi_query(itsDB, query.c_str());
  mapi_close_handle (hdl);
}

void ParmTableMonet::putNewDefCoeff (const string& parmName,
			    int srcnr, int statnr,
			    const MeqPolc& polc)
{
  LOG_TRACE_STAT_STR("putnewdefcoeff called for parm: "<<parmName<<flush);

  MeqParmHolder parm(parmName, srcnr, statnr, polc);
  string query = ParmTableSQLHelper::getDefInsertQuery(parm, itsTableName);
  MapiHdl hdl = mapi_query(itsDB, query.c_str());
  if (hdl == NULL){
    LOG_WARN_STR("db error: "<<mapi_error_str(itsDB));
  }
  LOG_TRACE_VAR_STR("result error: "<<mapi_result_error(hdl));
  mapi_close_handle (hdl);
}

vector<MeqParmHolder> ParmTableMonet::find (const string& parmName,
			    const MeqDomain& domain)
{
  LOG_TRACE_STAT_STR("find "<<parmName<<" in "<<itsTableName);
  vector<MeqParmHolder> set;

  string query;
  query = ParmTableSQLHelper::getFindQuery(parmName, domain, itsTableName);
  LOG_TRACE_VAR_STR("query: "<<query);

  MapiHdl hdl = mapi_query(itsDB, query.c_str());
  if (hdl != NULL)
  {
    mapi_fetch_all_rows(hdl);

    while (mapi_fetch_row(hdl))
    {
      char** resRow = mapi_fetch_field_array(hdl);
      set.push_back( ParmTableSQLHelper::readMeqParmHolder(resRow) );
    }
    mapi_close_handle (hdl);
  };

  return set;
}

vector<string> ParmTableMonet::getSources()
{
  LOG_TRACE_STAT_STR("retreiving sources from "<<itsTableName);
  vector<string> nams;
  string query = ParmTableSQLHelper::getSourcesQuery(itsTableName);
  MapiHdl hdl = mapi_query(itsDB, query.c_str());
  if (hdl == NULL)
  {
    ASSERTSTR(false, "database error, quitting");
  } else
  {
    int num_rows = mapi_fetch_all_rows(hdl);
    ASSERTSTR(num_rows>0,"no sources found in database");
    char * ers = mapi_error_str(itsDB);
    if (ers != NULL) LOG_WARN_STR("database error: "<<ers);
    ers = mapi_result_error(hdl);
    if (ers != NULL) LOG_WARN_STR("database error: "<<ers);

    while (mapi_fetch_row(hdl))
    {
      char** resRow = mapi_fetch_field_array(hdl);
      string s(resRow[0]);
      nams.push_back (s);
    }
    mapi_close_handle (hdl);
  }
  LOG_TRACE_STAT_STR("finished retreiving "<<nams.size()<<" sources from "<<itsTableName);
  return nams;
}

void ParmTableMonet::unlock()
  {};

}

//# Copyright (C) 2012-2014  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$
#include <lofar_config.h>

#include "sassqlqueries.h"
#include <utility>
#include <iostream>

using namespace std;

#define QUERYIT std::map<QString,QString>::iterator
#define STRIT std::vector<QString>::const_iterator

SASSqlQueries::SASSqlQueries()
{
    queryIdToTemplate["otdblogin"] = "SELECT OTDBlogin('%1','%2')";
    queryIdToTemplate["gettreelist"] = "SELECT treeid FROM gettreelist('%1','0',0,'','','')";
    queryIdToTemplate["now"] = "SELECT now()";
    queryIdToTemplate["getTreesInPeriod"] = "SELECT * from getTreesInPeriod('%1','%2','%3')";
    queryIdToTemplate["limitsfromgetVHitemList"] = "SELECT limits from getVHitemList(%1,'LOFAR.ObsSW.Observation.Scheduler.predecessors')"   ;
}


// TODO no exception thrown yet. What happens on mistakes?
QSqlQuery SASSqlQueries::doQuery(const QSqlDatabase &sasDB,
                                 const QString &queryId,
                                 const std::vector<QString> &queryArgs)
{
    // construct the query string from the template and the arguments
    // 1 Get template from storage
    QString templateQuery;
    QUERYIT queryIt = queryIdToTemplate.find(queryId);
    if( queryIt != queryIdToTemplate.end())
         templateQuery = queryIt->second;

    // Insert the strings
    for(STRIT arg = queryArgs.begin(); arg != queryArgs.end(); arg++)
        templateQuery = templateQuery.arg(*arg);

    // perform the query
    QSqlQuery query(sasDB);
    query.exec(templateQuery);
    return query;
}


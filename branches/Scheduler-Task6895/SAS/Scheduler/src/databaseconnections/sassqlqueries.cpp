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

SASSqlQueries::SASSqlQueries()
{

}

QSqlQuery SASSqlQueries::doOTDBlogin(QSqlDatabase sasDB,
                                     QString sasUserName,
                                     QString sasPassword)
{
    QSqlQuery query(sasDB);
    query.exec("SELECT OTDBlogin('" + sasUserName
               + "','" + sasPassword + "')");
    return query;
}


QSqlQuery SASSqlQueries::treeidFROMgettreelist(QSqlDatabase sasDB, QString tree)
{
    QSqlQuery query(sasDB);
    query.exec("SELECT treeid FROM gettreelist('"
                    + tree
                    + "','0',0,'','','')");
    return query;
}

QSqlQuery SASSqlQueries::now(QSqlDatabase sasDB)
{
    QSqlQuery query(sasDB);
    query.exec("SELECT now()");
    return query;
}

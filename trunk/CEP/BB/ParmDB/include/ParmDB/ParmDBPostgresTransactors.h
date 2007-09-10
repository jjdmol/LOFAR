//# ParmDBPostgresTransactors.h: 
//#
//# Copyright (C) 2007
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

#ifndef LOFAR_PARMDB_PARMDBPOSTGRESTRANSACTORS_H
#define LOFAR_PARMDB_PARMDBPOSTGRESTRANSACTORS_H

//# Skip all code if PostgreSQL is not configured in.
#if defined(HAVE_PGSQL)

#if defined(HAVE_PQXX)
# include <pqxx/connection>
# include <pqxx/transactor>
# include <pqxx/result>
#else
# error libpqxx, the C++ API to PostgreSQL, is required
#endif


#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>
#include <Common/lofar_sstream.h>
#include <ParmDB/ParmValue.h>

namespace LOFAR
{
namespace ParmDB
{
class ParmDomain;


class PQGetDomain: public pqxx::transactor<>
{
public:
    PQGetDomain(const string &regex, ParmDomain &domain);
    PQGetDomain(const vector<string> &names, ParmDomain &domain);

    void operator()(argument_type& transaction);
    void on_commit();

private:
    string          itsRegex;
    vector<string>  itsNames;
    ParmDomain      &itsDomain;

    // The result of the executed query must be stored internally, because
    // it will be written in operator() and will be read in on_commit().
    pqxx::result    itsPQResult;
};


class PQPutValue: public pqxx::transactor<>
{
public:
    PQPutValue(const map<string, ParmValueSet> &values);
    void operator()(argument_type& transaction);
//    void on_commit();

private:
    const map<string, ParmValueSet> &itsValues;
};


class PQGetValues: public pqxx::transactor<>
{
public:
    PQGetValues(const string &regex,
        const ParmDomain &domain,
        int parentId,
        map<string, ParmValueSet> &result);
    
    PQGetValues(const vector<string> &names,
        const ParmDomain &domain,
        int parentId,
        map<string, ParmValueSet> &result);
    
    void operator()(argument_type& transaction);
    void on_commit();

private:
    ParmValue process(pqxx::result::tuple row);

    string                      itsRegex;
    vector<string>              itsNames;
    ParmDomain                  itsDomain;
    int                         itsParentId;
    map<string, ParmValueSet>   &itsResult;

    // The result of the executed query must be stored internally, because
    // it will be written in operator() and will be read in on_commit().
    pqxx::result itsPQResult;
};


class PQPutDefaultValue: public pqxx::transactor<>
{
public:
    PQPutDefaultValue(const string &name, const ParmValue value);
    void operator()(argument_type& transaction);
//    void on_commit();

private:
    string      itsName;
    ParmValue   itsValue;
};


class PQGetDefaultValues: public pqxx::transactor<>
{
public:
    PQGetDefaultValues(const string &regex, map<string, ParmValueSet> &result);
    void operator()(argument_type& transaction);
    void on_commit();

private:
    ParmValue process(pqxx::result::tuple row);

    string                      itsRegex;
    map<string, ParmValueSet>   &itsResult;

    // The result of the executed query must be stored internally, because
    // it will be written in operator() and will be read in on_commit().
    pqxx::result itsPQResult;
};


class PQGetNames: public pqxx::transactor<>
{
public:
    PQGetNames(const string &regex, vector<string> &result);
    void operator()(argument_type& transaction);
    void on_commit();

private:
    string                      itsRegex;
    vector<string>              &itsResult;

    // The result of the executed query must be stored internally, because
    // it will be written in operator() and will be read in on_commit().
    pqxx::result itsPQResult;
};

class PQDeleteValues: public pqxx::transactor<>
{
public:
    PQDeleteValues(const string &regex,
        const ParmDomain &domain,
        int parentId);
    
    void operator()(argument_type& transaction);

private:
    string                      itsRegex;
    ParmDomain                  itsDomain;
    int                         itsParentId;
};

class PQDeleteDefValues: public pqxx::transactor<>
{
public:
    PQDeleteDefValues(const string &regex);
    void operator()(argument_type& transaction);

private:
    string                      itsRegex;
};

} //# namespace BBS
} //# namespace LOFAR

#endif
#endif

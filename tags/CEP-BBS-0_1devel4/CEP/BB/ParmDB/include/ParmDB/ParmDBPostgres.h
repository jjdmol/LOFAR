//# ParmDBPostgres.h: 
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

#ifndef LOFAR_PARMDB_PARMDBPOSTGRES_H
#define LOFAR_PARMDB_PARMDBPOSTGRES_H

//# Skip all code if PostgreSQL is not configured in.
#if defined(HAVE_PGSQL)

#if defined(HAVE_PQXX)
# include <pqxx/connection>
#else
# error libpqxx, the C++ API to PostgreSQL, is required
#endif

#include <Common/lofar_string.h>
#include <Common/lofar_smartptr.h>
#include <ParmDB/ParmDB.h>

#include <ParmDB/ParmValue.h>
#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
namespace ParmDB
{

class ParmDBPostgres: public ParmDBRep
{
public:
    ParmDBPostgres(const string &database,
        const string &user,
        const string &password,
        const string &host,
        const string &port);

//    ~ParmDBPostgres();

    //# ParmDBRep interface implementation
//    virtual void lock (bool lockForWrite);
//    virtual void unlock();

    // Get the domain range (time,freq) of the given parameters in the table.
    // This is the minimum and maximum value of these axes for all parameters.
    // An empty name pattern is the same as * (all parms).
    // <group>
    virtual ParmDomain getRange(const std::string& parmNamePattern) const;
    virtual ParmDomain
        getRange(const std::vector<std::string>& parmNames) const;

    // Get the parameter values for the given parameter and domain.
    // Note that the requested domain may contain multiple values.
    // A selection on parentId is done if >= 0.
    virtual ParmValueSet getValues(const std::string& parmName,
        const ParmDomain& domain,
        int parentId,
        ParmDBRep::TableType);

    // Get all values for a given domain and set of parm names.
    // If the parm name vector is empty, all parm names are taken.
    virtual void getValues(std::map<std::string,ParmValueSet>& result,
        const std::vector<std::string>& parmNames,
        const ParmDomain& domain,
        int parentId,
        ParmDBRep::TableType);

    // Get the parameter values for the given parameters and domain.
    // Only * and ? should be used in the pattern (no [] and {}).
    // A selection on parentId is done if >= 0.
    virtual void getValues(std::map<std::string,ParmValueSet>& result,
        const std::string& parmNamePattern,
        const ParmDomain& domain,
        int parentId,
        ParmDBRep::TableType);

    // Put the value for the given parameter and domain.
    // overwriteMask=true indicates that the solvableMask might be changed
    // and should be overwritten in an existing record.
    virtual void putValue(const std::string& parmName,
        ParmValue& value,
        ParmDBRep::TableType,
        bool overwriteMask = true);

    // Put the value for the given parameters and domain.
    // It only writes the parameters that have the same DBSeqNr as this ParmDB.
    // overwriteMask=true indicates that the solvableMask might be changed
    // and should be overwritten in an existing record.
    virtual void putValues(std::map<std::string,ParmValueSet>& values,
        ParmDBRep::TableType,
        bool overwriteMask = true);

    // Delete the records for the given parameters and domain.
    // If parentId>=0, only records with that parentid will be deleted.
    virtual void deleteValues(const std::string& parmNamePattern,
        const ParmDomain& domain,
        int parentId,
        ParmDBRep::TableType);

    // Get the default value for the given parameters.
    // Only * and ? should be used in the pattern (no [] and {}).
    virtual void getDefValues(std::map<std::string,ParmValueSet>& result,
        const std::string& parmNamePattern);

    // Put the default value.
    virtual void putDefValue(const std::string& parmName,
        const ParmValue& value);

    // Delete the default value records for the given parameters.
    virtual void deleteDefValues(const std::string& parmNamePattern);

    // Get the names of all parms matching the given (filename like) pattern.
    virtual std::vector<std::string> getNames(const std::string& pattern,
        ParmDBRep::TableType);

    // Clear database or table
    virtual void clearTables();

private:
    // Fill the map with default values.
    virtual void fillDefMap(std::map<std::string,ParmValue>& defMap);

    static string translatePattern(const string &pattern);

    scoped_ptr<pqxx::connection>    itsConnection;
};


} //# namespace BBS
} //# namespace LOFAR

//# Include function templates.
#include <ParmDB/ParmDBPostgres.tcc>

#endif
#endif

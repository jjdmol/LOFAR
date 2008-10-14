//# SourceDBCasa.h: Class for a Casa table holding sources and their parameters
//#
//# Copyright (C) 2008
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

// @file
// @brief Base class for a table holding sources and their parameters
// @author Ger van Diepen (diepen AT astron nl)

#ifndef LOFAR_PARMDB_SOURCEDBCASA_H
#define LOFAR_PARMDB_SOURCEDBCASA_H

//# Includes
#include <ParmDB/SourceDB.h>
#include <Common/lofar_vector.h>
#include <tables/Tables/Table.h>


namespace LOFAR {
namespace BBS {

  // @ingroup ParmDB
  // @{

  // @brief Class for a Casa table holding source parameters.
  class SourceDBCasa : public SourceDBRep
  {
  public:
    SourceDBCasa (const ParmDBMeta& pdm, bool forceNew);

    virtual ~SourceDBCasa();

    // Writelock and unlock the database tables.
    // The user does not need to lock/unlock, but it can increase performance
    // if many small accesses have to be done.
    // The default implementation does nothing.
    // <group>
    virtual void lock (bool lockForWrite);
    virtual void unlock();
    // </group>

    // Delete the records for the given parameters and domain.
    virtual void deleteValues (const string& sourceNamePattern);

    // Get the Cat-1 patch names in order of decreasing apparent flux.
    virtual vector<string> getCat1Patches();

    // Get the sources belonging to the given patch.
    virtual vector<SourceInfo> getPatchSources (const string& patchName);

    // Get the source type of the given source.
    virtual SourceInfo getSource (const string& sourceName);

    // Get the info of all sources matching the given (filename like) pattern.
    virtual vector<SourceInfo> getSources (const string& pattern);

    // Delete the sources records matching the given (filename like) pattern.
    virtual void deleteSources (const std::string& sourceNamePattern);

    // Clear database or table
    virtual void clearTables();

  private:
    // Create the source and patch table.
    void createTables (const string& tableName);

    //# Data members
    casa::Table itsPatchTable;
    casa::Table itsSourceTable;
  };

  // @}

} // namespace BBS
} // namespace LOFAR

#endif

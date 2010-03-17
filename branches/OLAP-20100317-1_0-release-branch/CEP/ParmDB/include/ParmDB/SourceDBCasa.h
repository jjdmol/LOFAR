//# SourceDBCasa.h: Class for a Casa table holding sources and their parameters
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
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

// @file
// @brief Base class for a table holding sources and their parameters
// @author Ger van Diepen (diepen AT astron nl)

#ifndef LOFAR_PARMDB_SOURCEDBCASA_H
#define LOFAR_PARMDB_SOURCEDBCASA_H

//# Includes
#include <ParmDB/SourceDB.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_set.h>
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

    // Check for duplicate patches or sources.
    // An exception is thrown if that is the case.
    virtual void checkDuplicates();

    // Find non-unique patch names.
    virtual vector<string> findDuplicatePatches();

    // Find non-unique source names.
    virtual vector<string> findDuplicateSources();

    // Test if the patch already exists.
    virtual bool patchExists (const string& patchName);

    // Test if the source already exists.
    virtual bool sourceExists (const string& sourceName);

    // Add a patch and return its patchId.
    // Nomally ra and dec should be filled in, but for moving patches
    // (e.g. sun) this is not needed.
    // <br>Optionally it is checked if the patch already exists.
    virtual uint addPatch (const string& patchName, int catType,
                           double apparentBrightness,
                           double ra, double dec,
                           bool check);

    // Add a source to a patch.
    // Its ra and dec and default parameters will be stored as default
    // values in the associated ParmDB tables. The names of the parameters
    // will be preceeded by the source name and a colon.
    // The map should contain the parameters belonging to the source type.
    // Missing parameters will default to 0.
    // <br>Optionally it is checked if the patch already exists.
    virtual void addSource (const string& patchName, const string& sourceName,
                            SourceInfo::Type sourceType,
                            const ParmMap& defaultParameters,
                            double ra, double dec,
                            bool check);

    // Add a source which forms a patch in itself (with the same name).
    // <br>Optionally it is checked if the patch or source already exists.
    virtual void addSource (const string& sourceName, int catType,
                            double apparentBrightness,
                            SourceInfo::Type sourceType,
                            const ParmMap& defaultParameters,
                            double ra, double dec,
                            bool check);

    // Get patch names in order of category and decreasing apparent flux.
    // category < 0 means all categories.
    // A brightness < 0 means no test on brightness.
    virtual vector<string> getPatches (int category, const string& pattern,
                                       double minBrightness,
                                       double maxBrightness);
;
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

    // Add a source for the given patch.
    void addSrc (uint patchId,
                 const string& sourceName,
                 SourceInfo::Type sourceType,
                 const ParmMap& defaultParameters,
                 double ra, double dec);

    // Find the duplicate patches or sources.
    vector<string> findDuplicates (casa::Table& table,
                                   const string& columnName);

    // Fill the patch and source set object from the tables.
    // They serve as a cache to find out if a patch or source name exists.
    void fillSets();

    //# Data members
    casa::Table      itsPatchTable;
    casa::Table      itsSourceTable;
    set<std::string> itsPatchSet;
    set<std::string> itsSourceSet;
    bool             itsSetsFilled;
  };

  // @}

} // namespace BBS
} // namespace LOFAR

#endif

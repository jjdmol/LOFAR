//# SourceDB.h: Base class for a table holding sources and their parameters
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

#ifndef LOFAR_PARMDB_SOURCEDB_H
#define LOFAR_PARMDB_SOURCEDB_H

//# Includes
#include <ParmDB/SourceInfo.h>
#include <ParmDB/ParmDBMeta.h>
#include <ParmDB/ParmDB.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>


namespace LOFAR {
namespace BBS {

  //# Forward Declarations
  class ParmMap;

  // @ingroup ParmDB
  // @{

  // @brief Abstract base class for a table holding source parameters.
  class SourceDBRep
  {
  public:
    // This creates the underlying ParmDB object.
    SourceDBRep (const ParmDBMeta& ptm, bool forceNew);

    virtual ~SourceDBRep();

    // Link to the DBRep by incrementing the count.
    void link()
      { itsCount++; }

    // Unlink by decrementing the count.
    int unlink()
      { return --itsCount; }

    // Writelock and unlock the database tables.
    // The user does not need to lock/unlock, but it can increase performance
    // if many small accesses have to be done.
    // The default implementation does nothing.
    // <group>
    virtual void lock (bool lockForWrite);
    virtual void unlock();
    // </group>

    // Get the associated ParmDB.
    ParmDB& getParmDB()
      { return itsParmDB; }

    // Check for duplicate patches or sources.
    // An exception is thrown if that is the case.
    virtual void checkDuplicates() = 0;

    // Find non-unique patch names.
    virtual vector<string> findDuplicatePatches() = 0;

    // Find non-unique source names.
    virtual vector<string> findDuplicateSources() = 0;

    // Test if the patch already exists.
    virtual bool patchExists (const string& patchName) = 0;

    // Test if the source already exists.
    virtual bool sourceExists (const string& sourceName) = 0;

    // Add a patch and return its patchId.
    // Nomally ra and dec should be filled in, but for moving patches
    // (e.g. sun) this is not needed.
    // <br>Optionally it is checked if the patch already exists.
    virtual uint addPatch (const string& patchName, int catType,
                           double apparentBrightness,
                           double ra, double dec,
                           bool check) = 0;

    // Add a source to a patch.
    // Its ra and dec and default parameters will be stored as default
    // values in the associated ParmDB tables. The names of the parameters
    // will be succeeded by a colon and the source name.
    // The map should contain the parameters belonging to the source type.
    // Missing parameters will default to 0.
    // <br>Optionally it is checked if the source already exists.
    virtual void addSource (const string& patchName, const string& sourceName,
                            SourceInfo::Type sourceType,
                            const ParmMap& defaultParameters,
                            double ra, double dec,
                            bool check) = 0;

    // Add a source which forms a patch in itself (with the same name).
    // <br>Optionally it is checked if the patch or source already exists.
    virtual void addSource (const string& sourceName, int catType,
                            double apparentBrightness,
                            SourceInfo::Type sourceType,
                            const ParmMap& defaultParameters,
                            double ra, double dec,
                            bool check) = 0;

    // Get patch names in order of category and decreasing apparent flux.
    // category < 0 means all categories.
    // A brightness < 0 means no test on brightness.
    virtual vector<string> getPatches (int category, const string& pattern,
                                       double minBrightness,
                                       double maxBrightness) = 0;

    // Get the sources belonging to the given patch.
    virtual vector<SourceInfo> getPatchSources (const string& patchName) = 0;

    // Get the source type of the given source.
    virtual SourceInfo getSource (const string& sourceName) = 0;

    // Get the info of all sources matching the given (filename like) pattern.
    virtual vector<SourceInfo> getSources (const string& pattern) = 0;

    // Delete the sources records matching the given (filename like) pattern.
    virtual void deleteSources (const std::string& sourceNamePattern) = 0;

    // Clear database or table
    virtual void clearTables() = 0;
    // Get the name and type of the SourceDB.

    const ParmDBMeta& getParmDBMeta() const
      { return itsParmDB.getParmDBMeta(); }

  private:
    int    itsCount;
    ParmDB itsParmDB;
  };


  // @brief Envelope class for a table holding source parameters
  class SourceDB
  {
  public:
    // Create the SourceDB object for the given database type.
    // It gets added to the map of open sourceDBs.
    explicit SourceDB (const ParmDBMeta& ptm, bool forceNew=false);

    // Copy contructor has reference semantics.
    SourceDB (const SourceDB&);

    // Delete underlying object if no more references to it.
    ~SourceDB()
      { decrCount(); }

    // Assignment has reference semantics.
    SourceDB& operator= (const SourceDB&);

    // Lock and unlock the database tables.
    // The user does not need to lock/unlock, but it can increase performance
    // if many small accesses have to be done.
    // <group>
    void lock (bool lockForWrite = true)
      { itsRep->lock (lockForWrite); }
    void unlock()
      { itsRep->unlock(); }

    // Get the associated ParmDB.
    ParmDB& getParmDB()
      { return itsRep->getParmDB(); }

    // Check for duplicate patches or sources.
    // An exception is thrown if that is the case.
    void checkDuplicates() const
      { itsRep->checkDuplicates(); }

    // Find non-unique patch names.
    vector<string> findDuplicatePatches()
      { return itsRep->findDuplicatePatches(); }

    // Find non-unique source names.
    vector<string> findDuplicateSources()
      { return itsRep->findDuplicateSources(); }

    // Test if the patch already exists.
    bool patchExists (const string& patchName) const
      { return itsRep->patchExists (patchName); }

    // Test if the source already exists.
    bool sourceExists (const string& sourceName) const
      { return itsRep->sourceExists (sourceName); }

    // Add a patch and return its patchId.
    // Nomally ra and dec should be filled in, but for moving patches
    // (e.g. sun) this is not needed.
    // <br>Optionally it is checked if the patch already exists.
    uint addPatch (const string& patchName, int catType,
                   double apparentBrightness,
                   double ra=-1e9, double dec=-1e9,
                   bool check = true)
      { return itsRep->addPatch (patchName, catType, apparentBrightness,
                                 ra, dec, check); }

    // Add a source to a patch.
    // Its ra and dec and default parameters will be stored as default
    // values in the associated ParmDB tables. The names of the parameters
    // will be succeeded by a colon and the source name.
    // The map should contain the parameters belonging to the source type.
    // Not all parameters need to be present. The ParmDB classes will
    // use a default of 0 for missing ones.
    void addSource (const string& patchName, const string& sourceName,
                    SourceInfo::Type sourceType,
                    const ParmMap& defaultParameters,
                    double ra=-1e9, double dec=-1e9,
                    bool check = true)
      { itsRep->addSource (patchName, sourceName, sourceType,
                           defaultParameters, ra, dec, check); }

    // Add a source which forms a patch in itself (with the same name).
    void addSource (const string& sourceName, int catType,
                    double apparentBrightness,
                    SourceInfo::Type sourceType,
                    const ParmMap& defaultParameters,
                    double ra=-1e9, double dec=-1e9,
                    bool check = true)
      { itsRep->addSource (sourceName, catType, apparentBrightness, sourceType,
                           defaultParameters, ra, dec, check); }

    // Get patch names in order of category and decreasing apparent flux.
    // category < 0 means all categories.
    // A brightness < 0 means no test on brightness.
    vector<string> getPatches (int category=-1, const string& pattern="",
                               double minBrightness=-1, double maxBrightness=-1)
     { return itsRep->getPatches (category, pattern,
                                  minBrightness, maxBrightness); }

    // Get the info of the sources belonging to the given patch.
    vector<SourceInfo> getPatchSources (const string& patchName)
      { return itsRep->getPatchSources (patchName); }

    // Get the source info of the given source.
    SourceInfo getSource (const string& sourceName)
      { return itsRep->getSource (sourceName); }

    // Get the info of all sources matching the given (filename like) pattern.
    vector<SourceInfo> getSources (const string& sourceNamePattern)
      { return itsRep->getSources (sourceNamePattern); }

    // Delete the sources records matching the given (filename like) pattern.
    void deleteSources (const std::string& sourceNamePattern)
      { itsRep->deleteSources (sourceNamePattern); }

    // Clear database tables (i.e. remove all rows from all tables).
    void clearTables()
      { itsRep->clearTables(); }

    // Get the name and type of the SourceDB.
    const ParmDBMeta& getParmDBMeta() const
      { return itsRep->getParmDBMeta(); }

  private:
    // Create a SourceDB object for an existing SourceDBRep.
    SourceDB (SourceDBRep*);

    // Decrement the refcount and delete if zero.
    void decrCount();

    SourceDBRep* itsRep;
  };

  // @}

} // namespace BBS
} // namespace LOFAR

#endif

//# SourceDB.h: Abstract base class to hold sources in a data base.
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

#ifndef LOFAR_SOURCEDB_SOURCEDB_H
#define LOFAR_SOURCEDB_SOURCEDB_H

// \file
// Abstract base class to hold sources in a table.

//# Includes
#include <SourceDB/SourceValue.h>
#include <SourceDB/SourceDBMeta.h>
#include <ParmDB/ParmDB.h>
#include <list>
#include <vector>


namespace LOFAR {
namespace SourceDB {

//# Forward Declarations.
class SourceDB;

// \ingroup SourceDB
// @{

class SourceDBRep
{
public:
  SourceDBRep()
    : itsCount(0)
  {}

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

  // Get the sources with the given names.
  // If the vector is empty, all names are retrieved.
  virtual std::list<SourceValue> getSources
                     (const std::vector<std::string>& sourceNames,
		      const ParmDB::ParmDomain&) = 0;

  // Get the sources in a circle around the given position.
  // The position and radius have to be given in radians.
  virtual std::list<SourceValue> getSources (double ra, double dec,
					     double radius,
					     const ParmDB::ParmDomain&,
					     double minFluxI = -1e30,
					     double maxFluxI = -1e30,
					     double minSpInx = -1e30,
					     double maxSpInx = -1e30) = 0;

  // Add the given sources.
  virtual void addSources (const std::list<SourceValue>& values) = 0;

  // Update the values of the given source.
  // It will check that it already exists.
  virtual void updateSource (const SourceValue& value) = 0;

  // Delete the given source record.
  virtual void deleteSource (const SourceValue&) = 0;

  // Delete the sources matching the given (filename like) pattern.
  virtual void deleteSources (const std::string& sourceNamePattern,
			      const ParmDB::ParmDomain&) = 0;

  // Get the names of all sources matching the given (filename like) pattern.
  virtual std::vector<std::string> getNames (const std::string& pattern) = 0;

  // Clear the source data base tables.
  virtual void clearTables() = 0;

  // Find matching records from the other data base in this one.
  // The records of other are written into the appropriate result data base.
  // Also the matching records of this are written.
  // This is useful to merge an LSM back into the GSM.
  // Before doing the actual merge, the matches and non-matches can be
  // inspected and possibly moved from match to non-match or vice-versa.
  virtual void match (const SourceDB& other,
		      SourceDB& match, SourceDB& nonMatch) = 0;

  // Merge the other source data base into this one.
  virtual void merge (const SourceDB& otherMatch,
		      const SourceDB& otherNonMatch) = 0;

  // Set or get the name and type.
  // <group>
  void setSourceDBMeta (const SourceDBMeta& ptm)
    { itsPTM = ptm; }
  const SourceDBMeta& getSourceDBMeta() const
    { return itsPTM; }
  // </group>

private:
  int          itsCount;
  SourceDBMeta itsPTM;
};


  // Actions to perform on source DB:
  //   get one or more sources based on name or cone
  //   get A-team
  //   match 2 catalogs
  //   extract LSM from GSM
  //   merge LSM back into GSM
  //   generate ParmDB from LSM
  //   update LSM from ParmDB
  //   add/delete/update sources in LSM
class SourceDB
{
public:
  // Create or open the SourceDB object for the given database type.
  // It is created if not existing or if forceNew=true.
  explicit SourceDB (const SourceDBMeta& ptm, bool forceNew=false);

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

  // Get the sources with the given names.
  // If the vector is empty, all names are retrieved.
  std::list<SourceValue> getSources
                  (const std::vector<std::string>& sourceNames,
		   const ParmDB::ParmDomain& domain)
    { return itsRep->getSources (sourceNames, domain); }

  // Get the sources in a circle around the given position.
  // The position and radius have to be given in radians.
  std::list<SourceValue> getSources (double ra, double dec, double radius,
				     const ParmDB::ParmDomain& domain)
    { return itsRep->getSources (ra, dec, radius, domain); }

  // Add the given sources.
  void addSources (std::list<SourceValue> values)
    { itsRep->addSources (values); }

  // Add a single given source.
  void addSource (const SourceValue& value);

  // Update the values of the given source.
  // It will check that it already exists.
  void updateSource (const SourceValue& value)
    { itsRep->updateSource (value); }

  // Delete the given source record.
  // Nothing is done if not found.
  void deleteSource (const SourceValue& sourceValue)
    { itsRep->deleteSource (sourceValue); }

  // Delete the sources matching the given (filename like) pattern.
  // Nothing is done if none found.
  void deleteSources (const std::string& sourceNamePattern,
		      const ParmDB::ParmDomain& domain)
    { itsRep->deleteSources (sourceNamePattern, domain); }

  // Get the names of all sources matching the given (filename like) pattern.
  std::vector<std::string> getNames (const std::string& pattern)
    { return itsRep->getNames (pattern); }

  // Clear the source data base tables.
  void clearTables()
    { itsRep->clearTables(); }

  // Find matching records from the other data base in this one.
  // The records of other are written into the appropriate result data base.
  // Also the matching records of this are written.
  // This is useful to merge an LSM back into the GSM.
  // Before doing the actual merge, the matches and non-matches can be
  // inspected and possibly moved from match to non-match or vice-versa.
  void match (const SourceDB& other, SourceDB& match, SourceDB& nonMatch)
    { itsRep->match (other, match, nonMatch); }

  // Merge the other source data base into this one.
  void merge (const SourceDB& otherMatch, const SourceDB& otherNonMatch)
    { itsRep->merge (otherMatch, otherNonMatch); }

  // Create a parm data base from the source data base.
  void createParmDB (ParmDB::ParmDB&);

  // Merge a parm data base back into this source data base.
  // It updates possibly changed parameters and adds new sources.
  void mergeParmDB (const ParmDB::ParmDB&);

private:
  // Create a SourceDB object for an existing SourceDBRep.
  SourceDB (SourceDBRep*);

  // Decrement the refcount and delete if zero.
  void decrCount();

  // Put a single parameter.
  void putParm (ParmDB::ParmDB& parmdb, ParmDB::ParmValue& pv,
		const std::string& sourceName,
		const std::string& parm,
		const ParmDB::ParmDomain& domain,
		double value, bool relativePert);

  //# Data Members
  SourceDBRep* itsRep;
};

// @}

} // namespace SourceDB
} // namespace LOFAR

#endif

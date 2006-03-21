//# ParmDB.h: Object to hold parameters in a table.
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

#ifndef LOFAR_PARMDB_PARMDB_H
#define LOFAR_PARMDB_PARMDB_H

// \file
// Object to hold parameters in a table.

//# Includes
#include <ParmDB/ParmValue.h>
#include <ParmDB/ParmDBMeta.h>
#include <vector>
#include <map>


namespace LOFAR {
namespace ParmDB {

//# Forward Declarations
class ParmDomain;


// \ingroup ParmDB
// @{

class ParmDBRep
{
public:
  // Define the types for the tables to use.
  enum TableType {
    // Use the normal table containing the most recent values.
    UseNormal,
    // Use the history table containing the old values (after a refit, etc.).
    UseHistory,
  };

  ParmDBRep()
    : itsCount(0), itsSeqNr(-1), itsDefFilled (false)
  {}

  virtual ~ParmDBRep();

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

  // Get the parameter values for the given parameter and domain.
  // Note that the requested domain may contain multiple values.
  // A selection on parentId is done if >= 0.
  virtual ParmValueSet getValues (const std::string& parmName,
				  const ParmDomain& domain,
				  int parentId,
				  ParmDBRep::TableType) = 0;

  // Get all values for a given domain and set of parm names.
  // If the parm name vector is empty, all parm names are taken.
  virtual void getValues (std::map<std::string,ParmValueSet>& result,
			  const std::vector<std::string>& parmNames,
			  const ParmDomain& domain,
			  int parentId,
			  ParmDBRep::TableType) = 0;

  // Get the parameter values for the given parameters and domain.
  // Only * and ? should be used in the pattern (no [] and {}).
  // A selection on parentId is done if >= 0.
  virtual void getValues (std::map<std::string,ParmValueSet>& result,
			  const std::string& parmNamePattern,
			  const ParmDomain& domain,
			  int parentId,
			  ParmDBRep::TableType) = 0;

  // Put the value for the given parameter and domain.
  // overwriteMask=true indicates that the solvableMask might be changed
  // and should be overwritten in an existing record.
  virtual void putValue (const std::string& parmName,
			 ParmValue& value,
			 ParmDBRep::TableType,
			 bool overwriteMask = true) = 0;

  // Put the value for the given parameters and domain.
  // It only writes the parameters that have the same DBSeqNr as this ParmDB.
  // overwriteMask=true indicates that the solvableMask might be changed
  // and should be overwritten in an existing record.
  virtual void putValues (std::map<std::string,ParmValueSet>& values,
			  ParmDBRep::TableType,
			  bool overwriteMask = true) = 0;

  // Delete the records for the given parameters and domain.
  // If parentId>=0, only records with that parentid will be deleted.
  virtual void deleteValues (const std::string& parmNamePattern,
			     const ParmDomain& domain,
			     int parentId,
			     ParmDBRep::TableType) = 0;

  // Get the default value for the given parameter.
  ParmValue getDefValue (const std::string& parmName);

  // Get the default value for the given parameters.
  // Only * and ? should be used in the pattern (no [] and {}).
  virtual void getDefValues (std::map<std::string,ParmValueSet>& result,
			     const std::string& parmNamePattern) = 0;

  // Put the default value.
  virtual void putDefValue (const std::string& parmName,
			    const ParmValue& value) = 0;

  // Delete the default value records for the given parameters.
  virtual void deleteDefValues (const std::string& parmNamePattern) = 0;

  // Get the names of all parms matching the given (filename like) pattern.
  virtual std::vector<std::string> getNames (const std::string& pattern,
					     ParmDBRep::TableType) = 0;

  // Clear database or table
  virtual void clearTables() = 0;

  // Set or get the name and type.
  // <group>
  void setParmDBMeta (const ParmDBMeta& ptm)
    { itsPTM = ptm;};
  const ParmDBMeta& getParmDBMeta() const
    { return itsPTM; };
  // </group>

  // Set or get ParmDB sequence nr.
  // <group>
  void setParmDBSeqNr (int seqnr)
    { itsSeqNr = seqnr; }
  int getParmDBSeqNr() const
    { return itsSeqNr; }
  // </group>

  // Set the default value map to being not filled.
  // This is needed after a delete, etc.
  void clearDefFilled()
    { itsDefFilled = false; }

private:
  // Fill the map with default values.
  virtual void fillDefMap (std::map<std::string,ParmValue>& defMap) = 0;

  int        itsCount;
  ParmDBMeta itsPTM;
  int        itsSeqNr;
  bool       itsDefFilled;
  std::map<std::string,ParmValue> itsDefValues;
};



class ParmDB
{
public:
  // Create the ParmDB object for the given database type.
  // It gets added to the map of open parmDBs.
  explicit ParmDB (const ParmDBMeta& ptm, bool forceNew=false);

  // Copy contructor has reference semantics.
  ParmDB (const ParmDB&);

  // Delete underlying object if no more references to it.
  ~ParmDB()
    { decrCount(); }

  // Assignment has reference semantics.
  ParmDB& operator= (const ParmDB&);

  // Lock and unlock the database tables.
  // The user does not need to lock/unlock, but it can increase performance
  // if many small accesses have to be done.
  // <group>
  void lock (bool lockForWrite = true)
    { itsRep->lock (lockForWrite); }
  void unlock()
    { itsRep->unlock(); }

  // Get the parameter values for the given parameter and domain.
  // The matchDomain argument is set telling if the found parameter
  // matches the domain exactly.
  // Note that the requested domain may contain multiple values.
  // A selection on parentId is done if >= 0.
  ParmValueSet getValues (const std::string& parmName,
			  const ParmDomain& domain,
			  int parentId = 0,
			  ParmDBRep::TableType tableType=ParmDBRep::UseNormal)
    { return itsRep->getValues (parmName, domain, parentId, tableType); }

  // Get the parameter values for the given parameters and domain.
  // An element in the returned vector belongs to the corresponding element
  // in the parmNames vector.
  // A selection on parentId is done if >= 0.
  void getValues (std::map<std::string,ParmValueSet>& result,
		  const std::vector<std::string>& parmNames,
		  const ParmDomain& domain,
		  int parentId = 0,
		  ParmDBRep::TableType tableType=ParmDBRep::UseNormal)
    { itsRep->getValues (result, parmNames, domain,
			 parentId, tableType); }

  // Get the parameter values for the given parameters and domain.
  // Only * and ? should be used in the pattern (no [] and {}).
  // A selection on parentId is done if >= 0.
  void getValues (std::map<std::string,ParmValueSet>& result,
		  const std::string& parmNamePattern,
		  const ParmDomain& domain,
		  int parentId = 0,
		  ParmDBRep::TableType tableType=ParmDBRep::UseNormal)
    { itsRep->getValues (result, parmNamePattern, domain,
			 parentId, tableType); }

  // Put the value for the given parameter and domain.
  // If it is a new domain, the value ID might be set.
  void putValue (const std::string& parmName,
		 ParmValue& value,
		 ParmDBRep::TableType tableType=ParmDBRep::UseNormal)
    { itsRep->putValue (parmName, value, tableType); }

  // Put the value for the given parameters and domain.
  // It only writes the parameters that have the same DBSeqNr as this ParmDB.
  void putValues (std::map<std::string,ParmValueSet>& values,
		  ParmDBRep::TableType tableType=ParmDBRep::UseNormal)
    { itsRep->putValues (values, tableType); }

  // Delete the records for the given parameters and domain.
  // If parentId>=0, only records with that parentid will be deleted.
  void deleteValues (const std::string& parmNamePattern,
		     const ParmDomain& domain,
		     int parentId = -1,
		     ParmDBRep::TableType tableType=ParmDBRep::UseNormal)
    { itsRep->deleteValues (parmNamePattern, domain, parentId, tableType); }

  // Get the initial value for the given parameter.
  ParmValue getDefValue (const std::string& parmName)
    { return itsRep->getDefValue (parmName); }

  // Get the default value for the given parameters.
  // Only * and ? should be used in the pattern (no [] and {}).
  void getDefValues (std::map<std::string,ParmValueSet>& result,
		     const std::string& parmNamePattern)
    { itsRep->getDefValues (result, parmNamePattern); }

  // Put the default value for the given parameter.
  void putDefValue (const std::string& parmName, const ParmValue& value)
    { itsRep->putDefValue (parmName, value); }

  // Delete the default value records for the given parameters.
  void deleteDefValues (const std::string& parmNamePattern)
    { itsRep->deleteDefValues (parmNamePattern); }

  // Get the names of all sources in the table.
  std::vector<std::string> getNames (const std::string& pattern,
		 ParmDBRep::TableType tableType = ParmDBRep::UseNormal)
    { return itsRep->getNames (pattern, tableType); }

  // Clear database or table
  void clearTables()
    { itsRep->clearTables(); }

  // Get the name and type.
  const ParmDBMeta& getParmDBMeta() const
    { return itsRep->getParmDBMeta(); }

  // Get ParmDB sequence nr.
  int getParmDBSeqNr() const
    { return itsRep->getParmDBSeqNr(); }

  // Get the ParmDB object of the opened database for the given index.
  // An exception is thrown if not found.
  static ParmDB getParmDB (uint index);

private:
  // Create a ParmDB object for an existing ParmDBRep.
  ParmDB (ParmDBRep*);

  // Decrement the refcount and delete if zero.
  void decrCount();

  ParmDBRep* itsRep;

  //# Keep a list of all open ParmDBs.
  static std::map<std::string,int> theirDBNames;
  static std::vector<ParmDBRep*>   theirParmDBs;
};

// @}

} // namespace ParmDB
} // namespace LOFAR

#endif

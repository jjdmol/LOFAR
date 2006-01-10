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
    {}

  virtual ~ParmDBRep();

  // Get the parameter values for the given parameter and domain.
  // Note that the requested domain may contain multiple values.
  // A selection on parentId is done if >= 0.
  virtual ParmValueSet getValues (const std::string& parmName,
				  const ParmDomain& domain,
				  int parentId,
				  ParmDBRep::TableType) = 0;

  // Get the parameter values for the given parameters and domain.
  // An element in the returned vector belongs to the corresponding element
  // in the parmNames vector.
  // A selection on parentId is done if >= 0.
  virtual std::vector<ParmValueSet>
      getValues (const std::vector<std::string>& parmNames,
		 const ParmDomain& domain,
		 int parentId,
		 ParmDBRep::TableType) = 0;
  
  // Get the parameter values for the given parameters and domain.
  // Only * and ? should be used in the pattern (no [] and {}).
  // A selection on parentId is done if >= 0.
  virtual std::vector<ParmValueSet>
      getPatternValues (const std::string& parmNamePattern,
			const ParmDomain& domain,
			int parentId,
			ParmDBRep::TableType) = 0;

  // Put the value for the given parameter and domain.
  // If it is a new domain, value's ParmValue::DBRef might be set.
  virtual void putValue (const std::string& parmName,
			 ParmValue& value,
			 ParmDBRep::TableType) = 0;

  // Put the value for the given parameters and domain.
  virtual void putValues (std::vector<ParmValueSet>& values,
			  ParmDBRep::TableType) = 0;

  // Delete the records for the given parameters and domain.
  // If parentId>=0, only records with that parentid will be deleted.
  virtual void deleteValues (const std::string& parmNamePattern,
			     const ParmDomain& domain,
			     int parentId,
			     ParmDBRep::TableType) = 0;

  // Get the default value for the given parameter.
  virtual ParmValue getDefValue (const std::string& parmName) = 0;

  // Get the default value for the given parameters.
  // Only * and ? should be used in the pattern (no [] and {}).
  virtual std::vector<ParmValueSet>
               getPatternDefValues (const std::string& parmNamePattern) = 0;

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

private:
  ParmDBMeta itsPTM;
};



class ParmDB
{
public:
  // Create the ParmDB object for the given database type.
  explicit ParmDB (const ParmDBMeta& ptm, bool forceNew=false);

  ~ParmDB()
    { delete itsRep; }

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
  std::vector<ParmValueSet>
      getValues (const std::vector<std::string>& parmNames,
		 const ParmDomain& domain,
		 int parentId = 0,
		 ParmDBRep::TableType tableType=ParmDBRep::UseNormal)
    { return itsRep->getValues (parmNames, domain, parentId, tableType); }

  // Get the parameter values for the given parameters and domain.
  // An element in the returned vector belongs to the corresponding element
  // in the parmNames vector.
  // Only * and ? should be used in the pattern (no [] and {}).
  // A selection on parentId is done if >= 0.
  std::vector<ParmValueSet>
      getPatternValues (const std::string& parmNamePattern,
			const ParmDomain& domain,
			int parentId = 0,
			ParmDBRep::TableType tableType=ParmDBRep::UseNormal)
    { return itsRep->getPatternValues (parmNamePattern, domain,
				       parentId, tableType); }

  // Put the value for the given parameter and domain.
  // If it is a new domain, the value ID might be set.
  void putValue (const std::string& parmName,
		 ParmValue& value,
		 ParmDBRep::TableType tableType=ParmDBRep::UseNormal)
    { itsRep->putValue (parmName, value, tableType); }

  // Put the value for the given parameters and domain.
  void putValues (std::vector<ParmValueSet>& values,
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
  std::vector<ParmValueSet>
                    getPatternDefValues (const std::string& parmNamePattern)
    { return itsRep->getPatternDefValues (parmNamePattern); }

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

private:
  // Forbid copy and assignment.
  ParmDB (const ParmDB&);
  ParmDB& operator= (const ParmDB&);

  ParmDBRep* itsRep;
};

// @}

} // namespace ParmDB
} // namespace LOFAR

#endif

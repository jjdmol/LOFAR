//# ParmDBAIPS.h: Object to hold parameters in an AIPS++ table.
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

#ifndef LOFAR_PARMDB_PARMDBAIPS_H
#define LOFAR_PARMDB_PARMDBAIPS_H

// \file
// Object to hold parameters in an AIPS++ table.

//# Includes
#include <ParmDB/ParmDB.h>
#include <casa/Arrays/Array.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ColumnsIndex.h>
#include <casa/Containers/RecordField.h>

namespace LOFAR {
namespace ParmDB {

// \ingroup ParmDB
// @{

class ParmDBAIPS : public ParmDBRep
{
public:
  ParmDBAIPS (const std::string& tableName, bool forceNew);

  virtual ~ParmDBAIPS();

  // Writelock and unlock the table.
  // It is not necessary to do this, but it can be useful if many
  // small accesses have to be done.
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
				  ParmDBRep::TableType);

  // Get all values for a given domain and set of parm names.
  // If the parm name vector is empty, all parm names are taken.
  virtual void getValues (std::map<std::string,ParmValueSet>& result,
			  const std::vector<std::string>& parmNames,
			  const ParmDomain& domain,
			  int parentId,
			  ParmDBRep::TableType);

  // Get the parameter values for the given parameters and domain.
  // Only * and ? should be used in the pattern (no [] and {}).
  // A selection on parentId is done if >= 0.
  virtual void getValues (std::map<std::string,ParmValueSet>& result,
			  const std::string& parmNamePattern,
			  const ParmDomain& domain,
			  int parentId,
			  ParmDBRep::TableType);

  // Put the value for the given parameter and domain.
  // overwriteMask=true indicates that the solvableMask might be changed
  // and should be overwritten in an existing record.
  virtual void putValue (const std::string& parmName,
			 ParmValue& value,
			 ParmDBRep::TableType,
			 bool overwriteMask = true);

  // Put the value for the given parameters and domain.
  // It only writes the parameters that have the same DBSeqNr as this ParmDB.
  // overwriteMask=true indicates that the solvableMask might be changed
  // and should be overwritten in an existing record.
  virtual void putValues (std::map<std::string,ParmValueSet>& values,
			  ParmDBRep::TableType,
			  bool overwriteMask = true);

  // Delete the records for the given parameters and domain.
  // If parentId>=0, only records with that parentid will be deleted.
  virtual void deleteValues (const std::string& parmNamePattern,
			     const ParmDomain& domain,
			     int parentId,
			     ParmDBRep::TableType);

  // Get the default value for the given parameters.
  // Only * and ? should be used in the pattern (no [] and {}).
  virtual void getDefValues (std::map<std::string,ParmValueSet>& result,
			     const std::string& parmNamePattern);

  // Put the default value.
  virtual void putDefValue (const std::string& parmName,
			    const ParmValue& value);

  // Delete the default value records for the given parameters.
  virtual void deleteDefValues (const std::string& parmNamePattern);

  // Get the names of all parms matching the given (filename like) pattern.
  virtual std::vector<std::string> getNames (const std::string& pattern,
					     ParmDBRep::TableType);

  // Clear database or table
  virtual void clearTables();

private:
  // Fill the map with default values.
  virtual void fillDefMap (std::map<std::string,ParmValue>& defMap);

  // Create a parmtable with the given name.
  void createTables (const string& tableName);

  // Extract the parm values from a table selection with a single parm name.
  // <group>
  void extractValues (std::map<std::string,ParmValueSet>& result,
		      const casa::Table& tab, int tabinx);
  ParmValue extractDefValue (const casa::Table& sel, int row);
  // </group>

  // Do the actual put of a value.
  void doPutValue (const string& parmName,
		   ParmValueRep& pval,
		   int tabinx,
		   bool overwriteMask);

  // Put the value for an existing parameter/domain.
  void putOldValue (ParmValueRep& pval,
		    casa::Table& table,
		    int rownr,
		    bool overwriteMask);

  // Put the value for a new parameter/domain.
  void putNewValue (const std::string& parmName,
		    ParmValueRep& value,
		    int tabinx);

  // Put a parameter value. Check if it is in a new row or not.
  void putValueCheck (const std::string& parmName,
		      ParmValueRep& value,
		      int tabinx,
		      bool overwriteMask);

  // Put the value for a new default parameter.
  void putNewDefValue (const std::string& parmName, const ParmValueRep& value);

  // Find the table subset containing the parameter values for the
  // requested domain.
  // A selection on parentId is done if >= 0.
  casa::Table find (const std::string& parmName, 
		    const ParmDomain& domain,
		    int parentId,
		    int tabinx);

  // Find the table index for a given type.
  int getTableIndex (ParmDBRep::TableType tableType) const
  {
    return (tableType==ParmDBRep::UseHistory ? 1:0);
  }

  // Create a select expression node on domain and parent id.
  casa::TableExprNode makeExpr (const casa::Table& table,
				const ParmDomain& domain,
				int parentId) const;

  // And two table select expressions, where the first one can be null.
  void andExpr (casa::TableExprNode& expr,
		const casa::TableExprNode& right) const;

  // Helper functions to convert from/to vectors.
  // <group>
  void toVector (std::vector<double>& vec,
		 const casa::Array<double>& arr) const;
  std::vector<double> toVector (const casa::Array<double>& arr) const;
  std::vector<int> toVector (const casa::IPosition& shape) const;
  casa::Array<double> fromVector (const std::vector<double>& vec,
				  const std::vector<int>& shape) const;
  casa::Array<bool> fromVector (const std::vector<bool>& vec,
				const std::vector<int>& shape) const;
  casa::Array<double> fromVector (const std::vector<double>& vec) const;
  // </group>

  casa::Table itsTables[3];    //# normal,old,default
};

// @}

} // namespace ParmDB
} // namespace LOFAR

#endif

//# SourceDBAIPS.h: Class to hold sources in an AIPS++ table.
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

#ifndef LOFAR_SOURCEDB_SOURCEDBAIPS_H
#define LOFAR_SOURCEDB_SOURCEDBAIPS_H

// \file
// Class to hold sources in an AIPS++ table.

//# Includes
#include <SourceDB/SourceDB.h>
#include <tables/Tables/Table.h>

namespace LOFAR {
namespace SourceDB {


// \ingroup SourceDB
// @{

class SourceDBAIPS: public SourceDBRep
{
public:
  SourceDBAIPS (const std::string& tableName, bool forceNew);

  virtual ~SourceDBAIPS();

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
		      const ParmDB::ParmDomain&) ;

  // Get the sources in a circle around the given position.
  // The position and radius have to be given in radians.
  virtual std::list<SourceValue> getSources (double ra, double dec,
					     double radius,
					     const ParmDB::ParmDomain&,
					     double minFluxI = -1e30,
					     double maxFluxI = -1e30,
					     double minSpInx = -1e30,
					     double maxSpInx = -1e30);

  // Add the given sources.
  virtual void addSources (const std::list<SourceValue>& values);

  // Update the values of the given source.
  // It will check that it already exists.
  virtual void updateSource (const SourceValue& value);

  // Delete the given source record.
  virtual void deleteSource (const SourceValue&);

  // Delete the sources matching the given (filename like) pattern.
  virtual void deleteSources (const std::string& sourceNamePattern,
			      const ParmDB::ParmDomain&);

  // Get the names of all sources matching the given (filename like) pattern.
  virtual std::vector<std::string> getNames (const std::string& pattern);

  // Clear the source data base tables.
  virtual void clearTables();

  // Find matching records from the other data base in this one.
  // The records of other are written into the appropriate result data base.
  // Also the matching records of this are written.
  // This is useful to merge an LSM back into the GSM.
  // Before doing the actual merge, the matches and non-matches can be
  // inspected and possibly moved from match to non-match or vice-versa.
  virtual void match (const SourceDB& other,
		      SourceDB& match, SourceDB& nonMatch);

  // Merge the other source data base into this one.
  virtual void merge (const SourceDB& otherMatch,
		      const SourceDB& otherNonMatch);

private:
  // Create the tables.
  void createTables (const std::string& tableName);

  // Set the quantum units for a column.
  void setQuant (casa::TableDesc& td, const casa::String& name,
		 const casa::String& unit);

  // Set the quantum units and measure Epoch for a column.
  void setEpoch (casa::TableDesc& td, const casa::String& name);

  // Append the source values from all rows in the table to the list.
  void extractValues (std::list<SourceValue>& result, const casa::Table& tab);

  // Put the source value into the given table row.
  void putValue (casa::Table& tab, unsigned rownr, const SourceValue& pvalue);

  // Create a select expression node on domain and parent id.
  casa::TableExprNode makeExpr (const casa::Table& table,
				const ParmDB::ParmDomain&) const;

  // And two table select expressions, where the first one can be null.
  void andExpr (casa::TableExprNode& expr,
		const casa::TableExprNode& right) const;


  //# Data members
  casa::Table itsTable;
};

// @}

} // namespace SourceDB
} // namespace LOFAR

#endif

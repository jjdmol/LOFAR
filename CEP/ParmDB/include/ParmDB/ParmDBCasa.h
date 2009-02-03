//# ParmDBCasa.h: Class to hold parameters in a Casa table
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

// @file
// @brief Class to hold parameters in a Casa table
// @author Ger van Diepen (diepen AT astron nl)

#ifndef LOFAR_PARMDB_PARMDBCASA_H
#define LOFAR_PARMDB_PARMDBCASA_H

//# Includes
#include <ParmDB/ParmDB.h>
#include <casa/Arrays/Array.h>
#include <tables/Tables/Table.h>

//# Forward Declarations
namespace casa {
  template<typename T> class ArrayColumn;
  template<typename T> class ROArrayColumn;
}


namespace LOFAR {
namespace BBS {


  // @ingroup ParmDB
  // @{

  // @brief Class to hold parameters in a Casa table
  class ParmDBCasa : public ParmDBRep
  {
  public:
    explicit ParmDBCasa (const std::string& tableName, bool forceNew=false);

    virtual ~ParmDBCasa();

    // Writelock and unlock the table.
    // It is not necessary to do this, but it can be useful if many
    // small accesses have to be done.
    // <group>
    virtual void lock (bool lockForWrite);
    virtual void unlock();
    // </group>

    // Get the domain range (time,freq) of the given parameters in the table.
    // This is the minimum and maximum value of these axes for all parameters.
    // An empty name pattern is the same as * (all parms).
    // <group>
    virtual Box getRange (const std::string& parmNamePattern) const;
    virtual Box getRange (const std::vector<std::string>& parmNames) const;
    // </group>

    // Set the default step values.
    virtual void setDefaultSteps (const vector<double>&);

    // Get the parameter values for the given parameters and domain.
    // The parmids form the indices in the result vector.
    virtual void getValues (vector<ParmValueSet>& values,
                            const vector<uint>& nameIds,
                            const vector<ParmId>& parmIds,
                            const Box& domain);

    // Put the values for the given parameter name and id.
    // If it is a new value, the new rowid will be stored in the ParmValueSet.
    // If it is a new name, the nameId will be filled in.
    virtual void putValues (const string& parmName, int& nameId,
                            ParmValueSet& values);

    // Delete the value records for the given parameters and domain.
    virtual void deleteValues (const std::string& parmNamePattern,
                               const Box& domain);

    // Get the default value for the given parameters.
    // Only * and ? should be used in the pattern (no [] and {}).
    virtual void getDefValues (ParmMap& result,
                               const std::string& parmNamePattern);

    // Put the default value.
    virtual void putDefValue (const string& name, const ParmValueSet& value);

    // Delete the default value records for the given parameters.
    virtual void deleteDefValues (const std::string& parmNamePattern);

    // Get the names of all parms matching the given (filename like) pattern.
    virtual std::vector<std::string> getNames (const std::string& pattern);

    // Get the id of a parameter.
    // If not found in the Names table, it returns -1.
    virtual int getNameId (const std::string& parmName);

    // Clear database or table
    virtual void clearTables();

  private:
    // Fill the map with default values.
    virtual void fillDefMap (ParmMap& defMap);

    // Create a parmtable with the given name.
    void createTables (const std::string& tableName);

    // Get a selection from the NAME table.
    // <group>
    casa::Table getNameSel (const std::string& parmNamePattern) const;
    casa::Vector<uint> getNameIds (const std::string& parmNamePattern) const;
    casa::Vector<uint> getNameIds (const std::vector<std::string>& parmNames) const;
    // </group>

    // Find the minmax range in the table.
    Box findRange (const casa::Table& table) const;

    // Extract the parm values from a table selection with a single parm name.
    // <group>
    //void extractValues (ParmMap& result, const casa::Table& table);
    pair<string,ParmValueSet> extractDefValue (const casa::Table& sel, int row);
    // </group>

    // Do the actual put of a value.
    void doPutValue (const string& parmName, int& nameId,
                     ParmValueSet& parmSet);

    // Put the value for an existing parameter/domain.
    void putOldValue (const ParmValue& parmValue,
                      ParmValue::FunkletType type);

    // Put the value for a new parameter/domain.
    void putNewValue (const string& name, int& nameId, ParmValueSet& parmSet,
                      ParmValue& parmValue, const Box& domain);

    // Put an entry into the NAME table.
    int putName (const string& name, const ParmValueSet& pset);

    // Put the value for a new default parameter.
    void putNewDefValue (const string& parmName, const ParmValueSet& value);

    // Put the begin/end of an irregular axis.
    void putInterval (const Axis& axis, casa::ArrayColumn<double>& col,
                      uint rownr);

    // Form an axis from the interval array in the given row.
    // If no interval array, return a regular axis made from (st,end,n).
    Axis::ShPtr getInterval (casa::ROArrayColumn<double>& col, uint rownr,
                             double st, double end, uint n);

    // Find the table subset containing the parameter values for the
    // requested domain.
    casa::Table find (const std::string& parmName, 
                      const Box& domain);

    // Create a select expression node on domain and parent id.
    casa::TableExprNode makeExpr (const casa::Table& table,
                                  const Box& domain) const;

    // And two table select expressions, where the first one can be null.
    void andExpr (casa::TableExprNode& expr,
                  const casa::TableExprNode& right) const;

    //# Data members
    casa::Table itsTables[3];    //# normal,names,default
  };

  // @}

} // namespace BBS
} // namespace LOFAR

#endif

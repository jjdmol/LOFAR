//# ResultSet.h: A set of Result results
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

#ifndef MEQ_RESULTSET_H
#define MEQ_RESULTSET_H

//# Includes
#include <iostream>
#include <DMI/DataRecord.h>
#include <DMI/DataField.h>
#include <MEQ/Result.h>

#pragma aidgroup Meq
#pragma type #Meq::ResultSet

// This class represents a result of a domain for which an expression
// has been evaluated.

namespace Meq {

class Cells;

class ResultSet : public DataRecord
{
public:
  typedef CountedRef<ResultSet> Ref;

  // ------------------------ CONSTRUCTORS
  // Create a ResultSet with the given number of results.
  // If <0, then the set is marked as a fail
  explicit ResultSet (int nresults=1);
  
  explicit ResultSet (const Request &req);
  
  ResultSet (int nresults,const Request &req);
  ResultSet (const Request &req,int nresults);
  
  // Construct from DataRecord. 
  ResultSet (const DataRecord &other,int flags=DMI::PRESERVE_RW,int depth=0);

  ~ResultSet();

  virtual TypeId objectType () const
  { return TpMeqResultSet; }
  
  // implement standard clone method via copy constructor
  virtual CountedRefTarget* clone (int flags, int depth) const
  { return new ResultSet(*this,flags,depth); }
  
  virtual void privatize (int flags = 0, int depth = 0);
  
  // validate record contents and setup shortcuts to them. This is called 
  // automatically whenever a ResultSet is made from a DataRecord
  // (or when the underlying DataRecord is privatized, etc.)
  virtual void validateContent ();
  

  // ------------------------ CELLS
  // Set or get the cells.
  // Attaches cells object (default as anon). Can also specify DMI::CLONE
  // to copy
  void setCells (const Cells *,int flags = DMI::ANON|DMI::NONSTRICT);
  // Attaches cells object (default is external). 
  void setCells (const Cells &cells,int flags = DMI::EXTERNAL|DMI::NONSTRICT)
  { setCells(&cells,flags); }

  bool hasCells () const
  { return itsCells; }
    
  const Cells& cells() const
  { DbgFailWhen(!itsCells,"no cells in Meq::Result");
    return *itsCells; }

  // ------------------------ RESULTS
  void allocateResults (int nresults);
    
  int numResults () const
    { return itsResults.valid() ? itsResults->size() : 0; }
  
  const Result & resultConst (int i) const
    { return itsResults.deref()[i].as<Result>(); }
  
  Result & result (int i)
    { return itsResults.dewr()[i].as_wr<Result>(); }
  
  Result & setResult (int i,Result *result);
  
  Result & setResult (int i,Result::Ref::Xfer &result);
  
  // creates new result at plane i with the given # of spids
  Result & setNewResult (int i,int nspids=0)
  { 
    Result::Ref resref(new Result(nspids),DMI::ANONWR); 
    return setResult(i,resref); 
  }

  // ------------------------ FAIL RESULTS
  // checks if this ResultSet has any fails in it
  bool hasFails () const;
  // returns the number of fails in the set
  int numFails () const;
  

  // dumps result to stream
  void show (std::ostream&) const;

  static int nctor;
  static int ndtor;
  
protected: 
  // disable public access to some DataRecord methods that would violate the
  // structure of the container
  DataRecord::remove;
  DataRecord::replace;
  DataRecord::removeField;
  
private:

  DataField::Ref itsResults;
  const Cells    *itsCells;
};


} // namespace Meq

#endif

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
#pragma aid Cells Results
#pragma type #Meq::ResultSet

// This class represents a result of a domain for which an expression
// has been evaluated.

namespace Meq {

class Cells;

class ResultSet : public DataRecord
{
public:
  typedef CountedRef<ResultSet> Ref;

  // Create a time,frequency result with the given number of results
  explicit ResultSet (int nresults=1);
  
  // Construct from DataRecord. 
  ResultSet (const DataRecord &other,int flags=DMI::PRESERVE_RW,int depth=0);

  ~ResultSet();

  virtual TypeId objectType () const
  { return TpMeqResultSet; }
  
  // validate record contents and setup shortcuts to them. This is called 
  // automatically whenever a ResultSet is made from a DataRecord
  virtual void validateContent ();
  
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

  int numResults () const
  { 
    return itsResults->size(); 
  }
  
  const Result & resultConst (int i) const
  { 
    return (*itsResults)[i].as<Result>(); 
  }
  
  Result & result (int i)
  { 
    return (*itsResults)[i].as_wr<Result>(); 
  }
  
  void setResult (int i,Result *result);
  
  void setResult (int i,Result::Ref::Xfer &result);
  
  // creates new result at plane i with the given # of spids
  Result & setNewResult (int i,int nspids=0)
  { Result *res = new Result(nspids); setResult(i,res); return *res; }
    

  void show (std::ostream&) const;

  static int nctor;
  static int ndtor;
  
private:
  DataField * itsResults;
  const Cells *itsCells;
};


} // namespace Meq

#endif

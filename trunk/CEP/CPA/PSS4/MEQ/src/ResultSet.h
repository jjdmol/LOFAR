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
#pragma aid Cells Results Fail Node Class Name Origin Line Message
#pragma type #Meq::ResultSet

// This class represents a result of a domain for which an expression
// has been evaluated.

namespace Meq {

class Cells;

class ResultSet : public DataRecord
{
public:
  typedef CountedRef<ResultSet> Ref;

  // Create a ResultSet with the given number of results.
  // If <0, then the set is marked as a fail
  explicit ResultSet (int nresults=1);
  
  // Construct from DataRecord. 
  ResultSet (const DataRecord &other,int flags=DMI::PRESERVE_RW,int depth=0);

  ~ResultSet();

  virtual TypeId objectType () const
  { return TpMeqResultSet; }
  
//  // implement standard clone method via copy constructor
//  virtual CountedRefTarget* clone (int flags, int depth) const
//  { return new ResultSet(*this,flags,depth); }
  
  // validate record contents and setup shortcuts to them. This is called 
  // automatically whenever a ResultSet is made from a DataRecord
  // (or when the underlying DataRecord is privatized, etc.)
  virtual void validateContent ();
  
  // This marks the ResultSet as a FAIL, and adds a fail-record.
  // Results, if any, are cleared, a Fail sub-record is created if necessary.
  // The Fail subrecord can contain multiple fail records.
  void addFail (const DataRecord *rec,int flags=DMI::ANON|DMI::READONLY);
  void addFail (const string &nodename,const string &classname,
                const string &origin,int origin_line,const string &msg);
#if defined(HAVE_PRETTY_FUNCTION)
# define ResultSet_FailLocation __PRETTY_FUNCTION__ "() " __FILE__ 
#elif defined(HAVE_FUNCTION)
# define ResultSet_FailLocation __FUNCTION__ "() " __FILE__ 
#else
# define ResultSet_FailLocation __FILE__ 
#endif

// define a macro for automatically generating the correct fail location 
#define MakeFailResult(res,msg) \
    (res).addFail(name(),objectType().toString(),ResultSet_FailLocation,__LINE__,msg);
    
  // checks if this ResultSet is a fail
  bool isFail () const
  { return itsIsFail; }
  
  int numFails () const;
  
  const DataRecord & getFail (int i=0) const;

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
    return itsResults.valid() ? itsResults->size() : 0;
  }
  
  const Result & resultConst (int i) const
  { 
    DbgFailWhen( isFail(),"ResultSet marked as a fail, can't access result" );
    return itsResults.deref()[i].as<Result>(); 
  }
  
  Result & result (int i)
  { 
    DbgFailWhen( isFail(),"ResultSet marked as a fail, can't access result" );
    return itsResults.dewr()[i].as_wr<Result>(); 
  }
  
  void setResult (int i,Result *result);
  
  void setResult (int i,Result::Ref::Xfer &result);
  
  // creates new result at plane i with the given # of spids
  Result & setNewResult (int i,int nspids=0)
  { 
    DbgFailWhen( isFail(),"ResultSet marked as a fail, can't set result");
    Result *res = new Result(nspids); setResult(i,res); 
    return *res; 
  }

  void show (std::ostream&) const;

  static int nctor;
  static int ndtor;
  
private:
  DataField::Ref itsResults;
  const Cells *itsCells;
  bool itsIsFail;
};


} // namespace Meq

#endif

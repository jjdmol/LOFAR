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

  // ------------------------ CONSTRUCTORS
  // Create a ResultSet with the given number of results.
  // If <0, then the set is marked as a fail
  explicit ResultSet (int nresults=1);
  
  ResultSet (int nresults,const Cells &cells,int flags = DMI::EXTERNAL|DMI::NONSTRICT);
  
  ResultSet (const Cells &cells,int flags = DMI::EXTERNAL|DMI::NONSTRICT);
  
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
  int numResults () const
    { return itsResults.valid() ? itsResults->size() : 0; }
  
  const Result & resultConst (int i) const
    { DbgFailWhen( isFail(),"ResultSet marked as a fail, can't access result");
      return itsResults.deref()[i].as<Result>(); 
    }
  
  Result & result (int i)
    { DbgFailWhen( isFail(),"ResultSet marked as a fail, can't access result" );
      return itsResults.dewr()[i].as_wr<Result>(); 
    }
  
  Result & setResult (int i,Result *result);
  
  Result & setResult (int i,Result::Ref::Xfer &result);
  
  // creates new result at plane i with the given # of spids
  Result & setNewResult (int i,int nspids=0)
  { 
    Result::Ref resref(new Result(nspids),DMI::ANONWR); 
    return setResult(i,resref); 
  }

  // ------------------------ FAIL RECORDS
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

  // macro for automatically generating the correct fail location and adding
  // a fail to the resultset
#define MakeFailResult(res,msg) \
    (res).addFail(name(),objectType().toString(),ResultSet_FailLocation,__LINE__,msg);
    
  // checks if this ResultSet is a fail
  bool isFail () const
  { return itsIsFail; }
  // returns the number of fail records 
  int numFails () const;
  // returns the i-th fail record
  const DataRecord & getFail (int i=0) const;
  
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
  void initNumResults (int nresults);

  DataField::Ref itsResults;
  const Cells    *itsCells;
  bool           itsIsFail;
};


} // namespace Meq

#endif

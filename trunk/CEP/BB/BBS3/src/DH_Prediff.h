//  DH_Prediff.h:  DataHolder
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////

#ifndef BBS3_DH_PREDIFF_H
#define BBS3_DH_PREDIFF_H

#include <Transport/DataHolder.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{

/**
   This class is a DataHolder which contains the difference between measured
   and predicted data and derivatives for each solvable parm (spid) for a
   certain domain [baseline, time, frequency, polarization].
*/
 //# Forward Declarations
class ParmData;

class DH_Prediff: public DataHolder
{
public:

  enum woStatus{New,Assigned,Executed};

  explicit DH_Prediff (const string& name = "dh_prediff");

  DH_Prediff(const DH_Prediff&);

  virtual ~DH_Prediff();

  DataHolder* clone() const;

  // Create a TPO object and set the table name in it.
  virtual void initPO (const string& tableName);

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  dcomplex* getDataPtr();
  void setData(dcomplex* dataPtr, int size);  // Size in number of dcomplex's

  void getParmData(vector<ParmData>& pdata); 
  void setParmData(const vector<ParmData>& pdata);

  int getNResults() const;
  void setNResults(int nr);

  int getNspids() const;
  void setNspids(int nr);

  int getNTimes() const;
  void setNTimes(int nr);

  int getNFreq() const;
  void setNFreq(int nr);

  void dump();

  void clearData();

private:
  /// Forbid assignment.
  DH_Prediff& operator= (const DH_Prediff&);

  // Fill the pointers to the data in the blob.
  virtual void fillDataPointers();

  int* itsNResults;         // The number of baseline-polarization combinations
  int* itsNspids;           // The number of solvable parameter identifiers
  int* itsNTimes;           // The number of times
  int* itsNFreq;            // The number of frequencies

  dcomplex* itsDataPtr;
  ParmData* itsParmDataPtr;

};


inline int DH_Prediff::getNResults() const
{ return *itsNResults;}

inline void DH_Prediff::setNResults(int nr)
{ *itsNResults = nr; }

inline int DH_Prediff::getNspids() const
{ return *itsNspids;}

inline void DH_Prediff::setNspids(int nr)
{ *itsNspids = nr; }

inline int DH_Prediff::getNTimes() const
{ return *itsNTimes; }

inline void DH_Prediff::setNTimes(int nr)
{ *itsNTimes = nr; }

inline int DH_Prediff::getNFreq() const
{ return *itsNFreq; }

inline void DH_Prediff::setNFreq(int nr)
{ *itsNFreq = nr; }

inline dcomplex* DH_Prediff::getDataPtr()
{ return itsDataPtr; }

} // namespace LOFAR

#endif 


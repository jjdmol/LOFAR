//# MeqResultVec.h: A vector of results.
//#
//# Copyright (C) 2005
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

#ifndef MNS_MEQRESULTVEC_H
#define MNS_MEQRESULTVEC_H

// \file MNS/MeqResultVec.h
// A result vector containing multiple results.

//# Includes
#include <BBS3/MNS/MeqResult.h>
#include <Common/lofar_iostream.h>

// This class represents multiple results.
// It is faster than a vector<MeqResult>.

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

class MeqResultVecRep
{
public:
  // Create a time,frequency result for the given number of spids.
  MeqResultVecRep (int size, int nspid);

  ~MeqResultVecRep()
    {}

  MeqResultVecRep* link()
    { itsCount++; return this; }

  static void unlink (MeqResultVecRep* rep)
    { if (rep != 0  &&  --rep->itsCount == 0) delete rep; }

  // Get the result vector.
  vector<MeqResult>& results()
    { return itsResult; }

  void show (ostream&) const;

private:
  // Forbid copy and assignment.
  MeqResultVecRep (const MeqResultVecRep&);
  MeqResultVecRep& operator= (const MeqResultVecRep&);

  int               itsCount;
  vector<MeqResult> itsResult;
};



class MeqResultVec
{
public:
  // Create a time,frequency result for the given number of parameters.
  explicit MeqResultVec (int size, int nspid = 0);

  MeqResultVec (const MeqResultVec&);

  ~MeqResultVec()
    { MeqResultVecRep::unlink (itsRep); }

  MeqResultVec& operator= (const MeqResultVec&);

  // Get the nr of results.
  int nresult() const
    { return itsRep->results().size(); }

  // Get the given result.
  // <group>
  const MeqResult& operator[] (int index) const
    { return itsRep->results()[index]; }
  MeqResult& operator[] (int index)
    { return itsRep->results()[index]; }
  // </group>

  friend ostream& operator<< (ostream& os, const MeqResultVec& result)
    { result.itsRep->show (os); return os; }

private:
  MeqResultVecRep* itsRep;
};

// @}

}

#endif

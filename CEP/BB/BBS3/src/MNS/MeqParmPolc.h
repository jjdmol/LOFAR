//# MeqParmPolc.h: Parameter with polynomial coefficients
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

#if !defined(MNS_MEQPARMPOLC_H)
#define MNS_MEQPARMPOLC_H

// \file MNS/MeqParmPolc.h
// Parameter with polynomial coefficients

//# Includes
#include <BBS3/MNS/MeqParm.h>
#include <BBS3/MNS/MeqPolc.h>
#include <Common/lofar_vector.h>

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

//# Forward declarations
class MeqDomain;
class MeqResult;


// This class contains the coefficients of a 2-dim polynomial.
// The order in time and frequency must be given.
// The nr of coefficients is (1+order(time)) * (1+order(freq)).
// The coefficients are numbered 0..N with the time as the most rapidly
// varying axis.
//
// Note that the domain is scaled between -1 and 1.
// So the coefficients are also valid for that domain.

class MeqParmPolc: public MeqParm
{
public:
  // Create a 2-dim polynomial with order 0 in x and y.
  MeqParmPolc (const string& name, MeqParmGroup*);

  virtual ~MeqParmPolc();

  // Set the polynomials.
  void setPolcs (const vector<MeqPolc>& polcs)
    { itsPolcs = polcs; }

  // Add a polynomial.
  void addPolc (const MeqPolc& polc)
    { itsPolcs.push_back (polc); }

  // Get the polynomials.
  const vector<MeqPolc>& getPolcs() const
    { return itsPolcs; }

  // Read the polcs for the given domain.
  virtual void readPolcs (const MeqDomain& domain);

  // Initialize the parameter for the given domain.
  // It also sets the spids and returns the number of spids found.
  virtual int initDomain (const MeqDomain&, int spidIndex);

  // Get the requested result of the parameter.
  virtual MeqResult getResult (const MeqRequest&);

  // Get the current value of the solvable parameter and store
  // it in the argument.
  virtual void getCurrentValue (MeqMatrix& value) const;

  // Update the parameter coefficients with the new values.
  virtual void update (const MeqMatrix& value);

  // Update the solvable parameter coefficients with the new values.
  // The vector contains all solvable values; it picks out the values
  // at the spid index of this parameter.
  virtual void update (const vector<double>& value);

  // Update the solvable parameter coefficients with the new values
  // in the table.
  // The default implementation throws a "not implemented" exception.
  virtual void updateFromTable();

  // Make the new values persistent.
  virtual void save();

private:
  vector<MeqPolc> itsPolcs;
};

// @}

}

#endif

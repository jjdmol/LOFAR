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

//# Includes
#include <MNS/MeqParm.h>
#include <MNS/MeqPolc.h>
#include <Common/lofar_vector.h>

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
  // The coefficients are solvable and initialized to 1.
  MeqParmPolc (const string& name);

  virtual ~MeqParmPolc();

  // Set the polynomials.
  void setPolcs (const vector<MeqPolc>& polcs)
    { itsPolcs = polcs; }

  // Get the polynomials.
  const vector<MeqPolc>& getPolcs() const
    { return itsPolcs; }

  // Initialize the parameter for the given domain.
  // It also sets the spids and returns the number of spids found.
  virtual int initDomain (const MeqDomain&, int spidIndex);

  // Make parameter solvable, thus perturbed values have to be calculated.
  // spidIndex is the index of the first spid of this parm.
  // It returns the number of spids in this parm.
  virtual void setSolvable (bool solvable);

  // Is the parameter solvable?
  bool isSolvable() const
    { return itsIsSolvable; }

  // Get the requested result of the parameter.
  virtual MeqResult getResult (const MeqRequest&);

  // Update the solvable parameters with the new values.
  virtual void update (const MeqMatrix& value);

  // Make the new values persistent.
  virtual void save();

private:
  vector<MeqPolc> itsPolcs;
  bool            itsIsSolvable;
};


#endif

//# ParmPolc.h: Parameter with polynomial coefficients
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

#ifndef MEQ_PARMPOLC_H
#define MEQ_PARMPOLC_H

//# Includes
#include <MEQ/Parm.h>
#include <MEQ/Polc.h>
#include <MEQ/Result.h>
#include <Common/lofar_vector.h>


namespace MEQ {

//# Forward declarations
class Domain;


// This class contains the coefficients of a 2-dim polynomial.
// The order in time and frequency must be given.
// The nr of coefficients is (1+order(time)) * (1+order(freq)).
// The coefficients are numbered 0..N with the time as the most rapidly
// varying axis.
//
// Note that the domain is scaled between -1 and 1.
// So the coefficients are also valid for that domain.

class ParmPolc: public Parm
{
public:
  // Create a 2-dim polynomial with order 0 in x and y.
  ParmPolc (const string& name);

  virtual ~ParmPolc();

  // Set the polynomials.
  void setPolcs (const vector<Polc>& polcs)
    { itsPolcs = polcs; }

  // Add a polynomial.
  void addPolc (const Polc& polc)
    { itsPolcs.push_back (polc); }

  // Get the polynomials.
  const vector<Polc>& getPolcs() const
    { return itsPolcs; }

  // Initialize the parameter for the given domain.
  // It also sets the spids and returns the number of spids found.
  virtual int initDomain (const Domain&, int spidIndex);

  // Get the requested result of the parameter.
  virtual int getResultImpl (Result::Ref&, const Request&, bool newReq);

  // Get the current values of the solvable parameter and store them
  // in the argument.
  virtual void getInitial (Vells& values) const;

  // Get the current value of the solvable parameter and store
  // it in the argument.
  virtual void getCurrentValue(Vells& value, bool denormalize) const;

  // Update the solvable parameters with the new values.
  virtual void update (const Vells& value);

  // Make the new values persistent.
  virtual void save();

private:
  vector<Polc> itsPolcs;
};


} // namespace MEQ


#endif

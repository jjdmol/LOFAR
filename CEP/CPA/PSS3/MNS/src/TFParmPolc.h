//# TFPolc.h: Parameter with polynomial coefficients
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

#if !defined(MNS_TFPARMPOLC_H)
#define MNS_TFPARMPOLC_H

//# Includes
#include <MNS/TFParm.h>
#include <Common/lofar_vector.h>

//# Forward declarations
class TFDomain;
class TFRange;


// This class contains the coefficients of a 2-dim polynomial.
// The order in time and frequency must be given.
// The nr of coefficients is (1+order(time)) * (1+order(freq)).
// The coefficients are numbered 0..N with the time as the most rapidly
// varying axis.

class TFParmPolc: public TFParm
{
public:
  // Create a 2-dim polynomial with given orders.
  // All coefficients are solvable and initialized to 1.
  TFParmPolc (unsigned int type, unsigned int orderX, unsigned int orderY);

  // Create a 2-dim polynomial with given initial values.
  // The shape of the matrix defines the polynomial orders.
  // False in the mask means that the coefficient is not solvable.
  TFParmPolc (unsigned int type, const Matrix<double>& values);
  TFParmPolc (unsigned int type, const Matrix<double>& values,
	      const Matrix<bool>& mask);

  virtual ~TFParmPolc();

  // Get the nr of coefficients.
  unsigned int ncoeff() const
    { return itsCurCoeff.size(); }

  // Initialize the parameter for the given domain.
  virtual void init (const TFDomain&);

  // Make parameter solvable, thus perturbed values have to be calculated.
  // spidIndex is index the first spid of this parm.
  // It returns the number of spids in this parm.
  virtual int setSolvable (int spidIndex);

  // Make the parameter non-solvable.
  virtual void clearSolvable();

  // Get the requested range of the parameter.
  virtual TFRange getRange (const TFRequest&);

  // Update the solvable parameter with the new value.
  virtual void update (const MnsMatrix& value);

  // Make the new value persistent (for the given domain).
  virtual void save (const TFDomain&);

private:
  int            itsNx;
  int            itsNy;
  vector<double> itsInitialCoeff;
  vector<double> itsCurCoeff;
  vector<bool>   itsMask;
  vector<int>    itsSpidInx;     //# -1 is not solvable
};


#endif

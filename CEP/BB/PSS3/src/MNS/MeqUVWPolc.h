//# MeqUVWPolc.h: Class to calculate the UVW from a fitted polynomial
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

#if !defined(MNS_MEQUVWPOLC_H)
#define MNS_MEQUVWPOLC_H

//# Includes
#include <PSS3/MNS/MeqResult.h>
#include <PSS3/MNS/MeqRequestId.h>
#include <PSS3/MNS/MeqParmPolc.h>
#include <scimath/Functionals/Polynomial.h>
#include <scimath/Mathematics/AutoDiff.h>
#include <scimath/Fitting/LinearFit.h>
#include <Common/lofar_string.h>

namespace LOFAR {

//# Forward Declarations
class MeqRequest;


class MeqUVWPolc
{
public:
  // Create with the given ra and dec of the phase reference position.
  MeqUVWPolc();

  // Calculate the polynomial coefficients by fitting to the given values.
  // It results in a polynomial for the given times and all frequencies.
  void calcCoeff (const Vector<double>& times, const Matrix<double>& uvws,
		  bool addPolc = true);

  // Calculate the UVW for the given domain.
  void calcUVW (const MeqRequest&);

  // Get the u, v and w.
  const MeqResult& getU() const
    { return itsU; }
  const MeqResult& getV() const
    { return itsV; }
  const MeqResult& getW() const
    { return itsW; }

  const MeqParmPolc& getUCoeff() const
    { return itsUCoeff; }
  const MeqParmPolc& getVCoeff() const
    { return itsVCoeff; }
  const MeqParmPolc& getWCoeff() const
    { return itsWCoeff; }

  void setName(const string& name);

private:
  Polynomial<AutoDiff<double> > itsPoly;
  LinearFit<Double> itsFitter;
  MeqParmPolc  itsUCoeff;
  MeqParmPolc  itsVCoeff;
  MeqParmPolc  itsWCoeff;
  MeqRequestId itsLastRequest;
  MeqResult itsU;
  MeqResult itsV;
  MeqResult itsW;
};

}

#endif

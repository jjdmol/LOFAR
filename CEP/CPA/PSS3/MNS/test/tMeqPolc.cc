//# tMeqPolc.cc: test program for class MeqPolc
//#
//# Copyright (C) 2003
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


#include <MNS/MeqPolc.h>
#include <MNS/MeqRequest.h>
#include <MNS/MeqResult.h>
#include <MNS/MeqMatrixTmp.h>
#include <Common/Debug.h>

using namespace casa;


bool compare(const MeqMatrix& m1, const MeqMatrix& m2)
{
  if (m1.nx() != m2.nx()  ||  m1.ny() != m2.ny()) {
    return false;
  }
  MeqMatrix res = sum(sqr(m1-m2));
  if (res.isDouble()) {
    return (res.getDouble() < 1.e-7);
  }
  dcomplex resc = res.getDComplex();
  return (LOFAR::real(resc) < 1.e-7  &&  LOFAR::imag(resc) < 1.e-7);
}

void doIt (MeqPolc& polc)
{
  MeqDomain domain(1,4, -2,3);
  MeqMatrix newc = polc.normalize (polc.getCoeff(), domain);
  MeqPolc newpolc;
  newpolc.setCoeff (newc);
  newpolc.setDomain (domain);
  newpolc.setNormalize (True);
  newpolc.setX0 (polc.getX0());
  newpolc.setY0 (polc.getY0());
  MeqMatrix backc = newpolc.denormalize (newpolc.getCoeff());
  // Check if final coefficients match original.
  Assert (compare(polc.getCoeff(), backc));
  // Evaluate both polynomials for some values.
  MeqRequest req(domain, 4, 4);
  MeqResult res1 = polc.getResult (req);
  MeqResult res2 = newpolc.getResult (req);
  Assert (compare(res1.getValue(), res2.getValue()));
}

int main()
{
  for (int i=0; i<2; i++) {
    MeqPolc polc;
    polc.setX0 (i*0.5);
    polc.setY0 (-i*2);

    polc.setCoeff(MeqMatrix(2.));
    doIt (polc);

    polc.setCoeff(MeqMatrix(2.,2,1,true));
    doIt (polc);

    polc.setCoeff(MeqMatrix(2.,1,2,true));
    doIt (polc);

    double c1[12] = {1.5, 2.1, -0.3, -2,
		     1.45, -2.3, 0.34, 1.7,
		     5, 1, 0, -1};
    polc.setCoeff(MeqMatrix(c1, 3, 4));
    doIt(polc);
    polc.setCoeff(MeqMatrix(c1, 4, 3));
    doIt(polc);
    polc.setCoeff(MeqMatrix(c1, 6, 2));
    doIt(polc);
    polc.setCoeff(MeqMatrix(c1, 2, 6));
    doIt(polc);
  }
  cout << "OK" << endl;
  return 0;
}

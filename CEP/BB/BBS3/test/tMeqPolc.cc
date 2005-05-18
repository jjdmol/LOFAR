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


#include <BBS3/MNS/MeqPolc.h>
#include <BBS3/MNS/MeqRequest.h>
#include <BBS3/MNS/MeqResult.h>
#include <BBS3/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace casa;


bool compare (const MeqMatrix& m1, const MeqMatrix& m2)
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

void doEval (MeqPolc& polc, const MeqDomain& domain, int nx, int ny)
{
  MeqRequest req(domain, nx, ny);
  MeqResult res = polc.getResult (req);
  cout << "domain: " << domain << ' ' << nx << ' ' << ny << endl;
  cout << "result: " << res.getValue() << endl;
  // Calculate the result for all perturbed values.
  int nspid = polc.makeSolvable (0);
  cout << "nspid=" << nspid << endl;
  MeqRequest reqm(domain, nx, ny, nspid);
  MeqResult resm = polc.getResult (reqm);
  ASSERT (compare (res.getValue(), resm.getValue()));
  for (int i=0; i<nspid; ++i) {
    MeqMatrix coeff = polc.getCoeff().clone();
    double* coeffp = coeff.doubleStorage();
    coeffp[i] += polc.getPerturbation();
    MeqPolc polcp (polc);
    polcp.setCoeff (coeff);
    MeqResult resp = polcp.getResult (req);
    ASSERT (compare (resp.getValue(), resm.getPerturbedValue (i)));
  }
}

void doIt (MeqPolc& polc)
{
  MeqDomain domain(2,6, -1,3);
  polc.setDomain (domain);
  // Evaluate the polynomial for some requests.
  doEval (polc, domain, 1, 1);
  doEval (polc, domain, 2, 2);
  doEval (polc, domain, 8, 1);
}

int main()
{
  for (int i=0; i<2; i++) {
    MeqPolc polc;
    polc.setX0 (i+1);
    polc.setY0 (-i*2);
    polc.setXScale (2);
    polc.setYScale (0.5);
    polc.setPerturbation (1, false);

    polc.setCoeff(MeqMatrix(2.));
    doIt (polc);

    polc.setCoeff(MeqMatrix(2.,2,1,true));
    doIt (polc);

    polc.setCoeff(MeqMatrix(2.,1,2,true));
    doIt (polc);

    polc.setCoeff(MeqMatrix(2.,3,3,true));
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

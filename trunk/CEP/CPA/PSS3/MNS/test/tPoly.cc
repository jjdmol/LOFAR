//# tExprPoly.cc: Test program for ExprPoly class
//# Copyright (C) 2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$


#include <MNS/TFParmSingle.h>
#include <MNS/TFExprPoly.h>
#include <MNS/TFRequest.h>
#include <MNS/MnsMatrix.h>
#include <MNS/MnsMatrixTmp.h>
#include <Common/Debug.h>
#include <aips/Fitting/LSQ.h>
#include <aips/OS/Timer.h>
#include <aips/Exceptions/Error.h>
#include <aips/iostream.h>
#include <aips/strstream.h>
#include <stdexcept>


void doIt()
{
  TFDomain domain(1, 2, 0.1, 2, 4, 0.2);
  cout << domain.startX() << ' ' << domain.endX() << ' '
       << domain.stepX() << ' ' << domain.nx() << endl;
  cout << domain.startY() << ' ' << domain.endY() << ' '
       << domain.stepY() << ' ' << domain.ny() << endl;
  TFDomain dom2(domain);
  cout << dom2.startX() << ' ' << dom2.endX() << ' '
       << dom2.stepX() << ' ' << dom2.nx() << endl;
  cout << dom2.startY() << ' ' << dom2.endY() << ' '
       << dom2.stepY() << ' ' << dom2.ny() << endl;
  TFParmSingle parm00(0, 1);
  TFParmSingle parm10(1, 1000);
  TFParmSingle parm20(2, 10);
  const vector<TFParm*>& parms = TFParm::getParmList();
  int nrspid = 0;
  nrspid += parms[0]->setSolvable (nrspid);
  nrspid += parms[1]->setSolvable (nrspid);
  nrspid += parms[2]->setSolvable (nrspid);
  Assert (nrspid == 3);
  vector<TFExpr*> vec(3);
  vec[0] = &parm00;
  vec[1] = &parm10;
  vec[2] = &parm20;
  TFExprPoly expr(vec, 3, 1);

  TFRange original = expr.getRange (TFRequest(domain, nrspid));
  const double* orval = original.getValue().doubleStorage();
  for (int i=0; i<nrspid; i++) {
    cout << "deriv parm " << i
	 << (original.getPerturbedValue(i) - original.getValue()) /
            MnsMatrix(original.getPerturbation(i)) << endl;
  }

  // Use AIPS++ LSQ solver. There are 3 unknowns.
  LSQ lnl(3);
  // Initial parameters.
  Double sol[3] = {2, 1, 1};
  double me[3*3], mu[1];
  // Define vectors for derivatives and value difference.
  Double un[3];
  Double kn[1];
  Int iter=0;
  Double fit = 1.0;
  uInt nr;
  Timer tim1;
  tim1.mark();
  while (iter<100 && (fit>0 || fit < -0.001)) {
    parm00.update (sol[0]);
    parm10.update (sol[1]);
    parm20.update (sol[2]);
    TFRange range = expr.getRange (TFRequest(domain, nrspid));
    const double* values = range.getValue().doubleStorage();
    vector<MnsMatrix> derivs(3);
    vector<const double*> derivptr(3);
    for (int i=0; i<3; i++) {
      derivs[i] = (range.getPerturbedValue(i) - range.getValue()) /
	          MnsMatrix(range.getPerturbation(i));
      derivptr[i] = derivs[i].doubleStorage();
    }
    for (int j=0; j<domain.ncells(); j++) {
      for (int i=0; i<3; i++) {
	un[i] = derivptr[i][j];
      }
      kn[0] = orval[j] - values[j];
      lnl.makeNorm(un, 1.0, kn);
    };
    if (!lnl.solveLoop(fit, nr, sol, mu, me)) {
      cout << "Error in loop: " << nr << endl;
      break;
    };
    iter++;
  };
  cout << "niter: " << iter << endl;
  if (fit > -1e-9 && fit <= 0) {
    cout << "Fit:       " << "ok" << endl;
  } else {
    cout << "Fit:       " << fit << endl;
  };
  cout << "Sol:       " << sol[0] << ", " << sol[1] << ", " << sol[2] << 
    endl;
  if (mu[0] == me[0] && mu[0] < 1e-15) {
    mu[0] = 0;
    me[0] = 0;
  };
  cout << "me:        " << mu[0] << ", " << me[0] << endl;
  cerr << "User time: " << tim1.user() << endl;
}


int main (int argc, char** argv)
{
  uInt nr = 100;
  if (argc > 1) {
    istrstream istr(argv[1]);
    istr >> nr;
  }
  try {
    doIt();
  } catch (AipsError& x) {
    cout << "Caught an AIPS++ exception: " << x.getMesg() << endl;
    return 1;
  } catch (std::exception& x) {
    cout << "Caught an exception: " << x.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}

//# tPoly.cc: Test program for ExprPoly class
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


#include <MNS/ParmTable.h>
#include <MNS/MeqParmSingle.h>
#include <MNS/MeqStoredParmPolc.h>
#include <MNS/MeqRequest.h>
#include <MNS/MeqMatrix.h>
#include <MNS/MeqMatrixTmp.h>
#include <Common/Debug.h>
#include <aips/Arrays/Matrix.h>
#include <aips/Fitting/LSQ.h>
#include <aips/OS/Timer.h>
#include <aips/Exceptions/Error.h>
#include <Common/lofar_iostream.h>
#include <stdexcept>


// Solve for a single 3rd-order polynomial.
void doIt1 (const string& dbtype, const string& dbname,
	    const string& pw, const string& ptabName)
{
  cout << endl << "test ParmPolc" << endl;
  ParmTable ptab(dbtype, ptabName, dbname, pw);
  MeqDomain domain(1, 2, 2, 4);
  MeqStoredParmPolc parm00("parmp", -1, -1, &ptab);
  int nrspid = 0;
  parm00.setSolvable(true);
  nrspid += parm00.initDomain (domain, nrspid);
  Assert (nrspid == 3);
  Assert (parm00.getPolcs().size() == 1);
  Assert (parm00.getPolcs()[0].ncoeff() == 3);
  cout << "coeff: " << parm00.getPolcs()[0].getCoeff() << endl;

  MeqRequest request (domain, 10, 1, nrspid);
  {
    Timer timer;
    for (int i=0; i<100000; i++) {
      MeqResult res = parm00.getResult (request);
    }
    timer.show();
  }
  MeqResult original = parm00.getResult (request);
  const double* orval = original.getValue().doubleStorage();
  cout << original << endl;

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
    parm00.update (MeqMatrix(sol, 3, 1));
    MeqResult result = parm00.getResult (request);
    const double* values = result.getValue().doubleStorage();
    vector<MeqMatrix> derivs(3);
    vector<const double*> derivptr(3);
    for (int i=0; i<3; i++) {
      derivs[i] = (result.getPerturbedValue(i) - result.getValue()) /
	          MeqMatrix(result.getPerturbation(i));
      derivptr[i] = derivs[i].doubleStorage();
    }
    for (int j=0; j<request.ncells(); j++) {
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
    cout << "Sol " << iter << ": " << sol[0] << ", " << sol[1] << ", " << sol[2] << 
    endl;
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


// Evaluate some polynomials.
void doIt2()
{
  cout << endl << "doIt2" << endl;
  {
    MeqParmPolc parm("parm1");
    MeqPolc polc;
    double coeff[2] = {2, 3};
    polc.setDomain (MeqDomain(2,4,0,2));
    polc.setCoeff  (MeqMatrix(coeff, 2, 1));
    parm.addPolc (polc);
    polc.setDomain (MeqDomain(4,6,0,2));
    polc.setCoeff  (MeqMatrix(coeff, 1, 2));
    parm.addPolc (polc);
    MeqDomain domain(2,6,0,2);
    MeqRequest request(domain,8,2);
    MeqResult result = parm.getResult (request);
    cout << result << endl;
  }
  {
    MeqParmPolc parm("parm4");
    MeqPolc polc;
    double coeff[1] = {2};
    polc.setDomain (MeqDomain(0,2,2,4));
    polc.setCoeff  (MeqMatrix(coeff, 1, 1));
    parm.addPolc (polc);
    double coeff2[9] = {2, 2.4, -2, -1, -2, 3, 1.2, 1.5, 1.4};
    polc.setDomain (MeqDomain(0,2,4,7));
    polc.setCoeff  (MeqMatrix(coeff2, 3, 3));
    parm.addPolc (polc);
    MeqDomain domain(0,2,2,7);
    MeqRequest request(domain,4,10);
    MeqResult result = parm.getResult (request);
    cout << result << endl;
  }
  {
    MeqParmPolc parm("parm5");
    MeqPolc polc;
    double coeff[1] = {2};
    polc.setDomain (MeqDomain(0,2,2,4));
    polc.setCoeff  (MeqMatrix(coeff, 1, 1));
    parm.addPolc (polc);
    parm.setSolvable(true);
    MeqDomain domain(0,2,2,4);
    int nrspid = 0;
    nrspid += parm.initDomain (domain, nrspid);
    MeqRequest request(domain,4,2,nrspid);
    MeqResult result = parm.getResult (request);
    cout << result << endl;
  }
  {
    MeqParmPolc parm("parm6");
    MeqPolc polc;
    double coeff2[9] = {2, 2.4, -2, -1, -2, 3, 1.2, 1.5, 1.4};
    polc.setDomain (MeqDomain(0,2,4,7));
    polc.setCoeff  (MeqMatrix(coeff2, 3, 3));
    parm.addPolc (polc);
    parm.setSolvable(true);
    MeqDomain domain(0,2,4,7);
    int nrspid = 0;
    nrspid += parm.initDomain (domain, nrspid);
    MeqRequest request(domain,2,4,nrspid);
    MeqResult result = parm.getResult (request);
    cout << result << endl;
  }
}


int main (int argc, char** argv)
{
  try {
    if (argc > 4) {
      doIt1(argv[1], argv[2], argv[3], argv[4]);
    }
    doIt2();
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

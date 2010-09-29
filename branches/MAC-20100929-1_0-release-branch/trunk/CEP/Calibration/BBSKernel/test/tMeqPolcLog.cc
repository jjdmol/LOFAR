//# tMeqPolcLog.cc: test program for class MeqPolcLog
//#
//# Copyright (C) 2003
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>
#include <BBSKernel/MNS/MeqPolcLog.h>
#include <BBSKernel/MNS/MeqRequest.h>
#include <BBSKernel/MNS/MeqResult.h>
#include <BBSKernel/MNS/MeqMatrixTmp.h>
#include <Common/Timer.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace casa;


/*
void setCoeff(ParmDB::ParmValue &pval, const MeqMatrix &value, const bool* mask)
{
  vector<int> shp(2);
  shp[0] = value.nx();
  shp[1] = value.ny();
  
  if (mask == 0) {
    pval.rep().setCoeff(value.doubleStorage(), shp);
  } else {
    pval.rep().setCoeff(value.doubleStorage(), mask, shp);
  }
}
*/

bool compare (const MeqMatrix& m1, const MeqMatrix& m2)
{
  if (m1.nx() != m2.nx()  ||  m1.ny() != m2.ny()) {
    return false;
  }
  MeqMatrix res = sum(sqr(m1-m2));
  if (!res.isComplex()) {
    return (res.getDouble() < 1.e-7);
  }
  dcomplex resc = res.getDComplex();
  return (LOFAR::real(resc) < 1.e-7  &&  LOFAR::imag(resc) < 1.e-7);
}


void doEval (MeqPolcLog& polc, const MeqDomain& domain, int nx, int ny)
{
  // Always use the entire mask as true.
  bool mask[100];
  for (int i=0; i<100; ++i) {
    mask[i] = true;
  }
  
  polc.clearSolvable();
  MeqRequest req(domain, nx, ny);
  MeqResult res = polc.getResult (req, 0, 0);
  cout << "domain: " << domain << ' ' << nx << ' ' << ny
       << "    pol: " << polc.getCoeff().nx() << ' '
       << polc.getCoeff().ny() << endl;
  cout << "result: " << res.getValue() << endl;
  
  // Calculate the result for all perturbed values.
  int nspid = polc.makeSolvable (0);
  cout << "nspid=" << nspid << endl;
  MeqRequest reqm(domain, nx, ny, nspid);
  MeqResult resm = polc.getResult (reqm, nspid, 0);
  ASSERT(compare(res.getValue(), resm.getValue()));
  
  for (int i=0; i<nspid; ++i) {
    ParmDB::ParmValue pval = polc.getParmValue().clone();
    MeqMatrix coeff = polc.getCoeff().clone();
    double* coeffp = coeff.doubleStorage();
    coeffp[i] += polc.getPerturbation(i);

    MeqPolcLog polcp (polc);
    polcp.update (coeff);
    MeqResult resp = polcp.getResult (req, 0, 0);
    ASSERT (compare (resp.getValue(), resm.getPerturbedValue (i)));
  }
}


void doIt (MeqPolcLog& polc)
{
  // Evaluate the polynomial for some requests.

  MeqDomain domain(20.0e6, 60.0e6, 0.0, 3600.0);
  doEval(polc, domain, 1, 1);
  doEval(polc, domain, 2, 2);
  doEval(polc, domain, 8, 1);

  MeqDomain domain2(10.0e6, 400.0e6, -1800.0, 1800.0);
  doEval(polc, domain2, 1, 1);
  doEval(polc, domain2, 2, 2);
  doEval(polc, domain2, 8, 1);
}


int main()
{
  try {
    // Always use the entire mask as true.
    bool mask[100];
    for (int i=0; i<100; ++i) {
      mask[i] = true;
    }
    
    ParmDB::ParmValue pval;
    pval.rep().setType("polclog");
    pval.rep().setPerturbation (1, false);
    pval.rep().setDomain(ParmDB::ParmDomain(2, 6, -1, 3));
    pval.rep().itsConstants.push_back(25.0e6);
    pval.rep().itsConstants.push_back(0.0);
    
    MeqPolcLog polc(pval);
    polc.setDomain(MeqDomain(20.0e6, 60.0e6, 0.0, 3600.0));

    polc.setCoeff(MeqMatrix(2.), mask);
    doIt(polc);
    
    polc.setCoeff(MeqMatrix(2.,2,1,true), mask);
    doIt(polc);
    
    polc.setCoeff(MeqMatrix(2.,3,3,true), mask);
    doIt(polc);

    double c1[12] = { 0.4, 0.3, 0.2, 0.1,
                     -0.75, 0.75, -0.75, 0.75,
                     -1.2, 3.4, 5.6, -7.8};
    
    polc.setCoeff(MeqMatrix(c1, 3, 4), mask);
    doIt(polc);
    
    polc.setCoeff(MeqMatrix(c1, 4, 3), mask);
    doIt(polc);
    
    polc.setCoeff(MeqMatrix(c1, 6, 2), mask);
    doIt(polc);
    
    polc.setCoeff(MeqMatrix(c1, 2, 6), mask);
    doIt(polc);

  } catch (std::exception& x) {
    cerr << "Caught exception: " << x.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}

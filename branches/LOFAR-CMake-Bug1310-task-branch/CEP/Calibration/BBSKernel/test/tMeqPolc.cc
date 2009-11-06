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

#include <lofar_config.h>
#include <BBSKernel/MNS/MeqPolc.h>
#include <BBSKernel/MNS/MeqRequest.h>
#include <BBSKernel/MNS/MeqResult.h>
#include <BBSKernel/MNS/MeqMatrixTmp.h>
#include <Common/Timer.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace casa;


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

void eval3 (int ncy, int ndx, int ndy, double stepx, double stepy,
	    const double* coeffData, double pert, MeqResult& res,
	    bool makediff=true)
{
  res = MeqResult(3*ncy);
  double* value = res.setDoubleFormat (ndx, ndy);
  if (makediff) {
    for (int i=0; i<3*ncy; ++i) {
      ///      res.setPerturbedDouble (i, ndx, ndy);
    }
  }
  NSTimer timer1("new");
  timer1.start();
      double valy = 0;
      for (int j=0; j<ndy; j++) {
	const double* mvalue = value;
	// Calculate the y-factors.
	double lastval = coeffData[0];
	double fact1   = coeffData[1];
	double fact2   = coeffData[2];
	double y = valy;
	for (int iy=1; iy<ncy; ++iy) {
	  lastval += y*coeffData[3*iy];
	  fact1   += y*coeffData[3*iy+1];
	  fact2   += y*coeffData[3*iy+2];
	  y *= valy;
	}
	*value++ = lastval;
	fact1 *= stepx;
	fact2 *= stepx*stepx;
	fact1 += fact2;
	fact2 *= 2;
	for (int ix=1; ix<ndx; ++ix) {
	  lastval += fact1;
	  fact1 += fact2;
	  *value++ = lastval;
	}
	if (makediff) {
	  // Calculate the perturbed value for the coefficients without
	  // a factor of x (thus c00, c10*y, c20*y^2, etc.).
	  // Do the same for c01*x, c11*x*y, etc. using step.
	  // Do the same for c02*x^2, c12*x^2*y, etc. using step1,step2.
	  double perty=pert;
	  for (int iy=0; iy<ncy; ++iy) {
	    double* valuep0 = res.getPerturbedValueRW(iy*3).doubleStorage()
	                      + j*ndx;
	    for (int i=0; i<ndx; ++i) {
	      valuep0[i] = mvalue[i] + perty;
	    }
	    perty *= valy;
	  }
	  perty=pert;
	  for (int iy=0; iy<ncy; ++iy) {
	    double* valuep1 = res.getPerturbedValueRW(iy*3+1).doubleStorage()
	                      + j*ndx;
	    double step1 = perty * stepx;
	    double val1 = 0;
	    for (int i=0; i<ndx; ++i) {
	      valuep1[i] = mvalue[i] + val1;
	      val1 += step1;
	    }
	    perty *= valy;
	  }
	  perty=pert;
	  for (int iy=0; iy<ncy; ++iy) {
	    double* valuep2 = res.getPerturbedValueRW(iy*3+2).doubleStorage()
	                      + j*ndx;
	    double step2a = perty * stepx * stepx;
	    double step2b = 2*step2a;
	    double val2 = 0;
	    for (int i=0; i<ndx; ++i) {
	      valuep2[i] = mvalue[i] + val2;
	      val2 += step2a;
	      step2a += step2b;
	    }
	    perty *= valy;
	  }
// 	  double perty=pert;
// 	  for (int iy=0; iy<ncy; ++iy) {
// 	    double* valuep0 = res.getPerturbedValueRW(iy*3).doubleStorage();
// 	    double* valuep1 = res.getPerturbedValueRW(iy*3+1).doubleStorage();
// 	    double step1 = perty * stepx;
// 	    double val1 = 0;
// 	    double* valuep2 = res.getPerturbedValueRW(iy*3+2).doubleStorage();
// 	    double step2a = step1 * stepx;
// 	    double step2b = 2*step2a;
// 	    double val2 = 0;
// 	    for (int i=0; i<ndx; ++i) {
// 	      valuep0[i+j*ndx] = mvalue[i] + perty;
// 	      valuep1[i+j*ndx] = mvalue[i] + val1;
// 	      val1 += step1;
// 	      valuep2[i+j*ndx] = mvalue[i] + val2;
// 	      val2 += step2a;
// 	      step2a += step2b;
// 	    }
// 	    perty *= valy;
// 	  }
	}
	valy += stepy;
      }
  timer1.stop();
  cout << ">>>" << endl;
  timer1.print(cout);
  cout << "<<<" << endl;
	
}

void doEval (MeqPolc& polc, const MeqDomain& domain, int nx, int ny)
{
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
  ASSERT (compare (res.getValue(), resm.getValue()));
  for (int i=0; i<nspid; ++i) {
    MeqMatrix coeff = polc.getCoeff().clone();
    double* coeffp = coeff.doubleStorage();
    coeffp[i] += polc.getPerturbation(i);
    MeqPolc polcp (polc);
    polcp.update (coeff);
    MeqResult resp = polcp.getResult (req, 0, 0);
    ASSERT (compare (resp.getValue(), resm.getPerturbedValue (i)));
  }

  const MeqMatrix& coeff = polc.getCoeff();
  if (coeff.nx() < 0) {
////  if (coeff.nx() == 3) {
    cout << "Evaluate with ncx=3" << endl;
    MeqResult rese;
    eval3 (coeff.ny(), nx, ny, 1./nx, 1./ny, coeff.doubleStorage(),
	   polc.getPerturbation(), rese, false);
    NSTimer tim1("ntot");
    tim1.start();
    eval3 (coeff.ny(), nx, ny, 1./nx, 1./ny, coeff.doubleStorage(),
	   polc.getPerturbation(), rese);
    tim1.stop();
    cout << ">>>" << endl;
    tim1.print(cout);
    cout << "<<<" << endl;
    ASSERT (compare (rese.getValue(), resm.getValue()));
    for (int i=0; i<nspid; ++i) {
      ASSERT (compare (rese.getPerturbedValue(i), resm.getPerturbedValue(i)));
    }
    NSTimer tim2("otot");
    tim2.start();
    rese = polc.getResult (reqm, nspid, 0);
    tim2.stop();
    cout << ">>>" << endl;
    tim2.print(cout);
    cout << "<<<" << endl;
    ASSERT (compare (rese.getValue(), resm.getValue()));
    for (int i=0; i<nspid; ++i) {
      ASSERT (compare (rese.getPerturbedValue(i), resm.getPerturbedValue(i)));
    }
  }
  // Get the result using analytical derivatives.
  // Compare if they match the computed ones.
  MeqResult resma = polc.getAnResult (reqm, nspid, 0);
  ASSERT (compare (res.getValue(), resma.getValue()));
  for (int i=0; i<nspid; ++i) {
    ASSERT (compare (resma.getPerturbedValue(i),
		     (resm.getPerturbedValue(i) - resm.getValue()) / polc.getPerturbation(i)));
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
  ///  doEval (polc, domain, 64, 1);
    ///  doEval (polc, domain, 64, 8);
    ///  doEval (polc, domain, 256, 1);
  MeqDomain domain2(0,4, 0,1);
  doEval (polc, domain2, 1, 1);
  doEval (polc, domain2, 2, 2);
  doEval (polc, domain2, 8, 1);
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
    pval.rep().setPerturbation (1, false);
    pval.rep().setDomain (ParmDB::ParmDomain(0,1,0,1));
    MeqPolc polc(pval);

    polc.setCoeff(MeqMatrix(2.), mask);
    doIt (polc);

    polc.setCoeff(MeqMatrix(2.,2,1,true), mask);
    doIt (polc);

    polc.setCoeff(MeqMatrix(2.,1,2,true), mask);
    doIt (polc);

    polc.setCoeff(MeqMatrix(2.,3,3,true), mask);
    doIt (polc);

    double c1[12] = {1.5, 2.1, -0.3, -2,
		     1.45, -2.3, 0.34, 1.7,
		     5, 1, 0, -1};
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

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


#include <BBS/MNS/MeqTabular.h>
#include <BBS/MNS/MeqRequest.h>
#include <BBS/MNS/MeqResult.h>
#include <BBS/MNS/MeqMatrixTmp.h>
#include <Common/Timer.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace casa;

// bool compare (const MeqMatrix& m1, const MeqMatrix& m2)
// {
//   if (m1.nx() != m2.nx()  ||  m1.ny() != m2.ny()) {
//     return false;
//   }
//   MeqMatrix res = sum(sqr(m1-m2));
//   if (!res.isComplex()) {
//     return (res.getDouble() < 1.e-7);
//   }
//   dcomplex resc = res.getDComplex();
//   return (LOFAR::real(resc) < 1.e-7  &&  LOFAR::imag(resc) < 1.e-7);
// }

// void eval3 (int ncy, int ndx, int ndy, double stepx, double stepy,
// 	    const double* coeffData, double pert, MeqResult& res,
// 	    bool makediff=true)
// {
//   res = MeqResult(3*ncy);
//   double* value = res.setDoubleFormat (ndx, ndy);
//   if (makediff) {
//     for (int i=0; i<3*ncy; ++i) {
//       ///      res.setPerturbedDouble (i, ndx, ndy);
//     }
//   }
//   NSTimer timer1("new");
//   timer1.start();
//       double valy = 0;
//       for (int j=0; j<ndy; j++) {
// 	const double* mvalue = value;
// 	// Calculate the y-factors.
// 	double lastval = coeffData[0];
// 	double fact1   = coeffData[1];
// 	double fact2   = coeffData[2];
// 	double y = valy;
// 	for (int iy=1; iy<ncy; ++iy) {
// 	  lastval += y*coeffData[3*iy];
// 	  fact1   += y*coeffData[3*iy+1];
// 	  fact2   += y*coeffData[3*iy+2];
// 	  y *= valy;
// 	}
// 	*value++ = lastval;
// 	fact1 *= stepx;
// 	fact2 *= stepx*stepx;
// 	fact1 += fact2;
// 	fact2 *= 2;
// 	for (int ix=1; ix<ndx; ++ix) {
// 	  lastval += fact1;
// 	  fact1 += fact2;
// 	  *value++ = lastval;
// 	}
// 	if (makediff) {
// 	  // Calculate the perturbed value for the coefficients without
// 	  // a factor of x (thus c00, c10*y, c20*y^2, etc.).
// 	  // Do the same for c01*x, c11*x*y, etc. using step.
// 	  // Do the same for c02*x^2, c12*x^2*y, etc. using step1,step2.
// 	  double perty=pert;
// 	  for (int iy=0; iy<ncy; ++iy) {
// 	    double* valuep0 = res.getPerturbedValueRW(iy*3).doubleStorage()
// 	                      + j*ndx;
// 	    for (int i=0; i<ndx; ++i) {
// 	      valuep0[i] = mvalue[i] + perty;
// 	    }
// 	    perty *= valy;
// 	  }
// 	  perty=pert;
// 	  for (int iy=0; iy<ncy; ++iy) {
// 	    double* valuep1 = res.getPerturbedValueRW(iy*3+1).doubleStorage()
// 	                      + j*ndx;
// 	    double step1 = perty * stepx;
// 	    double val1 = 0;
// 	    for (int i=0; i<ndx; ++i) {
// 	      valuep1[i] = mvalue[i] + val1;
// 	      val1 += step1;
// 	    }
// 	    perty *= valy;
// 	  }
// 	  perty=pert;
// 	  for (int iy=0; iy<ncy; ++iy) {
// 	    double* valuep2 = res.getPerturbedValueRW(iy*3+2).doubleStorage()
// 	                      + j*ndx;
// 	    double step2a = perty * stepx * stepx;
// 	    double step2b = 2*step2a;
// 	    double val2 = 0;
// 	    for (int i=0; i<ndx; ++i) {
// 	      valuep2[i] = mvalue[i] + val2;
// 	      val2 += step2a;
// 	      step2a += step2b;
// 	    }
// 	    perty *= valy;
// 	  }
// // 	  double perty=pert;
// // 	  for (int iy=0; iy<ncy; ++iy) {
// // 	    double* valuep0 = res.getPerturbedValueRW(iy*3).doubleStorage();
// // 	    double* valuep1 = res.getPerturbedValueRW(iy*3+1).doubleStorage();
// // 	    double step1 = perty * stepx;
// // 	    double val1 = 0;
// // 	    double* valuep2 = res.getPerturbedValueRW(iy*3+2).doubleStorage();
// // 	    double step2a = step1 * stepx;
// // 	    double step2b = 2*step2a;
// // 	    double val2 = 0;
// // 	    for (int i=0; i<ndx; ++i) {
// // 	      valuep0[i+j*ndx] = mvalue[i] + perty;
// // 	      valuep1[i+j*ndx] = mvalue[i] + val1;
// // 	      val1 += step1;
// // 	      valuep2[i+j*ndx] = mvalue[i] + val2;
// // 	      val2 += step2a;
// // 	      step2a += step2b;
// // 	    }
// // 	    perty *= valy;
// // 	  }
// 	}
// 	valy += stepy;
//       }
//   timer1.stop();
//   cout << ">>>" << endl;
//   timer1.print(cout);
//   cout << "<<<" << endl;
// 	
// }

bool evaluate_and_check(MeqTabular &tabular, const MeqDomain &domain, int nx, int ny, const double* const answer)
{
  // Do not compute perturbed values.
  tabular.clearSolvable();
  
  MeqRequest request(domain, nx, ny);
  MeqResult result = tabular.getResult(request, 0, 0);
  
  cout << "domain: " << domain << ' ' << nx << ' ' << ny;
  cout << "... ";
  
  /*
  cout << "domain : " << domain << ' ' << nx << ' ' << ny << endl;
  cout << "tabular: " << tabular.getCoeff().nx() << ' ' << tabular.getCoeff().ny() << endl;
  cout << "result : " << result.getValue() << endl;
  */
  
  if(answer == NULL)
  {
    cout << "NOT CHECKED" << endl;
    return true;
  }
  else
  {
    const double* const resultValues = result.getValue().doubleStorage();
    
    int i;
    bool ok = true;
    for(i = 0; i < request.nx() * request.ny(); i++)
    {
      if(abs(resultValues[i] - answer[i]) > 1.0e-7)
      {
        ok = false;
        break;
      }
    }
    
    if(!ok)
    {
      cout << "FAIL" << endl;
      cout << "!!!!" << endl << result.getValue() << endl << resultValues[i] << " <> " << answer[i] << endl << "!!!!" << endl;
    }
    else
    {
      cout << "PASS" << endl;
    }
    
    return ok;
  }
}


bool test_constant()
{
  cout << "test_constant(): testing with a constant input pattern - ";
  
  ParmDB::ParmValue parm;
  parm.rep().setType("tabular");
  parm.rep().setDomain(ParmDB::ParmDomain(0.0, 1.0, 0.0, 1.0));
  MeqTabular tabular(parm);

  bool mask[25];
  
  for(int i=0; i < 25; i++)
  {
    mask[i] = false;
  }

  double answer[25];
  for(int i=0; i<25; i++)
  {
    answer[i] = 0.23;
  }
  
  tabular.setCoeff(MeqMatrix(0.23, 1, 1), mask);
  
  return evaluate_and_check(tabular, MeqDomain(0.0, 1.0, 0.0, 1.0), 5, 5, answer);
}


bool test_line()
{
  cout << "test_line(): testing with a line input pattern" << endl;
  
  ParmDB::ParmValue parm;
  parm.rep().setType("tabular");
  parm.rep().setDomain(ParmDB::ParmDomain(0.0, 1.0, 0.0, 1.0));
  MeqTabular tabular(parm);

  bool mask[2] = {false, false};
  double values[2] = {-1.0, 1.0};
    
  double inScale = ((1.0 - 0.0) / 2.0);
  double outScale = ((0.75 - 0.25) / 50.0);
  double inCenter = 0.5 * inScale;
  double outCenter = 0.5 * outScale;
  double answer[50];
  for(int i=0; i<50; i++)
  {
    outCenter = 0.25 + i * outScale + 0.5 * outScale;
    double x = 1.0 - (outCenter - inCenter) / inScale;
    answer[i] = values[0] * x + values[1] * (1.0 - x);
  }
    
  tabular.setCoeff(MeqMatrix(values, 2, 1), mask);
  cout << "  - without boundary cells - ";
  if(!evaluate_and_check(tabular, MeqDomain(0.25, 0.75, 0.0, 1.0), 50, 1, answer))
  {
    return false;
  }
    
  cout << "  - with boundary cells (left) - ";
  
  outScale = ((0.75 - 0.0) / 50.0);
  inCenter = 0.5 * inScale;
  outCenter = 0.5 * outScale;
  int leftBoundaryCells = (int) ceil((inCenter - outCenter) / outScale);
  for(int i=0; i<leftBoundaryCells; i++)
  {
    outCenter = 0.0 + i * outScale + 0.5 * outScale;
    double x = 1.0 - (outCenter - (inCenter - inScale)) / inScale;
    answer[i] = 0.0 * x + values[0] * (1.0 - x);
  }
  
  for(int i=leftBoundaryCells; i<50; i++)
  {
    outCenter = 0.0 + i * outScale + 0.5 * outScale;
    double x = 1.0 - (outCenter - inCenter) / inScale;
    answer[i] = values[0] * x + values[1] * (1.0 - x);
  }
  if(!evaluate_and_check(tabular, MeqDomain(0.0, 0.75, 0.0, 1.0), 50, 1, answer))
  {
    return false;
  }
    
  outScale = ((1.0 - 0.25) / 50.0);
  inCenter = 0.5 * inScale;
  outCenter = 0.5 * outScale;
  int rightBoundaryCells = (int) ceil((inCenter - outCenter) / outScale);
  for(int i=0; i<50-rightBoundaryCells; i++)
  {
    outCenter = 0.25 + i * outScale + 0.5 * outScale;
    double x = 1.0 - (outCenter - inCenter) / inScale;
    answer[i] = values[0] * x + values[1] * (1.0 - x);
  }
  inCenter += inScale;
  for(int i=50-rightBoundaryCells; i<50; i++)
  {
    outCenter = 0.25 + i * outScale + 0.5 * outScale;
    double x = 1.0 - (outCenter - inCenter) / inScale;
    answer[i] = values[1] * x + 0.0 * (1.0 - x);
  }
  cout << "  - with boundary cells (right) - ";
  if(!evaluate_and_check(tabular, MeqDomain(0.25, 1.0, 0.0, 1.0), 50, 1, answer))
  {
    return false;
  }
    
  outScale = ((1.0 - 0.0) / 50.0);
  inCenter = 0.5 * inScale;
  outCenter = 0.5 * outScale;
  leftBoundaryCells = (int) ceil((inCenter - outCenter) / outScale);
  rightBoundaryCells = (int) ceil((inCenter - outCenter) / outScale);
  for(int i=0; i<leftBoundaryCells; i++)
  {
    outCenter = 0.0 + i * outScale + 0.5 * outScale;
    double x = 1.0 - (outCenter - (inCenter - inScale)) / inScale;
    answer[i] = 0.0 * x + values[0] * (1.0 - x);
  }
  for(int i=leftBoundaryCells; i<50-rightBoundaryCells; i++)
  {
    outCenter = 0.0 + i * outScale + 0.5 * outScale;
    double x = 1.0 - (outCenter - inCenter) / inScale;
    answer[i] = values[0] * x + values[1] * (1.0 - x);
  }
  inCenter += inScale;
  for(int i=50-rightBoundaryCells; i<50; i++)
  {
    outCenter = 0.0 + i * outScale + 0.5 * outScale;
    double x = 1.0 - (outCenter - inCenter) / inScale;
    answer[i] = values[1] * x + 0.0 * (1.0 - x);
  }
  cout << "  - with boundary cells (left and right) - ";
  if(!evaluate_and_check(tabular, MeqDomain(0.0, 1.0, 0.0, 1.0), 50, 1, answer))
  {
    return false;
  }
  
  return true;
}
    
int main()
{
  bool ok = true;
  
  try
  {
    ok = test_constant() && test_line();
        
/*    
    double values[9] = {1.0, 2.0, 3.0, 4.0, 5.0, 4.0, 3.0, 2.0, 1.0};
//    bool mask[9] = {false, false, false, false, false, false, false, false, false};
    tabular.setCoeff(MeqMatrix(values, 9, 1), mask);
    
    evaluate(tabular, MeqDomain(0.10, 0.90, 0.0, 1.0), 50, 1, NULL);
    evaluate(tabular, MeqDomain(0.0, 0.9, 0.0, 1.0), 50, 1, NULL);
    evaluate(tabular, MeqDomain(0.1, 1.0, 0.0, 1.0), 50, 1, NULL);
    evaluate(tabular, MeqDomain(0.0, 1.0, 0.0, 1.0), 50, 1, NULL);
    
    double values2[512];
    for(int i=0; i < 512; i++)
    {
      values2[i] = 10.0*sin((i*2.0*3.1415927)/50.0);
    }
    tabular.setCoeff(MeqMatrix(values2, 512, 1), mask);
    
    evaluate(tabular, MeqDomain(0.0, 1.0, 0.0, 1.0), 50, 1, NULL);
    evaluate(tabular, MeqDomain(0.25, 0.75, 0.0, 1.0), 256, 1, NULL);
    evaluate(tabular, MeqDomain(0.0, 1.0, 0.0, 1.0), 512, 1, NULL);
    */
  } catch (std::exception& ex) {
    cout << "Caught exception: " << ex.what() << endl;
    return 1;
  }
  
  if(!ok)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

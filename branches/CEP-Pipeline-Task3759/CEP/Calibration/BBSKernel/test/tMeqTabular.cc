//# tMeqPolc.cc: test program for class MeqPolc
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
#include <BBSKernel/MNS/MeqTabular.h>
#include <BBSKernel/MNS/MeqRequest.h>
#include <BBSKernel/MNS/MeqResult.h>
#include <BBSKernel/MNS/MeqMatrixTmp.h>
#include <Common/Timer.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace casa;

#define EPSILON 1.0e-12

// evaluate_and_check(): evaluates the request and compares the result with the given answer. Two numbers a
// and b are compared as follows: a == b <=> (abs(a - b) <= EPSILON) to allow for round-off error.
bool evaluate_and_check(MeqTabular &tabular, const MeqRequest &request, const double* const answer)
{
  // Do not compute perturbed values.
  tabular.clearSolvable();
  
  MeqResult result = tabular.getResult(request, 0, 0);
  
  cout << "domain: " << request.domain() << ' ' << request.nx() << ' ' << request.ny();
  cout << "... ";
  
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
      if(abs(resultValues[i] - answer[i]) > EPSILON)
      {
        ok = false;
        break;
      }
    }
    
    if(!ok)
    {
      cout << "FAIL" << endl;
      cout << "!!!!" << endl;
      cout << "result:" << endl;
      cout << setprecision(15);
      cout << result.getValue() << endl << endl;
      cout << "mismatch at index " << i << ": " << resultValues[i] << " != " << answer[i] << endl;
      cout << setprecision(5);
      cout << "!!!!" << endl;
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

  // Constuct constant (1x1) tabular  
  ParmDB::ParmValue parm;
  parm.rep().setType("tabular");
  parm.rep().setDomain(ParmDB::ParmDomain(0.0, 1.0, 0.0, 1.0));
  MeqTabular tabular(parm);
  tabular.setCoeff(MeqMatrix(0.23, 1, 1), false);

  // Construct expected answer
  MeqRequest request(MeqDomain(0.0, 1.0, 0.0, 1.0), 5, 5);
  double answer[25];
  for(int i=0; i < 25; i++)
  {
    answer[i] = 0.23;
  }
  
  return evaluate_and_check(tabular, request, answer);
}


bool test_line()
{
  cout << "test_line(): testing with a line input pattern" << endl;
  
  // Construct tabular representing a line from (0, 0, -1) to (1, 1, 1).
  ParmDB::ParmValue parm;
  parm.rep().setType("tabular");
  parm.rep().setDomain(ParmDB::ParmDomain(0.0, 1.0, 0.0, 1.0));
  MeqTabular tabular(parm);
  bool mask[2] = {false, false};
  double values[2] = {-1.0, 1.0};
  tabular.setCoeff(MeqMatrix(values, 2, 1), mask);

  double answer[50];
  int outCount = 50;
  double inScale = ((1.0 - 0.0) / 2.0);
  double inCenter = 0.5 * inScale;
  double outCenter;
  double x;
    
  cout << "  - without boundary cells - ";
  // Construct expected answer
  MeqDomain outDomain(0.25, 0.75, 0.0, 1.0); 
  double outScale = ((outDomain.endX() - outDomain.startX()) / outCount);
  for(int i = 0; i < outCount; i++)
  {
    outCenter = outDomain.startX() + i * outScale + 0.5 * outScale;
    x = 1.0 - (outCenter - inCenter) / inScale;
    answer[i] = values[0] * x + values[1] * (1.0 - x);
  }
  // Test  
  MeqRequest request(outDomain, outCount, 1);
  if(!evaluate_and_check(tabular, request, answer))
  {
    return false;
  }
  
  cout << "  - with boundary cells (left) - ";
  // Construct expected answer
  outDomain = MeqDomain(0.0, 0.75, 0.0, 1.0); 
  outScale = ((outDomain.endX() - outDomain.startX()) / outCount);
  outCenter = 0.5 * outScale;
  int leftBoundaryCells = (int) ceil((inCenter - outCenter) / outScale);
  for(int i = 0; i < leftBoundaryCells; i++)
  {
    outCenter = outDomain.startX() + i * outScale + 0.5 * outScale;
    x = 1.0 - (outCenter - (inCenter - inScale)) / inScale;
    answer[i] = 0.0 * x + values[0] * (1.0 - x);
  }
  for(int i = leftBoundaryCells; i < 50; i++)
  {
    outCenter = outDomain.startX() + i * outScale + 0.5 * outScale;
    x = 1.0 - (outCenter - inCenter) / inScale;
    answer[i] = values[0] * x + values[1] * (1.0 - x);
  }
  // Test  
  request = MeqRequest(outDomain, outCount, 1);
  if(!evaluate_and_check(tabular, request, answer))
  {
    return false;
  }

  cout << "  - with boundary cells (right) - ";
  // Construct expected answer
  outDomain = MeqDomain(0.25, 1.0, 0.0, 1.0); 
  outScale = ((outDomain.endX() - outDomain.startX()) / outCount);
  outCenter = 0.5 * outScale;
  int rightBoundaryCells = (int) ceil((inCenter - outCenter) / outScale);
  for(int i = 0; i < 50 - rightBoundaryCells; i++)
  {
    outCenter = outDomain.startX() + i * outScale + 0.5 * outScale;
    x = 1.0 - (outCenter - inCenter) / inScale;
    answer[i] = values[0] * x + values[1] * (1.0 - x);
  }
  for(int i=50-rightBoundaryCells; i<50; i++)
  {
    outCenter = outDomain.startX() + i * outScale + 0.5 * outScale;
    x = 1.0 - (outCenter - (inCenter + inScale)) / inScale;
    answer[i] = values[1] * x + 0.0 * (1.0 - x);
  }
  // Test  
  request = MeqRequest(outDomain, outCount, 1);
  if(!evaluate_and_check(tabular, request, answer))
  {
    return false;
  }
    
  cout << "  - with boundary cells (left and right) - ";
  // Construct expected answer
  outDomain = MeqDomain(0.0, 1.0, 0.0, 1.0); 
  outScale = ((outDomain.endX() - outDomain.startX()) / outCount);
  outCenter = 0.5 * outScale;
  leftBoundaryCells = (int) ceil((inCenter - outCenter) / outScale);
  rightBoundaryCells = (int) ceil((inCenter - outCenter) / outScale);
  for(int i = 0; i < leftBoundaryCells; i++)
  {
    outCenter = outDomain.startX() + i * outScale + 0.5 * outScale;
    x = 1.0 - (outCenter - (inCenter - inScale)) / inScale;
    answer[i] = 0.0 * x + values[0] * (1.0 - x);
  }
  for(int i = leftBoundaryCells; i < 50 - rightBoundaryCells; i++)
  {
    outCenter = outDomain.startX() + i * outScale + 0.5 * outScale;
    x = 1.0 - (outCenter - inCenter) / inScale;
    answer[i] = values[0] * x + values[1] * (1.0 - x);
  }
  for(int i = 50 - rightBoundaryCells; i < 50; i++)
  {
    outCenter = outDomain.startX() + i * outScale + 0.5 * outScale;
    x = 1.0 - (outCenter - (inCenter + inScale)) / inScale;
    answer[i] = values[1] * x + 0.0 * (1.0 - x);
  }
  // Test  
  request = MeqRequest(outDomain, outCount, 1);
  if(!evaluate_and_check(tabular, request, answer))
  {
    return false;
  }
  
  return true;
}
    

bool test_sin()
{
  cout << "test_sin(): testing with a sine input pattern (2 cycles, 512 samples)" << endl;
  
  // Construct tabular representing two cycles of a sine wave.
  ParmDB::ParmValue parm;
  parm.rep().setType("tabular");
  parm.rep().setDomain(ParmDB::ParmDomain(0.0, 1.0, 0.0, 1.0));
  MeqTabular tabular(parm);
  bool mask[512];
  double values[512];
  for(int i = 0; i < 512; i++)
  {
    mask[i] = false;
    values[i] = 10.0 * sin((i * 2.0 * 3.1415927) / 256.0);
  }
  tabular.setCoeff(MeqMatrix(values, 512, 1), mask);
    
  double inScale = ((1.0 - 0.0) / 512.0);
 
  // To test copy, use values as answers (as no interpolation
  // should be done in this case).
  cout << "  - complete copy - ";
  MeqRequest request(MeqDomain(0.0, 1.0, 0.0, 1.0), 512, 1);
  if(!evaluate_and_check(tabular, request, values))
  {
    return false;
  }
  
  cout << "  - partial copy (left 3/4) - ";
  request = MeqRequest(MeqDomain(0.0, 1.0 - 128.0 * inScale, 0.0, 1.0), 512 - 128, 1);
  if(!evaluate_and_check(tabular, request, values))
  {
    return false;
  }
  
  cout << "  - partial copy (right 3/4) - ";
  request = MeqRequest(MeqDomain(128.0 * inScale, 1.0, 0.0, 1.0), 512 - 128, 1);
  if(!evaluate_and_check(tabular, request, &(values[128])))
  {
    return false;
  }
  
  cout << "  - partial copy (middle 1/2) - ";
  request = MeqRequest(MeqDomain(128.0 * inScale, 1.0 - 128.0 * inScale, 0.0, 1.0), 512 - 256, 1);
  if(!evaluate_and_check(tabular, request, &(values[128])))
  {
    return false;
  }
  
  cout << "  - downsampling (512 -> 50 samples) - ";
  // Construct expected answer, using linear interpolation between
  // sample points.
  double answer[50];
  int outCount = 50;
  MeqDomain outDomain(0.0, 1.0, 0.0, 1.0);
  double outScale = ((outDomain.endX() - outDomain.startX()) / outCount);
  double inCenter, outCenter;
  double x;
  int index;
  for(int i = 0; i < 50; i++)
  {
    outCenter = outDomain.startX() + i * outScale + 0.5 * outScale;
    index = (int) ((outCenter - 0.5 * inScale) / inScale);
    inCenter = index * inScale + 0.5 * inScale;
    x = 1.0 - (outCenter - inCenter) / inScale;
    answer[i] = x * values[index] + (1.0 - x) * values[index + 1];
  }
  // Test
  request = MeqRequest(outDomain, 50, 1);
  if(!evaluate_and_check(tabular, request, answer))
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
    ok = test_constant() && test_line() && test_sin();
  } catch (std::exception& ex) {
    cerr << "Caught exception: " << ex.what() << endl;
    return 1;
  }
  
  return (ok ? 0 : 1);
}

//# tFillRow.cc: Test MeqMatrixComplexArr::fillRow
//#
//# Copyright (C) 2006
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
#include <BBS/MNS/MeqMatrix.h>
#include <Common/LofarLogger.h>
#include <casa/BasicMath/Math.h>


using namespace LOFAR;
using namespace std;

void doTest (int nx, int ny)
{
  // Make a complex array with an odd nr of elements.
  MeqMatrix arr;
  arr.setDCMat (nx, ny);
  dcomplex v0 = makedcomplex(0.5,0.3);
  dcomplex fact = makedcomplex(0.9,0.95);
  for (int i=0; i<ny; ++i) {
    arr.fillRowWithProducts (v0+i, fact*(0.1+i/10.), i);
  }
  cout << arr << endl;
  double* realp;
  double* imagp;
  arr.dcomplexStorage (realp, imagp);
  for (int i=0; i<ny; ++i) {
    dcomplex v = v0+i;
    dcomplex f = fact*(0.1+i/10.);
    for (int j=0; j<nx; ++j) {
      ASSERTSTR (casa::near(real(v),*realp) && casa::near(imag(v),*imagp),
		 "result=" << *realp << ',' << *imagp << ", expected=" << v);
      realp++;
      imagp++;
      v *= f;
    }
  }
}

int main()
{
  INIT_LOGGER("tFillRow");
  try {
    doTest(7,5);
    doTest(1,5);
    doTest(5,1);
    doTest(10,12);
  } catch (std::exception& x) {
    cerr << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}

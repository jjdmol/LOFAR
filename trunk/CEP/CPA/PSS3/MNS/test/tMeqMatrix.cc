//# tMeqMatrix.cc: Test program for MeqMatrix classes
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


#include <MNS/MeqMatrix.h>
#include <MNS/MeqMatrixTmp.h>
#include <aips/Arrays/Matrix.h>
#include <aips/Arrays/ArrayMath.h>
#include <aips/OS/Timer.h>
#include <aips/Exceptions/Error.h>
#include <aips/iostream.h>
#include <aips/strstream.h>


void showDouble (const MeqMatrixTmp& v)
{
  cout << v << endl;
}
void showDComplex (const MeqMatrixTmp& v)
{
  cout << v << endl;
}

void doIt()
{
  Double d1[] = {1,2,3,4,5,6};
  Double d2[] = {2,3,4,5,6,7};
  MeqMatrix v1 (d1, 2, 3);
  MeqMatrix v2(d2, 2, 3);
  MeqMatrix v3(double(10));
  showDouble (v1 + v2);
  showDouble (v1 + v2 + v1 + v2 + v3 + v3);
}

void doIt2 (uInt length, uInt nr)
{
  uInt i;
  {
    Double* d1 = new Double[length];
    Double* d2 = new Double[length];
    Double* d3 = new Double[length];
    Double* d4 = new Double[length];
    Double* d5 = new Double[length];
    Double* d6 = new Double[length];
    Double* d7 = new Double[length];
    Double* r  = new Double[length];
    for (i=0; i<length; i++) {
      d1[i] = d2[i] = d3[i] = d4[i] = d5[i] = d6[i] = d7[i] = 1;
    }
    double v3 = 10;
    Timer tim;
    for (i=0; i<nr; i++) {
      for(uInt j=0; j<length; j++) {
	r[j] = d1[j] + d2[j] + d3[j] + v3 + d4[j] + v3 + d5[j] + v3
	  + d6[j] + d7[j];
      }
    }
    tim.show ("C    ");
    delete [] d1;
    delete [] d2;
    delete [] d3;
    delete [] d4;
    delete [] d5;
    delete [] d6;
    delete [] d7;
    delete [] r;
  }
  {
    Double* d1 = new Double[length];
    for (i=0; i<length; i++) {
      d1[i] = 1;
    }
    MeqMatrix v1 (d1, length, 1);
    MeqMatrix v3(double(10));
    Timer tim;
    for (i=0; i<nr; i++) {
      v1 + v1 + v1 + v3 + v1 + v3 + v1 + v3 + v1 + v1;
    }
    tim.show ("Meq  ");
    delete [] d1;
  }
  {
    Array<Double> v1(IPosition(1,length));
    v1.set (1);
    double v3 = 10;
    Timer tim;
    for (i=0; i<nr; i++) {
      v1 + v1 + v1 + v3 + v1 + v3 + v1 + v3 + v1 + v1;
    }
    tim.show ("Array");
  }
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
    if (nr > 0) {
      doIt2 (1, nr);
      doIt2 (8, nr);
      doIt2 (128, nr);
      doIt2 (12800, nr/10);
    }
  } catch (AipsError x) {
    cout << "Caught an exception: " << x.getMesg() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}

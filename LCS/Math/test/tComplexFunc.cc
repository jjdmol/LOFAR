//# tComplexFunc.h: Test program for ComplexFunc
//#
//# Copyright (C) 2004
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


//# Includes
#include <Math/ComplexFunc.h>
#include <Common/LofarLogger.h>
#include <Common/Stopwatch.h>
#include <vector>
#include <sstream>

using namespace LOFAR;

void doIt (int nr1, int nr2)
{
  {
    std::complex<float> c1(2,3);
    std::complex<float> c2(3,4);
    std::complex<float> res;
    multiply(res,c1,c2);
    ASSERT (res == c1*c2);
    add(res,c1,c2);
    ASSERT (res == c1+c2);
  }
  {
    std::complex<double> c1(2,3);
    std::complex<double> c2(3,4);
    std::complex<double> res;
    multiply(res,c1,c2);
    ASSERT (res == c1*c2);
    add(res,c1,c2);
    ASSERT (res == c1+c2);
  }
  {
    std::complex<float> c1(2,3);
    std::complex<float> c2(3,4);
    std::vector<std::complex<float> > vec(nr2);
    std::complex<float>* res = &(vec[0]);
    Stopwatch timer;
    for (int i=0; i<nr1; i++) {
      for (int j=0; j<nr2; j++) {
	multiply(res[j],c1,c2);
      }
    }
    std::cout << nr1<<'*'<<nr2 <<"x vec multiply   "
	      << timer.sdelta() << std::endl;
  }
  {
    std::complex<float> c1(2,3);
    std::complex<float> c2(3,4);
    std::vector<std::complex<float> > vec(nr2);
    std::complex<float>* res = &(vec[0]);
    Stopwatch timer;
    for (int i=0; i<nr1; i++) {
      for (int j=0; j<nr2; j++) {
	res[j] = c1*c2;
      }
    }
    std::cout << nr1<<'*'<<nr2 <<"x vec operator*  "
	      << timer.sdelta() << std::endl;
  }
  {
    std::complex<float> c1(2,3);
    std::complex<float> c2(3,4);
    std::vector<std::complex<float> > vec(nr2);
    std::complex<float>* res = &(vec[0]);
    Stopwatch timer;
    for (int i=0; i<nr1; i++) {
      for (int j=0; j<nr2; j++) {
	res[j] = c1;
	res[j] *= c2;
      }
    }
    std::cout << nr1<<'*'<<nr2 <<"x vec operator*= "
	      << timer.sdelta() << std::endl;
  }
  {
    std::complex<float> c1(2,3);
    std::complex<float> c2(3,4);
    std::complex<float> res;
    Stopwatch timer;
    for (int i=0; i<nr1; i++) {
      for (int j=0; j<nr2; j++) {
	multiply(res,c1,c2);
      }
    }
    std::cout << nr1<<'*'<<nr2 <<"x res multiply   "
	      << timer.sdelta() << std::endl;
  }
  {
    std::complex<float> c1(2,3);
    std::complex<float> c2(3,4);
    std::complex<float> res;
    Stopwatch timer;
    for (int i=0; i<nr1; i++) {
      for (int j=0; j<nr2; j++) {
	res = c1*c2;
      }
    }
    std::cout << nr1<<'*'<<nr2 <<"x res operator*  "
	      << timer.sdelta() << std::endl;
  }
  {
    std::complex<float> c1(2,3);
    std::complex<float> c2(3,4);
    std::complex<float> res;
    Stopwatch timer;
    for (int i=0; i<nr1; i++) {
      for (int j=0; j<nr2; j++) {
	res = c1;
	res *= c2;
      }
    }
    std::cout << nr1<<'*'<<nr2 <<"x res operator*= "
	      << timer.sdelta() << std::endl;
  }
  {
    std::complex<float> c1(2,3);
    std::complex<float> c2(3,4);
    std::vector<std::complex<float> > vec(nr2);
    std::complex<float>* res = &(vec[0]);
    Stopwatch timer;
    for (int i=0; i<nr1; i++) {
      for (int j=0; j<nr2; j++) {
	add(res[j],c1,c2);
      }
    }
    std::cout << nr1<<'*'<<nr2 <<"x vec add        "
	      << timer.sdelta() << std::endl;
  }
  {
    std::complex<float> c1(2,3);
    std::complex<float> c2(3,4);
    std::vector<std::complex<float> > vec(nr2);
    std::complex<float>* res = &(vec[0]);
    Stopwatch timer;
    for (int i=0; i<nr1; i++) {
      for (int j=0; j<nr2; j++) {
	res[j] = c1+c2;
      }
    }
    std::cout << nr1<<'+'<<nr2 <<"x vec operator+  "
	      << timer.sdelta() << std::endl;
  }
  {
    std::complex<float> c1(2,3);
    std::complex<float> c2(3,4);
    std::vector<std::complex<float> > vec(nr2);
    std::complex<float>* res = &(vec[0]);
    Stopwatch timer;
    for (int i=0; i<nr1; i++) {
      for (int j=0; j<nr2; j++) {
	res[j] = c1;
	res[j] += c2;
      }
    }
    std::cout << nr1<<'*'<<nr2 <<"x vec operator+= "
	      << timer.sdelta() << std::endl;
  }
  {
    std::complex<float> c1(2,3);
    std::complex<float> c2(3,4);
    std::complex<float> res;
    Stopwatch timer;
    for (int i=0; i<nr1; i++) {
      for (int j=0; j<nr2; j++) {
	add(res,c1,c2);
      }
    }
    std::cout << nr1<<'*'<<nr2 <<"x res add        "
	      << timer.sdelta() << std::endl;
  }
  {
    std::complex<float> c1(2,3);
    std::complex<float> c2(3,4);
    std::complex<float> res;
    Stopwatch timer;
    for (int i=0; i<nr1; i++) {
      for (int j=0; j<nr2; j++) {
	res = c1+c2;
      }
    }
    std::cout << nr1<<'*'<<nr2 <<"x res operator+  "
	      << timer.sdelta() << std::endl;
  }
  {
    std::complex<float> c1(2,3);
    std::complex<float> c2(3,4);
    std::complex<float> res;
    Stopwatch timer;
    for (int i=0; i<nr1; i++) {
      for (int j=0; j<nr2; j++) {
	res = c1;
	res += c2;
      }
    }
    std::cout << nr1<<'*'<<nr2 <<"x res operator+= "
	      << timer.sdelta() << std::endl;
  }
}

int main(int argc, const char* argv[])
{
  try {
    INIT_LOGGER("tComplexFunc.log_prop");
    ASSERT (sizeof(std::complex<float>) == 2*sizeof(float));
    ASSERT (sizeof(std::complex<double>) == 2*sizeof(double));
    int nr1=100;
    int nr2=10000;
    if (argc > 1) {
      std::istringstream istr(argv[1]);
      istr >> nr1;
    }
    if (argc > 2) {
      std::istringstream istr(argv[2]);
      istr >> nr2;
    }
    doIt(nr1,nr2);
  } catch (std::exception& x) {
    std::cout << "Unexpected exception: " << x.what() << std::endl;
    return 1;
  }
  std::cout << "OK" << std::endl;
  return 0;
}

//  tLCSMath.h: Test program for LCSMath
//
//  Copyright (C) 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#include <Math/LCSMath.h>
#include <Common/Lorrays-Blitz.h>
#include <Common/LofarLogger.h>
#include <Common/Stopwatch.h>

using namespace LOFAR;

int main()
{
  try {
    INIT_LOGGER("tLCSMath.log_prop");
    {
      const int n = 10000;
      LoVec_double vec(n);
      for (int i=0; i<n; i++) {
	vec(i) = n-i-1;
      }
      Stopwatch timer;
      for (int j=0; j<100; j++) {
	LCSMath::sort (vec);
      }
      cout << "100x LCSMath 10^4 " << timer.sdelta() << endl;
      for (int i=0; i<n; i++) {
	ASSERT (vec(i) == double(i));
      }
    }
    {
      const int n = 10000;
      LoVec_double vec(n);
      for (int i=0; i<n; i++) {
	vec(i) = i;
      }
      Stopwatch timer;
      for (int j=0; j<100; j++) {
	LCSMath::sort (vec);
      }
      cout << "100x LCSMath order" << timer.sdelta() << endl;
      for (int i=0; i<n; i++) {
	ASSERT (vec(i) == double(i));
      }
    }
    {
      const int n = 10000;
      LoVec_double vec(n);
      for (int i=0; i<n; i++) {
	vec(i) = 0.;
      }
      Stopwatch timer;
      for (int j=0; j<100; j++) {
	LCSMath::sort (vec);
      }
      cout << "100x LCSMath equal" << timer.sdelta() << endl;
      for (int i=0; i<n; i++) {
	ASSERT (vec(i) == double(0));
      }
    }
    {
      const int n = 100000;
      LoVec_double vec(n);
      for (int i=0; i<n; i++) {
	vec(i) = n-i-1;
      }
      Stopwatch timer;
      for (int j=0; j<100; j++) {
	LCSMath::sort (vec);
      }
      cout << "100x LCSMath 10^5 " << timer.sdelta() << endl;
      for (int i=0; i<n; i++) {
	ASSERT (vec(i) == double(i));
      }
    }
    {
      const int n = 1000000;
      LoVec_double vec(n);
      for (int i=0; i<n; i++) {
	vec(i) = n-i-1;
      }
      Stopwatch timer;
      for (int j=0; j<10; j++) {
	LCSMath::sort (vec);
      }
      cout << " 10x LCSMath 10^6 " << timer.sdelta() << endl;
      for (int i=0; i<n; i++) {
	ASSERT (vec(i) == double(i));
      }
    }
    {
      const int n = 1000000;
      LoVec_double vec(n);
      for (int i=0; i<n; i++) {
	vec(i) = i;
      }
      Stopwatch timer;
      for (int j=0; j<10; j++) {
	LCSMath::sort (vec);
      }
      cout << " 10x LCSMath order" << timer.sdelta() << endl;
      for (int i=0; i<n; i++) {
	ASSERT (vec(i) == double(i));
      }
    }

    {
      const int n1=2;
      const int n2=3;
      const int n3=4;
      LoMat_double mat1(n1,n2);
      ASSERT (mat1.rows() == n1);
      ASSERT (mat1.cols() == n2);
      LoMat_double mat2(n2,n3);
      int k = 0;
      for (int j=0; j<n2; j++) {
	for (int i=0; i<n1; i++) {
	  mat1(i,j) = k++;
	}
	for (int i=0; i<n3; i++) {
	  mat2(j,i) = k++;
	}
      }
      LoMat_double mat3 = LCSMath::matMult (mat1, mat2);
      LoMat_double mat4(n1,n3);
      mat4 = sum(mat1(blitz::tensor::i, blitz::tensor::k) *
		 mat2(blitz::tensor::k, blitz::tensor::j),
		 blitz::tensor::k);
      ASSERT (mat3.extent(0) == n1);
      ASSERT (mat3.extent(1) == n3);
      ASSERT (mat4.extent(0) == n1);
      ASSERT (mat4.extent(1) == n3);
      for (int i=0; i<n3; i++) {
	for (int j=0; j<n1; j++) {
	  ASSERT (mat3(j,i) == mat4(j,i));
	}
      }
    }
    {
      const int n1=20;
      const int n2=30;
      const int n3=40;
      LoMat_double mat1(n1,n2);
      LoMat_double mat2(n2,n3);
      int k = 0;
      for (int j=0; j<n2; j++) {
	for (int i=0; i<n1; i++) {
	  mat1(i,j) = k++;
	}
	for (int i=0; i<n3; i++) {
	  mat2(j,i) = k++;
	}
      }
      Stopwatch timer;
      for (int i=0; i<1000; i++) {
	LoMat_double mat3 = LCSMath::matMult (mat1, mat2);
      }
      cout << "1000x MatMult -10 " << timer.sdelta() << endl;
    }
    {
      const int n1=20;
      const int n2=30;
      const int n3=40;
      LoMat_double mat1(n1,n2);
      LoMat_double mat2(n2,n3);
      int k = 0;
      for (int j=0; j<n2; j++) {
	for (int i=0; i<n1; i++) {
	  mat1(i,j) = k++;
	}
	for (int i=0; i<n3; i++) {
	  mat2(j,i) = k++;
	}
      }
      Stopwatch timer;
      for (int i=0; i<1000; i++) {
	LoMat_double mat4(n1,n3);
	mat4 = sum(mat1(blitz::tensor::i, blitz::tensor::k) *
		   mat2(blitz::tensor::k, blitz::tensor::j),
		   blitz::tensor::k);
      }
      cout << "1000x Blitz Mult  " << timer.sdelta() << endl;
    }
  } catch (...) {
    cout << "Unexpected exception" << endl;
    return 1;
  }
  std::cout << "OK" << std::endl;
  return 0;
}

//  lofar_pastd.cc
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

// Chris Broekema, november 2002 and february 2003

// This file implements the PASTd algorithm by B. Yang.

// Reference article:
// Bin Yang, Projection Approximation Subspace Tracking,
// IEEE Transactions on signal processing Vol. 43, pages 95-107,
// January 1995.

#include <PASTd.h>

using namespace blitz;


void pastd_step (LoVec_dcomplex x, LoMat_dcomplex W, LoVec_double d_tmp, int nmax, double Beta) {
  // This is the actual algorithm
  int nr = x.size();
  LoVec_dcomplex y; y.resize(nr);
  LoVec_dcomplex e; e.resize(nr);
  
  for (int n = 0; n < nr; n++) {
    
    y(n) = sum( W(Range::all(), n) * x );

    d_tmp(n) = (double)Beta * d_tmp(n) + abs(y(n)*y(n));

    e = x - (W(Range::all(), n) * y(n));

    W(Range::all(), n) = W(Range::all(), n) + (e * (conj(y(n)/d_tmp(n)))) ;

    x = x - (W(Range::all(), n) * y(n));
  }
}




int pastd (LoMat_dcomplex fifo, int numsnaps, int interval, double beta,
	   LoVec_double Evalue, LoMat_dcomplex Evector)
{
  
  int nantl = Evalue.ubound(firstDim) - Evalue.lbound(firstDim) + 1;

  LoMat_double d(nantl,numsnaps+1);  
  LoMat_dcomplex W(nantl, nantl);

  LoMat_dcomplex xq;
  LoVec_dcomplex x;
  LoVec_double d_tmp(nantl);

  // initialise using previous values (given as arguments),
  // or the real Eigen values and vectors using either EVD or SVD
  for (int i = 0; i < nantl; i++) {
    d(i,i) = Evalue(i);
  }
  //  d = Evalue;
  W = Evector;

  int nc = fifo.cols(); // fifo is row-major ordered
  int nr = fifo.rows();
  int snapcnt = -1;
  int epochs;

  xq.resize(nr, numsnaps);
  x.resize(nr);
  
  // this algorithm only used the last #numsnaps snapshots of the fifo 
  xq = fifo(Range::all(), Range(fifo.ubound(secondDim) - numsnaps + 1, toEnd));

  for (int snapidx = 0; snapidx < numsnaps; snapidx = snapidx + interval) {

    x = xq(Range::all(), snapidx);
    snapcnt++;

    d_tmp = d(Range::all(),snapcnt);
    pastd_step(x, W, d_tmp, nc, beta);
    d(Range::all(), snapcnt+1) = d_tmp;
  }

  // PASTd produces the conjugate of the EVD
  W = LCSMath::conj(W);

  // DEBUG
  // W should be equal to Evector
  double error = 0;
  //  cout << Evector << endl;
  for (int i = 0; i < nantl; i++) {
    error = error + abs(W(0,i) - Evector(0,i)) ;
  }
  // Return the new values for Eigen vectors and values.
  Evector = W;
  Evalue  = LCSMath::diag(d);

  return 0;  
}

//  MDL.cc
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

// Chris Broekema, november 2002.



/**************************************************************************/
/* This file implements the Minimun Description Length algorithm. This is */
/* used to estimate the number of interfering sources in a input signal.  */
/* MDL should give good answers, with an occasional over estimation.      */
/**************************************************************************/

#include <MDL.h>

/* TODO: Check if the cast to real needs to be done earlier */

using namespace blitz;

unsigned int mdl (LoMat_double d_mat, unsigned int nantl, unsigned int nsnsh)
{
  double nom;
  double denom;
  double mdlmin;
  int nnsnsh=-nsnsh;
  unsigned int kmin = 0;
  LoVec_double d;
  LoVec_double MDL(d_mat.cols());

  if (d_mat.cols() == d_mat.rows()) {
    d.resize(d_mat.cols());

    d = LCSMath::diag( d_mat );
    d = LCSMath::sort( d );
    d.reverseSelf(firstDim);
    for ( int k = d.lbound(firstDim); k < d.ubound(firstDim); k++ ) {

      denom = (1 /(double)(d.ubound(firstDim)-k)) * sum(d( Range( k+1, d.ubound(firstDim) ) ) ) ;
      nom = product(d(Range(k+1,d.ubound(firstDim)))) ;
      MDL(k) = nnsnsh * log(nom/(pow(denom,(1/(nantl-(k+1)))))) + (k+1)/2 * (2*nantl-(k+1)) * log(nsnsh);
      if ( k == 0 ) {
	// init
	kmin = k+1;
	mdlmin = MDL(k);
      } else {
	if (MDL(k) < mdlmin ) {
	  kmin = k+1;
	  mdlmin = MDL(k);
	}
      }
    }
  } else {
    cout << "MDL encountered non-square matrix" << endl;
  }
  return kmin;
}

unsigned int mdl (LoVec_double d_mat, unsigned int nantl, unsigned int nsnsh)
{
  double nom;
  double denom;
  double mdlmin;
  unsigned int kmin = 0;
  LoVec_double d;
  LoVec_double MDL(d_mat.size());

  d.resize(d_mat.size());
  d = d_mat;
  d = LCSMath::sort(d_mat);
  d.reverseSelf(firstDim);
  nsnsh=1;

  int nnsnsh=-nsnsh;
  for ( int k = d.lbound(firstDim); k < d.ubound(firstDim); k++ ) {

    denom = (1 /(double)(d.ubound(firstDim)-k)) * sum(d( Range( k+1, d.ubound(firstDim) ) ) ) ;
    nom = product(d(Range(k+1,d.ubound(firstDim)))) ;
    MDL(k) = nnsnsh * log(nom/(pow(denom,(1/(nantl-(k+1)))))) + (k+1)/2 * (2*nantl-(k+1)) * log(nsnsh);    cout << "MDL(k) : " << MDL(k) << endl;

    if ( k == 0 ) {
      // init
      kmin = k+1;
      mdlmin = MDL(k);
    } else {
      if (MDL(k) < mdlmin ) {
	kmin = k+1;
	mdlmin = MDL(k);
      }
    }
  }
  return kmin;
}

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


/**************************************************************************/
/* This file implements the Minimun Description Length algorithm. This is */
/* used to estimate the number of interfering sources in a input signal.  */
/* MDL should give good answers, with an occasional over estimation.      */
/**************************************************************************/

#include <MDL.h>


//using namespace blitz;

int mdl (const LoVec_double& d, int M, int N)
{
  double      nom;
  double      denom;
  double      mdlmin;
  int         min = 0;

  LoVec_double MDL (d.size ());

  for (int Ns = 0; Ns < M; Ns++) 
	{
	  denom = 0;
	  for (int i = Ns; i < M; i++) {
		denom += d (i);
	  }
	  denom *= (1 / (double) (M - Ns));
	  denom = pow (denom, (double) (M - Ns));

	  nom = 1;
	  for (int i = Ns; i < M; i++) {
		nom *= d (i);
	  }
	  
	  MDL (Ns) = -1 * N * log (abs (nom / denom)) + Ns / 2.0 * (2 * M - Ns) * log ((double) N); 

// 	  cout << nom << "    " << denom << "    " << MDL (Ns) << endl;

	  if (Ns == 0) {
		  mdlmin = MDL(Ns);
	  } else {
		if (MDL (Ns) < mdlmin) {
		  min = Ns;
		  mdlmin = MDL (Ns);
		}
	  }
	}
  return min;
}

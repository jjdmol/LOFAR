//  realft.h:
//
//  Copyright (C) 2000, 2001
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
//
//  $Log$
//  Revision 1.4  2002/05/08 14:21:20  wierenga
//  Only comments are allowed after #endif
//
//  Revision 1.3  2001/10/26 10:06:28  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.2  2001/02/05 14:53:05  loose
//  Added GPL headers
//

// realft.h: FFT of single real function
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_REALFT_H
#define BASESIM_REALFT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/** FFT of Single Real Function. 
	Calculates the Fourier Transform of a set of #n# real-valued data
	points. Replaces this data (which is stored in array #data[1..n]#)
	by the positive frequency half of its complex Fourier
	transform. The real-valued first and last components of the
	complex transform are returned as elements #data[1]# and
	#data[2]#, respectively. #n# must be a power of 2. This routine
	also calculates the inverse transform of a complex data array if
	it is the transform of real data. (Result in this case must be
	multiplied by #2/n#.)
*/

void realft(float data[], unsigned long n, int isign);


#endif /*BASESIM_REALFT_H*/

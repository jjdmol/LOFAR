//  four1.h:
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
//  Revision 1.1.1.1  2003/02/21 11:14:36  schaaf
//  copy from BaseSim tag "CEPFRAME"
//
//  Revision 1.5  2001/10/26 10:06:28  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.4  2001/02/05 14:53:05  loose
//  Added GPL headers
//

// four1.h: fast fourier transform
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_FOUR1_H
#define BASESIM_FOUR1_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

namespace LOFAR
{

/** Fast Fourier Transform (FFT).
	Replaces #data[1..2*nn]# by its discrete Fourier transform, if
	#isign# is input as 1; or replaces #data[1..2*nn] by #nn# times
	its inverse discrete Fourier transform, if #isign# is input as
	-1. #data# is a complex array of length #nn# or, equivalenty, a
	real array of length #2*nn#. #nn# {\bf must} be an integer power
	of 2 (this is not checked for!).
*/
void four1 (float data[], unsigned long nn, int isign);

}

#endif

//  Window.h
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

// Not yet implemented : chebwin(n, a) 
// 20-11 : Added welch window, supposedly better than Bartlett.
//       : Added Kaiser window, based on 0th order modified Bessel function

#ifndef STATIONSIM_WINDOW
#define STATIONSIM_WINDOW 1

#include <Common/Lorrays.h>

namespace stationsim_window
{
  
  LoVec_double welch (int size);
  LoVec_double hann (int size);
  LoVec_double hamming (int size);
  LoVec_double bartlett (int size);
  LoVec_double boxcar (int size);
  LoVec_double blackman (int size);
  LoVec_double kaiser (int size, double x);

}
double bessi0(double x);
#endif

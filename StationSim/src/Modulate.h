//  Modulate.h:
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
//

#ifndef DATAGEN_MODULATE_H
#define DATAGEN_MODULATE_H 1

#include <Common/Debug.h>
#include <Common/Lorrays.h>

// implements the cumsum() function for blitz vectors:
// (if y = cumsum(x), then y[i] = sum(x[0:i]))
template < class T >
    blitz::Array < T, 1 > cumsum (const blitz::Array < T, 1 > &x);

using namespace blitz;

namespace modulate 
{
  // AM, double sideband, suppreseed carrier
  // y = x*cos(2*pi*fc*t)
  LoVec_double amdsb (const LoVec_double & x,	// input time series
					  double fc,	// carrier frequency
					  double fs,	// sampling frequency
					  double phase);	// phase offset for carrier frequency

  // AM, double sideband, transmitted carrier
  // y = (x-opt)*cos(2*pi*fc*t)
  // if opt is not supplied, min(x) is used
  LoVec_double amdsb_tc (const LoVec_double & x,	// input time series
						 double fc,	// carrier frequency
						 double fs,	// sampling frequency
						 double phase,	// phase
						 double opt = 0);	// base
  
  // AM, single sideband
  // y = x*cos(2*pi*fc*t) + imag(hilbert(x))*sin(2*pi*fc*t)
  // not yet implemented
  LoVec_double amssb (const LoVec_double & x,	// input time series
					  double fc,	// carrier frequency
					  double fs,	// sampling frequency
					  double phase);
  
  // FM
  // y = cos(2*pi*fc*t + opt*cumsum(x))
  LoVec_double fm (const LoVec_double & x,	// input time series
				   double fc,	// carrier frequency
				   double fs,	// sampling frequency 
				   double opt,	// determines excursion in fq
				   double phase);
  
  // PM
  // y = cos(2*pi*fc*t + opt*x)
  LoVec_double pm (const LoVec_double & x,	// input time series
				   double fc,	// carrier frequency
				   double fs,	// sampling frequency 
				   double opt,	// determines excursion in phase
				   double phase);
  
};				// namespace modulate

#endif

//  Modulate.cc:
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

#include <Common/ArrayOperations.h>
#include <DataGen/Modulate.h>
#include <fftw.h>
#include <rfftw.h>

double theP;

// implements the cumsum() function for blitz vectors:
// (if y = cumsum(x), then y[tensor::i] = sum(x[0:tensor::i]))
template < class T >
    blitz::Array < T, 1 > cumsum (const blitz::Array < T, 1 > &x)
{
  using blitz::Array;

  Array < T, 1 > y (x.size ());
  T sum = 0;
  typename Array < T, 1 >::iterator out = y.begin ();

  for (typename Array < T, 1 >::const_iterator in = x.begin (); in != x.end (); ++in, ++out) {
    *out = sum += *in;
  }
  return y;
}

template blitz::Array < double, 1 > cumsum (const blitz::Array < double, 1 > &x);

namespace modulate 
{
  // copied from ArrayOperations.cc, since I can't be bothered to fix that right
  // now
  LoVec_dcomplex hilbert (const LoVec_double& input) 
  {
    int N = input.size ();
	int	centre;
    rfftwnd_plan fftplancomplex_forward = rfftwnd_create_plan (1, 
															   &N, 
															   FFTW_REAL_TO_COMPLEX,
															   FFTW_ESTIMATE);
    fftw_plan fftplancomplex_backward =	fftw_create_plan (N, FFTW_BACKWARD, FFTW_ESTIMATE);
    LoVec_dcomplex output (N);

    rfftwnd_one_real_to_complex (fftplancomplex_forward,
								 (fftw_real *) input.data (),
								 (fftw_complex *) output.data ());

    if (N % 2 != 0) {
      centre = N / 2;
	} else {
      centre = N / 2 - 1;
	}

    output (Range (1, centre)) *= 2;
    fftw_one (fftplancomplex_backward, 
			  (fftw_complex *) output.data (),
			  (fftw_complex *) output.data ());
    output /= N;


    rfftwnd_destroy_plan (fftplancomplex_forward);
    fftw_destroy_plan (fftplancomplex_backward);

    return output;
  }

  // AM, double sideband, suppressd carrier   
  // y = x*cos(2*pi*fc*t)  
  LoVec_double amdsb (const LoVec_double &x,
					  double fc, 
					  double fs, 
					  double phase) 
  {
    LoVec_double y (x.size ());

    y = x * cos (2 * M_PI * fc * (tensor::i / fs + phase));
    
	return y;
  }

  // AM, double sideband, transmitted carrier   
  // y = (x-opt)*cos(2*pi*fc*t)  
  LoVec_double amdsb_tc (const LoVec_double& x,
						 double fc, 
						 double fs, 
						 double phase, 
						 double opt) 
  {
    LoVec_double y (x.size ());

    if (opt == 0) {
      opt = min (x);
	}
    
	y = (x - opt) * cos (2 * M_PI * fc * (tensor::i / fs + phase));
    
	return y;
  }

  // AM, single sideband
  // y = x*cos(2*pi*fc*t) + imag(hilbert(x))*sin(2*pi*fc*t)
  LoVec_double amssb (const LoVec_double& x,
					  double fc, 
					  double fs, 
					  double phase) 
  {
    LoVec_double y (x.size ());

    y = x * cos (2 * M_PI * fc * (tensor::i / fs + phase)) +
	  imag (hilbert (x)) * sin (2 * M_PI * fc * (tensor::i / fs + phase));

    return y;
  }

  // FM
  // y = cos(2*pi*fc*t + opt*cumsum(x))
  LoVec_double fm (const LoVec_double& x,
				   double fc,	
				   double fs,	
				   double opt,	
				   double phase) 
  {
    LoVec_double y (x.size ());

    if (opt == 0) {
      opt = (fc / fs) * 2 * M_PI * max (x);
	}

    LoVec_double c = x;
    c (0) += theP;
    c = cumsum (c);
    theP = c (x.size () - 1);
    c *= opt;

    y = cos (2 * M_PI * fc * (tensor::i / fs + phase) + c);

    return y;
  }

  // PM
  // y = cos(2*pi*fc*t + opt*x)
  LoVec_double pm (const LoVec_double& x,
				   double fc,	
				   double fs,	
				   double opt,	
				   double phase) 
  {
    LoVec_double y (x.size ());

    if (opt == 0) {
      opt = M_PI / max (x);
	}

    y = cos (2 * M_PI * fc * (tensor::i / fs + phase) + opt * x);

    return y;
  }
}				// namespace modulate

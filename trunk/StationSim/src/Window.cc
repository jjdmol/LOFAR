//  Window.cc
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

#include <Window.h>
#include <math.h>

#define PI 3.1415926535897932384626433832795029

using namespace blitz;

namespace stationsim_window
{

  LoVec_double hamming (int size)
  {
    LoVec_double win(size);
    win = 0;
    
    double stride = (double) 2/(size-1);
    double a = -1;
    int index = win.lbound(firstDim);
    
    for (int i = 0; i < size; i++) {
      a = -1 + i * stride;
      win(index) = 0.54 + 0.46*cos(PI*a) ;
      index++;
    }
    return win;
  }

  LoVec_double hann (int size)
  {
    LoVec_double win(size);
    win = 0;
    
    double stride = (double) 2/(size-1);
    double a = -1;
    int index = win.lbound(firstDim);
    
    for (int i = 0; i < size; i++) {
      a = -1 + i * stride;
      win(index) = (1 + cos(PI*a)) / 2 ;
      index++;
    }
    return win;
  }

  LoVec_double bartlett (int size) 
  {
    LoVec_double win(size);
    win = 0;
    double stride = (double) 2/(size-1);
    int index = win.lbound(firstDim);
   
    for (double i = -1; i <= 1; i=i+stride) {
      win(index) = 1 - abs(i);
      index++;
    }
    return win;
  }

  LoVec_double boxcar (int size)
  {
    LoVec_double win(size);
    win = 1;
    return win;
  }
  
  LoVec_double blackman (int size)
  {
    LoVec_double win(size);
    win = 0;
    double stride = (double) 2/(size-1);
    double a = -1;
    int index = win.lbound(firstDim);

    for (int i = 0; i < size; i++) {
      a = -1 + i * stride;
      win(index) = 0.42 + 0.5 * cos(PI*a) + 0.08 * cos(2*PI*a) ;
      index++;
    }
    return win;
  }

  LoVec_double welch (int size) 
  {
    LoVec_double win(size);
    win = 0;
    double stride = (double) 2/(size-1);
    int index = win.lbound(firstDim);
   
    for (double i = -1; i <= 1; i=i+stride) {
      win(index) = 1 - pow(i,2);
      index++;
    }
    return win;
  }



  LoVec_double kaiser (int size, double x)
  {
    LoVec_double win(size);
    win = 0;
    double stride = (double) 2/(size-1);
    int index = win.lbound(firstDim);
    double base = bessi0(x);

    for (double i = -1; i <= 1; i=i+stride) {
      double tmp = bessi0(x * sqrt(1 - pow(i, 2)));
      win(index) = tmp / base;
      index++;
    }
    return win;
  }
}
  // polynomial approximation of the zeroth order modified Bessel function
  // from the Numerical Recipes in C p. 237
  // Used by the kaiser window
double bessi0(double x)
{
  double ax,ans;
  double y;
  
  ax=fabs(x);
  if (ax < 3.75)
    {
      y=x/3.75;
      y*=y;
      ans=1.0+y*(3.5156229+y*(3.0899424+y*(1.2067492
	     +y*(0.2659732+y*(0.360768e-1+y*0.45813e-2)))));
    }
  else
    {
      y=3.75/ax;
      ans=(exp(ax)/sqrt(ax))*(0.39894228+y*(0.1328592e-1
		+y*(0.225319e-2+y*(-0.157565e-2+y*(0.916281e-2
		+y*(-0.2057706e-1+y*(0.2635537e-1+y*(-0.1647633e-1
		+y*0.392377e-2))))))));
    }
  return ans;
}
  



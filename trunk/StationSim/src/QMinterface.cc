//  QMinterface.cc
//
//  Copyright (C) 2003
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

// Chris Broekema, april 2003.

// This interface reads an integer and returns it's "binary" equivalent.
// A workholder can call this function at creation with its QM integer,
// and at run time check only the relavant positions in the bool array
// to see which of the QM's it needs to provide.

// 1 = Plot beamshape
// 2 = Plot spectrum before and after beamforming


#include <QMinterface.h>
#include <blitz/blitz.h>
#include <blitz/array.h>

blitz::Array<bool, 1> qminterface ( const int input ) {
  int temp = input;

  blitz::Array<bool, 1> result (QM_BASE_SIZE) ;
  result = false;
  for ( int i = QM_BASE_SIZE; i > 1; i--) {
    double v = pow( 2, (double)(i-1) );
    if (temp / v >= 1) {
      
      result(i-1) = true;
      temp = temp - (int) v;
    }
  }
  if (temp == 1) {
    result(0) = true;
  } 

  return result;  
}

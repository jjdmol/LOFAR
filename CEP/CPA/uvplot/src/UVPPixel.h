//
// Copyright (C) 2002
// ASTRON (Netherlands Foundation for Research in Astronomy)
// P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#if !defined(UVPPIXEL_H)
#define UVPPIXEL_H

// $Id$


// Includes
//#include <NFRA.h>

#include <vector>
#include <string>

// Forward declarations


// The UVPPixel class implements a mapping between pixel coordinates in
// a data image or data cube and the row indices of all datapoints
// that are used to calculate the value of the pixel.
// 
// The exact quantity that is stored is unknown to the UVPPixel
// object. That knowledge belongs to the object that manages the
// UVPPixels.
//
class UVPPixel
{
 public:
  
  // Default constructor. The object initially contains no data, so
  // itsValue is set to zero and so is itsWeight.
  UVPPixel();
  

  // Add a new point to this UVPPixel. Value is simply the quantity
  // that should be plotted, rowIndex is the zero-relative index in
  // the UV database of the point that is added. Weight speaks for
  // itself. weight must be larger > 0. The method returns false on
  // error.
  //  bool    addPoint(double value,
  //                   int    rowIndex,
  //                   double weight);

  bool    addPointUniform(double value,
                          int    rowIndex);

  // Returns the weighted average value of this pixel.
  inline const double  *getAverageValue   () const;

  // Returns a vector of zero-relative row indices of all points that
  // were mapped to this pixel.
  std::vector<int> getRowIndex() const;



 private:

  // Weighted average value of all points that are mapped to this
  // particular pixel.
  double              itsAverageValue;
  
  // List of the values of all datapoints that have been added to the
  // pixel up to now.
  std::vector<double> itsValue;

  // itsWeight is the accumulated weight of all points that were used
  // to calculate itsValue.
  std::vector<double> itsWeight;

  // vector (list) of all row indices that are used to calculate
  // itsValue. Row indices are zero-relative in C++, but one-relative
  // if presented to the user.
  std::vector<int>    itsRowIndex;
};



//====================>>>  UVPPixel::getAverageValue  <<<====================

inline const double *UVPPixel::getAverageValue() const
{
  return &itsAverageValue;
}




#endif // UVPPIXEL_H

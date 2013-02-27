//# BaselineSelect.h: Convert MSSelection baseline string to a Matrix
//#
//# Copyright (C) 2010
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//#  $Id$

#ifndef MS_BASELINESELECT_H
#define MS_BASELINESELECT_H

// @file
// Convert MSSelection baseline string to a Matrix
// @author Ger van Diepen (diepen AT astron nl)

//# Includes
#include <Common/lofar_string.h>

//# Forward Declarations
namespace casa
{
  template<class T> class Matrix;
}

namespace LOFAR
{

// @ingroup MS
// @brief Convert MSSelection baseline string to a Matrix
// @{

// Class with a static function to convert a casacore MSSelection baseline
// string to a Matrix<Bool> telling which baselines are selected.

class BaselineSelect
{
public:
  // Parse the MSSelection baseline string and create a Matrix telling
  // which baselines are selected.
  static casa::Matrix<bool> convert (const string& msName,
                                     const string& baselineSelection);
};

// @}

} // end namespace

#endif

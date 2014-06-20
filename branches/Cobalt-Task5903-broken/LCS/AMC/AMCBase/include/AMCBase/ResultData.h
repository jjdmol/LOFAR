//# ResultData.h: Result data that will be received from the converter
//#
//# Copyright (C) 2002-2004
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
//# $Id$

#ifndef LOFAR_AMCBASE_RESULTDATA_H
#define LOFAR_AMCBASE_RESULTDATA_H

// \file
// Result data that will be received from the converter

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <AMCBase/Direction.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
  namespace AMC
  {
    // \addtogroup AMCBase
    // @{

    // Struct wrapping the result data that will be received from the
    // converter. The purpose of this struct is to hide the exact structure of
    // the data to be received from the converter. It is not really necessary
    // to make the data members private, since we need to have easy access to
    // the data members. However, note that, because the data members are
    // public, they become part of the interface! So, be careful in renaming
    // them. A number of constructors have been defined in order to ease the
    // creation of a %ResultData instance.
    struct ResultData
    {
    public:
      ResultData()
      {}

      ResultData(const vector<Direction>& dir) :
        direction(dir)
      {}

      ResultData(const Direction& dir) :
        direction(1, dir)
      {}

      vector<Direction> direction;
    };

    // @}

  } // namespace AMC

} // namespace LOFAR

#endif

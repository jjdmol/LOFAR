//# CasaGridder.h: Gridder for LOFAR data correcting for DD effects
//#
//# Copyright (C) 2009
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
//#
//# @author Ger van Diepen <diepen at astron dot nl>

#ifndef LOFAR_LOFARFT_CASAGRIDDER_H
#define LOFAR_LOFARFT_CASAGRIDDER_H

#include <Common/ParameterSet.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
  // @brief Gridder for LOFAR data correcting for DD effects
  //
  // @ingroup testgridder

  class CasaGridder /*: public askap::synthesis::IVisGridder*/
  {
  public:

    // Construct from the given parset.
    explicit CasaGridder (const ParameterSet&);

    virtual ~CasaGridder();

    // @brief Return the (unique) name of the gridder.
    static const std::string& gridderName();

    // @brief Register the gridder create function with its name.
    static void registerGridder();

  private:
    //# Data members.
  };

} //# end namespace

#endif

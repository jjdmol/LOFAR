//# LofarFTMachine.h: Gridder for LOFAR data correcting for DD effects
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

#ifndef LOFAR_CASAGRIDDER_LOFARFTMACHINE_H
#define LOFAR_CASAGRIDDER_LOFARFTMACHINE_H

#include <synthesis/MeasurementComponents/AWProjectFT.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

//# Forward declaration.
namespace casa {
  class Imager;
}

namespace LOFAR
{
  // @brief Gridder for LOFAR data correcting for DD effects
  //
  // @ingroup testgridder

  class LofarFTMachine : public casa::AWProjectFT
  {
  public:

    // Construct from the Imager object.
    explicit LofarFTMachine (const casa::Imager&);

    virtual ~LofarFTMachine();

    // Clone the object.
    LofarFTMachine* clone() const;

    // Return the (unique) name of the gridder.
    static const std::string& gridderName();

    // Create the FTMachine object.
    static FTMachine* createMachine (const casa::Imager&);

    // Register the gridder create function with its name.
    static void registerGridder();

  private:
    //# Data members.
  };

} //# end namespace

#endif

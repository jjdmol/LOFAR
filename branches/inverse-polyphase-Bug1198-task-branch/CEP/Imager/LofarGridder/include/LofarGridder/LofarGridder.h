//# LofarGridder.h: Test visibility gridder.
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

#ifndef LOFAR_LOFARGRIDDER_TESTGRIDDER_H
#define LOFAR_LOFARGRIDDER_TESTGRIDDER_H

#include <gridding/TableVisGridder.h>
#include <Common/ParameterSet.h>

namespace LOFAR
{
  // @brief Gridder to test dynamic loading of gridders
  //
  // @ingroup testgridder
  class LofarGridder : public askap::synthesis::TableVisGridder
  {
  public:

    // Standard two dimensional box gridding
    LofarGridder();

    // Clone this Gridder
    virtual askap::synthesis::IVisGridder::ShPtr clone();

    virtual ~LofarGridder();

    // @brief Function to create the gridder from a parset.
    // This function will be registered in the gridder registry.
    static askap::synthesis::IVisGridder::ShPtr makeGridder
    (const ParameterSet&);
    // @brief Return the (unique) name of the gridder.
    static const std::string& gridderName();

    // @brief Register the gridder create function with its name.
    static void registerGridder();

    // @brief Initialise the indices
    // @param[in] acc const data accessor to work with
    virtual void initIndices(const askap::synthesis::IConstDataAccessor& acc);

    // @brief Correct for gridding convolution function
    // @param image image to be corrected
    virtual void correctConvolution(casa::Array<double>& image);
				
  protected:
    // Initialize convolution function
    // @param[in] acc const data accessor to work with
    virtual void initConvolutionFunction
    (const askap::synthesis::IConstDataAccessor& acc);
  };

} //# end namespace

#endif

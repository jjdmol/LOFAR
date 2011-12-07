//# LofarWProjectGridder.h: Gridder for LOFAR data correcting for DD effects
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

#ifndef LOFAR_LOFARGRIDDER_LOFARWPROJECTGRIDDER_H
#define LOFAR_LOFARGRIDDER_LOFARWPROJECTGRIDDER_H

//# LOFAR includes
#include <BBSKernel/StationResponse.h>
#include <Common/ParameterSet.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

//# askap includes
#include <gridding/WProjectVisGridder.h>

//# casacore includes
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Quanta/MVDirection.h>

namespace LOFAR
{
  // @brief Gridder for LOFAR data correcting for DD effects
  //
  // @ingroup testgridder

  class LofarWProjectGridder : public askap::synthesis::WProjectVisGridder
  {
  public:

    // Construct from the given parset.
    explicit LofarWProjectGridder (const ParameterSet&);

    // Clone this Gridder
    virtual askap::synthesis::IVisGridder::ShPtr clone();

    virtual ~LofarWProjectGridder();

    // @brief Function to create the gridder from a parset.
    // This function will be registered in the gridder registry.
    static askap::synthesis::IVisGridder::ShPtr makeGridder
    (const ParameterSet&);

    // @brief Return the (unique) name of the gridder.
    static const std::string& gridderName();

    // @brief Register the gridder create function with its name.
    static void registerGridder();

  private:
    // Correct the visibilities before gridding or after degridding.
    void correctVisibilities (askap::accessors::IDataAccessor& acc,
                              bool forward);

    // Initialize the corrections.
    void initCorrections (const askap::accessors::IConstDataAccessor& acc);

    // Form an Instrument object.
    BBS::Instrument makeInstrument (const casa::MeasurementSet& ms);


    //# Data members.
    // Is the gridder correction initialized?
    bool itsInitialized;
    // The ParmDB to use.
    string itsParmDBName;
    // The StationResponse object to calculate the corrections.
    BBS::StationResponse::Ptr itsResponse;
    // Time and frequency interval in MS.
    double itsTimeInterval;
    double itsFreqInterval;
    double itsStartFreq;
    // The last facet center used.
    casa::MVDirection itsLastFacetCenter;
  };

} //# end namespace

#endif

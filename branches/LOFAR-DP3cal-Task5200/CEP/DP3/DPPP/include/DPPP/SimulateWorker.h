//# SimulateWorker.h: Simulate helper class processing some time / bl/ channel
//# Copyright (C) 2013
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
//# $Id: Demixer.h 23223 2012-12-07 14:09:42Z schoenmakers $
//#
//# @author Tammo Jan Dijkema

#ifndef DPPP_SIMULATEWORKER_H
#define DPPP_SIMULATEWORKER_H

#include <lofar_config.h>

#include <DPPP/DPInput.h>
#include <DPPP/DPBuffer.h>
#include <DPPP/Baseline.h>
#include <DPPP/Patch.h>
#include <DPPP/Position.h>

#include <StationResponse/Station.h>
#include <StationResponse/Types.h>

#include <casa/Arrays/Vector.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasFrame.h>
#include <vector>

namespace LOFAR {

  namespace DPPP {



    class SimulateWorker
    {
    typedef vector<StationResponse::Station::Ptr> BeamInfoList;

    public:
      // Construct the object.
      // Parameters are obtained from the parset using the given prefix.
      SimulateWorker (DPInput* input, const DPInfo& dpinfo,
                      vector<Patch::ConstPtr>* patchList,
                      uint firstBl, uint lastBl, uint firstCh, uint lastCh);


      // Process the data in the input buffers and store the result in the
      // output buffers.
      void process (const DPBuffer& bufin);

      // Get the timings of the various processing steps.
      // <group>
      double getTotalTime() const
        { return itsTimer.getElapsed(); }
      // </group>



    private:
      // Calculate the beam for the given sky direction and frequencies.
      // Apply it to the data.
      // If apply==False, nothing is done.
      void applyBeam (double time, const Position& pos, bool apply,
                      const casa::Vector<double>& chanFreqs,
                      dcomplex* data);

      // Convert a direction to ITRF.
      StationResponse::vector3r_t dir2Itrf (const casa::MDirection&);

      //# Data members.
      vector<Patch::ConstPtr>*               itsPatchList;
      uint                                   itsNSt; // number of stations
      double                                 itsRefFreq;
      //# The info needed to calculate the station beams.
      vector<StationResponse::Station::Ptr>  itsAntBeamInfo;
      //# Measure objects unique to this worker (thread).
      //# This is needed because they are not thread-safe.
      Position                               itsPhaseRef;
      casa::MDirection                       itsDelayCenter;
      casa::MDirection                       itsTileBeamDir;

      //# Variables for conversion of directions to ITRF.
      casa::MeasFrame                        itsMeasFrame;
      casa::MDirection::Convert              itsMeasConverter;
      vector<StationResponse::matrix22c_t>   itsBeamValues;  //# [nst,nch]


      //# Variables for the predict.
      vector<double>                         itsUVW;
      vector<dcomplex>                       itsModelVisPatch;  //# temp buffer
      vector<dcomplex>                       itsModelVis;

      vector<Baseline>                       itsBaselines;
      casa::Vector<double>                   itsChanFreqs;
      bool                                   itsApplyBeam;

      //# Timers.
      NSTimer                                itsTimer;
    };

  } //# end namespace
} //# end namespace

#endif

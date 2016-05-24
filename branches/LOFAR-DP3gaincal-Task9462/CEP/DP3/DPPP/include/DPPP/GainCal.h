//# GainCal.h: DPPP step class to calibrate (direction independent) gains
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
//# $Id: GainCal.h 21598 2012-07-16 08:07:34Z diepen $
//#
//# @author Tammo Jan Dijkema

#ifndef DPPP_GAINCAL_H
#define DPPP_GAINCAL_H

// @file
// @brief DPPP step class to apply a calibration correction to the data

#include <DPPP/DPInput.h>
#include <DPPP/DPBuffer.h>
#include <DPPP/StefCal.h>
#include <DPPP/Patch.h>
#include <DPPP/Predict.h>
#include <ParmDB/ParmFacade.h>
#include <ParmDB/ParmSet.h>
#include <DPPP/SourceDBUtil.h>
#include <DPPP/ApplyBeam.h>
#include <StationResponse/Station.h>
#include <StationResponse/Types.h>
#include <ParmDB/Parm.h>
#include <casa/Arrays/Cube.h>
#include <casa/Quanta/MVEpoch.h>
#include <measures/Measures/MEpoch.h>
#include <casa/Arrays/ArrayMath.h>

namespace LOFAR {

  class ParameterSet;

  namespace DPPP {
    // @ingroup NDPPP

    // This class is a DPStep class to calibrate (direction independent) gains.

    typedef vector<Patch::ConstPtr> PatchList;
    typedef std::pair<size_t, size_t >Baseline;

    class GainCal: public DPStep
    {
    public:
      // Construct the object.
      // Parameters are obtained from the parset using the given prefix.
      GainCal (DPInput*, const ParameterSet&, const string& prefix);

      virtual ~GainCal();

      // Process the data.
      // It keeps the data.
      // When processed, it invokes the process function of the next step.
      virtual bool process (const DPBuffer&);

      // Finish the processing of this step and subsequent steps.
      virtual void finish();

      // Update the general info.
      virtual void updateInfo (const DPInfo&);

      // Show the step parameters.
      virtual void show (std::ostream&) const;

      // Show the timings.
      virtual void showTimings (std::ostream&, double duration) const;


    private:
      // Perform stefcal (polarized or unpolarized)
      void stefcal();

      // Counts the number of antennas with non-flagged data,
      // Set a map for the used antennas in iS, returns the number of antennas
      uint setAntennaMaps (const casa::Bool* flag, uint freqCell);

      // Remove rows and colums corresponding to antennas with too much
      // flagged data from vis and mvis
      void removeDeadAntennas ();

      // Fills the matrices itsVis and itsMVis
      void fillMatrices (casa::Complex* model, casa::Complex* data,
                         float* weight, const casa::Bool* flag);

      // Initialize the parmdb
      void initParmDB();

      // Write out the solutions of the current parameter chunk (timeslotsperparmupdate)
      void writeSolutions (double startTime);

      //# Data members.
      DPInput*         itsInput;
      string           itsName;
      DPBuffer         itsBuf;
      bool             itsUseModelColumn;
      casa::Cube<casa::Complex> itsModelData;
      string           itsParmDBName;
      shared_ptr<BBS::ParmDB> itsParmDB;

      string           itsMode;

      uint             itsDebugLevel;
      bool             itsDetectStalling;

      vector<Baseline> itsBaselines;

      vector<casa::Cube<casa::DComplex> > itsSols; // for every timeslot, nSt x nCr x nFreqCells

      std::vector<StefCal>  iS;

      Predict          itsPredictStep;
      ApplyBeam        itsApplyBeamStep; // Beam step for applying beam to modelcol
      ResultStep*      itsResultStep; // For catching results from Predict or Beam
      bool             itsApplyBeamToModelColumn;

      casa::Vector<casa::String> itsAntennaUsedNames;
      map<string,int>  itsParmIdMap; //# -1 = new parm name

      uint             itsMaxIter;
      double           itsTolerance;
      bool             itsPropagateSolutions; // Not used currently, TODO: use this
      uint             itsSolInt;  // Time cell size
      uint             itsNChan;   // Frequency cell size
      uint             itsNFreqCells;
      uint             itsMinBLperAnt;

      uint             itsTimeSlotsPerParmUpdate;
      uint             itsConverged;
      uint             itsNonconverged;
      uint             itsStalled;
      uint             itsTimeStep; // Timestep within parameter update
      double           itsChunkStartTime; // First time value of chunk to be stored
      uint             itsNTimes;  // Timestep within solint
      NSTimer          itsTimer;
      NSTimer          itsTimerPredict;
      NSTimer          itsTimerSolve;
      NSTimer          itsTimerWrite;
      NSTimer          itsTimerFill;
    };

  } //# end namespace
}

#endif

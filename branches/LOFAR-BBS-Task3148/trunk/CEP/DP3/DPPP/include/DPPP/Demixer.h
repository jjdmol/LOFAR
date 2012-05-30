//# Demixer.h: DPPP step class to subtract A-team sources
//# Copyright (C) 2011
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
//# @author Ger van Diepen

#ifndef DPPP_DEMIXER_H
#define DPPP_DEMIXER_H

// @file
// @brief DPPP step class to average in time and/or freq

#include <DPPP/DPInput.h>
#include <DPPP/DPBuffer.h>
#include <DPPP/Patch.h>
#include <DPPP/PhaseShift.h>
#include <DPPP/BBSExpr.h>
#include <DPPP/Baseline.h>
#include <ParmDB/Axis.h>

#include <casa/Arrays/Cube.h>
#include <measures/Measures/MDirection.h>

// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
//#include <BBSKernel/VisDimensions.h>
//#include <DPPP/EstimateNew.h>
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST

#include <measures/Measures/MDirection.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Quanta/Quantum.h>

namespace LOFAR {

  namespace DPPP {
    class ParSet;

    // @ingroup NDPPP

    typedef vector<Patch>   PatchList;

    // This class is a DPStep class to subtract the strong A-team sources.
    // It is based on the demixing.py script made by Bas vd Tol and operates
    // per time chunk as follows:
    // <ul>
    //  <li> The data are phase-shifted and averaged for each source.
    //  <li> Demixing is done using the combined results.
    //  <li> For each source a BBS solve, smooth, and predict is done.
    //  <li> The predicted results are subtracted from the averaged data.
    // </ul>

    class Demixer: public DPStep
    {
    public:
      // Construct the object.
      // Parameters are obtained from the parset using the given prefix.
      Demixer (DPInput*, const ParSet&, const string& prefix);

      virtual ~Demixer();

      // Process the data.
      // It keeps the data.
      // When processed, it invokes the process function of the next step.
      virtual bool process (const DPBuffer&);

      // Finish the processing of this step and subsequent steps.
      virtual void finish();

      // Update the general info.
      virtual void updateInfo (DPInfo&);

      // Show the step parameters.
      virtual void show (std::ostream&) const;

      // Show the timings.
      virtual void showTimings (std::ostream&, double duration) const;

    private:
      void initUnknowns();

      // Solve gains and subtract sources.
      void demix();

      // Add the decorrelation factor contribution for each time slot.
      void addFactors (const DPBuffer& newBuf,
                       casa::Array<casa::DComplex>& factorBuf);

      // Calculate the decorrelation factors by averaging them.
      // Apply the P matrix to deproject the sources without a model.
      void makeFactors (const casa::Array<casa::DComplex>& bufIn,
                        casa::Array<casa::DComplex>& bufOut,
                        const casa::Cube<float>& weightSums,
                        uint nChanOut,
                        uint nChanAvg);

      // Deproject the sources without a model.
      void deproject (casa::Array<casa::DComplex>& factors,
                      vector<MultiResultStep*> avgResults,
                      uint resultIndex);

      // Convert a double value to a string (with sufficient precision).
      string toString (double value) const;

      // Convert a angle string with an optional unit to radians.
      // The default input unit is degrees.
      double getAngle (const casa::String& value) const;

      void dumpSolutions();

      //# Data members.
      DPInput*                 itsInput;
      string                   itsName;
      string                   itsSkyName;
      string                   itsInstrumentName;
//      double                   itsElevCutoff;   //# min source elevation (rad)
      vector<PhaseShift*>      itsPhaseShifts;
      vector<DPStep::ShPtr>    itsFirstSteps;   //# phaseshift/average steps
      vector<MultiResultStep*> itsAvgResults;   //# result of phaseshift/average
      MultiResultStep*         itsAvgResultSubtr; //# result of subtract avg
      string                   itsTargetSource; //# empty if no target model
      vector<string>           itsSubtrSources;
      vector<string>           itsModelSources;
      vector<string>           itsExtraSources;
      vector<string>           itsAllSources;
      vector<uint>             itsCutOffs;
      double                   itsTimeStart;
      double                   itsTimeInterval;
      vector<double>           itsTimeCenters;
      vector<double>           itsTimeWidths;
///      bool                     itsJointSolve;
      uint                     itsNDir;
      uint                     itsNModel;
      uint                     itsNBl;
      uint                     itsNCorr;
      uint                     itsNChanIn;
      uint                     itsNTimeIn;
      uint                     itsNChanOutSubtr;
      uint                     itsNChanAvgSubtr;
      uint                     itsNTimeAvgSubtr;
      uint                     itsNTimeChunkSubtr;
      uint                     itsNTimeOutSubtr;
      uint                     itsNChanOut;
      uint                     itsNChanAvg;
      uint                     itsNTimeAvg;
      uint                     itsNTimeChunk;
      uint                     itsNTimeOut;
      double                   itsStartTimeChunk;
      double                   itsTimeIntervalSubtr;
      double                   itsTimeIntervalAvg;
      casa::Array<casa::DComplex> itsFactorBuf; //# ncorr,nchan,nbl,ndir*ndir
      vector<casa::Array<casa::DComplex> > itsFactors; //# demix factors/time
      //# each Array is basically cube(ncorr,nchan,nbl) of matrix(ndir,ndir)
      casa::Array<casa::DComplex> itsFactorBufSubtr; //# factors for subtract
      vector<casa::Array<casa::DComplex> > itsFactorsSubtr;
      BBS::SolverOptions       itsSolveOpt;
      NSTimer                  itsTimer;
      NSTimer                  itsTimerPhaseShift;
      NSTimer                  itsTimerDemix;
      NSTimer                  itsTimerSolve;
      NSTimer                  itsTimerSubtract;

      uint                     itsTimeCount;
      PatchList                itsPatchList;
      vector<Baseline>         itsBaselines;
      casa::Vector<double>     itsFreqDemix;
      casa::Vector<double>     itsFreqSubtr;
      uint                     itsNTimeDemix;
      uint                     itsNStation;
      Position                 itsPhaseRef;
      casa::Array<double>      itsUnknowns;
      casa::Array<double>      itsErrors;
      casa::Array<double>      itsLastKnowns;
      vector<casa::MeasFrame>           itsFrames;
      vector<casa::MDirection::Convert> itsConverters;
    };

  } //# end namespace
}

#endif

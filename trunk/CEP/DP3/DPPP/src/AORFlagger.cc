//# AORFlagger.cc: DPPP step class to flag data based on rficonsole
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
//# $Id$
//#
//# @author Ger van Diepen

#include <lofar_config.h>
#include <DPPP/AORFlagger.h>
#include <DPPP/DPBuffer.h>
#include <DPPP/DPInfo.h>
#include <DPPP/ParSet.h>
#include <Common/LofarLogger.h>

#include <casa/OS/HostInfo.h>

#include <AOFlagger/msio/image2d.h>
#include <AOFlagger/msio/mask2d.h>
#include <AOFlagger/msio/timefrequencydata.h>
#include <AOFlagger/rfi/strategy/artifactset.h>
#include <AOFlagger/rfi/strategy/changeresolutionaction.h>
#include <AOFlagger/rfi/strategy/combineflagresults.h>
#include <AOFlagger/rfi/strategy/foreachcomplexcomponentaction.h>
#include <AOFlagger/rfi/strategy/foreachpolarisationblock.h>
#include <AOFlagger/rfi/strategy/frequencyselectionaction.h>
#include <AOFlagger/rfi/strategy/iterationblock.h>
#include <AOFlagger/rfi/strategy/setflaggingaction.h>
#include <AOFlagger/rfi/strategy/setimageaction.h>
#include <AOFlagger/rfi/strategy/slidingwindowfitaction.h>
#include <AOFlagger/rfi/strategy/statisticalflagaction.h>
#include <AOFlagger/rfi/strategy/thresholdaction.h>
#include <AOFlagger/rfi/strategy/timeselectionaction.h>

#include <Common/StreamUtil.h>
#include <Common/LofarLogger.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/RecordField.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/RecordGram.h>
#include <iostream>
#include <algorithm>

#ifdef _OPENMP
# include <omp.h>
#endif

using namespace casa;
using namespace rfiStrategy;

namespace LOFAR {
  namespace DPPP {

    AORFlagger::AORFlagger (DPInput* input,
                            const ParSet& parset, const string& prefix)
      : itsInput       (input),
        itsName        (prefix),
        itsBufIndex    (0),
        itsNTimes      (0),
        itsNTimesToDo  (0),
        itsFlagCounter (input, parset, prefix+"count."),
        itsMoveTime    (0),
        itsFlagTime    (0)
    {
      itsWindowSize  = parset.getUint   (prefix+"timewindow", 0);
      itsOverlap     = parset.getUint   (prefix+"overlap", 0);
      itsOverlapPerc = parset.getDouble (prefix+"overlapperc", 0);
      itsPulsarMode  = parset.getBool   (prefix+"pulsar", false);
      itsPedantic    = parset.getBool   (prefix+"pedantic", false);
      itsDoAutoCorr  = parset.getBool   (prefix+"autocorr", false);
      // Fill the strategy.
      fillStrategy (itsStrategy);
    }

    AORFlagger::~AORFlagger()
    {}

    void AORFlagger::show (std::ostream& os) const
    {
      os << "AOFlagger " << itsName << std::endl;
      os << "  timewindow:     " << itsWindowSize << std::endl;
      os << "  overlap:        " << itsOverlap << std::endl;
      os << "  pulsar:         " << itsPulsarMode << std::endl;
      os << "  pedantic:       " << itsPedantic << std::endl;
      os << "  autocorr:       " << itsDoAutoCorr << std::endl;
#ifdef _OPENMP
      uint nthread = omp_get_max_threads();
#else
      uint nthread = 1;
#endif
      os << "  nthreads (omp)  " << nthread << std::endl;
    }

    void AORFlagger::updateInfo (DPInfo& info)
    {
      info.setNeedVisData();
      info.setNeedWrite();
      // Get nr of threads.
#ifdef _OPENMP
      uint nthread = omp_get_max_threads();
#else
      uint nthread = 1;
#endif
      // Determine how much memory is available.
      double memory = HostInfo::memoryTotal() * 1024.;
      // Determine how much buffer space is needed per time slot.
      // The flagger needs 3 extra work buffers (data+flags) per thread.
      double timeSize = (sizeof(Complex) + sizeof(bool)) *
        (info.nbaselines() + 3*nthread) * info.nchan() * info.ncorr();
      // If no overlap is given, set it to 1%.
      if (itsOverlap == 0  &&  itsOverlapPerc == 0) {
        itsOverlapPerc = 1;
      }
      // If no time window given, determine it from the available memory.
      // Set 2 GB aside for other purposes.
      if (itsWindowSize == 0) {
        double nt = (memory - 2.*1024*1024*1024) / timeSize;
        if (itsOverlapPerc > 0) {
	  // Determine the overlap (add 0.5 for rounding).
	  // If itsOverLap is also given, it is the maximum.
          double tw = nt / (1 + 2*itsOverlapPerc/100);
          uint overlap = uint(itsOverlapPerc*tw/100 + 0.5);
          if (itsOverlap == 0  ||  overlap < itsOverlap) {
            itsOverlap = overlap;
          }
        }
        itsWindowSize = uint(std::max(1., nt-2*itsOverlap));
	// Make the window size divide the nr of times nicely (if known).
	// In that way we cannot have a very small last window.
	if (info.ntime() > 0) {
	  uint nwindow = 1 + (info.ntime() - 1) / itsWindowSize;
	  itsWindowSize = 1 + (info.ntime() - 1) / nwindow;
	  if (itsOverlapPerc > 0) {
	    uint overlap = uint(itsOverlapPerc*itsWindowSize/100 + 0.5);
	    if (overlap < itsOverlap) {
	      itsOverlap = overlap;
	    }
	  }
	}
      }
      if (itsOverlap == 0) {
        itsOverlap = uint(itsOverlapPerc*itsWindowSize/100);
      }
      // Check if it all fits in memory.
      ASSERTSTR ((itsWindowSize + 2*itsOverlap) * timeSize < memory,
                 "Timewindow " << itsWindowSize
                 << " and/or overlap " << itsOverlap
                 << " too large for available memory " << memory);
      // Size the buffer (need overlap on both sides).
      itsBuf.resize (itsWindowSize + 2*itsOverlap);
      // Initialize the flag counters.
      itsFlagCounter.init (info.nbaselines(), info.nchan(), info.ncorr());
      itsNTimesToDo = info.ntime();
    }

    void AORFlagger::showCounts (std::ostream& os) const
    {
      os << endl << "Flags set by AOFlagger " << itsName;
      os << endl << "=======================" << endl;
      itsFlagCounter.showBaseline (os, itsInput->getAnt1(),
                                   itsInput->getAnt2(), itsNTimes);
      itsFlagCounter.showChannel  (os, itsNTimes);
      itsFlagCounter.showCorrelation (os, itsNTimes);
    }

    void AORFlagger::showTimings (std::ostream& os, double duration) const
    {
      double flagDur = itsTimer.getElapsed();
      os << "  ";
      FlagCounter::showPerc1 (os, flagDur, duration);
      os << " AOFlagger " << itsName << endl;
      os << "          ";
      // move time and flag time are sum of all threads.
      // Scale them to a single elapsed time.
      double factor = (itsComputeTimer.getElapsed() /
                       (itsMoveTime + itsFlagTime));
      FlagCounter::showPerc1 (os, itsMoveTime*factor, flagDur);
      os << " of it spent in shuffling data" << endl;
      os << "          ";
      FlagCounter::showPerc1 (os, itsFlagTime*factor, flagDur);
      os << " of it spent in calculating flags" << endl;
    }

    // Alternative strategy is to flag in windows
    //  0 ..  n+2m
    //  n .. 2n+2m
    // 2n .. 3n+2m  etc.
    // and also update the flags in the overlaps
    bool AORFlagger::process (const DPBuffer& buf)
    {
      itsTimer.start();
      // Accumulate in the time window until the window and overlap are full. 
      itsNTimes++;
      itsBuf[itsBufIndex++] = buf;
      if (itsBufIndex == itsWindowSize+2*itsOverlap) {
        flag (2*itsOverlap);
      }
      itsTimer.stop();
      return true;
    }

    void AORFlagger::finish()
    {
      itsTimer.start();
      // Set window size to all entries left.
      itsWindowSize = itsBufIndex;
      if (itsWindowSize > 0) {
        // Flag the remaining time slots (without right overlap).
        flag (0);
      }
      itsBuf.clear();
      itsTimer.stop();
      // Let the next step finish its processing.
      getNextStep()->finish();
    }

    void AORFlagger::flag (uint rightOverlap)
    {
      // Get the sizes of the axes.
      // Note: OpenMP 2.5 needs signed iteration variables.
      int  nrbl   = itsBuf[0].getData().shape()[2];
      uint ncorr  = itsBuf[0].getData().shape()[0];
      ASSERTSTR (ncorr==4, "AOFlagger can only handle all 4 correlations");
      // Get antenna numbers in case applyautocorr is true.
      const Vector<int>& ant1 = itsInput->getAnt1();
      const Vector<int>& ant2 = itsInput->getAnt2();
      itsComputeTimer.start();
      // Now flag each baseline for this time window.
      // The baselines can be processed in parallel.
#pragma omp parallel
      {
	// Create thread-private counter object.
        FlagCounter counter;
	// Create thread-private strategy object.
	rfiStrategy::Strategy strategy;
	fillStrategy (strategy);
	// The for loop can be parallellized. This must be done dynamically,
	// because the execution times of iterations can vary.
#pragma omp for schedule(dynamic)
	// GCC-4.3 only supports OpenMP 2.5 that needs signed iteration
	// variables.
	for (int ib=0; ib<nrbl; ++ib) {
	  counter.init (itsFlagCounter);
	  // Do autocorrelations only if told so.
          if (ant1[ib] == ant2[ib]) {
            if (itsDoAutoCorr) {
              flagBaseline (0, itsWindowSize+rightOverlap, 0, ib,
                            counter, strategy);
            }
          } else {
            flagBaseline (0, itsWindowSize+rightOverlap, 0, ib,
                          counter, strategy);
          }
        } // end of OMP for
      } // end of OMP parallel
      itsComputeTimer.stop();
      itsTimer.stop();
      // Let the next step process the buffers.
      // If possible, discard the buffer processed to minimize memory usage.
      for (uint i=0; i<itsWindowSize; ++i) {
        getNextStep()->process (itsBuf[i]);
        cout << "discard " << itsBuf[i].getTime() << endl;
        itsBuf[i] = DPBuffer();
      }
      itsTimer.start();
      // Shift the buffers still needed to the beginning of the vector.
      // This is a bit easier than keeping a wrapped vector.
      // Note it is a cheap operation, because shallow copies are made.
      for (uint i=0; i<rightOverlap; ++i) {
        cout << "move "<<i+itsWindowSize<<" to "<<i<<endl;
        itsBuf[i] = itsBuf[i+itsWindowSize];
      }
      itsBufIndex = rightOverlap;
    }

    void AORFlagger::flagBaseline (uint leftOverlap, uint windowSize,
                                   uint rightOverlap, uint bl,
                                   FlagCounter& counter,
				   rfiStrategy::Strategy& strategy)
    {
      NSTimer moveTimer, flagTimer;
      moveTimer.start();
      // Get the sizes of the axes.
      uint ntime  = leftOverlap + windowSize + rightOverlap;
      uint nchan  = itsBuf[0].getData().shape()[1];
      uint blsize = nchan * itsBuf[0].getData().shape()[0];
      // Fill the rficonsole buffers and flag.
      // Create the objects for the real and imaginary data of all corr.
      Image2DPtr realXX = Image2D::CreateEmptyImagePtr(ntime, nchan);
      Image2DPtr imagXX = Image2D::CreateEmptyImagePtr(ntime, nchan);
      Image2DPtr realXY = Image2D::CreateEmptyImagePtr(ntime, nchan);
      Image2DPtr imagXY = Image2D::CreateEmptyImagePtr(ntime, nchan);
      Image2DPtr realYX = Image2D::CreateEmptyImagePtr(ntime, nchan);
      Image2DPtr imagYX = Image2D::CreateEmptyImagePtr(ntime, nchan);
      Image2DPtr realYY = Image2D::CreateEmptyImagePtr(ntime, nchan);
      Image2DPtr imagYY = Image2D::CreateEmptyImagePtr(ntime, nchan);
      for (uint i=0; i<ntime; ++i) {
        const Complex* data = itsBuf[i].getData().data() + bl*blsize;
        for (uint j=0; j<nchan; ++j) {
          realXX->SetValue (i, j, data->real());
          imagXX->SetValue (i, j, data->imag());
          data++;
          realXY->SetValue (i, j, data->real());
          imagXY->SetValue (i, j, data->imag());
          data++;
          realYX->SetValue (i, j, data->real());
          imagYX->SetValue (i, j, data->imag());
          data++;
          realYY->SetValue (i, j, data->real());
          imagYY->SetValue (i, j, data->imag());
          data++;
        }
      }
      Mask2DCPtr falseMask = Mask2D::CreateSetMaskPtr<false> (ntime, nchan);
      Image2DCPtr zeroData = Image2D::CreateZeroImagePtr (ntime, nchan);
      // Create original data.
      TimeFrequencyData origData(realXX, imagXX, realXY, imagXY,
                                 realYX, imagYX, realYY, imagYY);
      origData.SetIndividualPolarisationMasks (falseMask, falseMask,
                                               falseMask, falseMask);
      // Create contaminated data.
      TimeFrequencyData contData(origData);
      // Create revised data.
      TimeFrequencyData revData(zeroData, zeroData, zeroData, zeroData,
                                zeroData, zeroData, zeroData, zeroData);
      revData.SetIndividualPolarisationMasks (falseMask, falseMask,
                                              falseMask, falseMask);
      ////      boost::mutex mutex;
      ////      rfiStrategy::ArtifactSet artifacts(&mutex);
      // Create and fill the artifact set. A mutex is not needed.
      rfiStrategy::ArtifactSet artifacts(0);
      artifacts.SetOriginalData (origData);
      artifacts.SetContaminatedData (contData);
      artifacts.SetRevisedData (revData);
      // Execute the strategy to do the flagging.
      moveTimer.stop();
      flagTimer.start();
      strategy.Perform (artifacts, itsProgressListener);
      flagTimer.stop();
      // Put back the true flags and count newly set flags.
      moveTimer.start();
      Mask2DCPtr maskXX = artifacts.ContaminatedData().GetMask (XXPolarisation);
      Mask2DCPtr maskXY = artifacts.ContaminatedData().GetMask (XYPolarisation);
      Mask2DCPtr maskYX = artifacts.ContaminatedData().GetMask (YXPolarisation);
      Mask2DCPtr maskYY = artifacts.ContaminatedData().GetMask (YYPolarisation);
      for (uint i=leftOverlap; i<windowSize+leftOverlap; ++i) {
        bool* flags = itsBuf[i].getFlags().data() + bl*blsize;
        for (uint j=0; j<nchan; ++j) {
          // Only set if not already set.
          // Note that if first corr flag is true, all are true.
          // If any corr is newly set, set all corr.
          if (! flags[0]) {
            bool setFlag = true;
            if (maskXX->Value(i,j)) {
              counter.incrCorrelation(0);
            } else if (maskXY->Value(i,j)) {
              counter.incrCorrelation(1);
            } else if (maskYX->Value(i,j)) {
              counter.incrCorrelation(2);
            } else if (maskYY->Value(i,j)) {
              counter.incrCorrelation(3);
            } else {
              setFlag = false;
            }
            if (setFlag) {
              counter.incrBaseline(bl);
              counter.incrChannel(j);
              for (int k=0; k<4; ++k) {
                flags[k] = true;
              }
            }
          }
          flags += 4;
        }
      }
      moveTimer.stop();
#pragma omp critical(aorflagger_updatecounts)
      {
        // Add the counters to the overall object.
        itsFlagCounter.add (counter);
        // Add the timings.
        itsMoveTime += moveTimer.getElapsed();
        itsFlagTime += flagTimer.getElapsed();
      } // end of OMP critical
    }

    void AORFlagger::fillStrategy (rfiStrategy::Strategy& strategy)
    {
      strategy.Add(new SetFlaggingAction());
      ForEachPolarisationBlock* fepBlock = new ForEachPolarisationBlock();
      strategy.Add(fepBlock);
      ActionBlock* current = fepBlock;

      ForEachComplexComponentAction* focAction =
        new ForEachComplexComponentAction();
      focAction->SetOnAmplitude(true);
      focAction->SetOnImaginary(false);
      focAction->SetOnReal(false);
      focAction->SetOnPhase(false);
      focAction->SetRestoreFromAmplitude(false);
      current->Add(focAction);
      current = focAction;

      IterationBlock* iteration = new IterationBlock();
      iteration->SetIterationCount(2);
      iteration->SetSensitivityStart(4.0);
      current->Add(iteration);
      current = iteration;
		
      ThresholdAction* t2 = new ThresholdAction();
      t2->SetBaseSensitivity(1.0);
      if (itsPulsarMode) {
        t2->SetFrequencyDirectionFlagging(false);
      }
      current->Add(t2);

      CombineFlagResults* cfr2 = new CombineFlagResults();
      current->Add(cfr2);

      cfr2->Add(new FrequencySelectionAction());
      if (!itsPulsarMode) {
        cfr2->Add(new TimeSelectionAction());
      }
	
      current->Add(new SetImageAction());
      ChangeResolutionAction* changeResAction2 = new ChangeResolutionAction();
      if (itsPulsarMode) {
        changeResAction2->SetTimeDecreaseFactor(1);
      } else {
        changeResAction2->SetTimeDecreaseFactor(3);
      }
      changeResAction2->SetFrequencyDecreaseFactor(3);

      SlidingWindowFitAction* swfAction2 = new SlidingWindowFitAction();
      if (itsPulsarMode) {
        swfAction2->Parameters().timeDirectionWindowSize = 1;
      } else {
        swfAction2->Parameters().timeDirectionKernelSize = 2.5;
        swfAction2->Parameters().timeDirectionWindowSize = 10;
      }
      swfAction2->Parameters().frequencyDirectionKernelSize = 5.0;
      swfAction2->Parameters().frequencyDirectionWindowSize = 15;
      changeResAction2->Add(swfAction2);

      current->Add(changeResAction2);
      current->Add(new SetFlaggingAction());

      current = focAction;
      ThresholdAction* t3 = new ThresholdAction();
      if (itsPulsarMode) {
        t3->SetFrequencyDirectionFlagging(false);
      }
      current->Add(t3);
		
      SetFlaggingAction* setFlagsInAllPolarizations = new SetFlaggingAction();
      setFlagsInAllPolarizations->SetNewFlagging
        (SetFlaggingAction::PolarisationsEqual);

      strategy.Add(setFlagsInAllPolarizations);
      strategy.Add(new StatisticalFlagAction());

      if (itsPedantic) {
        CombineFlagResults* cfr3 = new CombineFlagResults();
        strategy.Add(cfr3);
        cfr3->Add(new FrequencySelectionAction());
        if (!itsPulsarMode) {
          cfr3->Add(new TimeSelectionAction());
        }
      } else {
        if (!itsPulsarMode) {
          strategy.Add(new TimeSelectionAction());
        }
      }
    }

  } //# end namespace
}

//# Demixer.cc: DPPP step class to subtract A-team sources
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

#include <lofar_config.h>
#include <DPPP/Demixer.h>
#include <DPPP/Averager.h>
#include <DPPP/PhaseShift.h>
#include <DPPP/DPBuffer.h>
#include <DPPP/DPInfo.h>
#include <DPPP/ParSet.h>
#include <ParmDB/Axis.h>
#include <ParmDB/SourceDB.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <Common/OpenMP.h>
#include <BBSKernel/MeasurementAIPS.h>

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <scimath/Mathematics/MatrixMathLA.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MCPosition.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/Quantum.h>

#include <iostream>
#include <iomanip>

#include <BBSKernel/ParmManager.h>
#include <Common/StreamUtil.h>
#include <DPPP/SourceDBUtil.h>
#include <DPPP/CursorUtilCasa.h>
#include <DPPP/EstimateMixed.h>
#include <DPPP/SubtractMixed.h>
#include <DPPP/Apply.h>
#include <DPPP/Simulate.h>
#include <boost/multi_array.hpp>

using namespace casa;
using namespace LOFAR::BBS;

namespace LOFAR {
  namespace DPPP {

    using LOFAR::operator<<;

    Demixer::Demixer (DPInput* input,
                      const ParSet& parset, const string& prefix)
      : itsInput         (input),
        itsName          (prefix),
        itsSkyName       (parset.getString(prefix+"skymodel", "sky")),
        itsInstrumentName(parset.getString(prefix+"instrumentmodel",
                                           "instrument")),
        itsElevCutoff    (getAngle(parset.getString(prefix+"elevationcutoff",
                                                    "0."))),
        itsTargetSource  (parset.getString(prefix+"targetsource", string())),
        itsSubtrSources  (parset.getStringVector (prefix+"subtractsources")),
        itsModelSources  (parset.getStringVector (prefix+"modelsources",
                                                  vector<string>())),
        itsExtraSources  (parset.getStringVector (prefix+"othersources",
                                                  vector<string>())),
///        itsJointSolve    (parset.getBool  (prefix+"jointsolve", true)),
        itsNTimeIn       (0),
        itsNChanAvgSubtr (parset.getUint  (prefix+"freqstep", 1)),
        itsNTimeAvgSubtr (parset.getUint  (prefix+"timestep", 1)),
        itsNTimeOutSubtr (0),
        itsNChanAvg      (parset.getUint  (prefix+"demixfreqstep",
                                           itsNChanAvgSubtr)),
        itsNTimeAvg      (parset.getUint  (prefix+"demixtimestep",
                                           itsNTimeAvgSubtr)),
        itsNTimeChunk    (parset.getUint  (prefix+"ntimechunk", 0)),
        itsNTimeOut      (0),
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
        itsTimeCount     (0),
        itsNStation      (input->antennaNames().size())
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
    {
      // Get and set solver options.
      itsSolveOpt.maxIter =
        parset.getUint  (prefix+"Solve.Options.MaxIter", 300);
      itsSolveOpt.epsValue =
        parset.getDouble(prefix+"Solve.Options.EpsValue", 1e-9);
      itsSolveOpt.epsDerivative =
        parset.getDouble(prefix+"Solve.Options.EpsDerivative", 1e-9);
      itsSolveOpt.colFactor =
        parset.getDouble(prefix+"Solve.Options.ColFactor", 1e-9);
      itsSolveOpt.lmFactor  =
        parset.getDouble(prefix+"Solve.Options.LMFactor", 1.0);
      itsSolveOpt.balancedEq =
        parset.getBool  (prefix+"Solve.Options.BalancedEqs", false);
      itsSolveOpt.useSVD  =
        parset.getBool  (prefix+"Solve.Options.UseSVD", true);
//      itsBBSExpr.setOptions (itsSolveOpt);
      /// Maybe optionally a parset parameter directions to give the
      /// directions of unknown sources.
      /// Or make sources a vector of vectors like [name, ra, dec] where
      /// ra and dec are optional.

      // Default nr of time chunks is maximum number of threads.
      if (itsNTimeChunk == 0) {

        itsNTimeChunk = OpenMP::maxThreads();
        LOG_DEBUG_STR("#threads: " << itsNTimeChunk);
      }
      // Check that time windows fit nicely.
      ASSERTSTR ((itsNTimeChunk * itsNTimeAvg) % itsNTimeAvgSubtr == 0,
                 "time window should fit final averaging integrally");
      itsNTimeChunkSubtr = (itsNTimeChunk * itsNTimeAvg) / itsNTimeAvgSubtr;
      // Collect all source names.
      itsNrModel = itsSubtrSources.size() + itsModelSources.size();
      itsNrDir   = itsNrModel + itsExtraSources.size() + 1;
      itsAllSources.reserve (itsNrDir);
      itsAllSources.insert (itsAllSources.end(),
                            itsSubtrSources.begin(), itsSubtrSources.end());
      itsAllSources.insert (itsAllSources.end(),
                            itsModelSources.begin(), itsModelSources.end());
      itsAllSources.insert (itsAllSources.end(),
                            itsExtraSources.begin(), itsExtraSources.end());
      itsAllSources.push_back (itsTargetSource);

// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
      // Get the patch names and positions from the SourceDB table.
      SourceDB sdb(ParmDBMeta("casa", itsSkyName));

      for(uint i = 0; i < itsNrModel; ++i) {
        itsPatchList.push_back(makePatch(sdb, itsAllSources[i]));
      }
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST

      // If the target source is given, add it to the model.
      // Because the target source has to be the last direction, it means
      // that (for the time being) no extra sources can be given.
      if (! itsTargetSource.empty()) {
        itsNrModel++;
        ASSERTSTR (itsExtraSources.empty(), "Currently no extrasources can "
                   "be given if the targetsource is given");
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
        itsPatchList.push_back(makePatch(sdb, itsTargetSource));
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
      }
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
      ASSERT(itsPatchList.size() == itsNrModel);
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST

      // Size buffers.
      itsFactors.resize      (itsNTimeChunk);
      itsFactorsSubtr.resize (itsNTimeChunkSubtr);
      itsPhaseShifts.reserve (itsNrDir-1);
      itsFirstSteps.reserve  (itsNrDir+1);   // one extra for itsAvgSubtr
      itsAvgResults.reserve  (itsNrDir);

//      // Get the patch names and positions from the SourceDB table.
//      SourceDB sdb(ParmDBMeta("casa", itsSkyName));

      // Create the steps for the sources to be removed.
      // Demixing consists of the following steps:
      // - phaseshift data to each demix source
      // - average data in each direction, also for original phasecenter.
      // - determine demix factors for all directions
      // - use BBS to predict and solve in each direction. It is possible to
      //   predict more directions than to solve (for strong sources in field).
      // - use BBS to subtract the solved sources using the demix factors.
      //   The averaging used here can be smaller than used when solving.
      for (uint i=0; i<itsNrDir-1; ++i) {
        // First make the phaseshift and average steps for each demix source.
        // The resultstep gets the result.
        // The phasecenter can be given in a parameter. Its name is the default.
        // Look up the source direction in the patch table.
        // If found, turn it into a vector of strings.
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
//        vector<PatchInfo> patchInfo (sdb.getPatchInfo (-1, itsAllSources[i]));
        vector<string> sourceVec (1, itsAllSources[i]);
//        if (! patchInfo.empty()) {
//          sourceVec[0] = toString(patchInfo[0].getRa());
//          sourceVec.push_back (toString(patchInfo[0].getDec()));
//        }
        if(i < itsNrModel) {
          sourceVec[0] = toString(itsPatchList[i].position[0]);
          sourceVec.push_back(toString(itsPatchList[i].position[1]));
        }
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
        PhaseShift* step1 = new PhaseShift (input, parset,
                                            prefix + itsAllSources[i] + '.',
                                            sourceVec);
        itsFirstSteps.push_back (DPStep::ShPtr(step1));
        itsPhaseShifts.push_back (step1);
        DPStep::ShPtr step2 (new Averager(input, prefix,
					  itsNChanAvg, itsNTimeAvg));
        step1->setNextStep (step2);
        MultiResultStep* step3 = new MultiResultStep(itsNTimeChunk);
        step2->setNextStep (DPStep::ShPtr(step3));
        // There is a single demix factor step which needs to get all results.
        itsAvgResults.push_back (step3);
      }

      // Now create the step to average the data themselves.
      DPStep::ShPtr targetAvg(new Averager(input, prefix,
                                           itsNChanAvg, itsNTimeAvg));
      itsFirstSteps.push_back (targetAvg);
      MultiResultStep* targetAvgRes = new MultiResultStep(itsNTimeChunk);
      targetAvg->setNextStep (DPStep::ShPtr(targetAvgRes));
      itsAvgResults.push_back (targetAvgRes);

      // Create the data average step for the subtract.
      DPStep::ShPtr targetAvgSubtr(new Averager(input, prefix,
                                                itsNChanAvgSubtr,
                                                itsNTimeAvgSubtr));
      itsAvgResultSubtr = new MultiResultStep(itsNTimeChunkSubtr);
      targetAvgSubtr->setNextStep (DPStep::ShPtr(itsAvgResultSubtr));
      itsFirstSteps.push_back (targetAvgSubtr);

// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
//      __init_source_list("demixer.mdl");

//      ASSERT(input->getAnt1().size() == input->getAnt2().size());
//      for(size_t i=0; i<input->getAnt1().size(); ++i) {
//        int ant1 = input->getAnt1()[i];
//        int ant2 = input->getAnt2()[i];
//        itsBaselinesTMP.append(BBS::baseline_t(ant1, ant2));
//      }
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
      for(size_t i = 0; i < input->getAnt1().size(); ++i) {
        itsBaselines.push_back(Baseline(input->getAnt1()[i],
          input->getAnt2()[i]));
      }
    }

    void Demixer::initUnknowns()
    {
      itsUnknowns.resize(IPosition(4, 8, itsNStation, itsNrModel,
        itsNTimeDemix));
      for(uint ts = 0; ts < itsNTimeDemix; ++ts) {
        for(uint dr = 0; dr < itsNrModel; ++dr) {
          for(uint st = 0; st < itsNStation; ++st) {
            itsUnknowns(IPosition(4, 0, st, dr, ts)) = 1.0;
            itsUnknowns(IPosition(4, 1, st, dr, ts)) = 0.0;
            itsUnknowns(IPosition(4, 2, st, dr, ts)) = 0.0;
            itsUnknowns(IPosition(4, 3, st, dr, ts)) = 0.0;
            itsUnknowns(IPosition(4, 4, st, dr, ts)) = 0.0;
            itsUnknowns(IPosition(4, 5, st, dr, ts)) = 0.0;
            itsUnknowns(IPosition(4, 6, st, dr, ts)) = 1.0;
            itsUnknowns(IPosition(4, 7, st, dr, ts)) = 0.0;
          }
        }
      }
    }

    Demixer::~Demixer()
    {
//// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
//#ifdef ESTIMATE_TIMER
//        LOG_DEBUG_STR("tTot: " << itsState.tTot);
//        LOG_DEBUG_STR("tSim: " << itsState.tSim);
//        LOG_DEBUG_STR("tEq: " << itsState.tEq);
//        LOG_DEBUG_STR("tLM: " << itsState.tLM);
//        LOG_DEBUG_STR("tSub: " << itsState.tSub);
//#endif
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
    }

    void Demixer::updateInfo (DPInfo& info)
    {
      // Get size info.
      itsNChanIn  = info.nchan();
      itsNrBl     = info.nbaselines();
      itsNrCorr   = info.ncorr();
      itsFactorBuf.resize (IPosition(4, itsNrCorr, itsNChanIn, itsNrBl,
                                     itsNrDir*(itsNrDir-1)/2));
      itsFactorBufSubtr.resize (IPosition(4, itsNrCorr, itsNChanIn, itsNrBl,
                                     itsNrDir*(itsNrDir-1)/2));
      itsTimeInterval = info.timeInterval();

      // Adapt averaging to available nr of channels and times.
      // Use a copy of the DPInfo, otherwise it is updated multiple times.
      DPInfo infocp(info);
      itsNTimeAvg = std::min (itsNTimeAvg, infocp.ntime());
      itsNChanAvg = infocp.update (itsNChanAvg, itsNTimeAvg);
      itsNTimeDemix = infocp.ntime();

//// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
//      Position patchPos;
//// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST

      // Let the internal steps update their data.
      for (uint i=0; i<itsFirstSteps.size(); ++i) {
        infocp = info;
        DPStep::ShPtr step = itsFirstSteps[i];
        while (step) {
          step->updateInfo (infocp);
          step = step->getNextStep();
        }
        if (i == 0) {
          // Keep the averaged time interval.
          itsNChanOut = infocp.nchan();
          itsTimeIntervalAvg = infocp.timeInterval();
        }
        // Create the BBS model expression for sources with a model.
        if (i < itsNrModel) {
//          itsBBSExpr.addModel (itsAllSources[i], infocp.phaseCenter());

// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
//          MDirection dirJ2000(MDirection::Convert(infocp.phaseCenter(),
//              MDirection::J2000)());
//          casa::Quantum<casa::Vector<casa::Double> > angles = dirJ2000.getAngle();
//          __init_lmn(i, angles.getBaseValue()(0), angles.getBaseValue()(1));

//          if(i == 0) {
//            patchPos[0] = angles.getBaseValue()(0);
//            patchPos[1] = angles.getBaseValue()(1);
//          }
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
        }
      }

      // Update the info of this object.
      info.setNeedVisData();
      info.setNeedWrite();
      itsNTimeAvgSubtr = std::min (itsNTimeAvgSubtr, info.ntime());
      itsNChanAvgSubtr = info.update (itsNChanAvgSubtr, itsNTimeAvgSubtr);
      itsNChanOutSubtr = info.nchan();
      itsTimeIntervalSubtr = info.timeInterval();
      ASSERTSTR (itsNChanAvg % itsNChanAvgSubtr == 0,
		 "Demix averaging " << itsNChanAvg
		 << " must be multiple of output averaging "
		 << itsNChanAvgSubtr);
      ASSERTSTR (itsNTimeAvg % itsNTimeAvgSubtr == 0,
		 "Demix averaging " << itsNTimeAvg
		 << " must be multiple of output averaging "
		 << itsNTimeAvgSubtr);
      // Store channel frequencies for the demix and subtract resolutions.
      itsFreqDemix = itsInput->chanFreqs (itsNChanAvg);
      itsFreqSubtr = itsInput->chanFreqs (itsNChanAvgSubtr);

// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
      LOG_DEBUG_STR("#directions: " << itsNrModel);
      LOG_DEBUG_STR("#stations: " << itsNStation);
      LOG_DEBUG_STR("#times: " << itsNTimeDemix);
      MDirection dirJ2000(MDirection::Convert(info.phaseCenter(), MDirection::J2000)());
      casa::Quantum<casa::Vector<casa::Double> > angles = dirJ2000.getAngle();
      itsPhaseRef = Position(angles.getBaseValue()(0),
        angles.getBaseValue()(1));
//      itsState.init(itsNrModel, itsNStation, nTimeDemix, itsBaselinesTMP,
//        itsFreqAxisDemix, itsFreqAxisSubtr,
//        angles.getBaseValue()(0), angles.getBaseValue()(1),
//        itsSolveOpt);

//      SourceDB sdb(ParmDBMeta("casa", itsSkyName));
//      itsState.patchPos = patchPos;
//      itsState.patch = makePatch(sdb, "CasA");
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST


      initUnknowns();
    }

    void Demixer::show (std::ostream& os) const
    {
      os << "Demixer " << itsName << std::endl;
      os << "  skymodel:       " << itsSkyName << std::endl;
      os << "  instrumentmodel:" << itsInstrumentName << std::endl;
      os << "  targetsource:   " << itsTargetSource << std::endl;
      os << "  subtractsources:" << itsSubtrSources << std::endl;
      os << "  modelsources:   " << itsModelSources << std::endl;
      os << "  extrasources:   " << itsExtraSources << std::endl;
///      os << "  jointsolve:     " << itsJointSolve << std::endl;
      os << "  freqstep:       " << itsNChanAvgSubtr << std::endl;
      os << "  timestep:       " << itsNTimeAvgSubtr << std::endl;
      os << "  demixfreqstep:  " << itsNChanAvg << std::endl;
      os << "  demixtimestep:  " << itsNTimeAvg << std::endl;
      os << "  ntimechunk:     " << itsNTimeChunk << std::endl;
      os << "  Solve.Options.MaxIter:       " << itsSolveOpt.maxIter << endl;
      os << "  Solve.Options.EpsValue:      " << itsSolveOpt.epsValue << endl;
      os << "  Solve.Options.EpsDerivative: " << itsSolveOpt.epsDerivative << endl;
      os << "  Solve.Options.ColFactor:     " << itsSolveOpt.colFactor << endl;
      os << "  Solve.Options.LMFactor:      " << itsSolveOpt.lmFactor << endl;
      os << "  Solve.Options.BalancedEqs:   " << itsSolveOpt.balancedEq << endl;
      os << "  Solve.Options.UseSVD:        " << itsSolveOpt.useSVD <<endl;
    }

    void Demixer::showTimings (std::ostream& os, double duration) const
    {
      const double self = itsTimer.getElapsed();

      os << "  ";
      FlagCounter::showPerc1 (os, self, duration);
      os << " Demixer " << itsName << endl;

      os << "          ";
      FlagCounter::showPerc1 (os, itsTimerPhaseShift.getElapsed(), self);
      os << " of it spent in phase shifting/averaging data" << endl;
      os << "          ";
      FlagCounter::showPerc1 (os, itsTimerDemix.getElapsed(), self);
      os << " of it spent in calculating decorrelation factors" << endl;
      os << "          ";
      FlagCounter::showPerc1 (os, itsTimerSolve.getElapsed(), self);
      os << " of it spent in solving source gains" << endl;
      os << "          ";
      FlagCounter::showPerc1 (os, itsTimerSubtract.getElapsed(), self);
      os << " of it spent in subtracting modeled sources" << endl;
    }

    bool Demixer::process (const DPBuffer& buf)
    {
      itsTimer.start();
      // Set start time of the chunk.
      if (itsNTimeIn == 0) {
        itsStartTimeChunk = buf.getTime() - itsInput->timeInterval() * 0.5;
      }
      // Update the count.
      itsNTimeIn++;
      // Make sure all required data arrays are filled in.
      DPBuffer newBuf(buf);
      RefRows refRows(newBuf.getRowNrs());
      if (newBuf.getUVW().empty()) {
        newBuf.setUVW(itsInput->fetchUVW(newBuf, refRows, itsTimer));
      }
      if (newBuf.getWeights().empty()) {
        newBuf.setWeights(itsInput->fetchWeights(newBuf, refRows, itsTimer));
      }
      if (newBuf.getFullResFlags().empty()) {
        newBuf.setFullResFlags(itsInput->fetchFullResFlags(newBuf, refRows,
                                                           itsTimer));
      }

      // Do the initial steps (phaseshift and average).
      itsTimerPhaseShift.start();
///#pragma omp parallel for
      for (int i=0; i<int(itsFirstSteps.size()); ++i) {
        itsFirstSteps[i]->process(newBuf);
      }
      itsTimerPhaseShift.stop();

      // For each itsNTimeAvg times, calculate the
      // phase rotation per direction.
      itsTimerDemix.start();
      addFactors (newBuf, itsFactorBuf);
      if (itsNTimeIn % itsNTimeAvg == 0) {
        makeFactors (itsFactorBuf, itsFactors[itsNTimeOut],
                     itsAvgResults[0]->get()[itsNTimeOut].getWeights(),
                     itsNChanOut,
                     itsNChanAvg);
        // Deproject sources without a model.
        deproject (itsFactors[itsNTimeOut], itsAvgResults, itsNTimeOut);
        itsFactorBuf = Complex();   // clear summation buffer
        itsNTimeOut++;
      }
      // Subtract is done with different averaging parameters, so
      // calculate the factors for it.
      addFactors (newBuf, itsFactorBufSubtr);
      if (itsNTimeIn % itsNTimeAvgSubtr == 0) {
        makeFactors (itsFactorBufSubtr, itsFactorsSubtr[itsNTimeOutSubtr],
                     itsAvgResultSubtr->get()[itsNTimeOutSubtr].getWeights(),
                     itsNChanOutSubtr,
                     itsNChanAvgSubtr);
        itsFactorBufSubtr = Complex();   // clear summation buffer
        itsNTimeOutSubtr++;
      }
      itsTimerDemix.stop();

      // Do BBS solve, etc. when sufficient time slots have been collected.
      if (itsNTimeOut == itsNTimeChunk) {
        demix();
        itsNTimeIn       = 0;
        itsNTimeOut      = 0;
        itsNTimeOutSubtr = 0;
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
        itsTimeCount += itsNTimeChunk;
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
      }

      itsTimer.stop();
      return true;
    }

    void Demixer::finish()
    {
      // Process remaining entries.
      if (itsNTimeIn > 0) {
        itsTimer.start();

        // Finish the initial steps (phaseshift and average).
        itsTimerPhaseShift.start();
///#pragma omp parallel for
        for (int i=0; i<int(itsFirstSteps.size()); ++i) {
          itsFirstSteps[i]->finish();
        }
        itsTimerPhaseShift.stop();
        // Only average if there is some data.
        itsTimerDemix.start();
        if (itsNTimeIn % itsNTimeAvg != 0) {
          makeFactors (itsFactorBuf, itsFactors[itsNTimeOut],
                       itsAvgResults[0]->get()[itsNTimeOut].getWeights(),
                       itsNChanOut,
                       itsNChanAvg);
          // Deproject sources without a model.
          deproject (itsFactors[itsNTimeOut], itsAvgResults, itsNTimeOut);
          itsNTimeOut++;
        }
        if (itsNTimeIn % itsNTimeAvgSubtr != 0) {
          makeFactors (itsFactorBufSubtr, itsFactorsSubtr[itsNTimeOutSubtr],
                       itsAvgResultSubtr->get()[itsNTimeOutSubtr].getWeights(),
                       itsNChanOutSubtr,
                       itsNChanAvgSubtr);
          itsNTimeOutSubtr++;
        }
        itsTimerDemix.stop();
        // Resize lists of mixing factors to the number of valid entries.
        itsFactors.resize(itsNTimeOut);
        itsFactorsSubtr.resize(itsNTimeOutSubtr);
        // Demix the source directions.
        demix();
        itsTimer.stop();
      }

// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
      dumpSolutions();
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST

      // Let the next steps finish.
      getNextStep()->finish();
    }

    void Demixer::addFactors (const DPBuffer& newBuf,
                              Array<DComplex>& factorBuf)
    {
      // Nothing to do if only target direction.
      if (itsNrDir <= 1) return;
      int ncorr  = newBuf.getData().shape()[0];
      int nchan  = newBuf.getData().shape()[1];
      int nbl    = newBuf.getData().shape()[2];
      //# If ever in the future a time dependent phase center is used,
      //# the machine must be reset for each new time, thus each new call
      //# to process.
      DComplex* test = factorBuf.data();

      for (uint i1=0; i1<itsNrDir-1; ++i1) {
        for (uint i0=i1+1; i0<itsNrDir; ++i0) {
          if (i0 == itsNrDir-1) {
///#pragma omp parallel for
            for (int i=0; i<nbl; ++i) {
//              const double* uvw       = newBuf.getUVW().data() + i*3;
              const bool*   flagPtr   = newBuf.getFlags().data() + i*ncorr*nchan;
              const float*  weightPtr = newBuf.getWeights().data() + i*ncorr*nchan;
              const DComplex* phasor1 = itsPhaseShifts[i1]->getPhasors().data() + i*nchan;
              DComplex* factorPtr     = factorBuf.data() + i*ncorr*nchan + ((i1 * (2 * itsNrDir - i1 - 1)) / 2 + (i0 - i1 - 1)) * ncorr * nchan * nbl;
              for (int j=0; j<nchan; ++j) {
                DComplex factor = conj(*phasor1++);
                for (int k=0; k<ncorr; ++k) {
                  ASSERTSTR(test == factorPtr, "test: " << reinterpret_cast<size_t>(test) << " factorPtr: " << reinterpret_cast<size_t>(factorPtr));
                  if (! *flagPtr) {
                    *factorPtr += factor * double(*weightPtr);
                  }
                  flagPtr++;
                  weightPtr++;
                  factorPtr++;
                  test++;
                }
              }
//              uvw += 3;
            }
    	  } else {
///#pragma omp parallel for
            for (int i=0; i<nbl; ++i) {
//              const double* uvw       = newBuf.getUVW().data() + i*3;
              const bool*   flagPtr   = newBuf.getFlags().data() + i*ncorr*nchan;
              const float*  weightPtr = newBuf.getWeights().data() + i*ncorr*nchan;
              const DComplex* phasor0 = itsPhaseShifts[i0]->getPhasors().data() + i*nchan;
              const DComplex* phasor1 = itsPhaseShifts[i1]->getPhasors().data() + i*nchan;
              DComplex* factorPtr     = factorBuf.data() + ((i1 * (2 * itsNrDir - i1 - 1)) / 2 + (i0 - i1 - 1)) * ncorr * nchan * nbl + i*ncorr*nchan;
              for (int j=0; j<nchan; ++j) {
                // Probably multiply with conj
                DComplex factor = *phasor0++ / *phasor1++;
                for (int k=0; k<ncorr; ++k) {
                  ASSERTSTR(test == factorPtr, "test: " << reinterpret_cast<size_t>(test) << " factorPtr: " << reinterpret_cast<size_t>(factorPtr));
                  if (! *flagPtr) {
                    *factorPtr += factor * double(*weightPtr);
                  }
                  flagPtr++;
                  weightPtr++;
                  factorPtr++;
                  test++;
                }
              }
//              uvw += 3;
            }
          }
        }
      }
    }

    void Demixer::makeFactors (const Array<DComplex>& bufIn,
                               Array<DComplex>& bufOut,
                               const Cube<float>& weightSums,
                               uint nChanOut,
                               uint nChanAvg)
    {
      // Nothing to do if only target direction.
      if (itsNrDir <= 1) return;
      ASSERT (! weightSums.empty());
      bufOut.resize (IPosition(5, itsNrDir, itsNrDir,
                               itsNrCorr, nChanOut, itsNrBl));
      bufOut = DComplex(1,0);
      const DComplex* phin = bufIn.data();
      for (uint d0=0; d0<itsNrDir; ++d0) {
        for (uint d1=d0+1; d1<itsNrDir; ++d1) {
          DComplex* ph1 = bufOut.data() + d0*itsNrDir + d1;
          DComplex* ph2 = bufOut.data() + d1*itsNrDir + d0;
          // Average for all channels and divide by the summed weights.
          const float* weightPtr = weightSums.data();
          for (uint k=0; k<itsNrBl; ++k) {
            for (uint c0=0; c0<nChanOut; ++c0) {
              DComplex sum[4];
              uint nch = std::min(nChanAvg, itsNChanIn-c0*nChanAvg);
              for (uint c1=0; c1<nch; ++c1) {
                for (uint j=0; j<itsNrCorr; ++j) {
                  sum[j] += *phin++;
                }
              }
              for (uint j=0; j<itsNrCorr; ++j) {
                *ph1 = sum[j] / double(*weightPtr++);
                *ph2 = conj(*ph1);
                ph1 += itsNrDir*itsNrDir;
                ph2 += itsNrDir*itsNrDir;
              }
            }
            ASSERTSTR(phin == bufIn.data() + ((d0 * (2 * itsNrDir - d0 - 1)) / 2 + (d1 - d0 - 1)) * itsNrBl * itsNrCorr * itsNChanIn + (k + 1) * itsNrCorr * itsNChanIn, "phin: " << reinterpret_cast<size_t>(phin) << " computed: " << reinterpret_cast<size_t>(bufIn.data() + ((d0 * (2 * itsNrDir - d0 - 1)) / 2 + (d1 - d0 - 1)) * itsNrBl * itsNrCorr * itsNChanIn + (k + 1) * itsNrCorr * itsNChanIn));
          }
        }
      }
    }

    void Demixer::deproject (Array<DComplex>& factors,
                             vector<MultiResultStep*> avgResults,
                             uint resultIndex)
    {
      // Nothing to do if only target direction or if all sources are modeled.
      if (itsNrDir <= 1  ||  itsNrDir == itsNrModel) return;
      // Get pointers to the data for the various directions.
      vector<Complex*> resultPtr(itsNrDir);
      for (uint j=0; j<itsNrDir; ++j) {
        resultPtr[j] = avgResults[j]->get()[resultIndex].getData().data();
      }
      // Sources without a model have to be deprojected.
      uint nrDeproject = itsNrDir - itsNrModel;
      // The projection matrix is given by
      //     P = I - A * inv(A.T.conj * A) * A.T.conj
      // where A is the last column of the demixing matrix M.
      // The BBS equations get:
      //     P * M' * v_predict = P * v_averaged
      // where M' is obtained by removing the last column of demixing matrix M.
      // The dimensions of the matrices/vectors are:
      //     P : NxN
      //     M' : Nx(N-1)
      //     v_predict : (N-1) x 1
      //     v_averaged: N x 1
      // where N is the number of modeled sources to use in demixing.
      // In the general case S sources might not have a source model.
      // In that case A is the NxS matrix containing all these columns
      // from M and M' is the Nx(N-S) matrix without all these columns.

      // Calculate P for all baselines,channels,correlations.
      IPosition shape = factors.shape();
      int nvis = shape[2] * shape[3] * shape[4];
      shape[1] = itsNrModel;
      Array<DComplex> newFactors (shape);
      IPosition inShape (2, itsNrDir, itsNrDir);
      IPosition outShape(2, itsNrDir, itsNrModel);
///#pragma omp parallel
      {
	casa::Matrix<DComplex> a(itsNrDir, nrDeproject);
	casa::Matrix<DComplex> ma(itsNrDir, itsNrModel);
	vector<DComplex> vec(itsNrDir);
///#pragma omp for
	for (int i=0; i<nvis; ++i) {
	  // Split the matrix into the modeled and deprojected sources.
	  // Copy the columns to the individual matrices.
	  const DComplex* inptr  = factors.data() + i*itsNrDir*itsNrDir;
	  DComplex* outptr = newFactors.data() + i*itsNrDir*itsNrModel;
	  casa::Matrix<DComplex> out (outShape, outptr, SHARE);
	  // Copying a bit of data is probably faster than taking a matrix subset.
	  objcopy (ma.data(), inptr, itsNrDir*itsNrModel);
	  objcopy (a.data(), inptr + itsNrDir*itsNrModel, itsNrDir*nrDeproject);
	  // Calculate conjugated transpose of A, multiply with A, and invert.
	  casa::Matrix<DComplex> at(adjoint(a));
	  casa::Matrix<DComplex> ata(invert(product(at, a)));
	  if (ata.empty()) {
	    ata.resize (nrDeproject, nrDeproject);
	  }
	  DBGASSERT(ata.ncolumn()==nrDeproject && ata.nrow()==nrDeproject);
	  // Calculate P = I - A * ata * A.T.conj
	  casa::Matrix<DComplex> aata(product(a,ata));
	  casa::Matrix<DComplex> p (-product(product(a, ata), at));
	  casa::Vector<DComplex> diag(p.diagonal());
	  diag += DComplex(1,0);
	  // Multiply the demixing factors with P (get stored in newFactors).
	  out = product(p, ma);
	  ///        cout << "p matrix: " << p;
	  // Multiply the averaged data point with P.
	  std::fill (vec.begin(), vec.end(), DComplex());
	  for (uint j=0; j<itsNrDir; ++j) {
	    for (uint k=0; k<itsNrDir; ++k) {
	      vec[k] += DComplex(resultPtr[j][i]) * p(k,j);
	    }
	  }
	  // Put result back in averaged data for those sources.
	  for (uint j=0; j<itsNrDir; ++j) {
	    resultPtr[j][i] = vec[j];
	  }
	  ///        cout << vec << endl;
	}
      }
      // Set the new demixing factors.
      factors.reference (newFactors);
    }


    void Demixer::demix()
    {
      // Only solve and subtract if multiple directions.
      if (itsNrDir > 1) {
        // Collect buffers for each direction.
        vector<vector<DPBuffer> > streams;
        for (size_t i=0; i<itsAvgResults.size(); ++i) {
          // Only the phased shifted and averaged data for directions which have
          // an associated model should be used to estimate the directional
          // response.
          if(i < itsNrModel) {
            streams.push_back (itsAvgResults[i]->get());
          }
          // Clear the buffers.
          itsAvgResults[i]->clear();
        }

        itsTimerSolve.start();

//        // Construct grids for parameter estimation.
//        Axis::ShPtr timeAxis (new RegularAxis (itsStartTimeChunk,
//                                               itsTimeIntervalAvg,
//                                               itsNTimeOut));
//        Grid visGrid(itsFreqAxisDemix, timeAxis);
//        // Solve for each time slot over all channels.
//        Grid solGrid(itsFreqAxisDemix->compress(itsFreqAxisDemix->size()),
//                     timeAxis);

        // Estimate model parameters.
//        LOG_DEBUG_STR("estimating....");
//        itsBBSExpr.estimate(buffers, visGrid, solGrid, itsFactors);
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
//        estimate(buffers, itsFactors, itsState, itsTimeCount);
//        estimate(itsAvgResultSubtr->get(), buffers, itsFactorsSubtr, itsState);
//        estimate(itsAvgResultSubtr->get(), buffers, itsFactors, itsFactorsSubtr,
//            itsState, itsTimeCount);
//        ASSERTSTR(itsNTimeAvgSubtr <= itsNTimeAvg && itsNTimeAvg % itsNTimeAvgSubtr == 0, "Subtr: " << itsNTimeAvgSubtr << " Demix: " << itsNTimeAvg);
//        demix2(itsAvgResultSubtr->get(), buffers, itsFactors, itsFactorsSubtr,
//          itsState, itsTimeCount, buffers[0].size(), itsNTimeAvg
//          / itsNTimeAvgSubtr, itsPatchList);
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST

        const size_t nTime = streams[0].size();

        LOG_DEBUG_STR("#models: " << itsNrModel);
        LOG_DEBUG_STR("#directions: " << itsNrDir);
        LOG_DEBUG_STR("target #time: " << itsAvgResultSubtr->get().size());
        LOG_DEBUG_STR("streams #dir: " << streams.size() << " #time: "
            << streams[0].size());
        LOG_DEBUG_STR("coeff #time: " << itsFactors.size() << " shape: "
            << itsFactors[0].shape());
        LOG_DEBUG_STR("coeffRes #time: " << itsFactorsSubtr.size() << " shape: "
            << itsFactorsSubtr[0].shape());

        const size_t nThread = OpenMP::maxThreads();
        const size_t nDr = itsNrModel;
        const size_t nSt = itsNStation;
        const size_t nBl = itsBaselines.size();
        const size_t nCh = itsFreqDemix.size();
        const size_t nCr = 4;
        const size_t timeFactor = itsNTimeAvg / itsNTimeAvgSubtr;
        vector<DPBuffer> &target = itsAvgResultSubtr->get();

        // Copy solutions from global solution array to thread private solution
        // array (solution propagation between chunks).
        boost::multi_array<double, 2> unknowns(boost::extents[nThread][nDr * nSt * 8]);
        const size_t nSlot = min(nTime, nThread);
        const size_t tSource = (itsTimeCount == 0 ? 0 : itsTimeCount - 1);
        for(size_t i = 0; i < nSlot; ++i) {
          copy(&(itsUnknowns(IPosition(4, 0, 0, 0, tSource))),
            &(itsUnknowns(IPosition(4, 0, 0, 0, tSource))) + nDr * nSt * 8,
            &(unknowns[i][0]));
        }

        boost::multi_array<double, 3> uvw(boost::extents[nThread][nSt][3]);
        boost::multi_array<dcomplex, 5> buffer(boost::extents[nThread][nDr][nBl][nCh][nCr]);

        const size_t nChRes = itsFreqSubtr.size();
        boost::multi_array<dcomplex, 4> residual(boost::extents[nThread][nBl][nChRes][nCr]);

#pragma omp parallel for
        for(size_t i = 0; i < nTime; ++i)
        {
            const size_t thread = OpenMP::threadNum();

            // compute elevation and determine which sources are visible.
                // NB. carefully consider makeNorm() indices!!

            // zero solutions for non-visible sources.

            // Simulate.
            size_t strides[3] = {1, nCr, nCh * nCr};
            size_t strides_split[2] = {1, 3};

            const_cursor<Baseline> cr_baseline(&(itsBaselines[0]));
            const_cursor<double> cr_freq(&(itsFreqDemix[0]));
            const_cursor<double> cr_freqRes(&(itsFreqSubtr[0]));

            cursor<double> cr_split(&(uvw[thread][0][0]), 2, strides_split);
            for(size_t dr = 0; dr < nDr; ++dr)
            {
                fill(&(buffer[thread][dr][0][0][0]),
                    &(buffer[thread][dr][0][0][0]) + nBl * nCh * nCr, dcomplex());

                const_cursor<double> cr_uvw = casa_const_cursor(streams[dr][i].getUVW());
                splitUVW(nSt, nBl, cr_baseline, cr_uvw, cr_split);

                cursor<dcomplex> cr_model(&(buffer[thread][dr][0][0][0]), 3, strides);
                simulate(itsPatchList[dr].position, itsPatchList[dr], nSt, nBl, nCh,
                    cr_baseline, cr_freq, cr_split, cr_model);
            }

            // Estimate.
            vector<const_cursor<fcomplex> > cr_data(nDr);
            vector<const_cursor<dcomplex> > cr_model(nDr);
            for(size_t dr = 0; dr < nDr; ++dr)
            {
                cr_data[dr] = casa_const_cursor(streams[dr][i].getData());
                cr_model[dr] = const_cursor<dcomplex>(&(buffer[thread][dr][0][0][0]),
                    3, strides);
            }

            const_cursor<bool> cr_flag = casa_const_cursor(streams[0][i].getFlags());
            const_cursor<float> cr_weight =
                casa_const_cursor(streams[0][i].getWeights());
            const_cursor<dcomplex> cr_mix = casa_const_cursor(itsFactors[i]);

            size_t strides_unknowns[3] = {1, 8, nSt * 8};
            cursor<double> cr_unknowns(&(unknowns[thread][0]), 3, strides_unknowns);
            estimate(nDr, nSt, nBl, nCh, cr_data, cr_model, cr_baseline,
                cr_flag, cr_weight, cr_mix, cr_unknowns);

            // Tweak solutions.
            const size_t nTimeResidual = min(timeFactor, target.size() - i * timeFactor);
            LOG_DEBUG_STR("timeFactor: " << timeFactor << "  nTime: " << nTime << " nTimeResidual: " << nTimeResidual);

            const size_t nDrRes = itsSubtrSources.size();
            LOG_DEBUG_STR("#sources to subtract: " << nDrRes);

            size_t strides_model[3] = {1, nCr, nChRes * nCr};
            cursor<dcomplex> cr_model_res(&(residual[thread][0][0][0]), 3, strides_model);

            for(size_t j = 0; j < nTimeResidual; ++j)
            {
                for(size_t dr = 0; dr < nDrRes; ++dr)
                {
                    // Re-simulate for residual if required.
                    if(timeFactor != 1 || nCh != nChRes)
                    {
                        fill(&(residual[thread][0][0][0]),
                            &(residual[thread][0][0][0]) + nBl * nChRes * nCr,
                            dcomplex());

                        const_cursor<double> cr_uvw = casa_const_cursor(target[i * timeFactor + j].getUVW());
                        splitUVW(nSt, nBl, cr_baseline, cr_uvw, cr_split);

                        rotateUVW(itsPhaseRef, itsPatchList[dr].position, nSt, cr_split);

                        simulate(itsPatchList[dr].position, itsPatchList[dr], nSt, nBl, nChRes,
                            cr_baseline, cr_freqRes, cr_split, cr_model_res);
                    }
                    else
                    {
                        copy(&(buffer[thread][dr][0][0][0]), &(buffer[thread][dr][0][0][0]) + nBl * nChRes * nCr, &(residual[thread][0][0][0]));
                    }

                    // Apply solutions.
                    size_t strides_coeff[2] = {1, 8};
                    const_cursor<double> cr_coeff(&(unknowns[thread][dr * nSt * 8]), 2, strides_coeff);
                    apply(nBl, nChRes, cr_baseline, cr_coeff, cr_model_res);

                    // Subtract.
                    cursor<fcomplex> cr_residual = casa_cursor(target[i * timeFactor + j].getData());

                    const casa::IPosition tmp_strides_mix_res = itsFactorsSubtr[i * timeFactor + j].steps();
                    LOG_DEBUG_STR("nDrRes: " << nDrRes << " itsNrDir: " << itsNrDir << " " << tmp_strides_mix_res[2] << " " << tmp_strides_mix_res[3] << " " << tmp_strides_mix_res[4]);
                    ASSERT(static_cast<size_t>(tmp_strides_mix_res[2]) == (itsNrDir) * (itsNrDir)
                        && static_cast<size_t>(tmp_strides_mix_res[3]) == nCr * (itsNrDir) * (itsNrDir)
                        && static_cast<size_t>(tmp_strides_mix_res[4]) == nChRes * nCr * (itsNrDir) * (itsNrDir));

                    // The target direction is always last, and therefore it has index itsNrDir - 1.
                    // The directions to subtract are always first, and therefore have indices 0..nDrRes.
                    const_cursor<dcomplex> cr_mix(&(itsFactorsSubtr[i * timeFactor + j](casa::IPosition(5, itsNrDir - 1, dr, 0, 0, 0))), 3, tmp_strides_mix_res.storage() + 2);
                    subtract(nBl, nChRes, cr_baseline, cr_residual, cr_model_res, cr_mix);
                }
            }

            // Copy solutions to global solution array.
            copy(&(unknowns[thread][0]), &(unknowns[thread][0]) + nDr * nSt * 8,
                &(itsUnknowns(IPosition(4, 0, 0, 0, itsTimeCount + i))));
        }

        itsTimerSolve.stop();

        // Let the next step process the data.
        itsTimer.stop();
        for (uint i=0; i<itsNTimeOutSubtr; ++i) {
          getNextStep()->process (itsAvgResultSubtr->get()[i]);
          itsAvgResultSubtr->get()[i].clear();
        }
        // Clear the vector in the MultiStep.
        itsAvgResultSubtr->clear();
        itsTimer.start();
      }
    }

/*
    void Demixer::demix()
    {
      // Only solve and subtract if multiple directions.
      if (itsNrDir > 1) {
        // Collect buffers for each direction.
        vector<vector<DPBuffer> > buffers;
        for (size_t i=0; i<itsAvgResults.size(); ++i) {
          // Only the phased shifted and averaged data for directions which have
          // an associated model should be used to estimate the directional
          // response.
          if(i < itsNrModel) {
            buffers.push_back (itsAvgResults[i]->get());
          }
          // Clear the buffers.
          itsAvgResults[i]->clear();
        }

        itsTimerSolve.start();
        // Construct grids for parameter estimation.
        Axis::ShPtr timeAxis (new RegularAxis (itsStartTimeChunk,
                                               itsTimeIntervalAvg,
                                               itsNTimeOut));
        Grid visGrid(itsFreqAxisDemix, timeAxis);
        // Solve for each time slot over all channels.
        Grid solGrid(itsFreqAxisDemix->compress(itsFreqAxisDemix->size()),
                     timeAxis);

        // Estimate model parameters.
        LOG_DEBUG_STR("estimating....");
//        itsBBSExpr.estimate(buffers, visGrid, solGrid, itsFactors);
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
//        estimate(buffers, itsFactors, itsState, itsTimeCount);
//        estimate(itsAvgResultSubtr->get(), buffers, itsFactorsSubtr, itsState);
//        estimate(itsAvgResultSubtr->get(), buffers, itsFactors, itsFactorsSubtr,
//            itsState, itsTimeCount);
        ASSERTSTR(itsNTimeAvgSubtr <= itsNTimeAvg && itsNTimeAvg % itsNTimeAvgSubtr == 0, "Subtr: " << itsNTimeAvgSubtr << " Demix: " << itsNTimeAvg);
        demix2(itsAvgResultSubtr->get(), buffers, itsFactors, itsFactorsSubtr,
          itsState, itsTimeCount, buffers[0].size(), itsNTimeAvg
          / itsNTimeAvgSubtr, itsPatchList);
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
        itsTimerSolve.stop();

//        itsTimerSubtract.start();
//        // Construct subtract grid if it differs from the grid used for
//        // parameter estimation.
//        Axis::ShPtr timeAxisSubtr (new RegularAxis (itsStartTimeChunk,
//                                                    itsTimeIntervalSubtr,
//                                                    itsNTimeOutSubtr));
//        Grid visGrid = Grid(itsFreqAxisSubtr, timeAxisSubtr);
//        // Subtract the sources.
//        LOG_DEBUG_STR("subtracting....");
//        LOG_DEBUG_STR("SHAPES SUBTRACT: " << itsFactorsSubtr[0].shape() << " "
//                      << itsFreqAxisSubtr->size() << " "
//                      << itsAvgResultSubtr->get()[0].getData().shape());
//        itsBBSExpr.subtract (itsAvgResultSubtr->get(), visGrid, itsFactorsSubtr,
//                             targetIndex, itsSubtrSources.size());
//        itsTimerSubtract.stop();
      }

      // Let the next step process the data.
      itsTimer.stop();
      for (uint i=0; i<itsNTimeOutSubtr; ++i) {
        getNextStep()->process (itsAvgResultSubtr->get()[i]);
        itsAvgResultSubtr->get()[i].clear();
      }
      // Clear the vector in the MultiStep.
      itsAvgResultSubtr->clear();
      itsTimer.start();
    }
*/
//    Axis::ShPtr Demixer::makeFreqAxis (uint nchanAvg)
//    {
//      casa::Vector<double> freq = itsInput->chanFreqs(nchanAvg);
//      casa::Vector<double> width = itsInput->chanWidths(nchanAvg);
//      ASSERT (allEQ (width, width[0]));

//      return Axis::ShPtr(new RegularAxis (freq[0] - width[0] * 0.5, width[0],
//        freq.size()));
//    }

    string Demixer::toString (double value) const
    {
      ostringstream os;
      os << setprecision(16) << value;
      return os.str();
    }

    double Demixer::getAngle (const String& value) const
    {
      double angle;
      Quantity q;
      ASSERTSTR (Quantity::read (q, value),
                 "Demixer: " + value + " is not a proper angle");
      if (q.getUnit().empty()) {
        angle = q.getValue() / 180. * C::pi;
      } else {
        ASSERTSTR (q.getFullUnit().getValue() == UnitVal::ANGLE,
                   "Demixer: " + value + " is not a proper angle");
        angle = q.getValue("rad");
      }
      return angle;
    }

    void Demixer::dumpSolutions()
    {
      LOG_DEBUG_STR("writing solutions...");

      try {
        ParmManager::instance().initCategory(INSTRUMENT,
          ParmDB(ParmDBMeta("casa", itsInstrumentName)));
      } catch (Exception &e) {
        THROW(Exception, "Failed to open instrument model parameter database: "
          << itsInstrumentName);
      }

      // Construct grids for parameter estimation.
      casa::Vector<double> freq = itsInput->chanFreqs(itsInput->nchan());
      casa::Vector<double> width = itsInput->chanWidths(itsInput->nchan());
      Axis::ShPtr freqAxis(new RegularAxis(freq[0] - width[0] * 0.5, width[0],
        1));
      Axis::ShPtr timeAxis(new RegularAxis(itsInput->startTime()
        - itsInput->timeInterval() * 0.5, itsTimeIntervalAvg, itsNTimeDemix));

      // Solve for each time slot over all channels.
      Grid solGrid(freqAxis, timeAxis);

      // Set parameter domain.
      ParmManager::instance().setDomain(solGrid.getBoundingBox());

      vector<string> names;
      const casa::Vector<casa::String> &tmp = itsInput->antennaNames();
      for(size_t i = 0; i < tmp.size(); ++i)
      {
         names.push_back(tmp(i));
      }
      LOG_DEBUG_STR("#stations: " << names.size());
      ASSERT(names.size() == itsNStation);

      vector<ParmProxy::Ptr> parms;
      BBS::ParmGroup group;
      for(size_t dr = 0; dr < itsNrModel; ++dr)
      {
        for(size_t st = 0; st < itsNStation; ++st)
        {
          parms.push_back(ParmManager::instance().get(INSTRUMENT,
            "DirectionalGain:0:0:Real:" + names[st] + ":" + itsAllSources[dr]));
          parms.push_back(ParmManager::instance().get(INSTRUMENT,
            "DirectionalGain:0:0:Imag:" + names[st] + ":" + itsAllSources[dr]));

          parms.push_back(ParmManager::instance().get(INSTRUMENT,
            "DirectionalGain:0:1:Real:" + names[st] + ":" + itsAllSources[dr]));
          parms.push_back(ParmManager::instance().get(INSTRUMENT,
            "DirectionalGain:0:1:Imag:" + names[st] + ":" + itsAllSources[dr]));

          parms.push_back(ParmManager::instance().get(INSTRUMENT,
            "DirectionalGain:1:0:Real:" + names[st] + ":" + itsAllSources[dr]));
          parms.push_back(ParmManager::instance().get(INSTRUMENT,
            "DirectionalGain:1:0:Imag:" + names[st] + ":" + itsAllSources[dr]));

          parms.push_back(ParmManager::instance().get(INSTRUMENT,
            "DirectionalGain:1:1:Real:" + names[st] + ":" + itsAllSources[dr]));
          parms.push_back(ParmManager::instance().get(INSTRUMENT,
            "DirectionalGain:1:1:Imag:" + names[st] + ":" + itsAllSources[dr]));
        }
      }

      vector<string> incl(1, "DirectionalGain:*"), excl;
      ParmGroup solvables = ParmManager::instance().makeSubset(incl, excl);
      LOG_DEBUG_STR("#parms in group: " << solvables.size());

      // Assign solution grid to solvables.
      ParmManager::instance().setGrid(solGrid, solvables);

      LOG_DEBUG_STR("#parms: " << parms.size());
      for(size_t ts = 0; ts < itsNTimeDemix; ++ts)
      {
        double *offset = &(itsUnknowns(IPosition(4, 0, 0, 0, ts)));
        for(size_t i = 0; i < parms.size(); ++i)
        {
          parms[i]->setCoeff(BBS::Location(0, ts), offset + i, 1);
        }
      }

      // Flush solutions to disk.
      ParmManager::instance().flush();
    }
  } //# end namespace
}

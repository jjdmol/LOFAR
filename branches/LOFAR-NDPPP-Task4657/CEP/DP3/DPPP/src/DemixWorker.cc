//# DemixerWorker.cc: Demixing helper class to demix a time chunk
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
//# $Id: Demixer.cc 24221 2013-03-12 12:24:48Z diepen $
//#
//# @author Ger van Diepen

#include <lofar_config.h>
#include <DPPP/DemixWorker.h>
#include <DPPP/Apply.h>
#include <DPPP/Averager.h>
#include <DPPP/CursorUtilCasa.h>
#include <DPPP/DPBuffer.h>
#include <DPPP/DPInfo.h>
#include <DPPP/EstimateMixed.h>
#include <DPPP/PhaseShift.h>
#include <DPPP/Simulate.h>
#include <DPPP/SourceDBUtil.h>
#include <DPPP/SubtractMixed.h>

#include <ParmDB/Axis.h>
#include <ParmDB/SourceDB.h>
#include <ParmDB/ParmDB.h>
#include <ParmDB/ParmSet.h>
#include <ParmDB/ParmCache.h>
#include <ParmDB/Parm.h>

#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/OpenMP.h>
#include <Common/StreamUtil.h>
#include <Common/lofar_iomanip.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_fstream.h>

#include <casa/Quanta/MVAngle.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/MatrixIter.h>
#include <scimath/Mathematics/MatrixMathLA.h>

using namespace casa;

namespace LOFAR {
  namespace DPPP {

    using LOFAR::operator<<;

    namespace
    {
      string toString (double value);
    } //# end unnamed namespace

    DemixWorker::DemixWorker (DPInput* input,
                              const string& prefix,
                              const DemixInfo& mixInfo,
                              const DPInfo& info)
      : itsMix         (&mixInfo),
        itsFilter      (input, mixInfo.selBL()),
        itsNrSolves    (0),
        itsNrConverged (0),
        itsNrNoDemix   (0),
        itsNrIncludeStrongTarget (0),
        itsNrIncludeCloseTarget  (0),
        itsNrIgnoreTarget        (0),
        itsNrDeprojectTarget     (0)
    {
      // Add a null step as the last step in the filter.
      DPStep::ShPtr nullStep(new NullStep());
      itsFilter.setNextStep (nullStep);
      // The worker will process up to chunkSize input time slots.
      // Size buffers accordingly.
      uint ndir = itsMix->ateamList().size();
      itsFactors.resize      (itsMix->chunkSize());
      itsFactorsSubtr.resize (itsMix->ntimeOutSubtr());
      itsOrigPhaseShifts.reserve (ndir);
      itsOrigFirstSteps.reserve  (ndir+1);   // also needed for target direction
      itsAvgResults.reserve  (ndir+1);

      // Create the solve and subtract steps for the sources to be removed.
      // Solving consists of the following steps:
      // - select the requested baselines (longer baselines may need no demix)
      // - phaseshift selected data to each demix source direction
      // - average selected data in each direction, also original phasecenter.
      // - determine and average demix factors for all directions
      // - predict and solve in each direction. It is possible to predict
      //   more directions than to solve (for strong sources in field).
      // Subtract consists of the following steps:
      // - average all data (possibly different averaging than used in solve)
      // - determine and average demix factors (using select output in solve)
      // - select the requested baselines
      // - subtract sources for selected data
      // - merge subtract result into averaged data. This is not needed if
      //   no selection is done.
      // Note that multiple time chunks are handled jointly, so a
      // MultiResultStep is used to catch the results of all time chunks.
      const vector<Patch::ConstPtr>& patchList = itsMix->ateamDemixList();
      vector<string> sourceVec(2);
      for (uint i=0; i<ndir; ++i) {
        // First make the phaseshift and average steps for each demix source.
        // The resultstep gets the result.
        // The phasecenter can be given in a parameter. Its name is the default.
        // Look up the source direction in the patch table.
        // If found, turn it into a vector of strings.
        sourceVec[0] = toString(patchList[i]->position()[0]);
        sourceVec[1] = toString(patchList[i]->position()[1]);
        PhaseShift* step1 = new PhaseShift(input, ParameterSet(),
                                           prefix+itsMix->sourceNames()[i]+'.',
                                           sourceVec);
        itsOrigFirstSteps.push_back (DPStep::ShPtr(step1));
        itsOrigPhaseShifts.push_back (step1);
        DPStep::ShPtr step2 (new Averager(input, prefix, itsMix->nchanAvg(),
                                          itsMix->ntimeAvg()));
        step1->setNextStep (step2);
        MultiResultStep* step3 = new MultiResultStep(itsMix->ntimeOut());
        step2->setNextStep (DPStep::ShPtr(step3));
        // There is a single demix factor step which needs to get all results.
        itsAvgResults.push_back (step3);
      }

      // Now create the step to average the data themselves.
      DPStep::ShPtr targetAvg(new Averager(input, prefix,
                                           itsMix->nchanAvg(),
                                           itsMix->ntimeAvg()));
      itsOrigFirstSteps.push_back (targetAvg);
      MultiResultStep* targetAvgRes = new MultiResultStep(itsMix->ntimeOut());
      targetAvg->setNextStep (DPStep::ShPtr(targetAvgRes));
      itsAvgResults.push_back (targetAvgRes);

      // Create the data average step for the subtract.
      // The entire average result is needed for the next NDPPP step.
      // Only the selected baselines need to be subtracted, so add a
      // filter step as the last one.
      itsAvgStepSubtr = DPStep::ShPtr(new Averager(input, prefix,
                                                   itsMix->nchanAvgSubtr(),
                                                   itsMix->ntimeAvgSubtr()));
      itsAvgResultFull  = new MultiResultStep(itsMix->ntimeOutSubtr());
      itsFilterSubtr    = new Filter(input, itsMix->selBL());
      itsAvgResultSubtr = new MultiResultStep(itsMix->ntimeOutSubtr());
      itsAvgStepSubtr->setNextStep (DPStep::ShPtr(itsAvgResultFull));
      itsAvgResultFull->setNextStep (DPStep::ShPtr(itsFilterSubtr));
      itsFilterSubtr->setNextStep (DPStep::ShPtr(itsAvgResultSubtr));

      // Let the internal steps update their data.
      itsFilter.setInfo (info);
      for (uint i=0; i<itsOrigFirstSteps.size(); ++i) {
        itsOrigFirstSteps[i]->setInfo (itsFilter.getInfo());
      }
      itsAvgStepSubtr->setInfo (info);

      // Size the various work buffers.
      itsFactorBuf.resize (IPosition(4, itsMix->ncorr(), itsMix->nchanIn(),
                                      itsMix->nbl(), ndir*(ndir+1)/2));
      itsFactorBufSubtr.resize (itsFactorBuf.shape());
      itsAvgUVW.resize (3, itsMix->nbl());
      itsStationUVW.resize (3, itsMix->nstation(), itsMix->ntimeOutSubtr());
      itsUVW.resize (3, itsMix->nstation());
      itsIndices.resize (ndir);
      itsStationsToUse.resize (ndir);
      itsUnknownsIndex.resize (ndir+1);
      for (uint i=0; i<ndir+1; ++i) {
        itsUnknownsIndex[i].resize (itsMix->nstation());
      }
      itsNrSourcesDemixed.resize (ndir);
      itsNrSourcesDemixed = 0;
      itsPredictVis.resize (itsMix->ncorr(), itsMix->nchanOut(),
                            itsMix->nbl());
      itsModelVis.resize (itsMix->ncorr() * itsMix->nchanOutSubtr() *
                          itsMix->nbl() * (ndir+1));
      itsAteamAmpl.resize (ndir);
      for (uint i=0; i<ndir; ++i) {
        itsAteamAmpl[i].resize (itsMix->nchanOut(), itsMix->nbl(),
                                itsMix->ntimeOut());
      }
      itsTargetAmpl.resize (itsMix->nchanOut(), itsMix->nbl(),
                            itsMix->ntimeOut());
      itsEstimate.update (ndir+1, itsMix->nbl(), itsMix->nstation(),
                          itsMix->nchanOut(), itsMix->maxIter(),
                          itsMix->propagateSolution());
    }

    void DemixWorker::process (const DPBuffer* bufin, uint nbufin,
                               DPBuffer* bufout, vector<double>* solutions)
    {
      itsTimer.start();
      itsTimerPredict.start();
      // Average and split the baseline UVW coordinates per station.
      // Do this at the demix time resolution.
      // The buffer has not been filtered yet, so the UVWs have to be filtered.
      uint ntime = avgSplitUVW (bufin, nbufin, itsMix->ntimeAvg(),
                                itsFilter.getIndicesBL());
      double timeStep = bufin[0].getExposure();
      double time = bufin[0].getTime() + (itsMix->ntimeAvg()-1) * 0.5*timeStep;
      // First do the predict of the coarse A-team sources at demix resolution.
      // It fills the indices of the sources to demix.
      // It also fills in the baselines that do not need to used.
      predictAteam (itsMix->ateamList(), ntime, time, timeStep);
      // If no sources to demix, simply average the input buffers.
      if (itsIndices.empty()) {
        itsTimerPredict.stop();
        itsTimerPhaseShift.start();
        average (bufin, nbufin, bufout);
        itsTimerPhaseShift.stop();
        itsTimer.stop();
        return;
      }
      // The target has to be predicted as well (also at demix resolution).
      predictTarget (itsMix->targetList(), ntime, time, timeStep);
      // Determine what needs to be done.
      // It fills in the steps to perform for the sources to demix.
      setupDemix();
      itsTimerPredict.stop();
      // Loop over the buffers and process them.
      itsNTimeOut = 0;
      itsNTimeOutSubtr = 0;
      for (uint i=0; i<nbufin; ++i) {
        // Do the filter step first.
        itsFilter.process (bufin[i]);
        const DPBuffer& selBuf = itsFilter.getBuffer();
        // Do the next steps (phaseshift and average) on the filter output.
        itsTimerPhaseShift.start();
        for (uint j=0; j<itsFirstSteps.size(); ++j) {
          itsFirstSteps[j]->process (selBuf);
        }
        // Do the average and filter step for the output for all data.
        itsAvgStepSubtr->process (bufin[i]);
        itsTimerPhaseShift.stop();

        // For each NTimeAvg times, calculate the phase rotation per direction
        // for the selected data.
        itsTimerDemix.start();
        addFactors (selBuf, itsFactorBuf);
        if ((i+1) % itsMix->ntimeAvg() == 0  ||  i == nbufin-1) {
          makeFactors (itsFactorBuf, itsFactors[itsNTimeOut],
                       itsAvgResults[0]->get()[itsNTimeOut].getWeights(),
                       itsMix->nchanOut(),
                       itsMix->nchanAvg());
          // Deproject sources without a model.
          deproject (itsFactors[itsNTimeOut], itsAvgResults, itsNTimeOut);
          itsFactorBuf = Complex();       // Clear summation buffer
          itsNTimeOut++;
        }
        // Subtract is done with different averaging parameters, so calculate
        // the factors for it (again for selected data only).
        addFactors (selBuf, itsFactorBufSubtr);
        if ((i+1) % itsMix->ntimeAvgSubtr() == 0  ||  i == nbufin-1) {
          makeFactors (itsFactorBufSubtr, itsFactorsSubtr[itsNTimeOutSubtr],
                       itsAvgResultSubtr->get()[itsNTimeOutSubtr].getWeights(),
                       itsMix->nchanOutSubtr(),
                       itsMix->nchanAvgSubtr());
          itsFactorBufSubtr = Complex();  // Clear summation buffer
          itsNTimeOutSubtr++;
        }
        itsTimerDemix.stop();
      }

      // Estimate gains and subtract source contributions.
      handleDemix (bufout, solutions);
      itsTimer.stop();
    }

    uint DemixWorker::avgSplitUVW (const DPBuffer* bufin, uint nbufin,
                                   uint ntimeAvg, const vector<uint>& selbl)
    {
      ASSERT (selbl.size() == size_t(itsAvgUVW.shape()[1]));
      // First average the UVWs to the predict time window.
      uint ntime = (nbufin + ntimeAvg - 1) / ntimeAvg;
      const DPBuffer* buf = bufin;
      uint nleft = nbufin;
      // Loop over nr of output time slots.
      MatrixIterator<double> uvwIter(itsStationUVW);
      for (uint i=0; i<ntime; ++i) {
        // Sum the times for this output time slot.
        // Only take the selected baselines into account.
        itsAvgUVW = 0.;
        uint ntodo = std::min(ntimeAvg, nleft);
        for (uint j=0; j<ntodo; ++j) {
          double* sumPtr = itsAvgUVW.data();
          for (uint k=0; k<selbl.size(); ++k) {
            const double* uvwPtr = buf->getUVW().data() + 3*selbl[k];
            for (int k1=0; k1<3; ++k1) {
              *sumPtr++ += uvwPtr[k1];
            }
          }
          buf++;
        }
        // Average the UVWs.
        itsAvgUVW /= double(ntodo);
        ///        cout<<"avguvw="<<ntodo<<itsAvgUVW<<endl;
        nleft -= ntodo;
        // Split the baseline UVW coordinates per station.
        nsplitUVW (itsMix->uvwSplitIndex(), itsMix->baselines(),
                   itsAvgUVW, uvwIter.matrix());
        uvwIter.next();
      }
      /// cout<<"stationuwv="<<itsStationUVW;
      return ntime;
    }

    void DemixWorker::predictTarget (const vector<Patch::ConstPtr>& patchList,
                                     uint ntime, double time, double timeStep)
    {
      // This is only needed for the short baselines.
      // So better to have a Bool array telling which baselines to use.
      // Also no filter step needed at beginning.
      itsTargetAmpl = 0;
      MatrixIterator<float> miter(itsTargetAmpl);
      MatrixIterator<double> uvwiter(itsStationUVW);
      double t = time;
      for (uint j=0; j<ntime; ++j) {
        for (uint i=0; i<patchList.size(); ++i) {
          itsPredictVis = dcomplex();
          simulate (itsMix->phaseRef(),
                    patchList[i],
                    itsMix->nstation(),
                    itsMix->nbl(),
                    itsMix->freqDemix().size(),
                    const_cursor<Baseline>(&(itsMix->baselines()[0])),
                    casa_const_cursor<double>(itsMix->freqDemix()),
                    casa_const_cursor<double>(uvwiter.matrix()),
                    casa_cursor<dcomplex>(itsPredictVis));
          // Apply beam for target patch.
          applyBeam (t, patchList[i]->position());
          addStokesI (miter.matrix());
        }
        miter.next();
        uvwiter.next();
        t += timeStep;
      }
    }

    void DemixWorker::addStokesI (Matrix<float>& ampl)
    {
      // Calculate the StokesI ampl ((XX+YY)/2).
      Array<dcomplex>::const_contiter iterEnd = itsPredictVis.cend();
      float* amplp = ampl.data();
      for (Array<dcomplex>::const_contiter iter=itsPredictVis.cbegin();
           iter!=iterEnd; ++iter) {
        float a = abs(*iter);
        ++iter; ++iter; ++iter;    // skip XY and YX
        // Add amplitude.
        *amplp++ += 0.5*(a + abs(*iter));
      }
      cout << "min/max ampl="<<min(ampl)<<' '<<max(ampl)<<endl;
    }

    void DemixWorker::predictAteam (const vector<Patch::ConstPtr>& patchList,
                                    uint ntime, double time, double timeStep)
    {
      for (uint i=0; i<patchList.size(); ++i) {
        itsAteamAmpl[i] = 0;
        MatrixIterator<float> miter(itsAteamAmpl[i]);
        MatrixIterator<double> uvwiter(itsStationUVW);
        double t = time;
        for (uint j=0; j<ntime; ++j) {
          itsPredictVis = dcomplex();
          simulate (itsMix->phaseRef(),
                    patchList[i],
                    itsMix->nstation(),
                    itsMix->nbl(),
                    itsMix->freqDemix().size(),
                    const_cursor<Baseline>(&(itsMix->baselines()[0])),
                    casa_const_cursor<double>(itsMix->freqDemix()),
                    casa_const_cursor<double>(itsStationUVW),
                    casa_cursor<dcomplex>(itsPredictVis));
          // Apply beam.
          applyBeam (t, patchList[i]->position());
          // Keep the StokesI ampl ((XX+YY)/2).
          addStokesI (miter.matrix());
          miter.next();
          uvwiter.next();
          t += timeStep;
        }
      }
      // Determine per A-source which stations to use.
      itsIndices.resize (0);
      vector<uint> antCount (itsMix->nstation());
      for (uint i=0; i<patchList.size(); ++i) {
        // Count the stations of baselines with sufficient amplitude.
        std::fill (antCount.begin(), antCount.end(), 0);
        MatrixIterator<float> miter(itsAteamAmpl[i], 0, 2);
        for (uint j=0; j<itsMix->nbl(); ++j, miter.next()) {
          if (max(miter.matrix()) > itsMix->ateamAmplThreshold()) {
            antCount[itsMix->getAnt1()[j]]++;
            antCount[itsMix->getAnt2()[j]]++;
            cout<<"baseline "<<itsMix->getAnt1()[j]<<' '<<itsMix->getAnt2()[j]<<" has ampl " << max(miter.matrix())<<endl;
          }
        }
        // Determine which stations have sufficient occurrence.
        itsStationsToUse[i].resize (0);
        for (uint j=0; j<antCount.size(); ++j) {
          if (antCount[j] >= itsMix->minNStation()) {
            itsStationsToUse[i].push_back (j);
          } else {
            cout << "ignore station " << j << " for src " << i << endl;
          }
        }
        // Use this A-team source if some stations have matched.
        if (! itsStationsToUse[i].empty()) {
          itsIndices.push_back (i);
          itsNrSourcesDemixed[i]++;
        }
      }
    }

    void DemixWorker::average (const DPBuffer* bufin, uint nbufin,
                               DPBuffer* bufout)
    {
      for (uint i=0; i<nbufin; ++i) {
        itsAvgStepSubtr->process (bufin[i]);
      }
      itsAvgStepSubtr->finish();
      ASSERT (itsAvgResultFull->get().size() <= itsMix->ntimeOutSubtr());
      for (uint i=0; i<itsAvgResultFull->get().size(); ++i) {
        bufout[i] = itsAvgResultFull->get()[i];
      }
      itsAvgResultFull->clear();
    }

    void DemixWorker::setupDemix()
    {
      // Decide if the target has to be included, ignored, or deprojected
      // which depends on the ratio target/Ateam of the total ampl.
      // Fill in the steps to be executed by removing the too weak A-sources.
      uint nsrc = itsIndices.size();
      itsPhaseShifts.resize (nsrc);
      itsFirstSteps.resize (nsrc+1);
      // Add target.
      itsFirstSteps[nsrc] = itsOrigFirstSteps[itsOrigFirstSteps.size() - 1];
      float minSumAmpl = 1e30;
      float maxSumAmpl = 0;
      // Add sources to be demixed.
      // Determine their minimum and maximum amplitude.
      for (uint i=0; i<nsrc; ++i) {
        itsPhaseShifts[i] = itsOrigPhaseShifts[itsIndices[i]];
        itsFirstSteps[i] = itsOrigFirstSteps[itsIndices[i]];
        float sumAmpl = sum(itsTargetAmpl[itsIndices[i]]);
        if (sumAmpl < minSumAmpl) {
          minSumAmpl = sumAmpl;
        }
        if (sumAmpl > maxSumAmpl) {
          maxSumAmpl = sumAmpl;
        }
      }
      float targetSumAmpl = sum(itsTargetAmpl);
      itsIncludeTarget = false;
      itsIgnoreTarget = false;
      itsNModel = nsrc;
      itsNDir   = nsrc+1;
      if (targetSumAmpl / maxSumAmpl > itsMix->ratio1()  ||
          targetSumAmpl > itsMix->targetAmplThreshold()) {
        itsIncludeTarget = true;
        itsNrIncludeStrongTarget++;
        itsNModel++;
      } else if (! itsMix->isAteamNearby()) {
        itsNrDeprojectTarget++;
      } else if (targetSumAmpl / minSumAmpl > itsMix->ratio2()) {
        itsIncludeTarget = true;
        itsNrIncludeCloseTarget++;
        itsNModel++;
      } else {
        itsIgnoreTarget = true;
        itsNrIgnoreTarget++;
      }
      // Determine the unknowns to be solved.
      // The unknowns are complex values of a Jones matrix per direction-station.
      // thus 8 real unknowns per direction-station.
      // Their names are DirectionalGain:i:j:Type::Station::Direction
      // where i:j gives the Jones element and Type is Real or Imag.
      // Fill the map of source/station to unknown seqnr (4 pol, ampl/phase).
      uint ndir = itsStationsToUse.size();
      uint nUnknown = 0;
      for (uint i=0; i<ndir; ++i) {
        // Initialize unknowns index.
        std::fill (itsUnknownsIndex[i].begin(), itsUnknownsIndex[i].end(), -1);
        for (uint j=0; j<itsStationsToUse[i].size(); ++j) {
          uint st = itsStationsToUse[i][j];
          itsUnknownsIndex[i][st] = nUnknown;
          nUnknown += 8;
        }
      }
      cout<<"nunkb="<<nUnknown<<endl;
      // Do the same for the target if it has to be solved as well.
      std::fill (itsUnknownsIndex[ndir].begin(),
                 itsUnknownsIndex[ndir].end(), -1);
      if (itsIncludeTarget) {
        for (uint j=0; j<itsUnknownsIndex[ndir].size(); ++j) {
          itsUnknownsIndex[ndir][j] = nUnknown;
          nUnknown += 8;
        }
      }
      cout<<"nunka="<<nUnknown<<endl;
    }

    void DemixWorker::applyBeam (double time, const Position& dir)
    {
    }

    void DemixWorker::handleDemix (DPBuffer* bufout, vector<double>* solutions)
    {
      itsTimerSolve.start();
      demix (solutions);
      itsTimerSolve.stop();
      // If selection was done, merge the subtract results back into the
      // buffer.
      if (itsMix->selBL().hasSelection()) {
	mergeSubtractResult ();
      }
      // Clear the input buffers.
      for (size_t i=0; i<itsAvgResults.size(); ++i) {
        itsAvgResults[i]->clear();
      }
      // Store the output buffers.
      for (uint i=0; i<itsMix->ntimeOutSubtr(); ++i) {
        if (itsMix->selBL().hasSelection()) {
          /// Probably make a deep copy, because in next iteration avgresult gets
          /// overridden.
          bufout[i] = itsAvgResultFull->get()[i];
        } else {
          bufout[i] = itsAvgResultSubtr->get()[i];
        }
      }
      // Clear the output buffers.
      itsAvgResultFull->clear();
      itsAvgResultSubtr->clear();
    }

    void DemixWorker::mergeSubtractResult()
    {
      // Merge the selected baselines from the subtract buffer into the
      // full buffer. Do it for all timestamps.
      for (uint i=0; i<itsNTimeOutSubtr; ++i) {
        const Array<Complex>& arr = itsAvgResultSubtr->get()[i].getData();
        size_t nr = arr.shape()[0] * arr.shape()[1];
        const Complex* in = arr.data();
        Complex* out = itsAvgResultFull->get()[i].getData().data();
        for (size_t j=0; j<itsFilter.getIndicesBL().size(); ++j) {
          size_t inx = itsFilter.getIndicesBL()[j];
          memcpy (out+inx*nr, in+j*nr, nr*sizeof(Complex));
        }
      }
    }

    void DemixWorker::addFactors (const DPBuffer& newBuf,
                                  Array<DComplex>& factorBuf)
    {
      // Nothing to do if only target direction.
      if (itsNDir <= 1) return;
      int ncorr  = newBuf.getData().shape()[0];
      int nchan  = newBuf.getData().shape()[1];
      int nbl    = newBuf.getData().shape()[2];
      int ncc    = ncorr*nchan;
      //# If ever in the future a time dependent phase center is used,
      //# the machine must be reset for each new time, thus each new call
      //# to process.
      // Add the weighted factors for each pair of directions.
      // The input factor is the phaseshift from target direction to
      // source direction. By combining them you get the shift from one
      // source direction to another.
      int dirnr = 0;
      for (uint i1=0; i1<itsNDir-1; ++i1) {
        for (uint i0=i1+1; i0<itsNDir; ++i0) {
          if (i0 == itsNDir-1) {
            // The last direction is the target direction, so no need to
            // combine the factors. Take conj to get shift source to target.
            for (int i=0; i<nbl; ++i) {
              const bool*   flagPtr   = newBuf.getFlags().data() + i*ncc;
              const float*  weightPtr = newBuf.getWeights().data() + i*ncc;
              DComplex* factorPtr     = factorBuf.data() + (dirnr*nbl + i)*ncc;
              const DComplex* phasor1 = itsPhaseShifts[i1]->getPhasors().data()
                                        + i*nchan;
              for (int j=0; j<nchan; ++j) {
                DBGASSERT (phasor1 < itsPhaseShifts[i1]->getPhasors().data()+itsPhaseShifts[i1]->getPhasors().size());
                DComplex factor = conj(*phasor1++);
                for (int k=0; k<ncorr; ++k) {
                  DBGASSERT (flagPtr < newBuf.getFlags().data()+newBuf.getFlags().size());
                  DBGASSERT (weightPtr < newBuf.getWeights().data()+newBuf.getWeights().size());
                  if (! *flagPtr) {
                    *factorPtr += factor * double(*weightPtr);
                  }
                  flagPtr++;
                  weightPtr++;
                  factorPtr++;
                }
              }
            }
          } else {
            // Different source directions; take both phase terms into account.
            for (int i=0; i<nbl; ++i) {
              const bool*   flagPtr   = newBuf.getFlags().data() + i*ncc;
              const float*  weightPtr = newBuf.getWeights().data() + i*ncc;
              DComplex* factorPtr     = factorBuf.data() + (dirnr*nbl + i)*ncc;
              const DComplex* phasor0 = itsPhaseShifts[i0]->getPhasors().data()
                                        + i*nchan;
              const DComplex* phasor1 = itsPhaseShifts[i1]->getPhasors().data()
                                        + i*nchan;
              for (int j=0; j<nchan; ++j) {
                DComplex factor = *phasor0++ * conj(*phasor1++);
                for (int k=0; k<ncorr; ++k) {
                  if (! *flagPtr) {
                    *factorPtr += factor * double(*weightPtr);
                  }
                  flagPtr++;
                  weightPtr++;
                  factorPtr++;
                }
              }
            }
          }
          // Next direction pair.
          dirnr++;
        }
      }
    }

    void DemixWorker::makeFactors (const Array<DComplex>& bufIn,
                                   Array<DComplex>& bufOut,
                                   const Cube<float>& weightSums,
                                   uint nChanOut,
                                   uint nChanAvg)
    {
      // Nothing to do if only target direction.
      if (itsNDir <= 1) return;
      ASSERT (! weightSums.empty());
      bufOut.resize (IPosition(5, itsNDir, itsNDir,
                               itsMix->ncorr(), nChanOut, itsMix->nbl()));
      bufOut = DComplex(1,0);
      int ncc = itsMix->ncorr()*nChanOut;
      int nccdd = ncc*itsNDir*itsNDir;
      int nccin = itsMix->ncorr()*itsMix->nchanIn();
      // Fill the factors for each combination of different directions.
      uint dirnr = 0;
      for (uint d0=0; d0<itsNDir; ++d0) {
        for (uint d1=d0+1; d1<itsNDir; ++d1) {
          // Average factors by summing channels.
          // Note that summing in time is done in addFactors.
          // The sum per output channel is divided by the summed weight.
          // Note there is a summed weight per baseline,outchan,corr.
          for (int k=0; k<int(itsMix->nbl()); ++k) {
            const DComplex* phin = bufIn.data() + (dirnr*itsMix->nbl() + k)*nccin;
            DComplex* ph1 = bufOut.data() + k*nccdd + (d0*itsNDir + d1);
            DComplex* ph2 = bufOut.data() + k*nccdd + (d1*itsNDir + d0);
            const float* weightPtr = weightSums.data() + k*ncc;
            for (uint c0=0; c0<nChanOut; ++c0) {
              // Sum the factors for the input channels to average.
              DComplex sum[4];
              // In theory the last output channel could consist of fewer
              // input channels, so take care of that.
              uint nch = std::min(nChanAvg, itsMix->nchanIn()-c0*nChanAvg);
              for (uint c1=0; c1<nch; ++c1) {
                for (uint j=0; j<itsMix->ncorr(); ++j) {
                  DBGASSERT (phin < bufIn.data()+bufIn.size());
                  sum[j] += *phin++;
                }
              }
              for (uint j=0; j<itsMix->ncorr(); ++j) {
                DBGASSERT (ph1 < bufOut.data()+bufOut.size());
                DBGASSERT (ph2 < bufOut.data()+bufOut.size());
                DBGASSERT (weightPtr < weightSums.data() + weightSums.size());
                *ph1 = sum[j] / double(*weightPtr++);
                *ph2 = conj(*ph1);
                ph1 += itsNDir*itsNDir;
                ph2 += itsNDir*itsNDir;
              }
            }
          }
          // Next input direction pair.
          dirnr++;
        }
      }
    }

    void DemixWorker::deproject (Array<DComplex>& factors,
                                 vector<MultiResultStep*> avgResults,
                                 uint resultIndex)
    {
      // Sources without a model have to be deprojected.
      // Optionally no deprojection of target direction.
      uint nrDeproject = itsNDir - itsNModel;
      if (itsIgnoreTarget) {
        nrDeproject--;
      }
      // Nothing to do if only target direction or nothing to deproject.
      if (itsNDir <= 1  ||  nrDeproject == 0) return;
      // Get pointers to the data for the various directions.
      vector<Complex*> resultPtr(itsNDir);
      for (uint j=0; j<itsNDir; ++j) {
        resultPtr[j] = avgResults[j]->get()[resultIndex].getData().data();
      }
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
      shape[1] = itsNModel;
      Array<DComplex> newFactors (shape);
      IPosition inShape (2, itsNDir, itsNDir);
      IPosition outShape(2, itsNDir, itsNModel);
      {
        Matrix<DComplex> a(itsNDir, nrDeproject);
        Matrix<DComplex> ma(itsNDir, itsNModel);
        vector<DComplex> vec(itsNDir);
        for (int i=0; i<nvis; ++i) {
          // Split the matrix into the modeled and deprojected sources.
          // Copy the columns to the individual matrices.
          const DComplex* inptr  = factors.data() + i*itsNDir*itsNDir;
          DComplex* outptr = newFactors.data() + i*itsNDir*itsNModel;
          Matrix<DComplex> out (outShape, outptr, SHARE);
          // Copying a bit of data is probably faster than taking a matrix
          // subset.
          objcopy (ma.data(), inptr, itsNDir*itsNModel);
          objcopy (a.data(), inptr + itsNDir*itsNModel, itsNDir*nrDeproject);
          // Calculate conjugated transpose of A, multiply with A, and invert.
          Matrix<DComplex> at(adjoint(a));
          Matrix<DComplex> ata(invert(product(at, a)));
          if (ata.empty()) {
            ata.resize (nrDeproject, nrDeproject);
          }
          DBGASSERT(ata.ncolumn()==nrDeproject && ata.nrow()==nrDeproject);
          // Calculate P = I - A * ata * A.T.conj
          Matrix<DComplex> aata(product(a,ata));
          Matrix<DComplex> p (-product(product(a, ata), at));
          Vector<DComplex> diag(p.diagonal());
          diag += DComplex(1,0);
          // Multiply the demixing factors with P (get stored in newFactors).
          out = product(p, ma);
          // Multiply the averaged data point with P.
          std::fill (vec.begin(), vec.end(), DComplex());
          for (uint j=0; j<itsNDir; ++j) {
            for (uint k=0; k<itsNDir; ++k) {
              vec[k] += DComplex(resultPtr[j][i]) * p(k,j);
            }
          }
          // Put result back in averaged data for those sources.
          for (uint j=0; j<itsNDir; ++j) {
            resultPtr[j][i] = vec[j];
          }
        }
      }
      // Set the new demixing factors.
      factors.reference (newFactors);
    }

    void DemixWorker::demix (vector<double>* solutions)
    {
      cout << "demix called"<<endl;
      // Determine the various sizes.
      const size_t nTime = itsAvgResults[0]->get().size();
      const size_t nTimeSubtr = itsAvgResultSubtr->get().size();
      const size_t multiplier = itsMix->ntimeAvg() / itsMix->ntimeAvgSubtr();
      const size_t nDr = itsPhaseShifts.size();
      const size_t nSt = itsMix->nstation();
      const size_t nBl = itsMix->baselines().size();
      const size_t nCh = itsMix->freqDemix().size();
      const size_t nChSubtr = itsMix->freqSubtr().size();
      const size_t nCr = 4;
      const size_t nSamples = nBl * nCh * nCr;
      // Define various cursors to iterate through arrays.
      const_cursor<double> cr_freq = casa_const_cursor(itsMix->freqDemix());
      const_cursor<double> cr_freqSubtr = casa_const_cursor(itsMix->freqSubtr());
      const_cursor<Baseline> cr_baseline(&(itsMix->baselines()[0]));
      // Do a solve and subtract for each predict time slot.
      for (size_t ts=0; ts<nTime; ++ts) {
        // Compute the model visibilities for each A-team source.
        size_t stride_model[3] = {1, nCr, nCr*nCh};
        for (size_t dr=0; dr<nDr; ++dr) {
          // Split the baseline UVW coordinates per station.
          nsplitUVW (itsMix->uvwSplitIndex(), itsMix->baselines(),
                     itsAvgResults[dr]->get()[ts].getUVW(), itsUVW);
          // Create cursors to step through UVW and model buffer.
          const_cursor<double> cr_uvw = casa_const_cursor(itsUVW);
          cursor<dcomplex> cr_model(&(itsModelVis[dr * nSamples]), 3,
                                    stride_model);
          // Initialize this part of the buffer.
          std::fill (itsModelVis.begin() + dr*nSamples,
                     itsModelVis.begin() + (dr+1)*nSamples, dcomplex());
          simulate(itsMix->ateamList()[dr]->position(),
                   itsMix->ateamList()[dr],
                   nSt, nBl, nCh, cr_baseline, cr_freq, cr_uvw, cr_model);
        }
        if (itsIncludeTarget) {
          // Do the same for the target sources which are added up.
          nsplitUVW (itsMix->uvwSplitIndex(), itsMix->baselines(),
                     itsAvgResults[nDr]->get()[ts].getUVW(), itsUVW);
          std::fill (itsModelVis.begin() + nDr*nSamples,
                     itsModelVis.begin() + (nDr+1)*nSamples, dcomplex());
          for (size_t dr=0; dr<itsMix->targetList().size(); ++dr) {
            const_cursor<double> cr_uvw = casa_const_cursor(itsUVW);
            cursor<dcomplex> cr_model(&(itsModelVis[nDr * nSamples]), 3,
                                      stride_model);
            simulate(itsMix->ateamList()[dr]->position(),
                     itsMix->ateamList()[dr],
                     nSt, nBl, nCh, cr_baseline, cr_freq, cr_uvw, cr_model);
          }
        }
        // A Jones matrix will be estimated for each pair of stations and
        // direction.
        // A single (overdetermined) non-linear set of equations for all
        // stations and directions is solved iteratively. The influence of
        // each direction on each other direction is given by the mixing
        // matrix.
        const_cursor<bool> cr_flag =
          casa_const_cursor(itsAvgResults[0]->get()[ts].getFlags());
        const_cursor<float> cr_weight =
          casa_const_cursor(itsAvgResults[0]->get()[ts].getWeights());
        const_cursor<dcomplex> cr_mix = casa_const_cursor(itsFactors[ts]);
        // Create a cursor per source.
        vector<const_cursor<fcomplex> > cr_data(itsNModel);
        vector<const_cursor<dcomplex> > cr_model(itsNModel);
        for (size_t dr=0; dr<itsNModel; ++dr) {
          cr_data[dr] =
            casa_const_cursor(itsAvgResults[dr]->get()[ts].getData());
          cr_model[dr] =
            const_cursor<dcomplex>(&(itsModelVis[dr * nSamples]), 3,
                                   stride_model);
        }
        // If solving the system succeeds, increment nconverged.
        bool converged = itsEstimate.estimate (itsUnknownsIndex,
                                               cr_baseline, cr_data,
                                               cr_model, cr_flag,
                                               cr_weight, cr_mix);
        // Copy solutions to overall solution array.
        solutions[ts] = itsEstimate.getSolution();
        itsNrSolves++;
        if (converged) {
          itsNrConverged++;
        }

        // Compute the residual.
        //
        // All the so-called "subtract sources" are subtracted from the
        // observed data. The previously estimated Jones matrices, as well as
        // the appropriate mixing weight are applied before subtraction.
        //
        // Note that the resolution of the residual can differ from the
        // resolution at which the Jones matrices were estimated.
        for (size_t ts_subtr = multiplier * ts,
               ts_subtr_end = min(ts_subtr + multiplier, nTimeSubtr);
             ts_subtr != ts_subtr_end; ++ts_subtr) {
          for (size_t dr = 0; dr < nDr; ++dr) {
            // Re-use simulation used for estimating Jones matrices if possible.
            cursor<dcomplex> cr_model_subtr(&(itsModelVis[dr * nSamples]),
                                            3, stride_model);
            // Re-simulate if required.
            if (multiplier != 1 || nCh != nChSubtr) {
              nsplitUVW (itsMix->uvwSplitIndex(), itsMix->baselines(),
                         itsAvgResultSubtr->get()[ts_subtr].getUVW(), itsUVW);
              // Rotate the UVW coordinates for the target direction to the
              // direction of source to subtract. This is required because at
              // the resolution of the residual the UVW coordinates for
              // directions other than the target are unavailable (unless the
              // resolution of the residual is equal to the resolution at which
              // the Jones matrices were estimated, of course).
              cursor<double> cr_uvw_split = casa_cursor(itsUVW);
              rotateUVW (itsMix->phaseRef(),
                         itsMix->targetList()[dr]->position(), nSt,
                         cr_uvw_split);
              // Initialize the visibility buffer.
              std::fill (itsModelVis.begin(), itsModelVis.end(), dcomplex());
              // Simulate visibilities at the resolution of the residual.
              size_t stride_model_subtr[3] = {1, nCr, nCr * nChSubtr};
              cr_model_subtr = cursor<dcomplex>(&(itsModelVis[0]), 3,
                                                stride_model_subtr);
              simulate(itsMix->ateamList()[dr]->position(),
                       itsMix->ateamList()[dr], nSt, nBl,
                       nChSubtr, cr_baseline, cr_freqSubtr, cr_uvw_split,
                       cr_model_subtr);
            }

            // Apply Jones matrices.
            size_t stride_unknowns[2] = {1, 8};
            const_cursor<double> cr_unknowns(&(solutions[ts][dr * nSt * 8]),
                                             2, stride_unknowns);
            apply (nBl, nChSubtr, cr_baseline, cr_unknowns, cr_model_subtr);

            // Subtract the source contribution from the data.
            cursor<fcomplex> cr_residual =
              casa_cursor(itsAvgResultSubtr->get()[ts_subtr].getData());

            // Construct a cursor to iterate over a slice of the mixing matrix
            // at the resolution of the residual. The "to" and "from" direction
            // are fixed. Since the full mixing matrix is 5-D, the slice is
            // therefore 3-D. Each individual value in the slice quantifies the
            // influence of the source to subtract on the target direction for
            // a particular correlation, channel, and baseline.
            //
            // The target direction is the direction with the highest index by
            // convention, i.e. index itsNDir - 1. The directions to subtract
            // have the lowest indices by convention, i.e. indices
            // [0, nDrSubtr).
            const IPosition &stride_mix_subtr =
              itsFactorsSubtr[ts_subtr].steps();
            size_t stride_mix_subtr_slice[3] = {
              static_cast<size_t>(stride_mix_subtr[2]),
              static_cast<size_t>(stride_mix_subtr[3]),
              static_cast<size_t>(stride_mix_subtr[4])
            };
            ASSERT(stride_mix_subtr_slice[0] == itsNDir * itsNDir
                   && stride_mix_subtr_slice[1] == itsNDir * itsNDir * nCr
                   && stride_mix_subtr_slice[2] == itsNDir * itsNDir * nCr * nChSubtr);

            IPosition offset(5, itsNDir - 1, dr, 0, 0, 0);
            const_cursor<dcomplex> cr_mix_subtr(&(itsFactorsSubtr[ts_subtr](offset)), 3,
                                                stride_mix_subtr_slice);

            // Subtract the source.
            subtract (nBl, nChSubtr, cr_baseline, cr_residual, cr_model_subtr,
                      cr_mix_subtr);
          }
        }
      }
    }


    namespace
    {
      string toString (double value)
      {
        ostringstream os;
        os << setprecision(16) << value;
        return os.str();
      }
    } //# end unnamed namespace

  } //# end namespace DPPP
} //# end namespace LOFAR

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
                              const DemixInfo& info,
                              const DPInfo& dpInfo)
      : itsInfo        (&info),
        itsDPInfo      (&dpInfo),
        itsFilter      (input, info.selBL()),
        itsNrConverged (0),
        itsNrNoDemix   (0),
        itsNrInclude1Target  (0),
        itsNrInclude2Target  (0),
        itsNrIgnoreTarget    (0),
        itsNrDeprojectTarget (0)
    {
      // Add a null step as the last step in the filter.
      DPStep::ShPtr nullStep(new NullStep());
      itsFilter.setNextStep (nullStep);
      itsFilter.updateInfo (dpInfo);
      // The worker will process up to chunkSize input time slots.
      // Size buffers accordingly.
      uint ndir = info.ateamList().size();
      itsFactors.resize      (itsInfo->chunkSize());
      itsFactorsSubtr.resize (itsInfo->ntimeOutSubtr());
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
      const vector<Patch::ConstPtr>& patchList = itsInfo->ateamDemixList();
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
                                           prefix+itsInfo->sourceNames()[i]+'.',
                                           sourceVec);
        itsOrigFirstSteps.push_back (DPStep::ShPtr(step1));
        itsOrigPhaseShifts.push_back (step1);
        DPStep::ShPtr step2 (new Averager(input, prefix, itsInfo->nchanAvg(),
                                          itsInfo->ntimeAvg()));
        step1->setNextStep (step2);
        MultiResultStep* step3 = new MultiResultStep(itsInfo->chunkSize());
        step2->setNextStep (DPStep::ShPtr(step3));
        // There is a single demix factor step which needs to get all results.
        itsAvgResults.push_back (step3);
      }

      // Now create the step to average the data themselves.
      DPStep::ShPtr targetAvg(new Averager(input, prefix,
                                           itsInfo->nchanAvg(),
                                           itsInfo->ntimeAvg()));
      itsOrigFirstSteps.push_back (targetAvg);
      MultiResultStep* targetAvgRes = new MultiResultStep(itsInfo->chunkSize());
      targetAvg->setNextStep (DPStep::ShPtr(targetAvgRes));
      itsAvgResults.push_back (targetAvgRes);

      // Create the data average step for the subtract.
      // The entire average result is needed for the next NDPPP step.
      // Only the selected baselines need to be subtracted, so add a
      // filter step as the last one.
      itsAvgStepSubtr = DPStep::ShPtr(new Averager(input, prefix,
                                                   itsInfo->nchanAvgSubtr(),
                                                   itsInfo->ntimeAvgSubtr()));
      itsAvgResultFull  = new MultiResultStep(itsInfo->ntimeOutSubtr());
      itsFilterSubtr    = new Filter(input, itsInfo->selBL());
      itsAvgResultSubtr = new MultiResultStep(itsInfo->ntimeOutSubtr());
      itsAvgStepSubtr->setNextStep (DPStep::ShPtr(itsAvgResultFull));
      itsAvgResultFull->setNextStep (DPStep::ShPtr(itsFilterSubtr));
      itsFilterSubtr->setNextStep (DPStep::ShPtr(itsAvgResultSubtr));

      // Size the various work buffers.
      itsAvgUVW.resize (3, itsInfo->nbl());
      itsStationUVW.resize (3, itsInfo->nstation(), itsInfo->ntimeOutSubtr());
      itsIndices.resize (ndir);
      itsStationsToUse.resize (ndir);
      itsPredictVis.resize (itsInfo->ncorr(), itsInfo->nchanOut(),
                            itsInfo->nbl());
      itsAteamAmpl.resize (ndir);
      for (uint i=0; i<ndir; ++i) {
        itsAteamAmpl[i].resize (itsInfo->nchanOut(), itsInfo->nbl(),
                                itsInfo->ntimeOut());
      }
      itsTargetAmpl.resize (itsInfo->nchanOut(), itsInfo->nbl(),
                            itsInfo->ntimeOut());
    }

    void DemixWorker::process (const DPBuffer* bufin, uint nbufin,
                               DPBuffer* bufout)
    {
      itsTimer.start();
      // Average and split the baseline UVW coordinates per station.
      // Do this at the demix time resolution.
      // The buffer has not been filtered yet, so the UVWs have to be filtered.
      uint ntime = avgSplitUVW (bufin, nbufin, itsInfo->ntimeAvg(),
                                itsFilter.getIndicesBL());
      double timeStep = bufin[0].getExposure();
      double time = bufin[0].getTime() + (itsInfo->ntimeAvg()-1) * 0.5*timeStep;
      // First do the predict of the coarse A-team sources at demix resolution.
      // It fills the indices of the sources to demix.
      // It also fills in the baselines that do not need to used.
      predictAteam (itsInfo->ateamList(), ntime, time, timeStep);
      // If no sources to demix, simply average the input buffers.
      if (itsIndices.empty()) {
        average (bufin, nbufin, bufout);
        return;
      }
      // The target has to be predicted as well (also at demix resolution).
      predictTarget (itsInfo->targetList(), ntime, time, timeStep);
      // Determine what needs to be done. It fills in the steps to perform
      // for the sources to demix.
      setupDemix();
      // Loop over the buffers and process them.
      itsNTimeOut = 0;
      itsNTimeOutSubtr = 0;
      for (uint i=0; i<nbufin; ++i) {
        // Do the filter step first.
        itsFilter.process (bufin[i]);
        const DPBuffer& selBuf = itsFilter.getBuffer();
        // Do the next steps (phaseshift and average) on the filter output.
        itsTimerPhaseShift.start();
        for (uint j=0; j<itsIndices.size(); ++j) {
          itsFirstSteps[itsIndices[j]]->process (selBuf);
        }
        // Do the average and filter step for the output for all data.
        itsAvgStepSubtr->process (bufin[i]);
        itsTimerPhaseShift.stop();

        // For each itsNTimeAvg times, calculate the phase rotation per direction
        // for the selected data.
        itsTimerDemix.start();
        addFactors (selBuf, itsFactorBuf);
        if (i % itsInfo->ntimeAvg() == 0) {
          makeFactors (itsFactorBuf, itsFactors[itsNTimeOut],
                       itsAvgResults[0]->get()[itsNTimeOut].getWeights(),
                       itsInfo->nchanOut(),
                       itsInfo->nchanAvg());
          // Deproject sources without a model.
          deproject (itsFactors[itsNTimeOut], itsAvgResults, itsNTimeOut);
          itsFactorBuf = Complex();       // Clear summation buffer
          itsNTimeOut++;
        }
        // Subtract is done with different averaging parameters, so calculate
        // the factors for it (again for selected data only).
        addFactors (selBuf, itsFactorBufSubtr);
        if (i % itsInfo->ntimeAvgSubtr() == 0) {
          makeFactors (itsFactorBufSubtr, itsFactorsSubtr[itsNTimeOutSubtr],
                       itsAvgResultSubtr->get()[itsNTimeOutSubtr].getWeights(),
                       itsInfo->nchanOutSubtr(),
                       itsInfo->nchanAvgSubtr());
          itsFactorBufSubtr = Complex();  // Clear summation buffer
          itsNTimeOutSubtr++;
        }
        itsTimerDemix.stop();
      }

      // Estimate gains and subtract source contributions.
      handleDemix (bufout);
      itsTimer.stop();
    }

    uint DemixWorker::avgSplitUVW (const DPBuffer* bufin, uint nbufin,
                                   uint ntimeAvg, const vector<uint>& selbl)
    {
      ASSERT (selbl.size() == itsAvgUVW.shape()[1]);
      // First average the UVWs to the predict time window.
      uint ntime = (nbufin + ntimeAvg - 1) / ntimeAvg;
      const DPBuffer* buf = bufin;
      uint nleft = nbufin;
      // Loop over nr of output time slots.
      MatrixIterator<double> uvwIter(itsStationUVW);
      for (uint i=0; i<ntime; ++i) {
        // Sum the times for this output time slot.
        itsAvgUVW = 0.;
        uint ntodo = std::min(ntimeAvg, nleft);
        for (uint j=0; j<ntodo; ++j) {
          double* sumPtr = itsAvgUVW.data();
          for (uint k=0; k<selbl.size(); ++k) {
            const double* uvwPtr = buf->getUVW().data() + 3*selbl[k];
            for (int k1=0; k1<3; ++k1) {
              *sumPtr++ = uvwPtr[k1];
            }
          }
          buf++;
        }
        // Average the UVWs.
        itsAvgUVW /= double(ntodo);
        nleft -= ntodo;
        // Split the baseline UVW coordinates per station.
        nsplitUVW (itsInfo->uvwSplitIndex(), itsInfo->baselines(),
                   itsAvgUVW, uvwIter.matrix());
        uvwIter.next();
      }
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
          itsPredictVis = 0;
          simulate (itsInfo->phaseRef(),
                    patchList[i],
                    itsInfo->nstation(),
                    itsInfo->nbl(),
                    itsInfo->freqDemix().size(),
                    const_cursor<Baseline>(&(itsInfo->baselines()[0])),
                    casa_const_cursor<double>(itsInfo->freqDemix()),
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
        *amplp += 0.5*(a + abs(*iter));
      }
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
          itsPredictVis = 0;
          simulate (itsInfo->phaseRef(),
                    patchList[i],
                    itsInfo->nstation(),
                    itsInfo->nbl(),
                    itsInfo->freqDemix().size(),
                    const_cursor<Baseline>(&(itsInfo->baselines()[0])),
                    casa_const_cursor<double>(itsInfo->freqDemix()),
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
      vector<uint> antCount (itsInfo->nstation());
      for (uint i=0; i<patchList.size(); ++i) {
        // Count the stations of baselines with sufficient amplitude.
        std::fill (antCount.begin(), antCount.end(), 0);
        MatrixIterator<float> miter(itsAteamAmpl[i], 0, 2);
        for (uint j=0; j<itsInfo->nbl(); ++j, miter.next()) {
          if (max(miter.matrix()) > itsInfo->ateamAmplThreshold()) {
            antCount[itsDPInfo->getAnt1()[j]]++;
            antCount[itsDPInfo->getAnt2()[j]]++;
          }
        }
        // Determine which stations have sufficient occurrence.
        itsStationsToUse[i].resize (0);
        for (uint j=0; j<antCount.size(); ++j) {
          if (antCount[j] >= itsInfo->minNStation()) {
            itsStationsToUse[i].push_back (j);
          }
        }
        // Use this A-team source if some stations have matched.
        if (! itsStationsToUse[i].empty()) {
          itsIndices.push_back (i);
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
      ASSERT (itsAvgResultFull->get().size() == itsInfo->ntimeOutSubtr());
      for (uint i=0; i<itsAvgResultFull->get().size(); ++i) {
        bufout[i] = itsAvgResultFull->get()[i];
      }
    }

    void DemixWorker::setupDemix()
    {
      // Decide if the target has to be included, ignored, or deprojected
      // which depends on the ratio target/Ateam of the total ampl.
      // Fill in the steps to be executed by removing the too weak A-sources.
      uint nsrc = itsIndices.size();
      itsPhaseShifts.resize (nsrc);
      itsFirstSteps.resize (nsrc+1);
      itsFirstSteps[nsrc] = itsOrigFirstSteps[itsOrigFirstSteps.size() - 1];
      float minSumAmpl = 1e30;
      float maxSumAmpl = 0;
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
      bool itsIncludeTarget = false;
      bool itsIgnoreTarget = false;
      if (targetSumAmpl / maxSumAmpl > itsInfo->ratio1()  ||
          targetSumAmpl > itsInfo->targetAmplThreshold()) {
        itsIncludeTarget = true;
        itsNrInclude1Target++;
      } else if (! itsInfo->isAteamNearby()) {
        itsNrDeprojectTarget++;
      } else if (targetSumAmpl / minSumAmpl > itsInfo->ratio2()) {
        itsIncludeTarget = true;
        itsNrInclude2Target++;
      } else {
        itsIgnoreTarget = true;
        itsNrIgnoreTarget++;
      }
      ///      if (includeTarget) {
      ///        itsFirstSteps.push_back (target);
      ///      }
      itsNModel = nsrc;
      itsNDir   = nsrc+1;
    }

    void DemixWorker::applyBeam (double time, const Position& dir)
    {
    }

    void DemixWorker::handleDemix (DPBuffer* bufout)
    {
      itsTimerSolve.start();
      demix (bufout);
      itsTimerSolve.stop();
      // If selection was done, merge the subtract results back into the
      // buffer.
      if (itsInfo->selBL().hasSelection()) {
	mergeSubtractResult();
      }
      // Clear the input buffers.
      for (size_t i=0; i<itsAvgResults.size(); ++i) {
        itsAvgResults[i]->clear();
      }
      // Store the output buffers.
      for (uint i=0; i<itsInfo->ntimeOutSubtr(); ++i) {
        if (itsInfo->selBL().hasSelection()) {
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
      uint ndir = itsIndices.size();
      if (ndir == 0) return;
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
      for (uint i1=0; i1<ndir-1; ++i1) {
        for (uint i0=i1+1; i0<ndir; ++i0) {
          if (i0 == ndir-1) {
            // The last direction is the target direction, so no need to
            // combine the factors. Take conj to get shift source to target.
            for (int i=0; i<nbl; ++i) {
              const bool*   flagPtr   = newBuf.getFlags().data() + i*ncc;
              const float*  weightPtr = newBuf.getWeights().data() + i*ncc;
              DComplex* factorPtr     = factorBuf.data() + (dirnr*nbl + i)*ncc;
              const DComplex* phasor1 = itsPhaseShifts[i1]->getPhasors().data()
                                        + i*nchan;
              for (int j=0; j<nchan; ++j) {
                DComplex factor = conj(*phasor1++);
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
      uint ndir = itsIndices.size();
      if (ndir <= 1) return;
      ASSERT (! weightSums.empty());
      bufOut.resize (IPosition(5, ndir, ndir,
                               itsInfo->ncorr(), nChanOut, itsInfo->nbl()));
      bufOut = DComplex(1,0);
      int ncc = itsInfo->ncorr()*nChanOut;
      int nccdd = ncc*ndir*ndir;
      int nccin = itsInfo->ncorr()*itsInfo->nchanIn();
      // Fill the factors for each combination of different directions.
      uint dirnr = 0;
      for (uint d0=0; d0<ndir; ++d0) {
        for (uint d1=d0+1; d1<ndir; ++d1) {
          // Average factors by summing channels.
          // Note that summing in time is done in addFactors.
          // The sum per output channel is divided by the summed weight.
          // Note there is a summed weight per baseline,outchan,corr.
          for (int k=0; k<int(itsInfo->nbl()); ++k) {
            const DComplex* phin = bufIn.data() + (dirnr*itsInfo->nbl() + k)*nccin;
            DComplex* ph1 = bufOut.data() + k*nccdd + (d0*ndir + d1);
            DComplex* ph2 = bufOut.data() + k*nccdd + (d1*ndir + d0);
            const float* weightPtr = weightSums.data() + k*ncc;
            for (uint c0=0; c0<nChanOut; ++c0) {
              // Sum the factors for the input channels to average.
              DComplex sum[4];
              // In theory the last output channel could consist of fewer
              // input channels, so take care of that.
              uint nch = std::min(nChanAvg, itsInfo->nchanIn()-c0*nChanAvg);
              for (uint c1=0; c1<nch; ++c1) {
                for (uint j=0; j<itsInfo->ncorr(); ++j) {
                  sum[j] += *phin++;
                }
              }
              for (uint j=0; j<itsInfo->ncorr(); ++j) {
                *ph1 = sum[j] / double(*weightPtr++);
                *ph2 = conj(*ph1);
                ph1 += ndir*ndir;
                ph2 += ndir*ndir;
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

    /*
    namespace {
      struct ThreadPrivateStorage
      {
        vector<double>    unknowns;
        vector<double>    uvw;
        vector<dcomplex>  model;
        vector<dcomplex>  model_subtr;
        size_t            count_converged;
      };

      void initThreadPrivateStorage(ThreadPrivateStorage &storage,
        size_t nDirection, size_t nStation, size_t nBaseline, size_t nChannel,
        size_t nChannelSubtr)
      {
        storage.unknowns.resize(nDirection * nStation * 8);
        storage.uvw.resize(nStation * 3);
        storage.model.resize(nDirection * nBaseline * nChannel * 4);
        storage.model_subtr.resize(nBaseline * nChannelSubtr * 4);
        storage.count_converged = 0;
      }
    } //# end unnamed namespace
    */

    void DemixWorker::demix (DPBuffer* bufout)
    {
      /*
      const size_t nTime = itsAvgResults[0]->get().size();
      const size_t nTimeSubtr = itsAvgResultSubtr->get().size();
      const size_t multiplier = itsInfo->ntimeAvg() / itsInfo().ntimeAvgSubtr();
      const size_t nDr = itsNModel;
      const size_t nDrSubtr = itsSubtrSources.size();
      const size_t nSt = itsInfo->nstation();
      const size_t nBl = itsInfo->baselines().size();
      const size_t nCh = itsInfo->freqDemix().size();
      const size_t nChSubtr = itsFreqSubtr.size();
      const size_t nCr = 4;
      const size_t nSamples = nBl * nCh * nCr;

      ThreadPrivateStorage threadStorage;
      initThreadPrivateStorage(threadStorage, nDr, nSt, nBl, nCh, nChSubtr);

      const_cursor<double> cr_freq = casa_const_cursor(itsFreqDemix);
      const_cursor<double> cr_freqSubtr = casa_const_cursor(itsFreqSubtr);
      const_cursor<Baseline> cr_baseline(&(itsBaselines[0]));

      for(size_t ts = 0; ts < nTime; ++ts)
      {
        const size_t thread = OpenMP::threadNum();
        ThreadPrivateStorage &storage = threadStorage[thread];

        // If solution propagation is disabled, re-initialize the thread-private
        // vector of unknowns.
        if(!itsPropagateSolutions)
        {
          copy(itsPrevSolution.begin(), itsPrevSolution.end(),
            storage.unknowns.begin());
        }

        // Simulate.
        //
        // Model visibilities for each direction of interest will be computed
        // and stored.
        size_t stride_uvw[2] = {1, 3};
        cursor<double> cr_uvw_split(&(storage.uvw[0]), 2, stride_uvw);

        size_t stride_model[3] = {1, nCr, nCr * nCh};
        fill(storage.model.begin(), storage.model.end(), dcomplex());
        for(size_t dr = 0; dr < nDr; ++dr)
        {
          const_cursor<double> cr_uvw =
            casa_const_cursor(itsAvgResults[dr]->get()[ts].getUVW());
          splitUVW(nSt, nBl, cr_baseline, cr_uvw, cr_uvw_split);

          cursor<dcomplex> cr_model(&(storage.model[dr * nSamples]), 3,
            stride_model);
          simulate(itsPatchList[dr]->position(), itsPatchList[dr], nSt,
            nBl, nCh, cr_baseline, cr_freq, cr_uvw_split, cr_model);
        }

        // Estimate Jones matrices.
        //
        // A Jones matrix will be estimated for each pair of station and
        // direction.
        //
        // A single (overdetermined) non-linear set of equations for all
        // stations and directions is solved iteratively. The influence of
        // each direction on each other direction is given by the mixing
        // matrix.
        const_cursor<bool> cr_flag =
          casa_const_cursor(itsAvgResults[0]->get()[ts].getFlags());
        const_cursor<float> cr_weight =
          casa_const_cursor(itsAvgResults[0]->get()[ts].getWeights());
        const_cursor<dcomplex> cr_mix = casa_const_cursor(itsFactors[ts]);

        vector<const_cursor<fcomplex> > cr_data(nDr);
        vector<const_cursor<dcomplex> > cr_model(nDr);
        for(size_t dr = 0; dr < nDr; ++dr)
        {
          cr_data[dr] =
            casa_const_cursor(itsAvgResults[dr]->get()[ts].getData());
          cr_model[dr] =
            const_cursor<dcomplex>(&(storage.model[dr * nSamples]), 3,
            stride_model);
        }

        bool converged = estimate(nDr, nSt, nBl, nCh, cr_baseline, cr_data,
          cr_model, cr_flag, cr_weight, cr_mix, &(storage.unknowns[0]));
        if(converged)
        {
          ++storage.count_converged;
        }

        // Compute the residual.
        //
        // All the so-called "subtract sources" are subtracted from the
        // observed data. The previously estimated Jones matrices, as well as
        // the appropriate mixing weight are applied before subtraction.
        //
        // Note that the resolution of the residual can differ from the
        // resolution at which the Jones matrices were estimated.
        for(size_t ts_subtr = multiplier * ts, ts_subtr_end = min(ts_subtr
          + multiplier, nTimeSubtr); ts_subtr != ts_subtr_end; ++ts_subtr)
        {
          for(size_t dr = 0; dr < nDrSubtr; ++dr)
          {
            // Re-use simulation used for estimating Jones matrices if possible.
            cursor<dcomplex> cr_model_subtr(&(storage.model[dr * nSamples]),
              3, stride_model);

            // Re-simulate if required.
            if(multiplier != 1 || nCh != nChSubtr)
            {
              const_cursor<double> cr_uvw =
                casa_const_cursor(itsAvgResultSubtr->get()[ts_subtr].getUVW());
              splitUVW(nSt, nBl, cr_baseline, cr_uvw, cr_uvw_split);

              // Rotate the UVW coordinates for the target direction to the
              // direction of source to subtract. This is required because at
              // the resolution of the residual the UVW coordinates for
              // directions other than the target are unavailable (unless the
              // resolution of the residual is equal to the resolution at which
              // the Jones matrices were estimated, of course).
              rotateUVW(itsPhaseRef, itsPatchList[dr]->position(), nSt,
                cr_uvw_split);

              // Zero the visibility buffer.
              fill(storage.model_subtr.begin(), storage.model_subtr.end(),
                dcomplex());

              // Simulate visibilities at the resolution of the residual.
              size_t stride_model_subtr[3] = {1, nCr, nCr * nChSubtr};
              cr_model_subtr = cursor<dcomplex>(&(storage.model_subtr[0]), 3,
                stride_model_subtr);
              simulate(itsPatchList[dr]->position(), itsPatchList[dr], nSt, nBl,
                nChSubtr, cr_baseline, cr_freqSubtr, cr_uvw_split,
                cr_model_subtr);
            }

            // Apply Jones matrices.
            size_t stride_unknowns[2] = {1, 8};
            const_cursor<double> cr_unknowns(&(storage.unknowns[dr * nSt * 8]),
              2, stride_unknowns);
            apply(nBl, nChSubtr, cr_baseline, cr_unknowns, cr_model_subtr);

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
            const_cursor<dcomplex> cr_mix_subtr =
              const_cursor<dcomplex>(&(itsFactorsSubtr[ts_subtr](offset)), 3,
              stride_mix_subtr_slice);

            // Subtract the source.
            subtract(nBl, nChSubtr, cr_baseline, cr_residual, cr_model_subtr,
              cr_mix_subtr);
          }
        }

        // Copy solutions to global solution array.
        copy(storage.unknowns.begin(), storage.unknowns.end(),
          &(itsUnknowns[(itsTimeIndex + ts) * nDr * nSt * 8]));
      }

      itsNConverged += threadStorage.count_converged;
      */
    }

    void DemixWorker::dumpSolutions (BBS::ParmDB& parmDB)
    {
      /*
      // Construct solution grid.
      const Vector<double>& freq      = itsInfo->chanFreqs();
      const Vector<double>& freqWidth = itsInfo->chanWidths();
      BBS::Axis::ShPtr freqAxis(new BBS::RegularAxis(freq[0] - freqWidth[0]
        * 0.5, freqWidth[0], 1));
      BBS::Axis::ShPtr timeAxis(new BBS::RegularAxis(itsInfo->startTime()
        - itsInfo->timeInterval() * 0.5, itsTimeIntervalAvg, itsNTimeDemix));
      BBS::Grid solGrid(freqAxis, timeAxis);

      // Create and initialize ParmDB.
      BBS::ParmSet parmSet;
      BBS::ParmCache parmCache(parmSet, solGrid.getBoundingBox());

      // Store the (freq, time) resolution of the solutions.
      vector<double> resolution(2);
      resolution[0] = freqWidth[0];
      resolution[1] = itsInfo->timeIntervalAvg();
      parmDB.setDefaultSteps(resolution);

      // Map station indices in the solution array to the corresponding antenna
      // names. This is required because solutions are only produced for
      // stations that participate in one or more baselines. Due to the baseline
      // selection or missing baselines, solutions may be available for less
      // than the total number of station available in the observation.
      const DPInfo &info = itsFilter.getInfo();
      const vector<int> &antennaUsed = info->antennaUsed();
      const Vector<String> &antennaNames = info->antennaNames();

      vector<BBS::Parm> parms;
      for(size_t dr = 0; dr < itsNModel; ++dr) {
        for(size_t st = 0; st < itsInfo->nstation(); ++st) {
          string name(antennaNames[antennaUsed[st]]);
          string suffix(name + ":" + itsInfo->sourceNames()[dr]);

          parms.push_back(BBS::Parm(parmCache, parmSet.addParm(parmDB,
            "DirectionalGain:0:0:Real:" + suffix)));
          parms.push_back(BBS::Parm(parmCache, parmSet.addParm(parmDB,
            "DirectionalGain:0:0:Imag:" + suffix)));

          parms.push_back(BBS::Parm(parmCache, parmSet.addParm(parmDB,
            "DirectionalGain:0:1:Real:" + suffix)));
          parms.push_back(BBS::Parm(parmCache, parmSet.addParm(parmDB,
            "DirectionalGain:0:1:Imag:" + suffix)));

          parms.push_back(BBS::Parm(parmCache, parmSet.addParm(parmDB,
            "DirectionalGain:1:0:Real:" + suffix)));
          parms.push_back(BBS::Parm(parmCache, parmSet.addParm(parmDB,
            "DirectionalGain:1:0:Imag:" + suffix)));

          parms.push_back(BBS::Parm(parmCache, parmSet.addParm(parmDB,
            "DirectionalGain:1:1:Real:" + suffix)));
          parms.push_back(BBS::Parm(parmCache, parmSet.addParm(parmDB,
            "DirectionalGain:1:1:Imag:" + suffix)));
        }
      }

      // Cache parameter values.
      parmCache.cacheValues();

      // Assign solution grid to parameters.
      for(size_t i = 0; i < parms.size(); ++i) {
        parms[i].setSolveGrid(solGrid);
      }

      // Write solutions.
      for(size_t ts = 0; ts < itsInfo->NTimeDemix; ++ts) {
        double *unknowns = &(itsUnknowns[ts * itsNModel * itsInfo->nstation()*8]);
        for(size_t i = 0; i < parms.size(); ++i) {
          parms[i].setCoeff(BBS::Location(0, ts), unknowns + i, 1);
        }
      }

      // Flush solutions to disk.
      parmCache.flush();
      */
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

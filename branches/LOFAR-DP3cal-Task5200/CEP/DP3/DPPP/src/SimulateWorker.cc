//# SimulateWorker.cc: Simulate helper class processing some time / bl/ channel
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
//# @author Tammo Jan Dijkema

#include <lofar_config.h>
#include <DPPP/SimulateWorker.h>
#include <DPPP/CursorUtilCasa.h>
#include <DPPP/DPBuffer.h>
#include <DPPP/DPInfo.h>
#include <DPPP/Simulate.h>
#include <DPPP/Baseline.h>

#include <Common/OpenMP.h>

#include <casa/Quanta/MVEpoch.h>
#include <casa/Arrays/Matrix.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MEpoch.h>

#include <map>
#include <algorithm>
#include <DPPP/Cursor.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MeasRef.h>

using namespace casa;

namespace LOFAR {
  namespace DPPP {

    using LOFAR::operator<<;

    SimulateWorker::SimulateWorker(DPInput* input, const DPInfo& info,
                                   vector<Patch::ConstPtr>* patchList,
                                   uint firstBl, uint lastBl, uint firstCh,
                                   uint lastCh)
        : itsPatchList(patchList)
    {
      // Set directions
      itsRefFreq = info.refFreq();
      itsDelayCenter = info.delayCenterCopy();
      itsTileBeamDir = info.tileBeamDirCopy();
      // Create the Measure ITRF conversion info given the array position.
      // The time and direction are filled in later.
      itsMeasFrame.set(info.arrayPosCopy());
      itsMeasFrame.set(MEpoch(MVEpoch(info.startTime() / 86400), MEpoch::UTC));
      itsMeasConverter.set(MDirection::J2000,
                           MDirection::Ref(MDirection::ITRF, itsMeasFrame));
      // Do a dummy conversion, because Measure initialization does not
      // seem to be thread-safe.
      dir2Itrf(itsDelayCenter);

      MDirection dirJ2000(MDirection::Convert(info.phaseCenterCopy(),
                                              MDirection::J2000)());
      Quantum<Vector<Double> > angles = dirJ2000.getAngle();
      itsPhaseRef = Position(angles.getBaseValue()[0],
                             angles.getBaseValue()[1]);

      // Fill baselines and antennas
      uint nBl=lastBl-firstBl;
      itsBaselines=vector<Baseline>(nBl);
      for (uint i=firstBl; i<lastBl; ++i) {
        itsBaselines[i]=Baseline(info.getAnt1()[i],info.getAnt2()[i]);
      }

      // Determine the stations actually used in the baselines for this thread
      vector<int> antennaUsed;
      if (nBl==info.nbaselines()) { // all baselines used
        antennaUsed=info.antennaUsed();
      } else { // fill antennaUsed
        antennaUsed.reserve(info.antennaUsed().size());

        map<int,bool> antennaUsedMap;
        for (vector<Baseline>::iterator bl_it=itsBaselines.begin(),
             bl_end=itsBaselines.end(); bl_it!=bl_end; ++bl_it) {
          antennaUsedMap[(*bl_it).first]=true;
          antennaUsedMap[(*bl_it).second]=true;
        }
        for (map<int,bool>::iterator map_it = antennaUsedMap.begin(),
             map_end = antennaUsedMap.end(); map_it != map_end; ++map_it ) {
          antennaUsed.push_back( map_it->first );
        }
        std::sort(antennaUsed.begin(),antennaUsed.end());
      }
      itsNSt=antennaUsed.size();

      // Read the antenna beam info from the MS.
      // Only take the stations actually used in this thread
      casa::Vector<casa::String> antennaUsedNames(info.antennaUsed().size());
      casa::Vector<int> antsUsed = info.antennaUsed();
      for (int ant=0, nAnts=info.antennaUsed().size(); ant<nAnts; ++ant) {
        antennaUsedNames[ant]=info.antennaNames()[antennaUsed[ant]];
      }
      input->fillBeamInfo (itsAntBeamInfo, antennaUsedNames);


      // Fill channels
      itsChanFreqs.resize(lastCh-firstCh);

      for (uint i=firstCh; i<lastCh; ++i) {
        itsChanFreqs[i-firstCh]=info.chanFreqs()[i];
      }

      // Fill buffers
      const size_t nDr = itsPatchList->size();
      const size_t nCh = itsChanFreqs.size();
      itsModelVis.resize(nBl * nCh * 4);
      itsModelVisPatch.resize(nBl * nCh * 4);
    }


    void SimulateWorker::process (const DPBuffer& bufin)
    {
      itsTimer.start();

      DPBuffer buf(bufin);
      // Determine the various sizes.
      const size_t nDr = itsPatchList->size();
      const size_t nBl = itsBaselines.size();
      const size_t nCh = itsChanFreqs.size();
      const size_t nCr = 4;
      // Define various cursors to iterate through arrays.
      const_cursor<double> cr_freq = casa_const_cursor(itsChanFreqs);
      const_cursor<Baseline> cr_baseline(&(itsBaselines[0]));

      // Simulate.
      //
      // Model visibilities for each direction of interest will be computed
      // and stored.

      double time = buf.getTime();

      size_t stride_uvw[2] = {1, 3};
      cursor<double> cr_uvw_split(&(itsUVW[0]), 2, stride_uvw);

      Complex* data=buf.getData().data();

      size_t stride_model[3] = {1, nCr, nCr * nCh};
      fill(itsModelVis.begin(), itsModelVis.end(), dcomplex());

      const_cursor<double> cr_uvw = casa_const_cursor(buf.getUVW());
      splitUVW(itsNSt, nBl, cr_baseline, cr_uvw, cr_uvw_split);
      cursor<dcomplex> cr_model(&(itsModelVisPatch[0]), 3, stride_model);

      // Convert the directions to ITRF for the given time.
      itsMeasFrame.resetEpoch (MEpoch(MVEpoch(time/86400), MEpoch::UTC));

      for(size_t dr = 0; dr < nDr; ++dr)
      {
        fill(itsModelVisPatch.begin(), itsModelVisPatch.end(), dcomplex());

        simulate(itsPhaseRef, (*itsPatchList)[dr], itsNSt, nBl, nCh,
                 cr_baseline, cr_freq, cr_uvw_split, cr_model);
        applyBeam(time, (*itsPatchList)[dr]->position(), itsApplyBeam,
                  itsChanFreqs, &(itsModelVisPatch[0]));

        for (size_t i = 0; i < itsModelVisPatch.size(); ++i) {
          itsModelVis[i] += itsModelVisPatch[i];
        }
      }

      //copy result of model to data at the end (so intermediate results are
      //in double precision)
      copy(itsModelVis.begin(),itsModelVis.end(),data);

      itsTimer.stop();
    }

    void SimulateWorker::applyBeam (double time, const Position& pos,
                                    bool apply, const Vector<double>& chanFreqs,
                                    dcomplex* data)
    {
      if (! apply) {
        return;
      }
      itsMeasFrame.resetEpoch (MEpoch(MVEpoch(time/86400), MEpoch::UTC));
      StationResponse::vector3r_t refdir  = dir2Itrf(itsDelayCenter);
      StationResponse::vector3r_t tiledir = dir2Itrf(itsTileBeamDir);
      // Convert the source direction to ITRF for the given time.
      MDirection dir (MVDirection(pos[0], pos[1]), MDirection::J2000);
      StationResponse::vector3r_t srcdir = dir2Itrf(dir);
      // Get the beam values for each station.
      uint nchan = chanFreqs.size();
      for (size_t st=0; st<itsNSt; ++st) {
        itsAntBeamInfo[st]->response (nchan, time, chanFreqs.cbegin(),
                                      srcdir, itsRefFreq,
                                      refdir, tiledir,
                                      &(itsBeamValues[nchan*st]));
      }
      // Apply the beam values of both stations to the predicted data.
      dcomplex tmp[4];
      for (size_t bl=0; bl<itsBaselines.size(); ++bl) {
        const StationResponse::matrix22c_t* left =
            &(itsBeamValues[nchan * itsBaselines[bl].first]);
        const StationResponse::matrix22c_t* right =
            &(itsBeamValues[nchan * itsBaselines[bl].second]);
        for (size_t ch=0; ch<nchan; ++ch) {
          dcomplex l[] = {left[ch][0][0], left[ch][0][1],
              left[ch][1][0], left[ch][1][1]};
          // Form transposed conjugate of right.
          dcomplex r[] = {conj(right[ch][0][0]), conj(right[ch][1][0]),
              conj(right[ch][0][1]), conj(right[ch][1][1])};
          // left*data
          tmp[0] = l[0] * data[0] + l[1] * data[2];
          tmp[1] = l[0] * data[1] + l[1] * data[3];
          tmp[2] = l[2] * data[0] + l[3] * data[2];
          tmp[3] = l[2] * data[1] + l[3] * data[3];
          // data*conj(right)
          data[0] = tmp[0] * r[0] + tmp[1] * r[2];
          data[1] = tmp[0] * r[1] + tmp[1] * r[3];
          data[2] = tmp[2] * r[0] + tmp[3] * r[2];
          data[3] = tmp[2] * r[1] + tmp[3] * r[3];
          data += 4;
        }
      }
    }

    StationResponse::vector3r_t SimulateWorker::dir2Itrf (
        const MDirection& dir)
    {
      const MDirection& itrfDir = itsMeasConverter(dir);
      const Vector<Double>& itrf = itrfDir.getValue().getValue();
      StationResponse::vector3r_t vec;
      vec[0] = itrf[0];
      vec[1] = itrf[1];
      vec[2] = itrf[2];
      return vec;
    }

  } //# end namespace DPPP
} //# end namespace LOFAR

//# GainCal.cc: DPPP step class to do a gain calibration
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
//# $Id: GainCal.cc 21598 2012-07-16 08:07:34Z diepen $
//#
//# @author Tammo Jan Dijkema

#include <lofar_config.h>
#include <DPPP/GainCal.h>
#include <DPPP/Simulate.h>
#include <DPPP/CursorUtilCasa.h>
#include <DPPP/DPBuffer.h>
#include <DPPP/DPInfo.h>
#include <DPPP/SourceDBUtil.h>
#include <DPPP/MSReader.h>
#include <ParmDB/SourceDB.h>
#include <Common/ParameterSet.h>
#include <Common/StringUtil.h>
#include <Common/LofarLogger.h>
#include <Common/OpenMP.h>

#include <casa/Arrays/ArrayMath.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>
#include <casa/OS/File.h>

#include <iostream>
#include <iomanip>

using namespace casa;
using namespace LOFAR::BBS;

/// Look at BBSKernel MeasurementExprLOFARUtil.cc and Apply.cc

namespace LOFAR {
  namespace DPPP {

    GainCal::GainCal (DPInput* input,
                        const ParameterSet& parset,
                        const string& prefix)
      : itsInput       (input),
        itsName        (prefix),
        itsSourceDBName (parset.getString (prefix + "sourcedb")),
        itsParmDBName  (parset.getString (prefix + "parmdb")),
        itsApplyBeam   (parset.getBool (prefix + "model.beam")),
        itsCellSizeTime (parset.getInt (prefix + "cellsize.time", 1)),
        itsCellSizeFreq (parset.getInt (prefix + "cellsize.freq", 0)),
        itsBaselines   (),
        itsThreadStorage (),
        itsPatchList   ()
    {
      BBS::SourceDB sourceDB(BBS::ParmDBMeta("", itsSourceDBName), false);

      vector<PatchInfo> patchInfo=sourceDB.getPatchInfo();
      vector<string> patchNames;

      vector<string> sourcePatterns=parset.getStringVector(prefix + "sources",
              vector<string>());
      patchNames=makePatchList(sourceDB, sourcePatterns);

      itsPatchList = makePatches (sourceDB, patchNames, patchNames.size());
    }

    GainCal::~GainCal()
    {}

    void GainCal::updateInfo (const DPInfo& infoIn)
    {
      info() = infoIn;
      info().setNeedVisData();
      info().setNeedWrite();

      uint nBl=info().nbaselines();
      for (uint i=0; i<nBl; ++i) {
        itsBaselines.push_back (Baseline(info().getAnt1()[i],
                                         info().getAnt2()[i]));
      }

      MDirection dirJ2000(MDirection::Convert(infoIn.phaseCenter(),
                                              MDirection::J2000)());
      Quantum<Vector<Double> > angles = dirJ2000.getAngle();
      itsPhaseRef = Position(angles.getBaseValue()[0],
                             angles.getBaseValue()[1]);
/*
      itsParmDB.reset(new BBS::ParmFacade(itsParmDBName));
*/

      const size_t nDr = itsPatchList.size();
      const size_t nSt = info().antennaUsed().size();
      const size_t nCh = info().nchan();
      // initialize storage
      const size_t nThread=OpenMP::maxThreads();
      itsThreadStorage.resize(nThread);
      for(vector<ThreadPrivateStorage>::iterator it = itsThreadStorage.begin(),
        end = itsThreadStorage.end(); it != end; ++it)
      {
        initThreadPrivateStorage(*it, nDr, nSt, nBl, nCh, nCh);
      }

      // Create the Measure ITRF conversion info given the array position.
      // The time and direction are filled in later.
      itsMeasFrame.set (info().arrayPos());
      itsMeasFrame.set (MEpoch(MVEpoch(info().startTime()/86400), MEpoch::UTC));
      itsMeasConverter.set (MDirection::J2000,
                            MDirection::Ref(MDirection::ITRF, itsMeasFrame));
      // Do a dummy conversion, because Measure initialization does not
      // seem to be thread-safe.
      dir2Itrf(info().delayCenter());

      // Read the antenna beam info from the MS.
      // Only take the stations actually used.
      casa::Vector<casa::String> antennaUsedNames(info().antennaUsed().size());
      casa::Vector<int> antsUsed = info().antennaUsed();
      for (int ant=0, nAnts=info().antennaUsed().size(); ant<nAnts; ++ant) {
        antennaUsedNames[ant]=info().antennaNames()[info().antennaUsed()[ant]];
      }
      itsInput->fillBeamInfo (itsAntBeamInfo, antennaUsedNames);
    }

    StationResponse::vector3r_t GainCal::dir2Itrf (const MDirection& dir)
    {
      const MDirection& itrfDir = itsMeasConverter(dir);
      const Vector<Double>& itrf = itrfDir.getValue().getValue();
      StationResponse::vector3r_t vec;
      vec[0] = itrf[0];
      vec[1] = itrf[1];
      vec[2] = itrf[2];
      return vec;
    }

    void GainCal::show (std::ostream& os) const
    {
      os << "GainCal " << itsName << std::endl;
      os << "  sourcedb:       " << itsSourceDBName << endl;
      os << "   number of patches: " << itsPatchList.size() << endl;
      os << "  parmdb:         " << itsParmDBName << endl;
      os << "  apply beam:     " << boolalpha << itsApplyBeam << endl;
    }

    void GainCal::showTimings (std::ostream& os, double duration) const
    {
      os << "  ";
      FlagCounter::showPerc1 (os, itsTimer.getElapsed(), duration);
      os << " GainCal " << itsName << endl;
    }



    bool GainCal::process (const DPBuffer& bufin)
    {
      itsTimer.start();
      DPBuffer buf(bufin);
      buf.getData().unique();
      RefRows refRows(buf.getRowNrs());

      buf.setUVW(itsInput->fetchUVW(buf, refRows, itsTimer));
      buf.setWeights(itsInput->fetchWeights(buf, refRows, itsTimer));
      buf.setFullResFlags(itsInput->fetchFullResFlags(buf, refRows, itsTimer));

      // Determine the various sizes.
      const size_t nDr = itsPatchList.size();
      const size_t nSt = info().antennaUsed().size();
      const size_t nBl = info().nbaselines();
      const size_t nCh = info().nchan();
      const size_t nCr = 4;
      const size_t nSamples = nBl * nCh * nCr;
      // Define various cursors to iterate through arrays.
      const_cursor<double> cr_freq = casa_const_cursor(info().chanFreqs());
      const_cursor<Baseline> cr_baseline(&(itsBaselines[0]));

      const size_t thread = OpenMP::threadNum();

      // Simulate.
      //
      // Model visibilities for each direction of interest will be computed
      // and stored.

      double time = buf.getTime();

      ThreadPrivateStorage &storage = itsThreadStorage[thread];
      size_t stride_uvw[2] = {1, 3};
      cursor<double> cr_uvw_split(&(storage.uvw[0]), 2, stride_uvw);

      Complex* data=buf.getData().data();

      size_t stride_model[3] = {1, nCr, nCr * nCh};
      fill(storage.model.begin(), storage.model.end(), dcomplex());

      const_cursor<double> cr_uvw = casa_const_cursor(buf.getUVW());
      splitUVW(nSt, nBl, cr_baseline, cr_uvw, cr_uvw_split);
      cursor<dcomplex> cr_model(&(storage.model_patch[0]), 3, stride_model);

      StationResponse::vector3r_t refdir = dir2Itrf(info().delayCenter());
      StationResponse::vector3r_t tiledir = dir2Itrf(info().tileBeamDir());
      // Convert the directions to ITRF for the given time.
      itsMeasFrame.resetEpoch (MEpoch(MVEpoch(time/86400), MEpoch::UTC));

      for(size_t dr = 0; dr < nDr; ++dr)
      {
        fill(storage.model_patch.begin(), storage.model_patch.end(), dcomplex());

        simulate(itsPhaseRef, itsPatchList[dr], nSt, nBl, nCh, cr_baseline,
                 cr_freq, cr_uvw_split, cr_model);
        applyBeam(time, itsPatchList[dr]->position(), itsApplyBeam,
                  info().chanFreqs(), &(itsThreadStorage[thread].model_patch[0]),
                  refdir, tiledir, &(itsThreadStorage[thread].beamvalues[0]));

        for (size_t i=0; i<itsThreadStorage[thread].model_patch.size();++i) {
          itsThreadStorage[thread].model[i]+=
              itsThreadStorage[thread].model_patch[i];
        }
      }

      //copy result of model to data
      copy(storage.model.begin(),storage.model.begin()+nSamples,data);

      itsTimer.stop();
      getNextStep()->process(buf);
      return false;
    }

    void GainCal::applyBeam (double time, const Position& pos, bool apply,
                             const Vector<double>& chanFreqs, dcomplex* data,
                             StationResponse::vector3r_t& refdir,
                             StationResponse::vector3r_t& tiledir,
                             StationResponse::matrix22c_t* beamvalues)
    {
      if (! apply) {
        return;
      }

      MDirection dir (MVDirection(pos[0], pos[1]), MDirection::J2000);
      StationResponse::vector3r_t srcdir = dir2Itrf(dir);
      // Get the beam values for each station.
      uint nchan = chanFreqs.size();
      uint nSt   = info().antennaUsed().size();
      for (size_t st=0; st<nSt; ++st) {
        itsAntBeamInfo[st]->response (nchan, time, chanFreqs.cbegin(),
                                      srcdir, info().refFreq(), refdir, tiledir,
                                      &(beamvalues[nchan*st]));
      }
      // Apply the beam values of both stations to the predicted data.
      dcomplex tmp[4];
      for (size_t bl=0; bl<info().nbaselines(); ++bl) {
        const StationResponse::matrix22c_t* left =
          &(beamvalues[nchan * info().getAnt1()[bl]]);
        const StationResponse::matrix22c_t* right =
          &(beamvalues[nchan * info().getAnt2()[bl]]);
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

    void GainCal::finish()
    {

      // Let the next steps finish.
      getNextStep()->finish();
    }

  } //# end namespace
}

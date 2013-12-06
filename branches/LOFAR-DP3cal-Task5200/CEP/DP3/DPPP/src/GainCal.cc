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
      const size_t nSt = info().antennaNames().size();
      const size_t nCh = info().nchan();
      // initialize storage
      const size_t nThread=OpenMP::maxThreads();
      itsThreadStorage.resize(nThread);
      for(vector<ThreadPrivateStorage>::iterator it = itsThreadStorage.begin(),
        end = itsThreadStorage.end(); it != end; ++it)
      {
        initThreadPrivateStorage(*it, nDr, nSt, nBl, nCh, nCh);
      }
    }

    void GainCal::show (std::ostream& os) const
    {
      os << "GainCal " << itsName << std::endl;
      os << "  sourcedb:       " << itsSourceDBName << endl;
      os << "  parmdb:         " << itsParmDBName << endl;
      os << "  number of patches: " << itsPatchList.size() << endl;
      if (!itsPatchList.empty()) {
        os << "  patch[0] position: " << itsPatchList[0]->position()[0] << ","
                                    << itsPatchList[0]->position()[1] << endl;
      }
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
      const size_t nSt = info().antennaNames().size();
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

      ThreadPrivateStorage &storage = itsThreadStorage[thread];
      size_t stride_uvw[2] = {1, 3};
      cursor<double> cr_uvw_split(&(storage.uvw[0]), 2, stride_uvw);

      Complex* data=buf.getData().data();

      size_t stride_model[3] = {1, nCr, nCr * nCh};
      fill(storage.model.begin(), storage.model.end(), dcomplex());
      for(size_t dr = 0; dr < nDr; ++dr)
      {
        const_cursor<double> cr_uvw = casa_const_cursor(buf.getUVW());

        splitUVW(nSt, nBl, cr_baseline, cr_uvw, cr_uvw_split);

        cursor<dcomplex> cr_model(&(storage.model[0]), 3, stride_model);
        simulate(itsPhaseRef, itsPatchList[dr], nSt,
          nBl, nCh, cr_baseline, cr_freq, cr_uvw_split, cr_model);
      }

      //copy result of model to data
      copy(storage.model.begin(),storage.model.begin()+nSamples,data);

      itsTimer.stop();
      getNextStep()->process(buf);
      return false;
    }

    void GainCal::finish()
    {

      // Let the next steps finish.
      getNextStep()->finish();
    }

    void GainCal::handleCal() {

    }

  } //# end namespace
}

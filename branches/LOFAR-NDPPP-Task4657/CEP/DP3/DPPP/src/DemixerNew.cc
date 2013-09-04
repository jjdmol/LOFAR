//# DemixerNew.cc: DPPP step class to subtract A-team sources in adaptive way
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
//# $Id: DemixerNew.cc 24221 2013-03-12 12:24:48Z diepen $
//#
//# @author Ger van Diepen

#include <lofar_config.h>
#include <DPPP/DemixerNew.h>
#include <DPPP/DemixWorker.h>
#include <DPPP/DemixInfo.h>
#include <DPPP/DPBuffer.h>
#include <DPPP/DPInfo.h>
#include <ParmDB/ParmDB.h>
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/OpenMP.h>
#include <Common/StreamUtil.h>
#include <Common/lofar_iostream.h>

using namespace casa;

namespace LOFAR {
  namespace DPPP {

    using LOFAR::operator<<;

    DemixerNew::DemixerNew (DPInput* input,
                            const ParameterSet& parset,
                            const string& prefix)
      : itsInput          (input),
        itsName           (prefix),
        itsDemixInfo      (parset, prefix),
        itsInstrumentName (parset.getString(prefix+"instrumentmodel",
                                            "instrument")),
        itsFilter         (input, itsDemixInfo.selBL()),
        itsNTime          (0)
    {
      ASSERTSTR (! itsInstrumentName.empty(),
                 "An empty name is given for the instrument model");
      /// Create ParmDB at first write!!!
      /// In that way instrumentmodel can be in MS directory.
      itsParmDB = boost::shared_ptr<BBS::ParmDB>
        (new BBS::ParmDB(BBS::ParmDBMeta("casa", itsInstrumentName),
                         true));
      ///      // Add a null step as the last step in the filter.
      ///      DPStep::ShPtr nullStep(new NullStep());
      ///      itsFilter.setNextStep (nullStep);
    }

    void DemixerNew::updateInfo (const DPInfo& infoIn)
    {
      info() = infoIn;
      // Handle possible data selection.
      itsFilter.updateInfo (infoIn);
      const DPInfo& infoSel = itsFilter.getInfo();
      itsDemixInfo.update (infoSel, info());
      // Update the info of this object.
      info().setNeedVisData();
      info().setNeedWrite();
      // Size the buffers.
      itsBufIn.resize (itsDemixInfo.ntimeChunk() * itsDemixInfo.chunkSize());
      itsBufOut.resize(itsDemixInfo.ntimeChunk() * itsDemixInfo.ntimeOutSubtr());
      // Create a worker per thread.
      int nthread = OpenMP::maxThreads();
      itsWorkers.reserve (nthread);
      for (int i=0; i<nthread; ++i) {
        itsWorkers.push_back (DemixWorker (itsInput, itsName, itsDemixInfo,
                                           getInfo()));
      }
    }

    void DemixerNew::show (ostream& os) const
    {
      os << "DemixerNew " << itsName << endl;
      os << "  instrumentmodel:    " << itsInstrumentName << endl;
      itsDemixInfo.show (os);
      if (itsDemixInfo.selBL().hasSelection()) {
        os << "    demixing " << itsFilter.getInfo().nbaselines()
           << " out of " << getInfo().nbaselines() << " baselines   ("
           << itsFilter.getInfo().antennaUsed().size()
           << " out of " << getInfo().antennaUsed().size()
           << " stations)" << std::endl;
      }
    }

    void DemixerNew::showCounts (ostream& os) const
    {
      os << endl << "Statistics for SmartDemixer" << itsName;
      os << endl << "===========================" << endl;
      // Add the counts of all workers.
      uint nsolves        = 0;
      uint nconverged     = 0;
      uint nnodemix       = 0;
      uint nincludeStrong = 0;
      uint nincludeClose  = 0;
      uint nignore        = 0;
      uint ndeproject     = 0;
      Vector<uint> nsources(itsDemixInfo.ateamList().size(), 0);
      for (size_t i=0; i<itsWorkers.size(); ++i) {
        nsolves        += itsWorkers[i].nSolves();
        nconverged     += itsWorkers[i].nConverged();
        nnodemix       += itsWorkers[i].nNoDemix();
        nincludeStrong += itsWorkers[i].nIncludeStrongTarget();
        nincludeClose  += itsWorkers[i].nIncludeCloseTarget();
        nignore        += itsWorkers[i].nIgnoreTarget();
        ndeproject     += itsWorkers[i].nDeprojectTarget();
        nsources       += itsWorkers[i].nsourcesDemixed();
      }
      os << "Nr of time chunks with:" << endl;
      os << setw(8) << nnodemix   << "  no demixing" << endl;
      os << setw(8) << nignore    << "  target ignored" << endl; 
      os << setw(8) << ndeproject << "  target deprojected" << endl; 
      os << setw(8) << nincludeStrong
         << "  target included because strong" << endl; 
      os << setw(8) << nincludeClose
         << "  target included because close" << endl; 
      os << "Nr of time chunks a source is demixed:" << endl;
      for (size_t i=0; i<nsources.size(); ++i) {
        os << setw(8) << nsources[i] << "  "
           << itsDemixInfo.ateamList()[i]->name() << endl;
      }
      os << "Converged solves: " << nconverged << " cells out of "
         << nsolves << endl;
    }

    void DemixerNew::showTimings (std::ostream& os, double duration) const
    {
      double self  = itsTimer.getElapsed();
      double demix = itsTimerDemix.getElapsed();
      double tottime = 0;
      double pretime = 0;
      double psatime = 0;
      double demtime = 0;
      double soltime = 0;
      for (uint i=0; i<itsWorkers.size(); ++i) {
        tottime += itsWorkers[i].getTotalTime();
        pretime += itsWorkers[i].getPredictTime();
        psatime += itsWorkers[i].getPhaseShiftTime();
        demtime += itsWorkers[i].getDemixTime();
        soltime += itsWorkers[i].getSolveTime();
      }
      os << "  ";
      FlagCounter::showPerc1 (os, self, duration);
      os << " DemixerNew " << itsName << endl;
      os << "          ";
      FlagCounter::showPerc1 (os, demix, self);
      os << " of it spent in demixing the data of which" << endl;
      os << "                ";
      FlagCounter::showPerc1 (os, pretime, tottime);
      os << " in predicting coarse model" << endl;
      os << "                ";
      FlagCounter::showPerc1 (os, psatime, tottime);
      os << " in phase shifting/averaging data" << endl;
      os << "                ";
      FlagCounter::showPerc1 (os, demtime, tottime);
      os << " in calculating decorrelation factors" << endl;
      os << "                ";
      FlagCounter::showPerc1 (os, soltime, tottime);
      os << " in estimating gains and computing residuals" << endl;
      os << "          ";
      FlagCounter::showPerc1 (os, itsTimerDump.getElapsed(), self);
      os << " of it spent in writing gain solutions to disk" << endl;
    }

    bool DemixerNew::process (const DPBuffer& buf)
    {
      itsTimer.start();
      // Collect sufficient data buffers.
      // Make sure all required data arrays are filled in.
      DPBuffer& newBuf = itsBufIn[itsNTime];
      newBuf = buf;
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
      // Process the data if entire buffer is filled.
      if (++itsNTime >= itsBufIn.size()) {
        processData();
        itsNTime = 0;
      }
      itsTimer.stop();
      return true;
    }
      
    void DemixerNew::processData()
    {
      itsTimerDemix.start();
      // Last batch might contain fewer time slots.
      uint timeWindowIn  = itsDemixInfo.chunkSize();
      uint timeWindowOut = itsDemixInfo.ntimeOutSubtr();
      int lastChunk = (itsNTime - 1) / timeWindowIn;
      int lastNTime = itsNTime - lastChunk*timeWindowIn;
      ///#pragma omp parallel for schedule dynamic
      for (int i=0; i<=lastChunk; ++i) {
        if (i == lastChunk) {
          cout<<"chunk="<<i*timeWindowIn<<' '<<lastNTime<<endl;
          processChunk (&(itsBufIn[i*timeWindowIn]), lastNTime,
                        &(itsBufOut[i*timeWindowOut]));
        } else {
          cout<<"chunk="<<i*timeWindowIn<<' '<<timeWindowIn<<endl;
          processChunk (&(itsBufIn[i*timeWindowIn]), timeWindowIn,
                        &(itsBufOut[i*timeWindowOut]));
        }
      }
      itsTimerDemix.stop();
      // Write the solutions into the instrument ParmDB.
      // Let the next steps process the results.
      // This can be done in parallel.
      ///#pragma omp parallel for num_thread(2)
      for (int i=0; i<2; ++i) {
        if (i == 0) {
          writeSolutions();
        } else {
          itsTimer.stop();
          itsTimerNext.start();
          uint ntimeOut = (lastChunk+1) * itsDemixInfo.ntimeAvgSubtr();
          for (uint i=0; i<ntimeOut; ++i) {
            getNextStep()->process (itsBufOut[i]);
          }
          itsTimerNext.stop();
          itsTimer.start();
        }
      }
    }

    void DemixerNew::processChunk (const DPBuffer* bufIn, int nbufin,
                                   DPBuffer* bufOut)
    {
      itsWorkers[OpenMP::threadNum()].process (bufIn, nbufin, bufOut);
    }

    void DemixerNew::finish()
    {
      itsTimer.start();
      // Process remaining entries.
      if (itsNTime > 0) {
        processData();
      }
      itsTimer.stop();
      // Let the next steps finish.
      getNextStep()->finish();
    }

    void DemixerNew::writeSolutions()
    {
      itsTimerDump.start();
      for (uint i=0; i<itsWorkers.size(); ++i) {
        itsWorkers[i].dumpSolutions (*itsParmDB);
      }
      itsTimerDump.stop();
    }

} //# end namespace DPPP
} //# end namespace LOFAR

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
#include <ParmDB/ParmValue.h>
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/OpenMP.h>
#include <Common/StreamUtil.h>
#include <Common/lofar_iostream.h>

#include <casa/Arrays/ArrayPartMath.h>

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
        itsNTime          (0),
        itsNTimeOut       (0)
    {
      ASSERTSTR (! itsInstrumentName.empty(),
                 "An empty name is given for the instrument model");
    }

    void DemixerNew::updateInfo (const DPInfo& infoIn)
    {
      info() = infoIn;
      // Handle possible data selection.
      itsFilter.updateInfo (infoIn);
      // Update itsDemixInfo and info().
      itsDemixInfo.update (itsFilter.getInfo(), info());
      // Update the info of this object.
      info().fillAntennaBeamInfo (itsInput);
      info().setNeedVisData();
      info().setNeedWrite();
      // Size the buffers.
      itsBufIn.resize (itsDemixInfo.ntimeChunk() * itsDemixInfo.chunkSize());
      itsBufOut.resize(itsDemixInfo.ntimeChunk() * itsDemixInfo.ntimeOutSubtr());
      itsSolutions.resize(itsDemixInfo.ntimeChunk() * itsDemixInfo.ntimeOut());
      itsPercSubtr.resize(itsDemixInfo.nbl(), itsDemixInfo.ateamList().size(),
                          getInfo().ntime());
      itsPercSubtr = -100;
      // Create a worker per thread.
      int nthread = OpenMP::maxThreads();
      itsWorkers.reserve (nthread);
      for (int i=0; i<nthread; ++i) {
        itsWorkers.push_back (DemixWorker (itsInput, itsName, itsDemixInfo,
                                           infoIn));
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
      os << endl << "Statistics for SmartDemixer " << itsName;
      os << endl << "===========================" << endl;
      // Add the statistics of all workers.
      uint nsolves        = 0;
      uint nconverged     = 0;
      uint nnodemix       = 0;
      uint nincludeStrong = 0;
      uint nincludeClose  = 0;
      uint nignore        = 0;
      uint ndeproject     = 0;
      Vector<uint> nsources(itsDemixInfo.ateamList().size(), 0);
      Vector<uint> nstation(itsDemixInfo.nstation(), 0);
      Matrix<uint> statsrcs(nsources.size(), nstation.size(), 0);
      Matrix<float> amplTotal (itsDemixInfo.nchanOutSubtr(),
                               itsDemixInfo.nbl(), 0.);
      Cube<float>   amplSubtr (itsDemixInfo.nchanOutSubtr(), itsDemixInfo.nbl(),
                               itsDemixInfo.ateamList().size(), 0.);
      for (size_t i=0; i<itsWorkers.size(); ++i) {
        nsolves        += itsWorkers[i].nSolves();
        nconverged     += itsWorkers[i].nConverged();
        nnodemix       += itsWorkers[i].nNoDemix();
        nincludeStrong += itsWorkers[i].nIncludeStrongTarget();
        nincludeClose  += itsWorkers[i].nIncludeCloseTarget();
        nignore        += itsWorkers[i].nIgnoreTarget();
        ndeproject     += itsWorkers[i].nDeprojectTarget();
        nsources       += itsWorkers[i].nsourcesDemixed();
        nstation       += itsWorkers[i].nstationsDemixed();
        statsrcs       += itsWorkers[i].statSourceDemixed();
        amplTotal      += itsWorkers[i].amplTotal();
        amplSubtr      += itsWorkers[i].amplSubtr();
      }
      // Show statistics.
      os << "Converged solves: " << nconverged << " cells out of "
         << nsolves << endl;
      os << "Nr of time chunks with:" << endl;
      os << setw(8) << nnodemix   << " times no demixing" << endl;
      os << setw(8) << nignore    << " times target ignored" << endl; 
      os << setw(8) << ndeproject << " times target deprojected" << endl; 
      os << setw(8) << nincludeStrong
         << " times target included because strong" << endl; 
      os << setw(8) << nincludeClose
         << " times target included because close" << endl;
      // Show how often a source/station is demixed.
      os << endl << "Nr of time chunks a station/source is demixed:" << endl;
      os << setw(15) << " ";
      for (size_t dr=0; dr<nsources.size(); ++dr) {
        os << setw(8) << itsDemixInfo.ateamList()[dr]->name();
      }
      os << " Overall" << endl;
      for (size_t st=0; st<nstation.size(); ++st) {
        os << setw(4) << st << ' ';
        uint inx = itsFilter.getInfo().antennaUsed()[st];
        string nm = itsFilter.getInfo().antennaNames()[inx];
        os << nm.substr(0,10);
        if (nm.size() < 10) {
          os << setw(10 - nm.size()) << ' ';
        }
        for (size_t dr=0; dr<nsources.size(); ++dr) {
          os << setw(8) << statsrcs(dr,st);
        }
        os << setw(8) << nstation[st] << endl;
      }
      os << "     Overall" << setw(3) << ' ';
      for (size_t dr=0; dr<nsources.size(); ++dr) {
        os << setw(8) << nsources[dr];
      }
      os << endl;
      // Show the subtract percentage medians in time per baseline/source.
      ASSERT (itsNTimeOut == itsPercSubtr.shape()[2]);
      Matrix<float> medianPerc = partialMedians(itsPercSubtr, IPosition(1,2));
      if (itsDemixInfo.verbose() > 12) {
        cout<<"percsubtr="<<itsPercSubtr<<medianPerc;
      }
      os << endl << "Median percentage of Stokes I amplitude subtracted" << endl;
      os << " baseline";
      for (size_t dr=0; dr<nsources.size(); ++dr) {
        os << setw(8) << itsDemixInfo.ateamList()[dr]->name();
      }
      os << "   Total" << endl;
      for (int bl=0; bl<medianPerc.shape()[0]; ++bl) {
        os << setw(4) << itsDemixInfo.getAnt1()[bl] << '-'
           << setw(2) << itsDemixInfo.getAnt2()[bl] << "  ";
        float sumPerc = 0;
        for (int dr=0; dr<medianPerc.shape()[1]; ++dr) {
          showPerc1 (os, medianPerc(bl,dr));
          sumPerc += medianPerc(bl,dr);
        }
        showPerc1 (os, sumPerc);
        os << endl;
      }
      // Show the totals.
      if (itsDemixInfo.verbose() > 12) {
        cout <<"ampltotal="<<amplTotal;
        cout <<"amplsubtr="<<amplSubtr;
      }
      // Sum over channel and baseline.
      float atot = sum(amplTotal);
      Vector<float> asub = partialSums(amplSubtr, IPosition(2,0,1));
      os << "  Total  ";
      float sumPerc = 0;
      for (uint dr=0; dr<asub.size(); ++dr) {
        float perc = (atot==0 ? 0 : asub[dr]/atot*100.);
        showPerc1 (os, perc);
        sumPerc += perc;
      }
      showPerc1 (os, sumPerc);
      os << endl;
    }

    void DemixerNew::showPerc1 (ostream& os, float perc) const
    {
      int p = int(10*perc + 0.5);
      os << std::setw(5) << p/10 << '.' << p%10 << '%';
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
      uint timeWindowSol = itsDemixInfo.ntimeOut();
      int lastChunk = (itsNTime - 1) / timeWindowIn;
      int lastNTimeIn = itsNTime - lastChunk*timeWindowIn;
      int ntimeOut = ((itsNTime + itsDemixInfo.ntimeAvgSubtr() - 1)
                      / itsDemixInfo.ntimeAvgSubtr());
      int ntimeSol = ((itsNTime + itsDemixInfo.ntimeAvg() - 1)
                      / itsDemixInfo.ntimeAvg());
      if (itsDemixInfo.verbose() > 10) {
        cout<<"NTimeOut="<<itsNTimeOut<<itsPercSubtr.shape()<<ntimeOut<<endl;
      }
      ASSERT (itsNTimeOut + ntimeOut <= itsPercSubtr.shape()[2]);
      ///#pragma omp parallel for schedule dynamic
      for (int i=0; i<=lastChunk; ++i) {
        if (i == lastChunk) {
          if (itsDemixInfo.verbose() > 10) {
            cout<<"chunk="<<i*timeWindowIn<<' '<<lastNTimeIn<<endl;
          }
          itsWorkers[OpenMP::threadNum()].process
            (&(itsBufIn[i*timeWindowIn]), lastNTimeIn,
             &(itsBufOut[i*timeWindowOut]),
             &(itsSolutions[i*timeWindowSol]),
             &(itsPercSubtr(0, 0, itsNTimeOut + i*timeWindowOut)));
        } else {
          if (itsDemixInfo.verbose() > 10) {
            cout<<"chunk="<<i*timeWindowIn<<' '<<timeWindowIn<<endl;
          }
          itsWorkers[OpenMP::threadNum()].process
            (&(itsBufIn[i*timeWindowIn]), timeWindowIn,
             &(itsBufOut[i*timeWindowOut]),
             &(itsSolutions[i*timeWindowSol]),
             &(itsPercSubtr(0, 0, itsNTimeOut + i*timeWindowOut)));
        }
      }
      itsTimerDemix.stop();
      // Write the solutions into the instrument ParmDB.
      // Let the next steps process the results.
      // This could be done in parallel.
      ///#pragma omp parallel for num_thread(2)
      for (int i=0; i<2; ++i) {
        if (i == 0) {
          itsTimerDump.start();
          double startTime = (itsBufIn[0].getTime() +
                              0.5 * itsBufIn[0].getExposure());
          writeSolutions (startTime, ntimeSol);
          itsTimerDump.stop();
        } else {
          itsTimer.stop();
          itsTimerNext.start();
          for (int j=0; j<ntimeOut; ++j) {
            getNextStep()->process (itsBufOut[j]);
            itsNTimeOut++;
          }
          itsTimerNext.stop();
          itsTimer.start();
        }
      }
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

    void DemixerNew::writeSolutions (double startTime, int ntime)
    {
      /// Note: make sure that all times have the same parms
      /// Different workers might decide differently on solving a source!!!
      // Construct solution grid.
      const Vector<double>& freq      = getInfo().chanFreqs();
      const Vector<double>& freqWidth = getInfo().chanWidths();
      BBS::Axis::ShPtr freqAxis(new BBS::RegularAxis(freq[0] - freqWidth[0]
        * 0.5, freqWidth[0], 1));
      BBS::Axis::ShPtr timeAxis(new BBS::RegularAxis
                                (startTime,
                                 itsDemixInfo.timeIntervalAvg(), ntime));
      BBS::Grid solGrid(freqAxis, timeAxis);
      // Create domain grid.
      BBS::Axis::ShPtr tdomAxis(new BBS::RegularAxis
                                (startTime,
                                 itsDemixInfo.timeIntervalAvg() * ntime, 1));
      BBS::Grid domainGrid(freqAxis, tdomAxis);

      // Open the ParmDB at the first write.
      // In that way the instrumentmodel ParmDB can be in the MS directory.
      if (! itsParmDB) {
        itsParmDB = boost::shared_ptr<BBS::ParmDB>
          (new BBS::ParmDB(BBS::ParmDBMeta("casa", itsInstrumentName),
                           true));
        itsParmDB->lock();
        // Store the (freq, time) resolution of the solutions.
        vector<double> resolution(2);
        resolution[0] = freqWidth[0];
        resolution[1] = itsDemixInfo.timeIntervalAvg();
        itsParmDB->setDefaultSteps(resolution);
      }
      // Write the solutions per parameter.
      const char* str01[] = {"0:","1:"};
      const char* strri[] = {"Real:","Imag:"};
      Matrix<double> values(1, ntime);
      for (size_t dr=0; dr<itsDemixInfo.ateamList().size(); ++dr) {
        int seqnr = 0;
        for (size_t st=0; st<itsDemixInfo.nstation(); ++st) {
          string suffix(itsDemixInfo.antennaNames()[st]);
          suffix += ":" + itsDemixInfo.ateamList()[dr]->name();
          for (int i=0; i<2; ++i) {
            for (int j=0; j<2; j++) {
              for (int k=0; k<2; ++k) {
                string name(string("DirectionalGain:") +
                            str01[i] + str01[j] + strri[k] + suffix);
                // Collect its solutions for all times in a single array.
                for (int ts=0; ts<ntime; ++ts) {
                  values(0, ts) = itsSolutions[ts][seqnr];
                }
                BBS::ParmValue::ShPtr pv(new BBS::ParmValue());
                pv->setScalars (solGrid, values);
                BBS::ParmValueSet pvs(domainGrid,
                                      vector<BBS::ParmValue::ShPtr>(1, pv));
                map<string,int>::const_iterator pit = itsParmIdMap.find(name);
                if (pit == itsParmIdMap.end()) {
                  // First time, so a new nameId will be set.
                  int nameId = -1;
                  itsParmDB->putValues (name, nameId, pvs);
                  itsParmIdMap[name] = nameId;
                } else {
                  // Parm has been put before.
                  int nameId = pit->second;
                  itsParmDB->putValues (name, nameId, pvs);
                }
              }
            }
          }
        }
      }
    }

} //# end namespace DPPP
} //# end namespace LOFAR

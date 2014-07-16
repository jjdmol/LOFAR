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
#include <ParmDB/ParmDB.h>
#include <ParmDB/ParmValue.h>
#include <ParmDB/SourceDB.h>
#include <Common/ParameterSet.h>
#include <Common/StringUtil.h>
#include <Common/LofarLogger.h>
#include <Common/OpenMP.h>

#include <fstream>
#include <ctime>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>
#include <casa/OS/File.h>

#include <vector>
#include <algorithm>

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
      : itsInput         (input),
        itsName          (prefix),
        itsSourceDBName  (parset.getString (prefix + "sourcedb")),
        itsParmDBName    (parset.getString (prefix + "parmdb")),
        itsApplyBeam     (parset.getBool (prefix + "usebeammodel", false)),
        itsMode          (parset.getString (prefix + "caltype")),
        itsTStep         (0),
        itsDebugLevel    (parset.getInt (prefix + "debuglevel", 0)),
        itsBaselines     (),
        itsThreadStorage  (),
        itsMaxIter       (parset.getInt (prefix + "maxiter", 50)),
        itsTolerance     (parset.getDouble (prefix + "tolerance", 1.e-5)),
        itsPropagateSolutions (parset.getBool(prefix + "propagatesolutions", false)),
        itsPatchList     (),
        itsOperation     (parset.getString(prefix + "operation", "solve")),
        itsConverged     (0),
        itsNonconverged  (0),
        itsStalled       (0)
    {
      BBS::SourceDB sourceDB(BBS::ParmDBMeta("", itsSourceDBName), false);

      vector<PatchInfo> patchInfo=sourceDB.getPatchInfo();
      vector<string> patchNames;

      vector<string> sourcePatterns=parset.getStringVector(prefix + "sources",
              vector<string>());
      patchNames=makePatchList(sourceDB, sourcePatterns);

      ASSERT(itsMode=="diagonal" || itsMode=="phaseonly" || itsMode=="fulljones");

      /*
      vector<string> parms = parset.getStringVector(prefix+"parms",vector<string>());
      uint numdiag=0;
      uint numoffdiag=0;
      for (vector<string>::iterator parmname=parms.begin(); parmname!=parms.end();++parmname) {
        if ((*parmname).length()<8 ||
            (*parmname).substr(0,5)!="Gain:") {
          THROW (Exception, "Can only solve for gains");
        }
        string parmpol=(*parmname).substr(5,3);
        if (parmpol=="0:1" || parmpol=="1:0") {
          numoffdiag++;
       } else if (parmpol=="0:0" || parmpol=="1:1") {
          numdiag++;
        } else {
          THROW (Exception, "Can only solve for gains");
        }
      }
      if (numoffdiag==2 && numdiag==2) {
        itsMode="fullgain";
      } else if (numoffdiag==0 && numdiag==2) {
        itsMode="diaggain";
      } else if (parms.size()==0) {
        itsMode="fullgain";
      } else {
        THROW (Exception, "Can only solve for diagonal or all gains");
      }

      string parmmode=parset.getString(prefix+"mode","COMPLEX");
      if (parmmode=="COMPLEX") {
        itsPhaseOnly = false;
      } else if (parmmode=="PHASE") {
        itsPhaseOnly = true;
        if (itsMode!="diaggain") {
          THROW (Exception, "Mode PHASE only works when solving for diagonal gains");
        }
      } else {
        THROW (Exception, "Can only handle the modes Phase and Complex");
      }
      */

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


      MDirection dirJ2000(MDirection::Convert(infoIn.phaseCenterCopy(),
                                              MDirection::J2000)());
      Quantum<Vector<Double> > angles = dirJ2000.getAngle();
      itsPhaseRef = Position(angles.getBaseValue()[0],
                             angles.getBaseValue()[1]);

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

      itsSols.reserve(info().ntime());

      // Read the antenna beam info from the MS.
      // Only take the stations actually used.
      itsAntennaUsedNames.resize(info().antennaUsed().size());
      casa::Vector<int> antsUsed = info().antennaUsed();
      for (int ant=0, nAnts=info().antennaUsed().size(); ant<nAnts; ++ant) {
        itsAntennaUsedNames[ant]=info().antennaNames()[info().antennaUsed()[ant]];
      }
      itsInput->fillBeamInfo (itsAntBeamInfo, itsAntennaUsedNames);
    }

    StationResponse::vector3r_t GainCal::dir2Itrf (const MDirection& dir,
                                                   MDirection::Convert& converter) const
    {
      const MDirection& itrfDir = converter(dir);
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
      os << "  max iter:       " << itsMaxIter << endl;
      os << "  tolerance:      " << itsTolerance << endl;
      os << "  propagate sols: " << boolalpha << itsPropagateSolutions << endl;
      os << "  mode:           " << itsMode << endl;
    }

    void GainCal::showTimings (std::ostream& os, double duration) const
    {
      double totaltime=itsTimer.getElapsed();
      os << "  ";
      FlagCounter::showPerc1 (os, itsTimer.getElapsed(), duration);
      os << " GainCal " << itsName << endl;

      os << "          ";
      FlagCounter::showPerc1 (os, itsTimerPredict.getElapsed(), totaltime);
      os << " of it spent in predict" << endl;

      os << "          ";
      FlagCounter::showPerc1 (os, itsTimerFill.getElapsed(), totaltime);
      os << " of it spent in reordering visibility data" << endl;

      os << "          ";
      FlagCounter::showPerc1 (os, itsTimerSolve.getElapsed(), totaltime);
      os << " of it spent in estimating gains and computing residuals" << endl;

      os << "          ";
      FlagCounter::showPerc1 (os, itsTimerWrite.getElapsed(), totaltime);
      os << " of it spent in writing gain solutions to disk" << endl;

      os << "          ";
      os <<"Converged: "<<itsConverged<<", stalled: "<<itsStalled<<", non converged: "<<itsNonconverged<<endl;
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
      float* weight = buf.getWeights().data();
      const Bool* flag=buf.getFlags().data();

      size_t stride_model[3] = {1, nCr, nCr * nCh};
      fill(storage.model.begin(), storage.model.end(), dcomplex());

      const_cursor<double> cr_uvw = casa_const_cursor(buf.getUVW());
      splitUVW(nSt, nBl, cr_baseline, cr_uvw, cr_uvw_split);
      cursor<dcomplex> cr_model(&(storage.model_patch[0]), 3, stride_model);

      StationResponse::vector3r_t refdir = dir2Itrf(info().delayCenterCopy(),storage.measConverter);
      StationResponse::vector3r_t tiledir = dir2Itrf(info().tileBeamDirCopy(),storage.measConverter);
      // Convert the directions to ITRF for the given time.
      storage.measFrame.resetEpoch (MEpoch(MVEpoch(time/86400), MEpoch::UTC));

      itsTimerPredict.start();
//#pragma omp parallel for
      for(size_t dr = 0; dr < nDr; ++dr)
      {
        fill(storage.model_patch.begin(), storage.model_patch.end(), dcomplex());

        simulate(itsPhaseRef, itsPatchList[dr], nSt, nBl, nCh, cr_baseline,
                 cr_freq, cr_uvw_split, cr_model);

        for(size_t i = 0; i < itsPatchList[dr]->nComponents(); ++i)
        { // Apply beam for every source, not only once per patch
          applyBeam(time, itsPatchList[dr]->component(i)->position(), itsApplyBeam,
                    info().chanFreqs(), &(itsThreadStorage[thread].model_patch[0]),
                    refdir, tiledir, &(itsThreadStorage[thread].beamvalues[0]),
                    storage.measConverter);
        }

        for (size_t i=0; i<itsThreadStorage[thread].model_patch.size();++i) {
          itsThreadStorage[thread].model[i]+=
              itsThreadStorage[thread].model_patch[i];
        }
      }

      itsTimerPredict.stop();
      //copy result of model to data
      if (itsOperation=="predict") {
        copy(storage.model.begin(),storage.model.begin()+nSamples,data);
      }

      if (itsOperation=="solve") {
        if (itsMode=="diagonal" || itsMode=="phaseonly") {
          stefcal(&storage.model[0], data, weight, flag, false);
        } else {
          stefcal(&storage.model[0], data, weight, flag, true);
        }
      }

      itsTimer.stop();
      itsTStep++;
      getNextStep()->process(buf);
      return false;
    }


    // Fills itsVis and itsMVis as matrices with all 00 polarizations in the
    // top left, all 11 polarizations in the bottom right, etc.
    void GainCal::fillMatricesUnpol (dcomplex* model, casa::Complex* data, float* weight,
                                const casa::Bool* flag) {
      itsTimerFill.start();
      vector<int>* antUsed=&itsAntUseds[itsAntUseds.size()-1];
      uint nSt=(*antUsed).size();
      vector<int>* antMap=&itsAntMaps[itsAntMaps.size()-1];

      const size_t nBl = info().nbaselines();
      const size_t nCh = info().nchan();
      const size_t nCr = 4;

      itsVis.resize (IPosition(5,nSt,2,nCh,nSt,2));
      itsMVis.resize(IPosition(5,nSt,2,nCh,nSt,2));

      itsVis=0;
      itsMVis=0;

      for (uint ch=0;ch<nCh;++ch) {
        for (uint bl=0;bl<nBl;++bl) {
          int ant1=(*antMap)[info().getAnt1()[bl]];
          int ant2=(*antMap)[info().getAnt2()[bl]];
          if (ant1==ant2 || ant1==-1 || ant2 == -1 || flag[bl*nCr*nCh+ch*nCr]) { // Only check flag of cr==0
            continue;
          }

          for (uint cr=0;cr<nCr;++cr) {
            itsVis (IPosition(5,ant1,cr%2,ch,ant2,cr/2)) = DComplex(data [bl*nCr*nCh+ch*nCr+cr])*DComplex(sqrt(weight[bl*nCr*nCh+ch*nCr+cr]));
            itsMVis(IPosition(5,ant1,cr%2,ch,ant2,cr/2)) =          model[bl*nCr*nCh+ch*nCr+cr] *DComplex(sqrt(weight[bl*nCr*nCh+ch*nCr+cr]));

            // Below is the complex conjugate. tcr is the correlation for the transposed
            itsVis (IPosition(5,ant2,cr/2,ch,ant1,cr%2)) = DComplex(conj(data [bl*nCr*nCh+ch*nCr+cr]))*DComplex(sqrt(weight[bl*nCr*nCh+ch*nCr+cr]));
            itsMVis(IPosition(5,ant2,cr/2,ch,ant1,cr%2)) =          conj(model[bl*nCr*nCh+ch*nCr+cr] )*DComplex(sqrt(weight[bl*nCr*nCh+ch*nCr+cr]));
          }
        }
      }
      itsTimerFill.stop();
    }


    void GainCal::fillMatricesPol (dcomplex* model, casa::Complex* data, float* weight,
                                const casa::Bool* flag) {
      itsTimerFill.start();
      vector<int>* antUsed=&itsAntUseds[itsAntUseds.size()-1];
      uint nSt=(*antUsed).size();
      vector<int>* antMap=&itsAntMaps[itsAntMaps.size()-1];

      const size_t nBl = info().nbaselines();
      const size_t nCh = info().nchan();
      const size_t nCr = 4;

      itsVis.resize (IPosition(4,nCr,nSt,nCh,nSt));
      itsMVis.resize(IPosition(4,nCr,nSt,nCh,nSt));

      itsVis=0;
      itsMVis=0;

      for (uint ch=0;ch<nCh;++ch) {
        for (uint bl=0;bl<nBl;++bl) {
          int ant1=(*antMap)[info().getAnt1()[bl]];
          int ant2=(*antMap)[info().getAnt2()[bl]];
          if (ant1==ant2 || ant1==-1 || ant2 == -1 || flag[bl*nCr*nCh+ch*nCr]) { // Only check flag of cr==0
            continue;
          }

          for (uint cr=0;cr<nCr;++cr) {
            itsVis (IPosition(4,cr,ant1,ch,ant2)) = DComplex(data [bl*nCr*nCh+ch*nCr+cr])*DComplex(sqrt(weight[bl*nCr*nCh+ch*nCr+cr]));
            itsMVis(IPosition(4,cr,ant1,ch,ant2)) =          model[bl*nCr*nCh+ch*nCr+cr] *DComplex(sqrt(weight[bl*nCr*nCh+ch*nCr+cr]));

            // Below is the complex conjugate. tcr is the correlation for the transposed
            uint tcr=cr;
            if (cr==1 || cr==2) {
              tcr=3-cr;
            }
            itsVis (IPosition(4,tcr,ant2,ch,ant1)) = DComplex(conj(data [bl*nCr*nCh+ch*nCr+cr]))*DComplex(sqrt(weight[bl*nCr*nCh+ch*nCr+cr]));
            itsMVis(IPosition(4,tcr,ant2,ch,ant1)) =          conj(model[bl*nCr*nCh+ch*nCr+cr] )*DComplex(sqrt(weight[bl*nCr*nCh+ch*nCr+cr]));
          }
        }
      }
      itsTimerFill.stop();
    }

    void GainCal::setAntUsedNotFlagged (const Bool* flag) {
      uint nCr=info().ncorr();
      uint nCh=info().nchan();
      uint nBl=info().nbaselines();

      vector<uint> dataPerAntenna(info().antennaNames().size(),0);

      // I assume antennas are numbered 0, 1, 2, ...
      for (uint bl=0;bl<nBl;++bl) {
        uint ant1=info().getAnt1()[bl];
        uint ant2=info().getAnt2()[bl];
        if (ant1==ant2) {
          continue;
        }
        for (uint ch=0;ch<nCh;++ch) {
          for (uint cr=0;cr<nCr;++cr) {
            if (!flag[bl*nCr*nCh + ch*nCr + cr]) {
              dataPerAntenna[ant1]++;
              dataPerAntenna[ant2]++;
            }
          }
        }
      }
      vector<int> antMap(info().antennaNames().size(),-1);

      const uint minBaselinesPerAntenna=4;
      for (uint i=0; i<dataPerAntenna.size(); ++i) {
        if (dataPerAntenna[i]>nCr*minBaselinesPerAntenna) {
          antMap[i] = 0;
        }
      }

      vector<int> antUsed;
      antUsed.reserve(info().antennaNames().size());
      for (uint i=0; i<antMap.size(); ++i) {
        if (antMap[i] == 0) {
          antMap[i] = antUsed.size();
          antUsed.push_back (i);
        }
      }

      itsAntUseds.push_back(antUsed);
      itsAntMaps.push_back(antMap);
    }

    void GainCal::stefcal (dcomplex* model, casa::Complex* data, float* weight,
                                const Bool* flag, bool pol) {
      setAntUsedNotFlagged(flag);
      if (pol) {
        fillMatricesPol(model,data,weight,flag);
      } else {
        fillMatricesUnpol(model,data,weight,flag);
      }

      vector<double> dgs;

      itsTimerSolve.start();
      double f2 = -1.0;
      double f3 = -0.5;
      double f1 = 1 - f2 - f3;
      double f2q = -0.5;
      double f1q = 1 - f2q;
      double omega = 0.5;
      uint nomega = 24;
      double c1 = 0.5;
      double c2 = 1.2;
      double dg  =1.0e29;
      double dgx =1.0e30;
      double dgxx;
      bool threestep = false;
      int nhit=0;
      int maxhit=5;

      uint nSt, nCr;

      if (pol) {
        nSt=itsAntUseds[itsAntUseds.size()-1].size();
        nCr=4;
      } else {
        nSt=itsAntUseds[itsAntUseds.size()-1].size()*2;
        nCr=1;
      }
      uint nCh = info().nchan();

      iS.g.resize(nSt,nCr);
      iS.gold.resize(nSt,nCr);
      iS.gx.resize(nSt,nCr);
      iS.gxx.resize(nSt,nCr);
      iS.h.resize(nSt,nCr);
      iS.z.resize(nSt*nCh,nCr);

      double ww; // Same as w, but specifically for pol==false
      Vector<DComplex> w(nCr);
      Vector<DComplex> t(nCr);

      // Initialize all vectors
      double fronormvis=0;
      double fronormmod=0;

      double fronormvis1=0;
      double fronormmod1=0;
      DComplex* vis_p;
      DComplex* mvis_p;

      vis_p=itsVis.data();
      mvis_p=itsMVis.data();

      uint vissize=itsVis.size();
      for (uint i=0;i<vissize;++i) {
        fronormvis1+=norm(*vis_p++);
        fronormmod1+=norm(*mvis_p++);
      }

      fronormvis=sqrt(fronormvis);
      fronormmod=sqrt(fronormmod);

      double ginit=1;
      if (nSt>0 && abs(fronormmod)>1.e-15) {
        ginit=sqrt(fronormvis/fronormmod);
      }
      if (pol) {
        for (uint st=0;st<nSt;++st) {
          iS.g(st,0)=1.;
          iS.g(st,1)=0.;
          iS.g(st,2)=0.;
          iS.g(st,3)=1.;
        }
      } else {
        iS.g=ginit;
      }

      iS.gx = iS.g;
      int sstep=0;

      uint iter=0;
      if (nSt==0) {
        iter=itsMaxIter;
      }
      for (;iter<itsMaxIter;++iter) {
        //cout<<"iter+1 = "<<iter+1<<endl;
        iS.gold=iS.g;

        if (pol) { // ======================== Polarized =======================
          for (uint st=0;st<nSt;++st) {
            iS.h(st,0)=conj(iS.g(st,0));
            iS.h(st,1)=conj(iS.g(st,1));
            iS.h(st,2)=conj(iS.g(st,2));
            iS.h(st,3)=conj(iS.g(st,3));
          }

          for (uint st1=0;st1<nSt;++st1) {
            mvis_p=&itsMVis(IPosition(4,0,0,0,st1));
            for (uint ch=0;ch<nCh;++ch) {
              for (uint st2=0;st2<nSt;++st2) {
                iS.z(ch*nSt+st2,0) = iS.h(st2,0) * itsMVis(IPosition(4,0,st2,ch,st1)) + iS.h(st2,2) * itsMVis(IPosition(4,2,st2,ch,st1)); //mvis_p[0], mvis_p[2];
                iS.z(ch*nSt+st2,1) = iS.h(st2,0) * itsMVis(IPosition(4,1,st2,ch,st1)) + iS.h(st2,2) * itsMVis(IPosition(4,3,st2,ch,st1)); //mvis_p[1], mvis_p[3];
                iS.z(ch*nSt+st2,2) = iS.h(st2,1) * itsMVis(IPosition(4,0,st2,ch,st1)) + iS.h(st2,3) * itsMVis(IPosition(4,2,st2,ch,st1)); //mvis_p[0], mvis_p[2];
                iS.z(ch*nSt+st2,3) = iS.h(st2,1) * itsMVis(IPosition(4,1,st2,ch,st1)) + iS.h(st2,3) * itsMVis(IPosition(4,3,st2,ch,st1)); //mvis_p[1], mvis_p[3];
                mvis_p+=4;
              }
            }

            w=0;
            t=0;

            for (uint ch=0;ch<nCh;++ch) {
              for (uint st2=0;st2<nSt;++st2) {
                w(0) += conj(iS.z(st2+nSt*ch,0))*iS.z(st2+nSt*ch,0) + conj(iS.z(st2+nSt*ch,2))*iS.z(st2+nSt*ch,2);
                w(1) += conj(iS.z(st2+nSt*ch,0))*iS.z(st2+nSt*ch,1) + conj(iS.z(st2+nSt*ch,2))*iS.z(st2+nSt*ch,3);
                w(3) += conj(iS.z(st2+nSt*ch,1))*iS.z(st2+nSt*ch,1) + conj(iS.z(st2+nSt*ch,3))*iS.z(st2+nSt*ch,3);
              }
            }
            w(2)=conj(w(1));

            t=0;
            vis_p=&itsVis(IPosition(4,0,0,0,st1));
            for (uint ch=0;ch<nCh;++ch) {
              for (uint st2=0;st2<nSt;++st2) {
                t(0) += conj(iS.z(st2+nSt*ch,0)) * itsVis(IPosition(4,0,st2,ch,st1)) + conj(iS.z(st2+nSt*ch,2)) * itsVis(IPosition(4,2,st2,ch,st1)); //vis_p[0], vis_p[2];
                t(1) += conj(iS.z(st2+nSt*ch,0)) * itsVis(IPosition(4,1,st2,ch,st1)) + conj(iS.z(st2+nSt*ch,2)) * itsVis(IPosition(4,3,st2,ch,st1)); //vis_p[1], vis_p[3];
                t(2) += conj(iS.z(st2+nSt*ch,1)) * itsVis(IPosition(4,0,st2,ch,st1)) + conj(iS.z(st2+nSt*ch,3)) * itsVis(IPosition(4,2,st2,ch,st1)); //vis_p[0], vis_p[2];
                t(3) += conj(iS.z(st2+nSt*ch,1)) * itsVis(IPosition(4,1,st2,ch,st1)) + conj(iS.z(st2+nSt*ch,3)) * itsVis(IPosition(4,3,st2,ch,st1)); //vis_p[1], vis_p[3];
                vis_p+=4;
              }
            }
            DComplex invdet= 1./(w(0) * w (3) - w(1)*w(2));
            iS.g(st1,0) = invdet * ( w(3) * t(0) - w(1) * t(2) );
            iS.g(st1,1) = invdet * ( w(3) * t(1) - w(1) * t(3) );
            iS.g(st1,2) = invdet * ( w(0) * t(2) - w(2) * t(0) );
            iS.g(st1,3) = invdet * ( w(0) * t(3) - w(2) * t(1) );
          }
        } else {// ======================== Nonpolarized =======================
          for (uint st=0;st<nSt;++st) {
            iS.h(st,0)=conj(iS.g(st,0));
          }

          for (uint st1=0;st1<nSt;++st1) {
            ww=0;
            t(0)=0;
            mvis_p=&itsMVis(IPosition(3,0,0,st1));
            vis_p = &itsVis(IPosition(3,0,0,st1));
            for (uint ch=0;ch<nCh;++ch) {
              for (uint st2=0;st2<nSt;++st2) {
                iS.z(ch*(nSt/2)+st2,0) = iS.h(st2,0) * itsMVis(IPosition(5,st2%(nSt/2),st2/(nSt/2),ch,st1%(nSt/2),st1/(nSt/2))); //*mvis_p;
                ww+=norm(iS.z(ch*(nSt/2)+st2,0));
                t(0)+=conj(iS.z(ch*(nSt/2)+st2,0)) * itsVis(IPosition(5,st2%(nSt/2),st2/(nSt/2),ch,st1%(nSt/2),st1/(nSt/2))); //*vis_p;
                mvis_p++;
                vis_p++;
              }
            }
            iS.g(st1,0)=t(0)/ww;
            if (itsMode=="phaseonly") {
              iS.g(st1,0)/=abs(iS.g(st1,0));
            }
          }
        }
        if (iter % 2 == 1) {
          if (dgx-dg <= 1.0e-3*dg) {
            if (itsDebugLevel>3) {
              cout<<"**"<<endl;
            }
            nhit++;
          } else {
            nhit=0;
          }

          if (nhit>=maxhit) {
            if (itsDebugLevel>3) {
              cout<<"Detected stall"<<endl;
            }
            itsStalled++;
            break;
          }

          dgxx = dgx;
          dgx  = dg;

          double fronormdiff=0;
          double fronormg=0;
          for (uint ant=0;ant<nSt;++ant) {
            for (uint cr=0;cr<nCr;++cr) {
              DComplex diff=iS.g(ant,cr)-iS.gold(ant,cr);
              fronormdiff+=abs(diff*diff);
              fronormg+=abs(iS.g(ant,cr)*iS.g(ant,cr));
            }
          }
          fronormdiff=sqrt(fronormdiff);
          fronormg=sqrt(fronormg);

          dg = fronormdiff/fronormg;
          if (itsDebugLevel>1) {
            dgs.push_back(dg);
          }

          if (dg <= itsTolerance) {
            itsConverged++;
            break;
          }

          if (itsDebugLevel>7) {
            cout<<"Averaged"<<endl;
          }
          for (uint ant=0;ant<nSt;++ant) {
            for (uint cr=0;cr<nCr;++cr) {
              iS.g(ant,cr) = (1-omega) * iS.g(ant,cr) + omega * iS.gold(ant,cr);
            }
          }

          if (!threestep) {
            threestep = (iter+1 >= nomega) ||
                ( max(dg,max(dgx,dgxx)) <= 1.0e-3 && dg<dgx && dgx<dgxx);
            if (itsDebugLevel>7) {
              cout<<"Threestep="<<boolalpha<<threestep<<endl;
            }
          }

          if (threestep) {
            if (itsDebugLevel>7) {
              cout<<"threestep"<<endl;
            }
            if (sstep <= 0) {
              if (dg <= c1 * dgx) {
                if (itsDebugLevel>7) {
                  cout<<"dg<=c1*dgx"<<endl;
                }
                for (uint ant=0;ant<nSt;++ant) {
                  for (uint cr=0;cr<nCr;++cr) {
                    iS.g(ant,cr) = f1q * iS.g(ant,cr) + f2q * iS.gx(ant,cr);
                  }
                }
              } else if (dg <= dgx) {
                if (itsDebugLevel>7) {
                  cout<<"dg<=dgx"<<endl;
                }
                for (uint ant=0;ant<nSt;++ant) {
                  for (uint cr=0;cr<nCr;++cr) {
                    iS.g(ant,cr) = f1 * iS.g(ant,cr) + f2 * iS.gx(ant,cr) + f3 * iS.gxx(ant,cr);
                  }
                }
              } else if (dg <= c2 *dgx) {
                if (itsDebugLevel>7) {
                  cout<<"dg<=c2*dgx"<<endl;
                }
                iS.g = iS.gx;
                sstep = 1;
              } else {
                //cout<<"else"<<endl;
                iS.g = iS.gxx;
                sstep = 2;
              }
            } else {
              if (itsDebugLevel>7) {
                cout<<"no sstep"<<endl;
              }
              sstep = sstep - 1;
            }
          }
          iS.gxx = iS.gx;
          iS.gx = iS.g;
        }
      }
      if (dg > itsTolerance && nSt>0) {
        if (nhit<maxhit) {
          itsNonconverged++;
        }
        if (itsDebugLevel>0) {
          cerr<<"!";
        }
      }

      if (itsDebugLevel>1) {
        cout<<"t: "<<itsTStep<<", iter:"<<iter<<", dg=[";
        if (dgs.size()>0) {
          cout<<dgs[0];
        }
        for (uint i=1;i<dgs.size();++i) {
          cout<<","<<dgs[i];
        }
        cout<<"]"<<endl;
      }

      if (nSt>0) {
        DComplex p = conj(iS.g(0,0))/abs(iS.g(0,0));
        // Set phase of first gain to zero
        for (uint st=0;st<nSt;++st) {
          for (uint cr=0;cr<nCr;++cr) {
            iS.g(st,cr)*=p;
          }
        }
      }

      //for (uint ant2=0;ant2<nSt;++ant2) {
        //cout<<"g["<<ant2<<"]={"<<g[ant2][0]<<", "<<g[ant2][1]<<", "<<g[ant2][2]<<", "<<g[ant2][3]<<"}"<<endl;
        //cout<<"w["<<ant2<<"]={"<<w[ant2][0]<<", "<<w[ant2][1]<<", "<<w[ant2][2]<<", "<<w[ant2][3]<<"}"<<endl;
      //}

      // Stefcal terminated (either by maxiter or by converging)
      // Let's save G...
      itsSols.push_back(iS.g.copy());

      if (itsDebugLevel>3) {
        cout<<"g="<<iS.g<<endl;
      }

      if (dg > itsTolerance && itsDebugLevel>1 && nSt>0) {
        cout<<endl<<"Did not converge: dg="<<dg<<" tolerance="<<itsTolerance<<", nants="<<nSt<<endl;
        if (itsDebugLevel>12) {
          cout<<"g="<<iS.g<<endl;
          exportToMatlab(0);
          THROW(Exception,"Klaar!");
        }
      }
      itsTimerSolve.stop();
    }


    void GainCal::exportToMatlab(uint ch) {
      ofstream mFile;
      uint nSt = itsMVis.shape()[1];
      mFile.open ("debug.txt");
      mFile << "# Created by NDPPP"<<endl;
      mFile << "# name: V"<<endl;
      mFile << "# type: complex matrix"<<endl;
      mFile << "# rows: "<<2*nSt<<endl;
      mFile << "# columns: "<<2*nSt<<endl;

      for (uint row=0;row<nSt;++row) {
        for (uint col=0;col<nSt;++col) {
          mFile << itsVis(IPosition(4,0,row,ch,col))<<" ";
          mFile << itsVis(IPosition(4,1,row,ch,col))<<" ";
        }
        mFile << endl;
        for (uint col=0;col<nSt;++col) {
          mFile << itsVis(IPosition(4,2,row,ch,col))<<" ";
          mFile << itsVis(IPosition(4,3,row,ch,col))<<" ";
        }
        mFile << endl;
      }

      mFile << endl;
      mFile << "# name: Vm"<<endl;
      mFile << "# type: complex matrix"<<endl;
      mFile << "# rows: "<<nSt*2<<endl;
      mFile << "# columns: "<<nSt*2<<endl;

      for (uint row=0;row<nSt;++row) {
        for (uint col=0;col<nSt;++col) {
          mFile << itsMVis(IPosition(4,0,row,ch,col))<<" ";
          mFile << itsMVis(IPosition(4,1,row,ch,col))<<" ";
        }
        mFile << endl;
        for (uint col=0;col<nSt;++col) {
          mFile << itsMVis(IPosition(4,2,row,ch,col))<<" ";
          mFile << itsMVis(IPosition(4,3,row,ch,col))<<" ";
        }
        mFile << endl;
      }

      mFile.close();
      THROW(Exception,"Wrote output to debug.txt -- stopping now");
    }

    void GainCal::applyBeam (double time, const Position& pos, bool apply,
                             const Vector<double>& chanFreqs, dcomplex* data,
                             StationResponse::vector3r_t& refdir,
                             StationResponse::vector3r_t& tiledir,
                             StationResponse::matrix22c_t* beamvalues,
                             casa::MDirection::Convert& converter)
    {
      if (! apply) {
        return;
      }

      MDirection dir (MVDirection(pos[0], pos[1]), MDirection::J2000);
      StationResponse::vector3r_t srcdir = dir2Itrf(dir,converter);
      // Get the beam values for each station.
      uint nchan = chanFreqs.size();
      uint nSt   = info().antennaUsed().size();
      uint nBl   = info().nbaselines();

//#pragma omp parallel for
      for (size_t st=0; st<nSt; ++st) {
        itsAntBeamInfo[st]->response (nchan, time, chanFreqs.cbegin(),
                                      srcdir, info().refFreq(), refdir, tiledir,
                                      &(beamvalues[nchan*st]));
      }
      // Apply the beam values of both stations to the predicted data.
      dcomplex tmp[4];
      for (size_t bl=0; bl<nBl; ++bl) {
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
      itsTimer.start();
      itsTimerWrite.start();

      if (itsOperation!="solve") {
        getNextStep()->finish();
        return;
      }

      uint nSt=info().antennaUsed().size();

      uint ntime=itsSols.size();

      // Construct solution grid.
      const Vector<double>& freq      = getInfo().chanFreqs();
      const Vector<double>& freqWidth = getInfo().chanWidths();
      BBS::Axis::ShPtr freqAxis(new BBS::RegularAxis(freq[0] - freqWidth[0]
        * 0.5, getInfo().totalBW(), 1));
      BBS::Axis::ShPtr timeAxis(new BBS::RegularAxis
                                (info().startTime(),
                                 info().timeInterval(), ntime));
      BBS::Grid solGrid(freqAxis, timeAxis);
      // Create domain grid.
      BBS::Axis::ShPtr tdomAxis(new BBS::RegularAxis
                                (info().startTime(),
                                 info().timeInterval() * ntime, 1));
      BBS::Grid domainGrid(freqAxis, tdomAxis);

      // Open the ParmDB at the first write.
      // In that way the instrumentmodel ParmDB can be in the MS directory.
      if (! itsParmDB) {
        itsParmDB = boost::shared_ptr<BBS::ParmDB>
          (new BBS::ParmDB(BBS::ParmDBMeta("casa", itsParmDBName),
                           true));
        itsParmDB->lock();
        // Store the (freq, time) resolution of the solutions.
        vector<double> resolution(2);
        resolution[0] = freqWidth[0];
        resolution[1] = info().timeInterval();
        itsParmDB->setDefaultSteps(resolution);
      }
      // Write the solutions per parameter.
      const char* str0101[] = {"0:0:","1:0:","0:1:","1:1:"}; // Conjugate transpose!
      const char* strri[] = {"Real:","Imag:"};
      Matrix<double> values(1, ntime);

      DComplex sol;

      for (size_t st=0; st<nSt; ++st) {
        uint seqnr = 0; // To take care of real and imaginary part
        string suffix(itsAntennaUsedNames[st]);

        for (int i=0; i<4; ++i) {
          if ((itsMode=="diagonal" || itsMode=="phaseonly") && (i==1||i==2)) {
            continue;
          }
          int kmax;
          if (itsMode=="phaseonly") {
            kmax=1;
          } else {
            kmax=2;
          }
          for (int k=0; k<kmax; ++k) {
            string name(string("Gain:") +
                        str0101[i] + (itsMode=="phaseonly"?"Phase:":strri[k]) + suffix);
            // Collect its solutions for all times in a single array.
            for (uint ts=0; ts<ntime; ++ts) {
              if (itsAntMaps[ts][st]==-1) {
                if (itsMode!="phaseonly" && k==0 && (i==0||i==3)) {
                  values(0, ts) = 1;
                } else {
                  values(0, ts) = 0;
                }
              } else {
                int rst=itsAntMaps[ts][st];
                if (itsMode=="fulljones") {
                  if (seqnr%2==0) {
                    values(0, ts) = real(itsSols[ts](rst,seqnr/2));
                  } else {
                    values(0, ts) = -imag(itsSols[ts](rst,seqnr/2)); // Conjugate transpose!
                  }
                } else if (itsMode=="diagonal") {
                  uint sSt=itsSols[ts].size()/2;
                  if (seqnr%2==0) {
                    values(0, ts) = real(itsSols[ts](i/3*sSt+rst,0)); // nSt times Gain:0:0 at the beginning, then nSt times Gain:1:1
                  } else {
                    values(0, ts) = -imag(itsSols[ts](i/3*sSt+rst,0)); // Conjugate transpose!
                  }
                } else {
                  uint sSt=itsSols[ts].size()/2;
                  values(0, ts) = -arg(itsSols[ts](i/3*sSt+rst,0)); // nSt times Gain:0:0 at the beginning, then nSt times Gain:1:1 // Transpose!
                }
              }
            }
            cout.flush();
            seqnr++;
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

      itsTimerWrite.stop();
      itsTimer.stop();
      // Let the next steps finish.
      getNextStep()->finish();
    }


  } //# end namespace
}

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

#include <casa/Arrays/ArrayMath.h>
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
      : itsInput       (input),
        itsName        (prefix),
        itsSourceDBName (parset.getString (prefix + "sourcedb")),
        itsParmDBName  (parset.getString (prefix + "parmdb")),
        itsApplyBeam   (parset.getBool (prefix + "model.beam")),
        itsCellSizeTime (parset.getInt (prefix + "cellsize.time", 1)),
        itsCellSizeFreq (parset.getInt (prefix + "cellsize.freq", 0)),
        itsBaselines   (),
        itsThreadStorage (),
        itsMaxIter     (parset.getInt (prefix + "maxiter", 1000)),
        itsPropagateSolutions (parset.getBool(prefix + "propagatesolutions", false)),
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


      MDirection dirJ2000(MDirection::Convert(infoIn.phaseCenterCopy(),
                                              MDirection::J2000)());
      Quantum<Vector<Double> > angles = dirJ2000.getAngle();
      itsPhaseRef = Position(angles.getBaseValue()[0],
                             angles.getBaseValue()[1]);

      const size_t nDr = itsPatchList.size();
      const size_t nSt = info().antennaUsed().size();
      const size_t nCh = info().nchan();
      // initialize storage
      const size_t nThread=1;//OpenMP::maxThreads();
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
      os << "  propagate sols: " << boolalpha << itsPropagateSolutions << endl;
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

      for(size_t dr = 0; dr < nDr; ++dr)
      {
        fill(storage.model_patch.begin(), storage.model_patch.end(), dcomplex());

        simulate(itsPhaseRef, itsPatchList[dr], nSt, nBl, nCh, cr_baseline,
                 cr_freq, cr_uvw_split, cr_model);
        applyBeam(time, itsPatchList[dr]->position(), itsApplyBeam,
                  info().chanFreqs(), &(itsThreadStorage[thread].model_patch[0]),
                  refdir, tiledir, &(itsThreadStorage[thread].beamvalues[0]),
                  storage.measConverter);

        for (size_t i=0; i<itsThreadStorage[thread].model_patch.size();++i) {
          itsThreadStorage[thread].model[i]+=
              itsThreadStorage[thread].model_patch[i];
        }
      }

      //copy result of model to data
      //copy(storage.model.begin(),storage.model.begin()+nSamples,data);

      //cout<<"storage.model[4]="<<storage.model[4]<<endl;

      // Begin StefCal

      double f2 = -1.0;
      double f3 = -0.5;
      double f1 = 1 - f2 - f3;
      double f2q = -0.5;
      double f1q = 1 - f2q;
      double omega = 0.5;
      uint nomega = 24;
      double c1 = 0.5;
      double c2 = 1.2;
      double dg  =1.0e30;
      double dgx =1.0e30;
      double dgxx;
      bool threestep = false;
      double tol=1.0e-10;

      vector<vector<DComplex> > g(nSt), gold(nSt), gx(nSt), gxx(nSt);
      vector<vector<DComplex> > h(nSt);
      vector<DComplex> z(4);
      vector<vector<DComplex> > w(nSt);
      vector<vector<DComplex> > tt(nSt);

      // Initialize all vectors
      if (itsPropagateSolutions && itsSols.size()>0) {
        g = itsSols[itsSols.size()-1];
      } else {
        for (uint st=0;st<nSt;++st) {
          vector<DComplex> gst(4,0.0);
          gst[0]=1.0;
          gst[3]=1.0;
          g[st] = gst;
        }
      }


      gx = g;
      int sstep=0;

      uint iter=0;
      for (;iter<itsMaxIter;++iter) {
        //cout<<"iter+1 = "<<iter+1<<endl;
        gold=g;
        h=g;
        for (uint st=0;st<nSt;++st) {
          h[st][0]=conj(h[st][0]);
          h[st][1]=conj(h[st][1]);
          h[st][2]=conj(h[st][2]);
          h[st][3]=conj(h[st][3]);

          w[st] =vector<DComplex>(4);
          tt[st]=vector<DComplex>(4);
        }

        for (uint bl=0;bl<nBl;++bl) {
          // Assume nCh = 1 for now

          uint ant1=info().getAnt1()[bl];
          uint ant2=info().getAnt2()[bl];
          if (ant1==ant2 || flag[bl*nCr]) {
            continue;
          }

          DComplex vis[4];
          for (uint cr=0;cr<4;++cr) {
            vis[cr] = data[bl*nCr+cr];
          }
          //Complex *vis =&data[bl*nCr];
          dcomplex *mvis=&storage.model[bl*nCr];

          // Upper diagonal, ant1 < ant2
          //cout<<"mvis0"<<mvis[0]<<endl;
          z[0] = h[ant1][0] * mvis[0] + h[ant1][2] * mvis[2];
          z[1] = h[ant1][0] * mvis[1] + h[ant1][2] * mvis[3];
          z[2] = h[ant1][1] * mvis[0] + h[ant1][3] * mvis[2];
          z[3] = h[ant1][1] * mvis[1] + h[ant1][3] * mvis[3];

          w[ant2][0] += conj(z[0]) * z[0] + conj(z[2]) * z[2];
          w[ant2][1] += conj(z[0]) * z[1] + conj(z[2]) * z[3];
          w[ant2][2] += conj(z[1]) * z[0] + conj(z[3]) * z[2];
          w[ant2][3] += conj(z[1]) * z[1] + conj(z[3]) * z[3];

          tt[ant2][0] += conj(z[0]) * vis[0] + conj(z[2]) * vis[2];
          tt[ant2][1] += conj(z[0]) * vis[1] + conj(z[2]) * vis[3];
          tt[ant2][2] += conj(z[1]) * vis[0] + conj(z[3]) * vis[2];
          tt[ant2][3] += conj(z[1]) * vis[1] + conj(z[3]) * vis[3];

          // Lower diagonal, ant1 > ant2
          z[0] = h[ant2][0] * conj(mvis[0]) + h[ant2][2] * conj(mvis[1]);
          z[1] = h[ant2][0] * conj(mvis[2]) + h[ant2][2] * conj(mvis[3]);
          z[2] = h[ant2][1] * conj(mvis[0]) + h[ant2][3] * conj(mvis[1]);
          z[3] = h[ant2][1] * conj(mvis[2]) + h[ant2][3] * conj(mvis[3]);

          w[ant1][0] += conj(z[0]) * z[0] + conj(z[2]) * z[2];
          w[ant1][1] += conj(z[0]) * z[1] + conj(z[2]) * z[3];
          w[ant1][2] += conj(z[1]) * z[0] + conj(z[3]) * z[2];
          w[ant1][3] += conj(z[1]) * z[1] + conj(z[3]) * z[3];

          tt[ant1][0] += conj(z[0] * vis[0] + z[2] * vis[1]);
          tt[ant1][1] += conj(z[0] * vis[2] + z[2] * vis[3]);
          tt[ant1][2] += conj(z[1] * vis[0] + z[3] * vis[1]);
          tt[ant1][3] += conj(z[1] * vis[2] + z[3] * vis[3]);
        }


        for (uint ant2=0;ant2<nSt;++ant2) {
          //cout<<"g["<<ant2<<"]={"<<g[ant2][0]<<", "<<g[ant2][1]<<", "<<g[ant2][2]<<", "<<g[ant2][3]<<"}"<<endl;
          //cout<<"w["<<ant2<<"]={"<<w[ant2][0]<<", "<<w[ant2][1]<<", "<<w[ant2][2]<<", "<<w[ant2][3]<<"}"<<endl;
        }
        for (uint ant2=0;ant2<nSt;++ant2) {
          //cout<<"tt["<<ant2<<"]={"<<tt[ant2][0]<<", "<<tt[ant2][1]<<", "<<tt[ant2][2]<<", "<<tt[ant2][3]<<"}"<<endl;
        }

        for (uint ant=0;ant<nSt;++ant) {
          DComplex invdet= 1./(w[ant][0] * w [ant][3] - w[ant][1]*w[ant][2]);
          g[ant][0] = invdet * ( w[ant][3] * tt[ant][0] - w[ant][1] * tt[ant][2] );
          g[ant][1] = invdet * ( w[ant][3] * tt[ant][1] - w[ant][1] * tt[ant][3] );
          g[ant][2] = invdet * ( w[ant][0] * tt[ant][2] - w[ant][2] * tt[ant][0] );
          g[ant][3] = invdet * ( w[ant][0] * tt[ant][3] - w[ant][2] * tt[ant][1] );
        }

        if (iter % 2 == 1) {
          dgxx = dgx;
          dgx  = dg;

          double fronormdiff=0;
          double fronormg=0;
          for (uint ant=0;ant<nSt;++ant) {
            for (uint cr=0;cr<nCr;++cr) {
              DComplex diff=g[ant][cr]-gold[ant][cr];
              fronormdiff+=abs(diff*diff);
              fronormg+=abs(g[ant][cr]*g[ant][cr]);
            }
          }
          fronormdiff=sqrt(fronormdiff);
          fronormg=sqrt(fronormg);

          dg = fronormdiff/fronormg;

          if (dg <= tol) {
            break;
          }

          for (uint ant=0;ant<nSt;++ant) {
            for (uint cr=0;cr<nCr;++cr) {
              g[ant][cr] = (1-omega) * g[ant][cr] + omega * gold[ant][cr];
            }
          }

          if (!threestep) {
            threestep = (iter+1 >= nomega) ||
                ( max(dg,max(dgx,dgxx)) <= 1.0e-3 && dg<dgx && dgx<dgxx);
            //cout<<"Threestep="<<boolalpha<<threestep<<endl;
          }

          if (threestep) {
            //cout<<"threestep"<<endl;
            if (sstep <= 0) {
              if (dg <= c1 * dgx) {
                //cout<<"dg<=c1*dgx"<<endl;
                for (uint ant=0;ant<nSt;++ant) {
                  for (uint cr=0;cr<nCr;++cr) {
                    g[ant][cr] = f1q * g[ant][cr] + f2q * gx[ant][cr];
                  }
                }
              } else if (dg <= dgx) {
                //cout<<"dg<=dgx"<<endl;
                for (uint ant=0;ant<nSt;++ant) {
                  for (uint cr=0;cr<nCr;++cr) {
                    g[ant][cr] = f1 * g[ant][cr] + f2 * gx[ant][cr] + f3 * gxx[ant][cr];
                  }
                }
              } else if (dg <= c2 *dgx) {
                //cout<<"dg<=c2*dgx"<<endl;
                g = gx;
                sstep = 1;
              } else {
                //cout<<"else"<<endl;
                g = gxx;
                sstep = 2;
              }
            } else {
              //cout<<"no sstep"<<endl;
              sstep = sstep - 1;
            }
          }
          gxx = gx;
          gx = g;
        }
      }
      if (dg > tol) {
        cerr<<"!";
      }

      //cout<<"iter:"<<iter<<"?"<<endl;

      DComplex p = conj(g[0][0])/abs(g[0][0]);
      // Set phase of first gain to zero
      for (uint st=0;st<nSt;++st) {
        for (uint cr=0;cr<nCr;++cr) {
           g[st][cr]*=p;
        }
      }

      // Stefcal terminated (either by maxiter or by converging)
      // Let's save G...
      itsSols.push_back(g);


      itsTimer.stop();
      getNextStep()->process(buf);
      return false;
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
#pragma omp parallel for
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
      itsTimer.start();
      uint nSt=info().antennaUsed().size();

      uint ntime=itsSols.size();

      // Construct solution grid.
      const Vector<double>& freq      = getInfo().chanFreqs();
      const Vector<double>& freqWidth = getInfo().chanWidths();
      BBS::Axis::ShPtr freqAxis(new BBS::RegularAxis(freq[0] - freqWidth[0]
        * 0.5, freqWidth[0], 1));
      BBS::Axis::ShPtr timeAxis(new BBS::RegularAxis
                                (info().startTime(),
                                 info().timeInterval(), info().ntime()));
      BBS::Grid solGrid(freqAxis, timeAxis);
      // Create domain grid.
      BBS::Axis::ShPtr tdomAxis(new BBS::RegularAxis
                                (info().startTime(),
                                 info().timeInterval() * info().ntime(), 1));
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
      const char* str01[] = {"0:","1:"};
      const char* strri[] = {"Real:","Imag:"};
      Matrix<double> values(1, ntime);

      for (size_t st=0; st<nSt; ++st) {
        uint seqnr = 0;
        string suffix(itsAntennaUsedNames[st]);

        for (int i=0; i<2; ++i) {
          for (int j=0; j<2; j++) {
            for (int k=0; k<2; ++k) {
              string name(string("Gain:") +
                          str01[i] + str01[j] + strri[k] + suffix);
              // Collect its solutions for all times in a single array.
              for (uint ts=0; ts<ntime; ++ts) {
                if (seqnr%2==0) {
                  values(0, ts) = real(itsSols[ts][st][seqnr/2]);
                } else {
                  values(0, ts) = imag(itsSols[ts][st][seqnr/2]);
                }
              }
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
      }
      itsTimer.stop();
      // Let the next steps finish.
      getNextStep()->finish();
    }

  } //# end namespace
}

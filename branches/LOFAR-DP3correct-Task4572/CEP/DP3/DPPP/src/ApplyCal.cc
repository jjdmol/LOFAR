//# ApplyCal.cc: DPPP step class to apply a calibration correction to the data
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
//# $Id: ApplyCal.cc 21598 2012-07-16 08:07:34Z diepen $
//#
//# @author Ger van Diepen

#include <lofar_config.h>
#include <DPPP/ApplyCal.h>
#include <DPPP/DPBuffer.h>
#include <DPPP/DPInfo.h>
#include <Common/ParameterSet.h>
#include <Common/StringUtil.h>
#include <Common/LofarLogger.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/OS/File.h>
#include <iostream>
#include <iomanip>

using namespace casa;
using namespace LOFAR::BBS;

/// Look at BBSKernel MeasurementExprLOFARUtil.cc and Apply.cc

namespace LOFAR {
  namespace DPPP {

    ApplyCal::ApplyCal (DPInput* input,
                        const ParameterSet& parset,
                        const string& prefix)
      : itsInput       (input),
        itsName        (prefix),
        itsParmDBName  (parset.getString (prefix + "parmdb")),
        itsCorrectType (parset.getString (prefix + "correction")),
        itsSigma       (parset.getDouble (prefix + "sigma", 0.)),
        itsParmCache   (itsParmSet),
        itsTimeInterval (-1),
        itsUseAP       (False),
        itsNChan       (0),
        itsNPol        (0)
    {
      ASSERT (!itsParmDBName.empty());
      // Possible corrections one (or more?) of:
      //   Gain (real/imag or ampl/phase), RM, TEC, Clock, Bandpass
    }

    ApplyCal::~ApplyCal()
    {}

    void ApplyCal::updateInfo (const DPInfo& infoIn)
    {
      info() = infoIn;
      info().setNeedVisData();
      info().setNeedWrite();
      itsTimeInterval = infoIn.timeInterval();

      // Open the ParmDB.
      File parmdbFile(itsParmDBName);

      ASSERTSTR (parmdbFile.exists(), "ParmDB " + itsParmDBName +
                 " does not exist");
      BBS::ParmDBMeta pdb("casa", itsParmDBName);
      ///      // Use ParmFacade to get corrections in correct grid?
      itsParmDB.reset(new BBS::ParmDB(pdb));


      //vector<string> parnames;
      //parnames=itsParmDB->getNames("*");

      /*
      for (vector<string>::iterator it = parnames.begin();it!=parnames.end();++it) {
        cout << *it <<endl;
      }
      cout << "dat waren de parameters" << endl;
      */

      // Form the frequency axis for this time slot.
      vector<double> freqs, freqWidths;
      freqs.resize (infoIn.chanFreqs().size());
      freqWidths.resize (freqs.size());
      infoIn.chanFreqs().tovector  (freqs);
      infoIn.chanWidths().tovector (freqWidths);
      itsFreqAxis = BBS::Axis::ShPtr (new BBS::OrderedAxis(freqs, freqWidths));

      // Handle the correction type.
      // Form the Parm objects for all parameters involved.

      string corrType = toLower(itsCorrectType);

      if (corrType == "clock") {
        fillParms ("Clock:");
      } else if (corrType == "gain") {
        string prefix1 = "Real:";
        string prefix2 = "Imag:";
        // Test if real/imag or ampl/phase is used.
        if (itsParmDB->getNameId("Gain:0:0:Real:" +
                                 infoIn.antennaNames()[0]) < 0) {
          prefix1  = "Ampl:";
          prefix2  = "Phase:";
          itsUseAP = true;
        }
        fillParms ("Gain:0:0:" + prefix1);
        fillParms ("Gain:0:0:" + prefix2);
        //fillParms ("Gain:0:1:" + prefix1);
        //fillParms ("Gain:0:1:" + prefix2);
        //fillParms ("Gain:1:0:" + prefix1);
        //fillParms ("Gain:1:0:" + prefix2);
        fillParms ("Gain:1:1:" + prefix1);
        fillParms ("Gain:1:1:" + prefix2);
      } else if (corrType == "rm") {
        fillParms ("RotationMeasure:");
      } else if (corrType == "tec") {
        fillParms ("TEC:");
      } else if (corrType == "bandpass") {
        fillParms ("Bandpass:0:0:");
        fillParms ("Bandpass:1:1:");
      } else {
        THROW (Exception, "Correction type " + itsCorrectType +
                         " is unknown");
      }
    }

    void ApplyCal::fillParms (const string& parmPrefix)
    {
      vector<Parm>& parms = itsParms[parmPrefix];
      ASSERTSTR (parms.empty(), "Parm " + parmPrefix + " multiply used");
      parms.reserve (info().antennaNames().size());
      for (uint i=0; i<info().antennaNames().size(); ++i) {
        string name = casa::String(parmPrefix) + info().antennaNames()[i];
        ASSERTSTR (itsParmDB->getNameId(name) >= 0,
                   "ParmDB parm " + name + " does not exist");
        ParmId id = itsParmSet.addParm(*itsParmDB, name);
        parms.push_back(Parm(itsParmCache, id));
      }
    }

    void ApplyCal::show (std::ostream& os) const
    {
      os << "ApplyCal " << itsName << std::endl;
      os << "  parmdb:         " << itsParmDBName << endl;
      os << "  correction:     " << itsCorrectType << endl;
      os << "  sigma:          " << itsSigma << endl;
    }

    void ApplyCal::showTimings (std::ostream& os, double duration) const
    {
      os << "  ";
      FlagCounter::showPerc1 (os, itsTimer.getElapsed(), duration);
      os << " ApplyCal " << itsName << endl;
    }

    bool ApplyCal::process (const DPBuffer& bufin)
    {
      itsTimer.start();
      DPBuffer buf(bufin);
      buf.getData().unique();
      RefRows rowNrs(buf.getRowNrs());

      // If needed, cache parm values for the next 100 time slots.
      // getTime geeft midden van interval
      double stime = buf.getTime() - 0.5*itsTimeInterval;


      /*
      if (stime > itsLastTime) {
        itsLastTime = stime + 100*itsTimeInterval;
        BBS::Box domain(make_pair(stime, 0.), make_pair(itsLastTime, 1e10));
        itsParmCache.reset(domain);
      }
      */

      // Form the grid for this time slot.
      Axis::ShPtr timeAxis (new BBS::RegularAxis(stime, itsTimeInterval, 1));

      // Loop through all baselines in the buffer.
      int nbl = bufin.getData().shape()[2];

      //as wordt grid, dan voor ieder van parms waarde ophalen

      //#pragma omp parallel for
      for (int i=0; i<nbl; ++i) {
        correct (buf, i);
      }
      itsTimer.stop();
      getNextStep()->process(buf);
      return false;
    }

    void ApplyCal::finish()
    {
      // Let the next steps finish.
      getNextStep()->finish();
    }

    void ApplyCal::correct (DPBuffer& buf, int bl)
    {
      Complex* data = buf.getData().data();
      int npol  = buf.getData().shape()[0];
      int nchan = buf.getData().shape()[1];

      for (int i=bl*npol*nchan;i<(bl+1)*npol*nchan;i++) {
        cout<<data[i]<<endl;
      }

      //rmParm.getResult (coeffs, grid);
    }

    // Corrections can be constant or can vary in freq.
    // TODO: this is a scalar effect, so should not be implemented as matrix
    void ApplyCal::applyTEC (Complex* vis, const DComplex& tec)
    {
      ///Matrix phase = (tec * -8.44797245e9) / freq;
      ///Matrix shift = tocomplex(cos(phase), sin(phase));
    }

    void ApplyCal::applyClock (Complex* vis, const DComplex* lhs,
                               const DComplex* rhs)
    {
      ///Matrix phase = freq * (delay() * casa::C::_2pi);
      ///Matrix shift = tocomplex(cos(phase), sin(phase));
      for (uint i=0; i<itsNChan; ++i) {
        DComplex factor = 1. / (lhs[i] * conj(rhs[i]));
        for (uint j=0; j<itsNPol; ++j) {
          *vis++ *= factor;
        }
      }
    }

    void ApplyCal::applyBandpass (Complex* vis, const DComplex* lhs,
                                  const DComplex* rhs)
    {
    }

    void ApplyCal::applyRM (Complex* vis, const DComplex* lhs,
                            const DComplex* rhs)
    {
      // Precompute lambda squared for the current frequency point.
      /*
      const double lambda = C::c / grid[FREQ]->center(f);
      const double lambda2 = lambda * lambda;

      double *sample = origin + f;
      for (unsigned int t = 0; t < nTime; ++t) {
        *sample = lambda2;
        sample += nFreq;
      }

      Matrix chi = rm() * lambdaSqr;
      Matrix cosChi = cos(chi);
      Matrix sinChi = sin(chi);

      JonesMatrix::View result;
      result.assign(0, 0, cosChi);
      result.assign(0, 1, -sinChi);
      result.assign(1, 0, sinChi);
      result.assign(1, 1, cosChi);
      */
    }

    void ApplyCal::applyJones (Complex* vis, const DComplex* lhs,
                               const DComplex* rhs)
    {
      for (uint i=0; i<itsNChan; ++i) {
        // Compute the Mueller matrix.

        DComplex mueller[4][4];
        mueller[0][0] = lhs[0] * conj(rhs[0]);
        mueller[0][1] = lhs[0] * conj(rhs[1]);
        mueller[1][0] = lhs[0] * conj(rhs[2]);
        mueller[1][1] = lhs[0] * conj(rhs[3]);

        mueller[0][2] = lhs[1] * conj(rhs[0]);
        mueller[0][3] = lhs[1] * conj(rhs[1]);
        mueller[1][2] = lhs[1] * conj(rhs[2]);
        mueller[1][3] = lhs[1] * conj(rhs[3]);

        mueller[2][0] = lhs[2] * conj(rhs[0]);
        mueller[2][1] = lhs[2] * conj(rhs[1]);
        mueller[3][0] = lhs[2] * conj(rhs[2]);
        mueller[3][1] = lhs[2] * conj(rhs[3]);

        mueller[2][2] = lhs[3] * conj(rhs[0]);
        mueller[2][3] = lhs[3] * conj(rhs[1]);
        mueller[3][2] = lhs[3] * conj(rhs[2]);
        mueller[3][3] = lhs[3] * conj(rhs[3]);

        // Apply Mueller matrix to visibilities.
        DComplex xx, xy, yx, yy;
        xx = (mueller[0][0] * DComplex(vis[0]) +
              mueller[0][1] * DComplex(vis[1]) +
              mueller[0][2] * DComplex(vis[2]) +
              mueller[0][3] * DComplex(vis[3]));

        xy = (mueller[1][0] * DComplex(vis[0]) +
              mueller[1][1] * DComplex(vis[1]) +
              mueller[1][2] * DComplex(vis[2]) +
              mueller[1][3] * DComplex(vis[3]));

        yx = (mueller[2][0] * DComplex(vis[0]) +
              mueller[2][1] * DComplex(vis[1]) +
              mueller[2][2] * DComplex(vis[2]) +
              mueller[2][3] * DComplex(vis[3]));

        yy = (mueller[3][0] * DComplex(vis[0]) +
              mueller[3][1] * DComplex(vis[1]) +
              mueller[3][2] * DComplex(vis[2]) +
              mueller[3][3] * DComplex(vis[3]));

        vis[0] = xx;
        vis[1] = xy;
        vis[2] = yx;
        vis[3] = yy;
        vis += 4;

      }
    }

    // Inverts complex input matrix (in place??)
    // TODO: what does this sigma term do? It is added, should it be added back?
    void ApplyCal::invert (DComplex* v)
    {
      // Add the variance of the nuisance term to the elements on the diagonal.
      const double variance = 0;//itsSigma * itsSigma;
      DComplex v0 = v[0] + variance;
      DComplex v3 = v[3] + variance;
      // Compute inverse in the usual way.
      DComplex invDet(1.0 / (v0 * v3 - v[1] * v[2]));
      v[0] = v3 * invDet;
      v[2] = v[2] * -invDet;
      v[1] = v[1] * -invDet;
      v[3] = v0 * invDet;
    }

  } //# end namespace
}

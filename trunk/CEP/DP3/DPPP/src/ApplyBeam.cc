//# ApplyBeam.cc: DPPP step class to ApplyBeam visibilities
//# Copyright (C) 2015
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
#include <DPPP/ApplyBeam.h>

#include <iostream>
//#include <iomanip>
#include <Common/ParameterSet.h>
#include <Common/Timer.h>
#include <Common/StringUtil.h>
#include <Common/OpenMP.h>
#include <DPPP/DPInfo.h>
#include <DPPP/FlagCounter.h>
#include <DPPP/Position.h>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasConvert.h>

#include <stddef.h>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

using namespace casa;
using namespace LOFAR::BBS;

namespace LOFAR {
  namespace DPPP {

    ApplyBeam::ApplyBeam(DPInput* input, const ParameterSet& parset,
                     const string& prefix, bool substep)
        :
          itsInput(input),
          itsName(prefix),
          itsUseChannelFreq(parset.getBool(prefix + "usechannelfreq", true)),
          itsDebugLevel(parset.getInt(prefix + "debuglevel", 0))
    {
      // only read 'invert' parset key if it is a separate step
      // if applybeam is called from gaincal/predict, the invert key should always be false
      if (substep) {
        itsInvert=false;
      } else {
        itsInvert=parset.getBool(prefix + "invert", true);
      }
      string mode=toLower(parset.getString(prefix + "beammode","default"));
      ASSERT (mode=="default" || mode=="array_factor" || mode=="element");
      if (mode=="default") {
        itsMode=DEFAULT;
      } else if (mode=="array_factor") {
        itsMode=ARRAY_FACTOR;
      } else if (mode=="element") {
        itsMode=ELEMENT;
      } else {
        THROW(Exception, "Beammode should be DEFAULT, ARRAY_FACTOR or ELEMENT");
      }
    }

    ApplyBeam::ApplyBeam()
    {
    }

    ApplyBeam::~ApplyBeam()
    {
    }

    void ApplyBeam::updateInfo(const DPInfo& infoIn)
    {
      info() = infoIn;
      info().setNeedVisData();
      info().setWriteData();

      MDirection dirJ2000(
          MDirection::Convert(infoIn.phaseCenter(), MDirection::J2000)());
      Quantum<Vector<Double> > angles = dirJ2000.getAngle();
      itsPhaseRef = Position(angles.getBaseValue()[0],
                             angles.getBaseValue()[1]);

      const size_t nSt = info().nantenna();
      const size_t nCh = info().nchan();

      itsBeamValues.resize(OpenMP::maxThreads());

      // Create the Measure ITRF conversion info given the array position.
      // The time and direction are filled in later.
      itsMeasConverters.resize(OpenMP::maxThreads());
      itsMeasFrames.resize(OpenMP::maxThreads());
      itsAntBeamInfo.resize(OpenMP::maxThreads());

      for (uint thread = 0; thread < OpenMP::maxThreads(); ++thread) {
        itsBeamValues[thread].resize(nSt * nCh);
        itsMeasFrames[thread].set(info().arrayPosCopy());
        itsMeasFrames[thread].set(
            MEpoch(MVEpoch(info().startTime() / 86400), MEpoch::UTC));
        itsMeasConverters[thread].set(
            MDirection::J2000,
            MDirection::Ref(MDirection::ITRF, itsMeasFrames[thread]));
        itsInput->fillBeamInfo(itsAntBeamInfo[thread], info().antennaNames());
      }
    }

    void ApplyBeam::show(std::ostream& os) const
    {
      os << "ApplyBeam " << itsName << endl;
      os << "  mode:              ";
      if (itsMode==DEFAULT)
        os<<"default";
      else if (itsMode==ARRAY_FACTOR)
        os<<"array_factor";
      else os<<"element";
      os << endl;
      os << "  use channelfreq:   " << boolalpha << itsUseChannelFreq << endl;
      os << "  invert:            " << boolalpha << itsInvert << endl;
    }

    void ApplyBeam::showTimings(std::ostream& os, double duration) const
    {
      os << "  ";
      FlagCounter::showPerc1(os, itsTimer.getElapsed(), duration);
      os << " ApplyBeam " << itsName << endl;
    }

    bool ApplyBeam::process(const DPBuffer& bufin)
    {
      itsTimer.start();
      itsBuffer.copy (bufin);
      Complex* data=itsBuffer.getData().data();

      double time = itsBuffer.getTime();

      //Set up directions for beam evaluation
      StationResponse::vector3r_t refdir, tiledir;

      for (uint thread = 0; thread < OpenMP::maxThreads(); ++thread) {
        itsMeasFrames[thread].resetEpoch(
            MEpoch(MVEpoch(time / 86400), MEpoch::UTC));
        //Do a conversion on all threads, because converters are not
        //thread safe and apparently need to be used at least once
        refdir = dir2Itrf(info().delayCenter(), itsMeasConverters[thread]);
        tiledir = dir2Itrf(info().tileBeamDir(), itsMeasConverters[thread]);
      }

      uint thread = OpenMP::threadNum();

      StationResponse::vector3r_t srcdir = refdir;
      applyBeam(info(), time, data, srcdir, refdir, tiledir,
                itsAntBeamInfo[thread], itsBeamValues[thread],
                itsUseChannelFreq, itsInvert, itsMode);

      itsTimer.stop();
      getNextStep()->process(itsBuffer);
      return false;
    }

    StationResponse::vector3r_t ApplyBeam::dir2Itrf(
        const MDirection& dir, MDirection::Convert& measConverter)
    {
      const MDirection& itrfDir = measConverter(dir);
      const Vector<Double>& itrf = itrfDir.getValue().getValue();
      StationResponse::vector3r_t vec;
      vec[0] = itrf[0];
      vec[1] = itrf[1];
      vec[2] = itrf[2];
      return vec;
    }

    void ApplyBeam::finish()
    {
      // Let the next steps finish.
      getNextStep()->finish();
    }
  } //# end namespace
}

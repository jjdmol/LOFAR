//#  tDelayCompensation.cc: stand-alone test program for WH_DelayCompensation
//#
//#  Copyright (C) 2009
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <WH_DelayCompensation.h>
#include <Interface/Parset.h>
#include <AMCBase/Direction.h>
#include <Common/Exception.h>
#include <Interface/RSPTimeStamp.h>

#include <cassert>

using namespace LOFAR;
using namespace LOFAR::RTCP;

void doTest()
{
  unsigned psetNumber = 0;

  Parset parset("tDelayCompensation.parset");

  std::vector<Parset::StationRSPpair> inputs = parset.getStationNamesAndRSPboardNumbers(psetNumber);

  unsigned nrBeams    = parset.nrBeams();
  double   startTime  = parset.startTime();
  double   sampleFreq = parset.sampleRate();
  unsigned seconds    = static_cast<unsigned>(floor(startTime));
  unsigned samples    = static_cast<unsigned>((startTime - floor(startTime)) * sampleFreq);

  TimeStamp ts = TimeStamp(seconds, samples, parset.clockSpeed());

  WH_DelayCompensation w(&parset, inputs[0].station, ts);

  unsigned nrPencilBeams = 1;
  Matrix<double> delays(nrBeams, nrPencilBeams);
  Matrix<AMC::Direction> prev_directions(nrBeams, nrPencilBeams), directions(nrBeams, nrPencilBeams);
 
  for (unsigned i = 0; i < 256; i ++) {
    prev_directions = directions;

    w.getNextDelays(directions, delays);
    cout << "Directions & Delay: " << directions[0][0] << ", " << delays[0][0] << endl;

    assert(!isnan(delays[0][0]));

    // source (NCP) should traverse with decreasing longitude and latitude
    if (i > 0) {
      assert(directions[0][0].longitude() < prev_directions[0][0].longitude());
      assert(directions[0][0].latitude() < prev_directions[0][0].latitude());
    }
  }
}


int main()
{
  try {
    doTest();
  } catch (Exception &ex) {
    std::cerr << "Caught Exception: " << ex.what() << std::endl;
    return 1;
  } catch (std::exception &ex) {
    std::cerr << "Caught std::exception: " << ex.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Caught unknown exception" << std::endl;
    return 1;
  }

  return 0;
}

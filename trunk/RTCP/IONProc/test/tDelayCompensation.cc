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

#include <cassert>

using namespace LOFAR;
using namespace LOFAR::RTCP;

void doTest()
{
  const unsigned psetNumber = 0;

  ParameterSet pset("tDelayCompensation.parset");
  Parset       ps(&pset);

  std::vector<Parset::StationRSPpair> inputs = ps.getStationNamesAndRSPboardNumbers(psetNumber);
  const unsigned  nrBeams = ps.nrBeams();
  const double startTime = ps.startTime();
  const double sampleFreq = ps.sampleRate();
  const unsigned seconds    = static_cast<unsigned>(floor(startTime));
  const unsigned samples    = static_cast<unsigned>((startTime - floor(startTime)) * sampleFreq);

  TimeStamp ts = TimeStamp(seconds, samples);
  WH_DelayCompensation w( &ps, inputs[0].station, ts );

  std::vector<double> delays(nrBeams);
  std::vector<AMC::Direction> prev_directions(nrBeams),directions(nrBeams);
 
  for( unsigned i = 0; i < 256; i++ ) {
    prev_directions = directions;

    w.getNextDelays( directions, delays );
    cout << "Directions & Delay: " << directions[0] << ", " << delays[0] << endl;

    assert( !isnan( delays[0] ) );

    // source (NCP) should traverse with decreasing longitude and latitude
    if( i > 0 ) {
      assert( directions[0].longitude() < prev_directions[0].longitude() );
      assert( directions[0].latitude() < prev_directions[0].latitude() );
    }
  }
}

int main (int argc, char **argv)
{
  int retval = 0;

#if defined HAVE_MPI
  MPI_Init(&argc, &argv);
#else
  argc = argc; argv = argv;    // Keep compiler happy ;-)
#endif

  try {
    doTest();
  } catch (Exception& e) {
    std::cerr << "Caught Exception: " << e.what() << std::endl;
    retval = 1;
/*
  } catch (std::exception& e) {
    std::cerr << "Caught std::exception: " << e.what() << std::endl;
    retval = 1;
  } catch (...) {
    std::cerr << "Caught unknown exception " << std::endl;
    retval = 1;
*/
  }

#if defined HAVE_MPI
  MPI_Finalize();
#endif

  return retval;
}

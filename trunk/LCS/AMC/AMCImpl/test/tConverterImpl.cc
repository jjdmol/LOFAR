//#  tConverterImpl.cc: test program for the ConverterImpl class.
//#
//#  Copyright (C) 2002-2004
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
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <AMCImpl/ConverterImpl.h>
#include <AMCBase/SkyCoord.h>
#include <AMCBase/EarthCoord.h>
#include <AMCBase/TimeCoord.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <iostream>

using namespace LOFAR::AMC;
using namespace LOFAR;
using namespace std;

int main(int, const char* const argv[])
{
  INIT_LOGGER(argv[0]);

  try {

    SkyCoord sky(1,0.4);
    ASSERT (sky.angle0() == 1);
    ASSERT (sky.angle1() == 0.4);
    cout << sky << endl;
    EarthCoord pos(0.2, 1, 3);
    ASSERT (pos.longitude() == 0.2);
    ASSERT (pos.latitude() == 1);
    ASSERT (pos.height() == 3);
    cout << pos << endl;
    EarthCoord dwl(0.111646531, 0.921760253, 25);
    cout << TimeCoord::getUTCDiff() << endl;
    TimeCoord time(10.5);
    ASSERT (time.mjd() == 10.5);
    TimeCoord someTime(2004, 11, 19, 15, 22);
    cout << someTime.mjd() << endl;
    cout << someTime << endl;
    TimeCoord someTime2 (someTime.mjd(), 1./24);
    cout << someTime2 << endl;
    TimeCoord someTime3 (someTime.mjd(), 1.);

    ConverterImpl conv;

    {
      SkyCoord result = conv.j2000ToAzel (sky, dwl, someTime);
      cout << result << endl;
      result = conv.azelToJ2000 (result, dwl, someTime);
      cout << result << endl;
    }

    {
      vector<SkyCoord> skies(4);
      vector<EarthCoord> poss(2);
      vector<TimeCoord> times(3);
      skies[0] = SkyCoord(1,0.3);
      skies[1] = SkyCoord(-1,-0.3);
      skies[2] = SkyCoord(2,0.1);
      skies[3] = SkyCoord(1.5,-0.3);
      poss[0] =  EarthCoord (0.1, 0.8, 1000);
      poss[1] = dwl;
      times[0] = someTime;
      times[1] = someTime2;
      times[2] = someTime3;
      vector<SkyCoord> result = conv.j2000ToAzel (skies, poss, times);
      ASSERT (result.size() == 4*2*3);
      for (unsigned int i=0; i<4*2*3; i++) {
	cout << result[i] << endl;
      }
      int inx=0;
      for (unsigned int i=0; i<3; i++) {
	for (unsigned int j=0; j<2; j++) {
	  vector<SkyCoord> sk(4);
	  for (unsigned int k=0; k<4; k++) {
	    sk[k] = result[inx++];
	  }
	  vector<SkyCoord> res2 = conv.azelToJ2000 (sk, poss[j], times[i]);
	  for (unsigned int k=0; k<4; k++) {
	    cout << res2[k];
	  }
	  cout << endl;
	}
      }
    }

  } catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }

  cout << "OK" << endl;
  return 0;
  
}

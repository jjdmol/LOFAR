//#  tConverter.cc: test program for the AMC converter classes.
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
#include <AMCBase/SkyCoord.h>
#include <AMCBase/EarthCoord.h>
#include <AMCBase/TimeCoord.h>
#include <AMCBase/AMCClient/ConverterClient.h>
#include <AMCImpl/ConverterImpl.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_fstream.h>

using namespace LOFAR;
using namespace LOFAR::AMC;

// Maximum allowable error (absolute or relative).
const double epsilon = 1.2e-12;


bool compare(double x, double y, 
             double eps = std::numeric_limits<double>::epsilon())
{
  if (x == y) return true;
  if (fabs(x-y) <= eps) return true;
  if (fabs(x) > fabs(y)) return (fabs((x-y)/x) <= eps);
  else return (fabs((x-y)/y) <= eps);
}


template<typename T>
ostream& operator<<(ostream& os, const vector<T>& v)
{
  for (uint i=0; i<v.size(); i++) {
    os << endl << "  [" << i << "]: " << v[i];
  }
  return os;
}


Converter* createConverter()
{
  ifstream ifs("tConverter.in");
  if (!ifs) {
    LOG_ERROR("Failed to open file \"tConverter.in\" for reading");
    return 0;
  }

  string type;
  string host("localhost");
  uint16 port(31337);

  ifs >> type >> host >> port;

  if (type == "impl" || type == "IMPL" || type == "Impl") 
    return new ConverterImpl();
  if (type == "client" || type == "CLIENT" || type == "Client")
    return new ConverterClient(host, port);

  LOG_ERROR_STR("Invalid converter type (" << type << ") specified");
  return 0;
}


int main(int /*argc*/, const char* const argv[])
{
  INIT_LOGGER(argv[0]);

  try {

    Converter* conv = createConverter();
    if (conv == 0) return 1;
    
    vector<SkyCoord>   skies(4);
    vector<EarthCoord> poss(2);
    vector<TimeCoord>  times(3);

    // Four arbitrary directions (angles in radians).
    skies[0] = SkyCoord (1, 0.3);
    skies[1] = SkyCoord (-1, -0.3);
    skies[2] = SkyCoord (2, 0.1);
    skies[3] = SkyCoord (1.5, -0.3);

    poss[0] =  // arbitrary position
      EarthCoord (0.1, 0.8, 1000, EarthCoord::WGS84);
    poss[1] =  // Dwingeloo observatory
      EarthCoord (0.111646531, 0.921760253, 25, EarthCoord::WGS84);

    times[0] = // arbitrary time (added 0.1 usec to avoid rouding errors)
      TimeCoord (2004, 11, 19, 15, 22, 39.0000001);
    times[1] = // same arbitrary time + 1 hour
      TimeCoord (times[0].mjd(), 1./24);
    times[2] = // same arbitrary time + 1 day
      TimeCoord (times[0].mjd(), 1.);

    cout.precision(9);

    cout << "**** Original data ****" << endl;
    cout << "skies = " << skies << endl;
    cout << "poss = " << poss << endl;
    cout << "times = " << times << endl;


    // Check conversion J2000 --> AZEL and vice versa
    {
      vector<SkyCoord> result = conv->j2000ToAzel (skies, poss, times);
      ASSERT (result.size() == skies.size() * poss.size() * times.size());
      cout << "\n**** Convert from J2000 to AZEL and vice versa ****\n";
      cout << "result = " << result << endl;
      for (uint i = 0; i < skies.size() * poss.size() * times.size(); i++) {
        ASSERT (result[i].type() == SkyCoord::AZEL);
      }
      int inx=0;
      for (uint i = 0; i < times.size(); i++) {
        for (uint j = 0; j < poss.size(); j++) {
          vector<SkyCoord> sk(skies.size());
          for (uint k = 0; k < skies.size(); k++) {
            sk[k] = result[inx++];
          }
          // Convert back from AZEL to J2000; should yield original data
          vector<SkyCoord> res2 = conv->azelToJ2000 (sk, poss[j], times[i]);
          for (uint k = 0; k < skies.size(); k++) {
            ASSERT (res2[k].type() == SkyCoord::J2000);
            ASSERT (compare(res2[k].angle0(), skies[k].angle0(), epsilon));
            ASSERT (compare(res2[k].angle1(), skies[k].angle1(), epsilon));
          }
        }
      }
    }    

    // Check conversion J2000 --> ITRF 
    {
      vector<SkyCoord> result = conv->j2000ToItrf (skies, poss, times);
      ASSERT (result.size() == skies.size() * poss.size() * times.size());
      cout << "\n**** Convert from J2000 to ITRF and vice versa ****\n";
      cout << "result = " << result << endl;
      for (uint i = 0; i < skies.size() * poss.size() * times.size(); i++) {
        ASSERT (result[i].type() == SkyCoord::ITRF);
      }
      int inx=0;
      for (uint i = 0; i < times.size(); i++) {
        for (uint j = 0; j < poss.size(); j++) {
          vector<SkyCoord> sk(skies.size());
          for (uint k = 0; k < skies.size(); k++) {
            sk[k] = result[inx++];
          }
          // Convert back from ITRF to J2000; should yield original data
          vector<SkyCoord> res2 = conv->itrfToJ2000 (sk, poss[j], times[i]);
          for (uint k = 0; k < skies.size(); k++) {
            ASSERT (res2[k].type() == SkyCoord::J2000);
            ASSERT (compare(res2[k].angle0(), skies[k].angle0(), epsilon));
            ASSERT (compare(res2[k].angle1(), skies[k].angle1(), epsilon));
          }
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

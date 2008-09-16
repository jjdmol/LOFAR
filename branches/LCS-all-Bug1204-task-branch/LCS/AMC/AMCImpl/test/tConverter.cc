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
#include <AMCImpl/ConverterImpl.h>
#include <AMCBase/Direction.h>
#include <AMCBase/Position.h>
#include <AMCBase/Epoch.h>
#include <AMCBase/ConverterClient.h>
#include <AMCBase/RequestData.h>
#include <AMCBase/ResultData.h>
#include <AMCBase/Exceptions.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_fstream.h>
#include <Common/lofar_smartptr.h>
#include <limits>

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


void checkJ2000ToAzelPreConditions(Converter* conv)
{
  RequestData request;
  ResultData result;
  uint count(0);

  // direction type must be J2000
  request.direction = vector<Direction>(1, Direction(0,0,Direction::AZEL));
  try {
    conv->j2000ToAzel(result, request);
  } catch (AssertError&) {
    count++;
  } catch (ConverterException&) {
    count++;
  }
  ASSERT(count == 1);
}


void checkAzelToJ2000PreConditions(Converter* conv)
{
  RequestData request;
  ResultData result;
  uint count(0);

  // position and epoch must have equal sizes.
  request.epoch = vector<Epoch>(1, Epoch());
  try {
    conv->azelToJ2000(result, request);
  } catch (AssertError&) {
    count++;
  } catch (ConverterException&) {
    count++;
  }
  ASSERT(count == 1);

  // If position and epoch have sizes unequal to one, then their size
  // must be equal to the size of direction.
  request.direction = vector<Direction>(1, Direction());
  request.epoch.clear();
  try {
    conv->azelToJ2000(result, request);
  } catch (AssertError&) {
    count++;
  } catch (ConverterException&) {
    count++;
  }
  ASSERT(count == 2);

  // direction type must be AZEL
  request.position = vector<Position>(1, Position());
  request.epoch = vector<Epoch>(1, Epoch());
  try {
    conv->azelToJ2000(result, request);
  } catch (AssertError&) {
    count++;
  } catch (ConverterException&) {
    count++;
  }
  ASSERT(count == 3);
}


void checkJ2000ToItrfPreConditions(Converter* conv)
{
  RequestData request;
  ResultData result;
  uint count(0);

  // direction type must be J2000
  request.direction = vector<Direction>(1, Direction(0,0,Direction::AZEL));
  try {
    conv->j2000ToItrf(result, request);
  } catch (AssertError&) {
    count++;
  } catch (ConverterException&) {
    count++;
  }
  ASSERT(count == 1);
}


void checkItrfToJ2000PreConditions(Converter* conv)
{
  RequestData request;
  ResultData result;
  uint count(0);

  // position and epoch must have equal sizes.
  request.epoch = vector<Epoch>(1, Epoch());
  try {
    conv->itrfToJ2000(result, request);
  } catch (AssertError&) {
    count++;
  } catch (ConverterException&) {
    count++;
  }
  ASSERT(count == 1);

  // If position and epoch have sizes unequal to one, then their size
  // must be equal to the size of direction.
  request.direction = vector<Direction>(1, Direction());
  request.epoch.clear();
  try {
    conv->itrfToJ2000(result, request);
  } catch (AssertError&) {
    count++;
  } catch (ConverterException&) {
    count++;
  }
  ASSERT(count == 2);

  // direction type must be ITRF
  request.position = vector<Position>(1, Position());
  request.epoch = vector<Epoch>(1, Epoch());
  try {
    conv->itrfToJ2000(result, request);
  } catch (AssertError&) {
    count++;
  } catch (ConverterException&) {
    count++;
  }
  ASSERT(count == 3);
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


void checkPreConditions(Converter* conv)
{
  checkJ2000ToAzelPreConditions(conv);
  checkAzelToJ2000PreConditions(conv);
  checkJ2000ToItrfPreConditions(conv);
  checkItrfToJ2000PreConditions(conv);
}


void checkConverter(Converter* conv)
{

  vector<Direction>   skies(4);
  vector<Position> poss(2);
  vector<Epoch>  times(3);

  // Four arbitrary directions (angles in radians).
  skies[0] = Direction (1, 0.3);
  skies[1] = Direction (-1, -0.3);
  skies[2] = Direction (2, 0.1);
  skies[3] = Direction (1.5, -0.3);

  poss[0] =  // arbitrary position
    Position (0.1, 0.8, 1000, Position::WGS84);
  poss[1] =  // Dwingeloo observatory
    Position (0.111646531, 0.921760253, 25, Position::WGS84);

  times[0] = // arbitrary time (added 0.1 usec to avoid rouding errors)
    Epoch (2004, 11, 19, 15, 22, 39.0000001);
  times[1] = // same arbitrary time + 1 hour
    Epoch (times[0].mjd(), 1./24);
  times[2] = // same arbitrary time + 1 day
    Epoch (times[0].mjd(), 1.);

  RequestData request(skies, poss, times);

  cout.precision(9);

  cout << "**** Original data ****" << endl;
  cout << "skies = " << skies << endl;
  cout << "poss = " << poss << endl;
  cout << "times = " << times << endl;

  // Check conversion J2000 --> AZEL and vice versa
  {
    ResultData result;
    conv->j2000ToAzel(result, request);
    ASSERT (result.direction.size() == 
            skies.size() * poss.size() * times.size());
    cout << "\n**** Convert from J2000 to AZEL and vice versa ****\n";
    cout << "result = " << result.direction << endl;
    for (uint i = 0; i < skies.size() * poss.size() * times.size(); i++) {
      ASSERT (result.direction[i].type() == Direction::AZEL);
    }
    int inx=0;
    for (uint i = 0; i < times.size(); i++) {
      for (uint j = 0; j < poss.size(); j++) {
        vector<Direction> sk(skies.size());
        for (uint k = 0; k < skies.size(); k++) {
          sk[k] = result.direction[inx++];
        }
        // Convert back from AZEL to J2000; should yield original data
        ResultData res2;
        conv->azelToJ2000 (res2, RequestData(sk, poss[j], times[i]));
        for (uint k = 0; k < skies.size(); k++) {
          ASSERT (res2.direction[k].type() == Direction::J2000);
          ASSERT (compare(res2.direction[k].longitude(), 
                          skies[k].longitude(), epsilon));
          ASSERT (compare(res2.direction[k].latitude(), 
                          skies[k].latitude(), epsilon));
        }
      }
    }
  }

  // Check conversion J2000 --> ITRF 
  {
    ResultData result;
    conv->j2000ToItrf (result, request);
    ASSERT (result.direction.size() == 
            skies.size() * poss.size() * times.size());
    cout << "\n**** Convert from J2000 to ITRF and vice versa ****\n";
    cout << "result = " << result.direction << endl;
    for (uint i = 0; i < skies.size() * poss.size() * times.size(); i++) {
      ASSERT (result.direction[i].type() == Direction::ITRF);
    }
    int inx=0;
    for (uint i = 0; i < times.size(); i++) {
      for (uint j = 0; j < poss.size(); j++) {
        vector<Direction> sk(skies.size());
        for (uint k = 0; k < skies.size(); k++) {
          sk[k] = result.direction[inx++];
        }
        // Convert back from ITRF to J2000; should yield original data
        ResultData res2;
        conv->itrfToJ2000 (res2, RequestData(sk, poss[j], times[i]));
        for (uint k = 0; k < skies.size(); k++) {
          ASSERT (res2.direction[k].type() == Direction::J2000);
          ASSERT (compare(res2.direction[k].longitude(), 
                          skies[k].longitude(), epsilon));
          ASSERT (compare(res2.direction[k].latitude(), 
                          skies[k].latitude(), epsilon));
        }
      }
    }
  }    
  cout << "OK" << endl;
}


int main(int /*argc*/, const char* const argv[])
{
  INIT_LOGGER(argv[0]);

  try {
    scoped_ptr<Converter> conv(createConverter());
    ASSERT(conv);
    checkPreConditions(conv.get());
    checkConverter(conv.get());
  }
  catch (LOFAR::Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }

  return 0;
}

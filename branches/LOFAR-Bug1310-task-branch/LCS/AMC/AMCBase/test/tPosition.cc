//#  tPosition.cc: test program for the Position class
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
#include <AMCBase/Position.h>
#include <AMCBase/Direction.h>
#include <AMCBase/Exceptions.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_math.h>
#include <limits>

using namespace LOFAR;
using namespace LOFAR::AMC;


bool compare(double x, double y, 
             double eps = std::numeric_limits<double>::epsilon())
{
  if (x == y) return true;
  if (abs(x-y) <= eps) return true;
  if (abs(x) > abs(y)) return (abs((x-y)/x) <= eps);
  else return (abs((x-y)/y) <= eps);
}


bool compare(const Position& p1, const Position& p2, double eps)
{
  const vector<double>& v1 = p1.coord().get();
  const vector<double>& v2 = p2.coord().get();
  return 
    compare(v1[0], v2[0], eps) && 
    compare(v1[1], v2[1], eps) && 
    compare(v1[2], v2[2], eps);
}


int main(int, const char* argv[])
{
  INIT_LOGGER(argv[0]);

  const double a   = Earth::equatorialRadius();
  const double f   = Earth::flattening();
  const double b   = a * (1 - f);
  const double deg = M_PI/180;
  const double eps = a * std::numeric_limits<double>::epsilon();

  try {

    //## --  A couple of positions  -- ##//

    Position pe0  (0,       0,  0, Position::WGS84); // Equator, lon=0, h=0
    Position pe90 (90*deg,  0, 10, Position::WGS84); // Equator, lon=90, h=10
    Position pe180(180*deg, 0,-20, Position::WGS84); // Equator, lon=180, h=-20
    Position pe270(270*deg, 0, 30, Position::WGS84); // Equator, lon=270, h=30
    Position pnp  (0,  90*deg,-40, Position::WGS84); // North pole, h=-40
    Position psp  (0, -90*deg, 50, Position::WGS84); // South pole, h=50
    Position pexl (6*deg, 53*deg, 50, Position::WGS84); // Near Exloo

    // Show the positions.
    cout.precision(16);
    cout << "====  A couple of positions  ====" << endl;
    cout << "pe0   = " << pe0   << endl;
    cout << "pe90  = " << pe90  << endl;
    cout << "pe180 = " << pe180 << endl;
    cout << "pe270 = " << pe270 << endl;
    cout << "pnp   = " << pnp   << endl;
    cout << "psp   = " << psp   << endl;
    cout << "pexl  = " << pexl  << endl;
    cout << endl;

    // Compare WGS84 positions with their expected positions in ITRF.
    vector<double> xyz(3);
    xyz[0] = a; xyz[1] = 0; xyz[2] = 0;
    ASSERT(compare(pe0, Position(xyz, Position::ITRF), eps));
    xyz[0] = 0; xyz[1] = a+10; xyz[2] = 0;
    ASSERT(compare(pe90, Position(xyz, Position::ITRF), eps));
    xyz[0] = -(a-20); xyz[1] = 0; xyz[2] = 0;
    ASSERT(compare(pe180, Position(xyz, Position::ITRF), eps));
    xyz[0] = 0; xyz[1] = -(a+30); xyz[2] = 0;
    ASSERT(compare(pe270, Position(xyz, Position::ITRF), eps));
    xyz[0] = 0; xyz[1] = 0; xyz[2] = b-40;
    ASSERT(compare(pnp, Position(xyz, Position::ITRF), eps));
    xyz[0] = 0; xyz[1] = 0; xyz[2] = -(b+50);
    ASSERT(compare(psp, Position(xyz, Position::ITRF), eps));
    xyz[0] = 3825637.140852076; xyz[1] = 402090.6660932263; 
    xyz[2] = 5070583.435129914;
    ASSERT(compare(pexl, Position(xyz, Position::ITRF), eps));

    // Check inner products for same and opposite positions.
    ASSERT(compare(pe0  *pe0  ,       a*a,      eps));
    ASSERT(compare(pe90 *pe90 ,  (a+10)*(a+10), eps));
    ASSERT(compare(pe180*pe180,  (a-20)*(a-20), eps));
    ASSERT(compare(pe270*pe270,  (a+30)*(a+30), eps));
    ASSERT(compare(pnp  *pnp  ,  (b-40)*(b-40), eps));
    ASSERT(compare(psp  *psp  ,  (b+50)*(b+50), eps));
    ASSERT(compare(pe0  *pe180,      -a*(a-20), eps));
    ASSERT(compare(pe90 *pe270, -(a+10)*(a+30), eps));
    ASSERT(compare(pnp  *psp  , -(b-40)*(b+50), eps));

    // Check inner products for perpendicular positions. 
    // \note We need to increase the error margin to a*eps, because of 
    // rounding errors in the sine and cosine functions.
    ASSERT(compare(pe0  *pe90 , 0, a*eps));
    ASSERT(compare(pe90 *pe180, 0, a*eps));
    ASSERT(compare(pe180*pe270, 0, a*eps));
    ASSERT(compare(pe270*pe0  , 0, a*eps));
    ASSERT(compare(pe0  *pnp  , 0, a*eps));
    ASSERT(compare(pe90 *pnp  , 0, a*eps));
    ASSERT(compare(pe180*pnp  , 0, a*eps));
    ASSERT(compare(pe270*pnp  , 0, a*eps));
    ASSERT(compare(pe0  *psp  , 0, a*eps));
    ASSERT(compare(pe90 *psp  , 0, a*eps));
    ASSERT(compare(pe180*psp  , 0, a*eps));
    ASSERT(compare(pe270*psp  , 0, a*eps));


    //## --  A couple of directions  -- ##//

    // Create directions from the (x,y,z) coordinates of our positions.
    Direction de0  (pe0  .coord(), Direction::ITRF);
    Direction de90 (pe90 .coord(), Direction::ITRF);
    Direction de180(pe180.coord(), Direction::ITRF);
    Direction de270(pe270.coord(), Direction::ITRF);
    Direction dnp  (pnp  .coord(), Direction::ITRF);
    Direction dsp  (psp  .coord(), Direction::ITRF);
    Direction dexl (pexl .coord(), Direction::ITRF);

    // Show the directions.
    cout << "====  A couple of directions  ====" << endl;
    cout << "de0   = " << de0   << endl;
    cout << "de90  = " << de90  << endl;
    cout << "de180 = " << de180 << endl;
    cout << "de270 = " << de270 << endl;
    cout << "dnp   = " << dnp   << endl;
    cout << "dsp   = " << dsp   << endl;
    cout << "dexl  = " << dexl  << endl;
    cout << endl;

    // Inner product of direction with associated position (and vice versa)
    // should yield the norm of the position.
    ASSERT(compare(pe0  *de0  ,    pe0.coord().radius(), eps));
    ASSERT(compare(pe90 *de90 ,   pe90.coord().radius(), eps));
    ASSERT(compare(pe180*de180,  pe180.coord().radius(), eps));
    ASSERT(compare(pe270*de270,  pe270.coord().radius(), eps));
    ASSERT(compare(pnp  *dnp  ,    pnp.coord().radius(), eps));
    ASSERT(compare(psp  *dsp  ,    psp.coord().radius(), eps));
    ASSERT(compare(pe0  *de180,   -pe0.coord().radius(), eps));
    ASSERT(compare(pe90 *de270,  -pe90.coord().radius(), eps));
    ASSERT(compare(pnp  *dsp  ,   -pnp.coord().radius(), eps));

    ASSERT(compare(de0  *pe0  ,    pe0.coord().radius(), eps));
    ASSERT(compare(de90 *pe90 ,   pe90.coord().radius(), eps));
    ASSERT(compare(de180*pe180,  pe180.coord().radius(), eps));
    ASSERT(compare(de270*pe270,  pe270.coord().radius(), eps));
    ASSERT(compare(dnp  *pnp  ,    pnp.coord().radius(), eps));
    ASSERT(compare(dsp  *psp  ,    psp.coord().radius(), eps));
    ASSERT(compare(de0  *pe180, -pe180.coord().radius(), eps));
    ASSERT(compare(de90 *pe270, -pe270.coord().radius(), eps));
    ASSERT(compare(dnp  *psp  ,   -psp.coord().radius(), eps));


    // Inner product of direction with perpendicular position (and vice versa)
    // should yield zero.
    ASSERT(compare(pe0  *de90 , 0, eps));
    ASSERT(compare(pe90 *de180, 0, eps));
    ASSERT(compare(pe180*de270, 0, eps));
    ASSERT(compare(pe270*de0  , 0, eps));
    ASSERT(compare(pe0  *dnp  , 0, eps));
    ASSERT(compare(pe90 *dnp  , 0, eps));
    ASSERT(compare(pe180*dnp  , 0, eps));
    ASSERT(compare(pe270*dnp  , 0, eps));
    ASSERT(compare(pe0  *dsp  , 0, eps));
    ASSERT(compare(pe90 *dsp  , 0, eps));
    ASSERT(compare(pe180*dsp  , 0, eps));
    ASSERT(compare(pe270*dsp  , 0, eps));

    ASSERT(compare(de0  *pe90 , 0, eps));
    ASSERT(compare(de90 *pe180, 0, eps));
    ASSERT(compare(de180*pe270, 0, eps));
    ASSERT(compare(de270*pe0  , 0, eps));
    ASSERT(compare(de0  *pnp  , 0, eps));
    ASSERT(compare(de90 *pnp  , 0, eps));
    ASSERT(compare(de180*pnp  , 0, eps));
    ASSERT(compare(de270*pnp  , 0, eps));
    ASSERT(compare(de0  *psp  , 0, eps));
    ASSERT(compare(de90 *psp  , 0, eps));
    ASSERT(compare(de180*psp  , 0, eps));
    ASSERT(compare(de270*psp  , 0, eps));

  }

  catch (LOFAR::Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }

  return 0;
}

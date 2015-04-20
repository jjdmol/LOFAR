//# AntennaFieldHBA.cc: Representation of an HBA antenna field.
//#
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
//# $Id$

#include <lofar_config.h>
#include <StationResponse/AntennaFieldHBA.h>
#include <StationResponse/MathUtil.h>
#include <StationResponse/MathUtil.h>
#include <fits/FITS/BasicFITS.h>
#include <measures/Measures.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>

namespace LOFAR
{
namespace StationResponse
{

AntennaFieldHBA::AntennaFieldHBA(const string &name,
    const CoordinateSystem &coordinates, const AntennaModelHBA::ConstPtr &model)
    :   AntennaField(name, coordinates),
        itsAntennaModel(model)
{
  casa::Bool ok=true;
  casa::String message;

  itsIntegrals = casa::ReadFITS(("~/Prog/LOFAR-Beam-Task6814/testdir/beamnorm/beamintmap-"+name+".fits").c_str(),ok,message);
  if (!ok) {
    cout << "Read failed: " << message << endl;
  }
}

matrix22c_t AntennaFieldHBA::response(real_t time, real_t freq,
    const vector3r_t &direction, const vector3r_t &direction0) const
{
    matrix22c_t response=itsAntennaModel->response(freq, itrf2field(direction),
        itrf2field(direction0)) * rotation(time, direction);

    return response;
}

diag22c_t AntennaFieldHBA::arrayFactor(real_t, real_t freq,
    const vector3r_t &direction, const vector3r_t &direction0) const
{
    diag22c_t af=itsAntennaModel->arrayFactor(freq, itrf2field(direction),
        itrf2field(direction0));

    return af;
}

raw_response_t AntennaFieldHBA::rawResponse(real_t time, real_t freq,
    const vector3r_t &direction, const vector3r_t &direction0) const
{
    raw_response_t result = itsAntennaModel->rawResponse(freq,
        itrf2field(direction), itrf2field(direction0));

    cout<<"Position: "<<position()<<endl;
    cout<<"Direction: "<<direction<<endl;
    casa::MeasFrame frame(casa::MPosition(casa::MVPosition(position()[0],
                                                           position()[1],
                                                           position()[2])));
    frame.set(casa::MEpoch(casa::MVEpoch(time/86400), casa::MEpoch::UTC));

    std::pair<double,double> azel=getAzEl(direction, frame);
    cout<<"1. az="<<azel.first<<", el="<<azel.second<<endl;
    frame.resetEpoch(casa::MEpoch(casa::MVEpoch((time+3600)/86400), casa::MEpoch::UTC));

    azel=getAzEl(direction, frame);
    cout<<"2. az="<<azel.first<<", el="<<azel.second<<endl;

    casa::MeasFrame frame2(casa::MPosition(casa::MVPosition(position()[0],
                                                           position()[1],
                                                           position()[2])));
    frame2.set(casa::MEpoch(casa::MVEpoch((time+3600)/86400), casa::MEpoch::UTC));

    azel=getAzEl(direction, frame2);
    cout<<"3. az="<<azel.first<<", el="<<azel.second<<endl;

    result.response = result.response * rotation(time, direction);
    return result;
}

raw_array_factor_t AntennaFieldHBA::rawArrayFactor(real_t, real_t freq,
    const vector3r_t &direction, const vector3r_t &direction0) const
{
    return itsAntennaModel->rawArrayFactor(freq, itrf2field(direction),
        itrf2field(direction0));
}

matrix22c_t AntennaFieldHBA::elementResponse(real_t time, real_t freq,
    const vector3r_t &direction) const
{
    return itsAntennaModel->elementResponse(freq, itrf2field(direction))
        * rotation(time, direction);
}

std::pair<double,double> AntennaFieldHBA::getAzEl(const vector3r_t &direction,
                                                  const casa::MeasFrame &frame)
{
     // Ik wil van ITRF (direction) naar AZEL
     // Nodig: *alleen maar* station position, toch?
     casa::MDirection::Convert mconv(
         casa::MDirection::ITRF,
         casa::MDirection::Ref(casa::MDirection::AZELGEO, frame));

     casa::MDirection itrfdir(casa::MVDirection(direction[0],direction[1],direction[2]));

     casa::MDirection azeldir=mconv(itrfdir);

     std::pair<double,double> azel;

     azel.first=azeldir.getAngle().getValue()[0];
     azel.second=azeldir.getAngle().getValue()[1];
     return azel;
}

} //# namespace StationResponse
} //# namespace LOFAR

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
#include <Common/LofarLogger.h>
#include <fits/FITS/BasicFITS.h>
#include <Common/LofarLocators.h>
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

  FileLocator locator("$LOFARROOT/share/beamnorms");
  string fitsFile=locator.locate("beamintmap-"+name+".fits");
  itsIntegrals = casa::ReadFITS(fitsFile.c_str(),ok,message);
  if (!ok) {
    LOG_WARN_STR("Could not read beam normalization fits file: " << message);
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

    real_t norm=getNormalization(freq, direction);
    result.response = result.response * rotation(time, direction);
    result.response[0][0]/=norm; result.response[1][0]/=norm;
    result.response[0][1]/=norm; result.response[1][1]/=norm;
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

real_t AntennaFieldHBA::getNormalization(real_t freq,
                                         const vector3r_t &direction) const
{
  // Get indices for azimuth and elevation
  const uint gridsize=50;

  std::pair<double,double> azel=getAzEl(position(), direction);
  double az=azel.first;
  double el=azel.second;

  uint x_index=0;
  uint y_index=0;
  uint freq_index=0;

  double xx_pix=sin(az)*gridsize*(.5-el/casa::C::pi);
  double yy_pix=cos(az)*gridsize*(.5-el/casa::C::pi);
  x_index=xx_pix+.5*gridsize;
  y_index=yy_pix+.5*gridsize;

  // Get index for frequency
  const double freq_min=100.e6;
  const double freq_max=250.e6;
  const uint numfreqs=16;

  // Round to int, so add 0.5 and then truncate
  freq_index=int((freq-freq_min)/(freq_max-freq_min)*(numfreqs-1)+0.5);

  ASSERTSTR(!itsIntegrals.empty(), "No beamnorms found for station "<<name());

  //cout<<"Name="<<name()<<", itsIntegrals.shape="<<itsIntegrals.shape()<<endl;
  //cout<<"Name="<<name()<<", freq="<<freq<<", az="<<az<<", el="<<el
  //    <<", index=["<<x_index<<", "<<y_index<<", "<<freq_index<<"], norm="
  //    <<itsIntegrals(casa::IPosition(3,x_index,y_index,freq_index))<<endl;

  // Todo: interpolation for frequency
  return real_t(itsIntegrals(casa::IPosition(3,x_index,y_index,freq_index)));
}


std::pair<double,double> AntennaFieldHBA::getAzEl(const vector3r_t &position,
                                                  const vector3r_t &direction)
{
     using namespace casa;
     MDirection dir_itrf(MVDirection(direction[0],direction[1],direction[2]),
                         MDirection::ITRF);

     casa::MVPosition mvPosition(position[0], position[1], position[2]);
     casa::MPosition pos(mvPosition, casa::MPosition::ITRF);

     MeasFrame frame(pos);

     MDirection::Convert itrf2azel(
         dir_itrf,
         MDirection::Ref(MDirection::AZEL, frame));

     MDirection dir_azel = itrf2azel();

     std::pair<double,double> azel;

     azel.first =dir_azel.getAngle().getValue()[0];
     azel.second=dir_azel.getAngle().getValue()[1];
     return azel;
}

} //# namespace StationResponse
} //# namespace LOFAR

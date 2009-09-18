//# PiercePoint.cc: Pierce point for a direction (ra,dec) on the sky.
//#
//# Copyright (C) 2007
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

#include <BBSKernel/Expr/PiercePoint.h>
#include <BBSKernel/Expr/PValueIterator.h>
#include <BBSKernel/Expr/Request.h>
#include <Common/LofarLogger.h>

#include <casa/BasicSL/Constants.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MeasConvert.h>

namespace LOFAR
{
namespace BBS
{

PiercePoint::PiercePoint(const Station &station, const Expr &direction)
    : itsStation(station)
{
    // TODO: Get height as parameter?
    addChild(direction); 
    // get antenna position (ITRF).
    const casa::MVPosition &ant_pos = itsStation.position.getValue();
    const double x_ant = ant_pos(0);
    const double y_ant = ant_pos(1);
    const double z_ant = ant_pos(2);
    
    // get lon, lat, h
    casa::Vector<casa::Double> ang;
    casa::MPosition::Convert loc2(itsStation.position, casa::MPosition::WGS84);
    casa::MPosition locwgs84(loc2());
    ang= locwgs84.getAngle().getValue();
    itsLong = ang(0);
    itsLat = ang(1);
    itsHeight = locwgs84.getValue().getLength().getValue();
    // height above surface, use it to calculate earth_radius at position of itsStation
    double inproduct_xyz=std::sqrt(x_ant*x_ant
				   +y_ant*y_ant
				   +z_ant*z_ant);
    itsEarthRadius = inproduct_xyz - itsHeight;

}

ResultVec PiercePoint::getResultVec(const Request &request)
{
    // Check preconditions.
    ASSERTSTR(request.getTimeslotCount() > 0, "Need time values.");

    // Evaluate children.
    ResultVec tmpAzel;
    const ResultVec &azel = getChild(0).getResultVecSynced(request, tmpAzel);

    // Create result.
    ResultVec result(4);

    // Compute main value.
    evaluate(request, azel[0].getValue(), azel[1].getValue(),
        result[0].getValueRW(), result[1].getValueRW(), result[2].getValueRW(),
        result[3].getValueRW());
    
    // Compute perturbed values.  
    enum PValues
    { PV_AZ, PV_EL, N_PValues };
    
    const Result *pvSet[N_PValues] = {&(azel[0]), &(azel[1])};
    PValueSetIterator<N_PValues> pvIter(pvSet);
    
    while(!pvIter.atEnd())
    {
        const Matrix &pvAz = pvIter.value(PV_AZ);
        const Matrix &pvEl = pvIter.value(PV_EL);

        evaluate(request, pvAz, pvEl,
            result[0].getPerturbedValueRW(pvIter.key()),
            result[1].getPerturbedValueRW(pvIter.key()),
            result[2].getPerturbedValueRW(pvIter.key()),
            result[3].getPerturbedValueRW(pvIter.key()));

        pvIter.next();
    }
    
    return result;
}

void PiercePoint::evaluate(const Request &request, const Matrix &in_az,
    const Matrix &in_el, Matrix &out_x, Matrix &out_y, Matrix &out_z,
    Matrix &out_alpha)
{
    const size_t nTimeslots = request.getTimeslotCount();
    
    //get long lat needed for rotation
    double sinlon = std::sin(itsLong);
    double coslon = std::cos(itsLong);
    double sinlat = std::sin(itsLat);
    double coslat = std::cos(itsLat);

    

    
    const casa::MVPosition &ant_pos = itsStation.position.getValue();
    const double x_ant = ant_pos(0);
    const double y_ant = ant_pos(1);
    const double z_ant = ant_pos(2);
     

    // Result is only time variable.
    double *x = out_x.setDoubleFormat(1, nTimeslots);
    double *y = out_y.setDoubleFormat(1, nTimeslots);
    double *z = out_z.setDoubleFormat(1, nTimeslots);

    //use lon,lat,height instead?? or convert at MeqMIM??
    double *alpha = out_alpha.setDoubleFormat(1, nTimeslots);




    for(size_t i = 0; i < nTimeslots; ++i)
    {

      //calculate alpha, thi sis de angle of the line of sight with the phase screen (i.e. alpha' in the document)
      *alpha = std::asin(std::cos(in_el.getDouble(0,i))*(itsEarthRadius+itsHeight)
            /(itsEarthRadius+iono_height));
      double sinaz=std::sin(in_az.getDouble(0,i));
      double cosaz=std::cos(in_az.getDouble(0,i));
      double sinel=std::sin(in_el.getDouble(0,i));
      double cosel=std::cos(in_el.getDouble(0,i));
      //direction in local coordinates
      double dir_vector[3]={sinaz*cosel,cosaz*cosel,sinel};
      //now convert
      double xdir = -1*sinlon*dir_vector[0]-sinlat*coslon*dir_vector[1]+coslat*coslon*dir_vector[2];
      double ydir = coslon*dir_vector[0]-sinlat*sinlon*dir_vector[1]+coslat*sinlon*dir_vector[2];
      double zdir = coslat*dir_vector[1]+sinlat*dir_vector[2];
	    

      *x=x_ant+xdir*(itsEarthRadius+iono_height)*std::sin(0.5*casa::C::pi-in_el.getDouble(0,i)-*alpha)/cosel;
      *y=y_ant+ydir*(itsEarthRadius+iono_height)*std::sin(0.5*casa::C::pi-in_el.getDouble(0,i)-*alpha)/cosel;
      *z=z_ant+zdir*(itsEarthRadius+iono_height)*std::sin(0.5*casa::C::pi-in_el.getDouble(0,i)-*alpha)/cosel;
      ++x;++y;++z;++alpha;
    }
}



} //# namespace BBS
} //# namespace LOFAR


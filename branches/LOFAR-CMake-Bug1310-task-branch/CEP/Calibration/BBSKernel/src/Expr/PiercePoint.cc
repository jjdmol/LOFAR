//# PiercePoint.cc: Pierce point for a direction (ra,dec) on the sky.
//#
//# Copyright (C) 2007
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
    
    // get antenna position (ITRF).
    const casa::MVPosition &ant_pos = itsStation.position.getValue();
    const double x_ant = ant_pos(0);
    const double y_ant = ant_pos(1);
    const double z_ant = ant_pos(2);
    
    // get lon, lat, h
    casa::Vector<casa::Double> ang;
    casa::Double height;
    casa::MPosition::Convert loc2(itsStation.position, casa::MPosition::WGS84);
    casa::MPosition locwgs84(loc2());
    ang= locwgs84.getAngle().getValue();
    height = locwgs84.getValue().getLength().getValue();

    // height above surface, use it to calculate earth_radius
    double inproduct_xyz=std::sqrt(x_ant*x_ant
				   +y_ant*y_ant
				   +z_ant*z_ant);
    double earth_radius = inproduct_xyz - height;

    // Result is only time variable.
    double *x = out_x.setDoubleFormat(1, nTimeslots);
    double *y = out_y.setDoubleFormat(1, nTimeslots);
    double *z = out_z.setDoubleFormat(1, nTimeslots);

    //use lon,lat,height instead?? or convert at MeqMIM??
    double *alpha = out_alpha.setDoubleFormat(1, nTimeslots);
    for(size_t i = 0; i < nTimeslots; ++i)
    {
        *alpha = std::asin(std::cos(in_el.getDouble(0,i))*inproduct_xyz
            /(earth_radius+iono_height));
        *x=x_ant+(earth_radius+iono_height)*std::sin(in_az.getDouble(0,i))
            *std::sin(0.5*casa::C::pi-in_el.getDouble(0,i)-*alpha);
        *y=y_ant+(earth_radius+iono_height)*std::cos(in_az.getDouble(0,i))
            *std::sin(0.5*casa::C::pi-in_el.getDouble(0,i)-*alpha);
        *z=z_ant+(earth_radius+iono_height)*std::tan(in_el.getDouble(0,i))
            *std::sin(0.5*casa::C::pi-in_el.getDouble(0,i)-*alpha);
        ++x;++y;++z;++alpha;
    }
}


#ifdef EXPR_GRAPH
std::string MeqPP::getLabel()
{
    return std::string("MeqPP\\n PiercePoint through the ionosphere of a source station combination.");
}
#endif

} //# namespace BBS
} //# namespace LOFAR


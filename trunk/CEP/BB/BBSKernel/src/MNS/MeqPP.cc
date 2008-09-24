//# MeqPP.cc:PiercePoints for a direction (ra,dec) on the sky.
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
//# $Id: MeqPP.cc 10498 2007-09-07 09:07:45Z zwieten $

#include <lofar_config.h>

#include <BBSKernel/MNS/MeqPP.h>
#include <BBSKernel/MNS/MeqSource.h>
#include <BBSKernel/MNS/MeqStation.h>
#include <BBSKernel/MNS/MeqRequest.h>
#include <Common/LofarLogger.h>

#include <measures/Measures/MPosition.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MeasConvert.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

MeqPP::MeqPP(MeqExpr &direction, MeqStation &station)
{//get height as parameter?
  addChild(direction); 
  addChild(station.getPosX());
  addChild(station.getPosY());
  addChild(station.getPosZ());
}


MeqResultVec MeqPP::getResultVec(const MeqRequest& request)
{
    // Check preconditions.
    ASSERTSTR(request.ny() > 0, "Need time values.");

    // Evaluate children.
    MeqResultVec res_azel;
    MeqResult res_x, res_y, res_z;
    const MeqResultVec &azel =
      getChild(MeqPP::IN_AZEL).getResultVecSynced(request, res_azel);
    const MeqResult &x =
        getChild(MeqPP::IN_X).getResultSynced(request, res_x);
    const MeqResult &y =
        getChild(MeqPP::IN_Y).getResultSynced(request, res_y);
    const MeqResult &z =
        getChild(MeqPP::IN_Z).getResultSynced(request, res_z);
    
    // Check preconditions.
    ASSERTSTR(!x.getValue().isArray() && !y.getValue().isArray()
        && !z.getValue().isArray(), "Variable station positions are not"
        " supported yet.");

    // Create result.
    MeqResultVec result(4, request.nspid());

    // Evaluate main value.
    evaluate(request,azel[0].getValue(), azel[1].getValue(), x.getValue(), y.getValue(),
        z.getValue(), result[0].getValueRW(), result[1].getValueRW(), result[2].getValueRW(), result[3].getValueRW());
    
    // Evaluate perturbed values.  
    const MeqParmFunklet *perturbedParm;
    for(int i = 0; i < request.nspid(); ++i)
    {
        // Find out if this perturbed value needs to be computed.
    	if(azel[0].isDefined(i))
    	{
    	    perturbedParm = azel[0].getPerturbedParm(i);
    	}
    	else if(azel[1].isDefined(i))
    	{
    	    perturbedParm = azel[1].getPerturbedParm(i);
    	}
    	else if(x.isDefined(i))
    	{
    	    perturbedParm = x.getPerturbedParm(i);
    	}
    	else if(y.isDefined(i))
    	{
    	    perturbedParm = y.getPerturbedParm(i);
    	}
    	else if(z.isDefined(i))
    	{
    	    perturbedParm = z.getPerturbedParm(i);
    	}
    	else
    	{
    	    continue;
    	}

        evaluate(request, azel[0].getPerturbedValue(i), azel[1].getPerturbedValue(i),
            x.getPerturbedValue(i), y.getPerturbedValue(i),
            z.getPerturbedValue(i), result[0].getPerturbedValueRW(i),
            result[1].getPerturbedValueRW(i), result[2].getValueRW(), result[3].getValueRW());
        
        result[0].setPerturbedParm(i, perturbedParm);
        result[1].setPerturbedParm(i, perturbedParm);
        result[2].setPerturbedParm(i, perturbedParm);
        result[3].setPerturbedParm(i, perturbedParm);
    }
    
    return result;
}


void MeqPP::evaluate(const MeqRequest& request, const MeqMatrix &in_az,
    const MeqMatrix &in_el, const MeqMatrix &in_x, const MeqMatrix &in_y,
    const MeqMatrix &in_z, MeqMatrix &out_x, MeqMatrix &out_y, MeqMatrix &out_z, MeqMatrix &out_alpha)
{
  
    
    
    //calculate alpha_prime:
    double x_ant=in_x.getDouble(0, 0);
    double y_ant=in_y.getDouble(0, 0);
    double z_ant=in_z.getDouble(0, 0);
    // get lon, lat, h

    const casa::MVPosition vpos(x_ant,y_ant,z_ant);
    const casa::MPosition pos(vpos,casa::MPosition::ITRF);
    casa::Vector<casa::Double> ang;
    casa::Double height;
    casa::MPosition::Convert loc2(pos, casa::MPosition::WGS84);
    casa::MPosition locwgs84(loc2());
    ang= locwgs84.getAngle().getValue();
    height = locwgs84.getValue().getLength().getValue();
    // height above surface, use it to calculate earth_radius

    double inproduct_xyz=std::sqrt(x_ant*x_ant
				   +y_ant*y_ant
				   +z_ant*z_ant);
    double earth_radius = inproduct_xyz - height;

    // Result is only time variable.
    double *x = out_x.setDoubleFormat(1, request.ny());
    double *y = out_y.setDoubleFormat(1, request.ny());
    double *z = out_z.setDoubleFormat(1, request.ny());
    //use lon,lat,height instead?? or convert at MeqMIM??
    double *alpha = out_alpha.setDoubleFormat(1, request.ny());
    double pi=3.141592653589793116;
    for(int i = 0; i < request.ny(); ++i)
    {
      *alpha = std::asin(std::cos(in_el.getDouble(0,i))*inproduct_xyz/(earth_radius+iono_height));
      *x=x_ant+(earth_radius+iono_height)*std::sin(in_az.getDouble(0,i))*std::sin(0.5*pi-in_el.getDouble(0,i)-*alpha);
      *y=y_ant+(earth_radius+iono_height)*std::cos(in_az.getDouble(0,i))*std::sin(0.5*pi-in_el.getDouble(0,i)-*alpha);
      *z=z_ant+(earth_radius+iono_height)*std::tan(in_el.getDouble(0,i))*std::sin(0.5*pi-in_el.getDouble(0,i)-*alpha);
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


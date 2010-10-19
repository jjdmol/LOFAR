//# MeqAzEl.cc: Azimuth and elevation for a direction (ra,dec) on the sky.
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

#include <BBSKernel/MNS/MeqAzEl.h>
#include <BBSKernel/MNS/MeqSource.h>
#include <BBSKernel/MNS/MeqStation.h>
#include <BBSKernel/MNS/MeqRequest.h>
#include <Common/LofarLogger.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Quanta/Quantum.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

MeqAzEl::MeqAzEl(MeqSource &source, MeqStation &station)
{
    addChild(source.getRa());
    addChild(source.getDec());
    addChild(station.getPosX());
    addChild(station.getPosY());
    addChild(station.getPosZ());
}


MeqResultVec MeqAzEl::getResultVec(const MeqRequest& request)
{
    // Check preconditions.
    ASSERTSTR(request.ny() > 0, "Need time values.");

    // Evaluate children.
    MeqResult res_ra, res_dec, res_x, res_y, res_z;
    const MeqResult &ra =
        getChild(MeqAzEl::IN_RA).getResultSynced(request, res_ra);
    const MeqResult &dec =
        getChild(MeqAzEl::IN_DEC).getResultSynced(request, res_dec);
    const MeqResult &x =
        getChild(MeqAzEl::IN_X).getResultSynced(request, res_x);
    const MeqResult &y =
        getChild(MeqAzEl::IN_Y).getResultSynced(request, res_y);
    const MeqResult &z =
        getChild(MeqAzEl::IN_Z).getResultSynced(request, res_z);
    
    // Check preconditions.
    ASSERTSTR(!ra.getValue().isArray() && !dec.getValue().isArray(), "Variable"
        " source positions are not supported yet.");
    ASSERTSTR(!x.getValue().isArray() && !y.getValue().isArray()
        && !z.getValue().isArray(), "Variable station positions are not"
        " supported yet.");

    // Create result.
    MeqResultVec result(2, request.nspid());

    // Evaluate main value.
    evaluate(request, ra.getValue(), dec.getValue(), x.getValue(), y.getValue(),
        z.getValue(), result[0].getValueRW(), result[1].getValueRW());
    
    // Evaluate perturbed values.  
    const MeqParmFunklet *perturbedParm;
    for(int i = 0; i < request.nspid(); ++i)
    {
        // Find out if this perturbed value needs to be computed.
    	if(ra.isDefined(i))
    	{
    	    perturbedParm = ra.getPerturbedParm(i);
    	}
    	else if(dec.isDefined(i))
    	{
    	    perturbedParm = dec.getPerturbedParm(i);
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

        evaluate(request, ra.getPerturbedValue(i), dec.getPerturbedValue(i),
            x.getPerturbedValue(i), y.getPerturbedValue(i),
            z.getPerturbedValue(i), result[0].getPerturbedValueRW(i),
            result[1].getPerturbedValueRW(i));
        
        result[0].setPerturbedParm(i, perturbedParm);
        result[1].setPerturbedParm(i, perturbedParm);
    }
    
    return result;
}


void MeqAzEl::evaluate(const MeqRequest& request, const MeqMatrix &in_ra,
    const MeqMatrix &in_dec, const MeqMatrix &in_x, const MeqMatrix &in_y,
    const MeqMatrix &in_z, MeqMatrix &out_az, MeqMatrix &out_el)
{
    MPosition position(MVPosition(in_x.getDouble(0, 0), in_y.getDouble(0, 0),
        in_z.getDouble(0, 0)), MPosition::Ref(MPosition::ITRF));
    Quantum<double> qepoch(0, "s");
    MEpoch epoch(qepoch, MEpoch::UTC);    
        
    // Create and initialize a frame.
    MeasFrame frame;
    frame.set(position);
    frame.set(epoch);

    // Create conversion engine.
    MDirection dir(MVDirection(in_ra.getDouble(0, 0), in_dec.getDouble(0, 0)),
        MDirection::Ref(MDirection::J2000));    
    MDirection::Convert converter = MDirection::Convert(dir,
        MDirection::Ref(MDirection::AZEL, frame));
        
    // Result is only time variable.
    double *az = out_az.setDoubleFormat(1, request.ny());
    double *el = out_el.setDoubleFormat(1, request.ny());
    
    for(int i = 0; i < request.ny(); ++i)
    {
        // Update reference frame.
        qepoch.setValue(request.y(i));
        epoch.set(qepoch);
        frame.set(epoch);
        
        // Compute azimuth and elevation.
        MDirection azel(converter());
        Vector<Double> vec_azel = azel.getValue().getAngle("rad").getValue();
        *az = vec_azel(0);
        *el = vec_azel(1);
        ++az; ++el;
    }
}


#ifdef EXPR_GRAPH
std::string MeqAzEl::getLabel()
{
    return std::string("MeqAzEl\\nAzimuth and elevation of a source.");
}
#endif

} //# namespace BBS
} //# namespace LOFAR


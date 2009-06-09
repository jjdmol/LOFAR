//# AzEl.cc: Azimuth and elevation for a direction (ra,dec) on the sky.
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

#include <BBSKernel/Expr/AzEl.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/PValueIterator.h>

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

AzEl::AzEl(const Station &station, const Source::ConstPtr &source)
    :   itsStation(station),
        itsSource(source)
{
    addChild(source->getRa());
    addChild(source->getDec());
}


ResultVec AzEl::getResultVec(const Request& request)
{
    // Check preconditions.
    ASSERTSTR(request.getTimeslotCount() > 0, "Need time values.");

    // Evaluate children.
    Result tmpRa, tmpDec;
    const Result &resRa = getChild(0).getResultSynced(request, tmpRa);
    const Result &resDec = getChild(1).getResultSynced(request, tmpDec);
    const Matrix &ra = resRa.getValue();
    const Matrix &dec = resDec.getValue();
    
    // Check preconditions.
    ASSERTSTR(!ra.isArray() && !dec.isArray(), "Variable source positions are"
        " not supported yet.");

    // Create result.
    ResultVec result(2);

    // Compute main value.
    evaluate(request, ra, dec, result[0].getValueRW(), result[1].getValueRW());
    
    // Compute the perturbed values.
    enum PValues
    {
        PV_RA, PV_DEC, N_PValues
    };
    
    const Result *pvSet[N_PValues] = {&resRa, &resDec};
    PValueSetIterator<N_PValues> pvIter(pvSet);
    
    while(!pvIter.atEnd())
    {
        const Matrix &pvRa = pvIter.value(PV_RA);
        const Matrix &pvDec = pvIter.value(PV_DEC);

        evaluate(request, pvRa, pvDec,
            result[0].getPerturbedValueRW(pvIter.key()),
            result[1].getPerturbedValueRW(pvIter.key()));

        pvIter.next();
    }
    
    return result;
}


void AzEl::evaluate(const Request& request, const Matrix &in_ra,
    const Matrix &in_dec, Matrix &out_az, Matrix &out_el)
{
    Quantum<double> qepoch(0, "s");
    MEpoch epoch(qepoch, MEpoch::UTC);    
        
    // Create and initialize a frame.
    MeasFrame frame;
    frame.set(itsStation.position);
    frame.set(epoch);

    // Create conversion engine.
    MDirection dir(MVDirection(in_ra.getDouble(0, 0), in_dec.getDouble(0, 0)),
        MDirection::Ref(MDirection::J2000));    
    MDirection::Convert converter = MDirection::Convert(dir,
        MDirection::Ref(MDirection::AZEL, frame));
        
    // Result is only time variable.
    const size_t nTimeslots = request.getTimeslotCount();
    double *az = out_az.setDoubleFormat(1, nTimeslots);
    double *el = out_el.setDoubleFormat(1, nTimeslots);
    
    Axis::ShPtr timeAxis(request.getGrid()[TIME]);
    for(size_t i = 0; i < nTimeslots; ++i)
    {
        // Update reference frame.
        qepoch.setValue(timeAxis->center(i));
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

#if 0
std::string AzEl::getLabel()
{
    return std::string("AzEl\\nAzimuth and elevation of a source.");
}
#endif

} //# namespace BBS
} //# namespace LOFAR


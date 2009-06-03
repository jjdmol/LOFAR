//# StatUVW.cc: UVW coordinates of a station in meters.
//#
//# Copyright (C) 2002
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

#include <BBSKernel/Expr/StatUVW.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/MatrixTmp.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>

#include <measures/Measures/MBaseline.h>
#include <measures/Measures/MCBaseline.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasFrame.h>
#include <casa/Quanta/MVuvw.h>
#include <stdint.h>

//using namespace casa;

namespace LOFAR
{
namespace BBS
{

StatUVW::StatUVW(const Station &station, const casa::MPosition &arrayRef,
    const PhaseRef::ConstPointer &phaseRef)
    :   itsStation(station),
        itsArrayRef(arrayRef),
        itsPhaseRef(phaseRef),
        itsLastReqId(InitRequestId)
{
    itsU.reset(new ExprResult());
    itsV.reset(new ExprResult());
    itsW.reset(new ExprResult());
}    

StatUVW::~StatUVW()
{
}

void StatUVW::calculate(const Request &request) const
{
    // Allocate result.
    size_t nTime = request[TIME]->size();

    Matrix Um, Vm, Wm;
    double *u = Um.setDoubleFormat(1, nTime);
    double *v = Vm.setDoubleFormat(1, nTime);
    double *w = Wm.setDoubleFormat(1, nTime);

    ValueSet::Ptr Ut(new ValueSet());
    ValueSet::Ptr Vt(new ValueSet());
    ValueSet::Ptr Wt(new ValueSet());

    Ut->assign(Um);
    Vt->assign(Vm);
    Wt->assign(Wm);
    
    itsU->setValue(Ut);
    itsV->setValue(Vt);
    itsW->setValue(Wt);

    // Use cached UVW coordinates if available.
    const Grid &reqGrid = request.getGrid();

    size_t nDone = 0;
    for(size_t i = 0; i < nTime; ++i)
    {
        Time time(reqGrid[TIME]->center(i));
        map<Time, Uvw>::const_iterator it = itsUvwCache.find(time);

        if(it != itsUvwCache.end())
        {
            u[i] = it->second.u;
            v[i] = it->second.v;
            w[i] = it->second.w;
            ++nDone;
        }
    }

    // If all done then return.
    if(nDone == nTime)
    {
        return;
    }

    // Compute missing UVW coordinates using the AIPS++ measures.

    // Get the station position relative to the array reference position
    // (to keep values small).
    const casa::MPosition mPos(itsStation.position.getValue()
        - itsArrayRef.getValue());

    //# Setup coordinate transformation engine.
    casa::Quantum<double> qEpoch(0.0, "s");
    casa::MEpoch mEpoch(qEpoch, casa::MEpoch::UTC);

    casa::MeasFrame frame(itsArrayRef);
    frame.set(itsPhaseRef->getPhaseRef());
    frame.set(mEpoch);

    casa::MVBaseline mvBaseline(mPos.getValue());
    casa::MBaseline mBaseline(mvBaseline, casa::MBaseline::ITRF);
    mBaseline.getRefPtr()->set(frame);

    casa::MBaseline::Convert convertor(mBaseline, casa::MBaseline::J2000);

    //# Compute missing UVW coordinates.
    for(size_t i = 0; i < nTime; ++i)
    {
        const double time = reqGrid[TIME]->center(i);
        map<Time, Uvw>::iterator it = itsUvwCache.find(Time(time));

        if(it == itsUvwCache.end())
        {
            qEpoch.setValue(time);
            mEpoch.set(qEpoch);
            frame.set(mEpoch);

            casa::MVuvw uvw2000(convertor().getValue(),
                itsPhaseRef->getPhaseRef().getValue());
            const casa::Vector<double> &xyz = uvw2000.getValue();

            u[i] = xyz(0);
            v[i] = xyz(1);
            w[i] = xyz(2);

            // Update UVW cache.
            itsUvwCache[Time(time)] = Uvw(xyz(0), xyz(1), xyz(2));
        }
    }

    itsLastReqId = request.getId();
}

} // namespace BBS
} // namespace LOFAR

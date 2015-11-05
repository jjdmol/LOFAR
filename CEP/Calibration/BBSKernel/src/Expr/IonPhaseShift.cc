//# IonPhaseShift.cc: Compute the ionospheric phase shift for a particular
//# pierce point using a single layer ionospheric model.
//#
//# Copyright (C) 2010
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
#include <BBSKernel/Expr/IonPhaseShift.h>
#include <Common/lofar_smartptr.h>

namespace LOFAR
{
namespace BBS
{

IonPhaseShift::IonPhaseShift(const Expr<Vector<4> >::ConstPtr &piercePoint,
    const Expr<Scalar>::ConstPtr &r0,
    const Expr<Scalar>::ConstPtr &beta)
    :   itsPiercePoint(piercePoint),
        itsR0(r0),
        itsBeta(beta)
{
    connect(itsPiercePoint);
    connect(itsR0);
    connect(itsBeta);
}

IonPhaseShift::~IonPhaseShift()
{
    for(vector<Expr<Vector<2> >::ConstPtr>::const_reverse_iterator it =
        itsTECWhite.rbegin(), end = itsTECWhite.rend(); it != end; ++it)
    {
        disconnect(*it);
    }

    for(vector<Expr<Vector<3> >::ConstPtr>::const_reverse_iterator it =
        itsCalPiercePoint.rbegin(), end = itsCalPiercePoint.rend(); it != end;
        ++it)
    {
        disconnect(*it);
    }

    disconnect(itsBeta);
    disconnect(itsR0);
    disconnect(itsPiercePoint);
}

unsigned int IonPhaseShift::nArguments() const
{
    return 3 + itsCalPiercePoint.size() + itsTECWhite.size();
}

ExprBase::ConstPtr IonPhaseShift::argument(unsigned int i) const
{
    ASSERT(i < nArguments());

    if(i == 0)
    {
        return itsPiercePoint;
    }
    else if(i == 1)
    {
        return itsR0;
    }
    else if(i == 2)
    {
        return itsBeta;
    }
    else if(i >= 3 && i < itsCalPiercePoint.size() + 3)
    {
        return itsCalPiercePoint[i - 3];
    }
    else
    {
        ASSERT(i >= itsCalPiercePoint.size() + 3);
        ASSERT(i - itsCalPiercePoint.size() - 3 < itsTECWhite.size());

        return itsTECWhite[i - itsCalPiercePoint.size() - 3];
    }
}

const JonesMatrix IonPhaseShift::evaluateExpr(const Request &request,
    Cache &cache, unsigned int grid) const
{
    // Evaluate arguments.
    const unsigned int nArg = nArguments();
    vector<FlagArray> flags;
    flags.reserve(nArg);

    const Vector<4> piercePoint =
        itsPiercePoint->evaluate(request, cache, grid);
    flags.push_back(piercePoint.flags());

    PiercePoint::ConstPtr tmp = dynamic_pointer_cast<const PiercePoint>(itsPiercePoint);
//    LOG_DEBUG_STR("Name: " << tmp->name());
//    LOG_DEBUG_STR("PiercePoint:X:" << piercePoint.value(0));
//    LOG_DEBUG_STR("PiercePoint:Y:" << piercePoint.value(1));
//    LOG_DEBUG_STR("PiercePoint:Z:" << piercePoint.value(2));
//    LOG_DEBUG_STR("PiercePoint:alpha:" << piercePoint.value(3));

    const Scalar r0 = itsR0->evaluate(request, cache, grid);
    flags.push_back(r0.flags());
//    LOG_DEBUG_STR("r_0: " << r0.value());

    const Scalar beta = itsBeta->evaluate(request, cache, grid);
    flags.push_back(beta.flags());
//    LOG_DEBUG_STR("beta: " << beta.value());

    vector<Vector<3> > calPiercePoint;
    calPiercePoint.reserve(itsCalPiercePoint.size());
    for(unsigned int i = 0; i < itsCalPiercePoint.size(); ++i)
    {
        const Vector<3> value =
            itsCalPiercePoint[i]->evaluate(request, cache, grid);
        calPiercePoint.push_back(value);
        flags.push_back(value.flags());
//        LOG_DEBUG_STR("CalPiercePoint:X:" << value.value(0));
//        LOG_DEBUG_STR("CalPiercePoint:Y:" << value.value(1));
//        LOG_DEBUG_STR("CalPiercePoint:Z:" << value.value(2));
    }

    vector<Vector<2> > tecWhite;
    tecWhite.reserve(itsTECWhite.size());
    for(unsigned int i = 0; i < itsTECWhite.size(); ++i)
    {
        const Vector<2> value = itsTECWhite[i]->evaluate(request, cache, grid);
        tecWhite.push_back(value);
        flags.push_back(value.flags());
//        LOG_DEBUG_STR("TECfit_white:0:" << value.value(0));
//        LOG_DEBUG_STR("TECfit_white:1:" << value.value(1));
    }

    EXPR_TIMER_START();

    // Compute main value.
    Grid reqGrid(request[grid]);
    const size_t nFreq = reqGrid[FREQ]->size();
    const size_t nTime = reqGrid[TIME]->size();

    // Check preconditions.
    ASSERT(calPiercePoint.size() == tecWhite.size());

    // Compute (differential) total electron content (TEC) at the piercepoint
    // (see memo by Bas van der Tol).
    Matrix r0sqr = r0.value() * r0.value();
    Matrix beta_2 = beta.value() * 0.5;
//    LOG_DEBUG_STR("r0sqr: " << r0sqr);
//    LOG_DEBUG_STR("beta: " << beta_2);

    Matrix tecX(0.0, 1, nTime);
    Matrix tecY(0.0, 1, nTime);
    for(size_t i = 0; i < calPiercePoint.size(); ++i)
    {
        Matrix dx = calPiercePoint[i].value(0) - piercePoint.value(0);
        Matrix dy = calPiercePoint[i].value(1) - piercePoint.value(1);
        Matrix dz = calPiercePoint[i].value(2) - piercePoint.value(2);
        Matrix weight = pow((dx * dx + dy * dy + dz * dz) / r0sqr, beta_2);

//        LOG_DEBUG_STR("dx: " << dx);
//        LOG_DEBUG_STR("dy: " << dy);
//        LOG_DEBUG_STR("dz: " << dz);
//        LOG_DEBUG_STR("weight: " << weight);

        tecX += weight * tecWhite[i].value(0);
        tecY += weight * tecWhite[i].value(1);
    }
    tecX *= -0.5;
    tecY *= -0.5;

    // Correct TEC value for the slant (angle of the line of sight w.r.t. the
    // normal to the ionospheric thin layer at the pierce point position). A
    // large slant implies a longer path through the ionosphere, and thus a
    // higher TEC value.
    tecX /= cos(piercePoint.value(3));
    tecY /= cos(piercePoint.value(3));

    // Convert from slanted TEC to phase shift.
    //
    // TODO: Because MeqMatrix cannot handle the elementwise product of an N x 1
    // times a 1 x M array, we have to write it out ourselves. Maybe this could
    // be made possible in MeqMatrix instead?
    Matrix shiftX, shiftY;
    shiftX.setDCMat(nFreq, nTime);
    shiftY.setDCMat(nFreq, nTime);
    double *shiftX_re, *shiftX_im;
    double *shiftY_re, *shiftY_im;
    shiftX.dcomplexStorage(shiftX_re, shiftX_im);
    shiftY.dcomplexStorage(shiftY_re, shiftY_im);

    ASSERT(!tecX.isComplex() && tecX.isArray()
        && static_cast<size_t>(tecX.nelements()) == nTime);
    const double *tecItX = tecX.doubleStorage();
    ASSERT(!tecY.isComplex() && tecY.isArray()
        && static_cast<size_t>(tecY.nelements()) == nTime);
    const double *tecItY = tecY.doubleStorage();

    for(size_t t = 0; t < nTime; ++t)
    {
        for(size_t f = 0; f < nFreq; ++f)
        {
            // Phase shift is equal to -k/nu * TECU, where k is a scaling
            // constant that is equal to 1.0e16 * c * r_e, where nu denotes
            // frequency, c the speed of light, and r_e the classical electron
            // radius (which is defined as e^2 / (4 * pi * e0 * m_e * c^2),
            // where e is the elementary charge, e0 is the electric constant,
            // and m_e elektron mass).
            //
            // (See e.g. http://en.wikipedia.org/wiki/Total_electron_content)
            //
            // TODO: Find out if and how to sign of the phase shift due to the
            // ionosphere is related to the definition of the plane wave used.

            double k_nu = 8.44797245e9 / reqGrid[FREQ]->center(f);

            double phaseX = k_nu * (*tecItX);
            double phaseY = k_nu * (*tecItY);

            *shiftX_re++ = std::cos(phaseX);
            *shiftX_im++ = std::sin(phaseX);
            *shiftY_re++ = std::cos(phaseY);
            *shiftY_im++ = std::sin(phaseY);
        }

        ++tecItX;
        ++tecItY;
    }

//    LOG_DEBUG_STR("TEC X: " << tecX);
//    LOG_DEBUG_STR("TEC Y: " << tecY);
//    LOG_DEBUG_STR("SHIFT X: " << shiftX);
//    LOG_DEBUG_STR("SHIFT Y: " << shiftY);

    JonesMatrix result;
    result.setFlags(mergeFlags(flags.begin(), flags.end()));
    result.assign(0, 0, shiftX);
    result.assign(0, 1, Matrix(makedcomplex(0.0, 0.0)));
    result.assign(1, 0, Matrix(makedcomplex(0.0, 0.0)));
    result.assign(1, 1, shiftY);

    EXPR_TIMER_STOP();

    return result;
}

} //# namespace BBS
} //# namespace LOFAR

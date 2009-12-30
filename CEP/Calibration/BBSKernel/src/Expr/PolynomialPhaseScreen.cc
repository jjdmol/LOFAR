//# PolynomialPhaseScreen.cc: Ionospheric phase for a station, direction pair
//# due to a global polynomial phase screen.
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

#include <BBSKernel/Expr/PolynomialPhaseScreen.h>

namespace LOFAR
{
namespace BBS
{

PolynomialPhaseScreen::~PolynomialPhaseScreen()
{
    for(unsigned int i = 0; i < itsCoeff.size(); ++i)
    {
        disconnect(itsCoeff[i]);
    }
    disconnect(itsPiercePoint);
}

unsigned int PolynomialPhaseScreen::nArguments() const
{
    return itsCoeff.size() + 1;
}

ExprBase::ConstPtr PolynomialPhaseScreen::argument(unsigned int i) const
{
    DBGASSERT(i < nArguments());
    if(i == 0)
    {
        return itsPiercePoint;
    }
    else
    {
        return itsCoeff[i - 1];
    }
}

const Scalar PolynomialPhaseScreen::evaluateExpr(const Request &request,
    Cache &cache, unsigned int grid) const
{
    // Allocate result.
    Scalar result;

    // Evaluate arguments.
    const unsigned int nArg = nArguments();
    vector<FlagArray> flags;
    flags.reserve(nArg);

    const Vector<4> pp = itsPiercePoint->evaluate(request, cache, grid);
    flags.push_back(pp.flags());

    vector<Scalar> coeff;
    coeff.reserve(nArg - 1);
    for(unsigned int i = 0; i < nArg - 1; ++i)
    {
        coeff.push_back(itsCoeff[i]->evaluate(request, cache, grid));
        flags.push_back(coeff[i].flags());
    }

    // Evaluate flags.
    result.setFlags(mergeFlags(flags.begin(), flags.end()));

    // Compute main value.
    vector<Scalar::View> coeffValue;
    coeffValue.reserve(nArg - 1);
    for(unsigned int i = 0; i < nArg - 1; ++i)
    {
        coeffValue.push_back(coeff[i].view());
    }
    result.assign(evaluateImpl(request[grid], pp.view(), coeffValue));

    // Compute perturbed values.
    Vector<4>::Iterator ppIt(pp);
    bool atEnd = ppIt.atEnd();

    vector<Scalar::Iterator> coeffIt;
    coeffIt.reserve(nArg - 1);
    for(unsigned int i = 0; i < nArg - 1; ++i)
    {
        coeffIt.push_back(Scalar::Iterator(coeff[i]));
        atEnd = atEnd && coeffIt.back().atEnd();
    }

    PValueKey key;
    while(!atEnd)
    {
        key = ppIt.key();
        for(unsigned int i = 0; i < nArg - 1; ++i)
        {
            key = std::min(key, coeffIt[i].key());
        }

        for(unsigned int i = 0; i < nArg - 1; ++i)
        {
            coeffValue[i] = coeffIt[i].value(key);
        }

        result.assign(key, evaluateImpl(request[grid], ppIt.value(key),
            coeffValue));

        ppIt.advance(key);
        atEnd = ppIt.atEnd();
        for(unsigned int i = 0; i < nArg - 1; ++i)
        {
            coeffIt[i].advance(key);
            atEnd = atEnd && coeffIt[i].atEnd();
        }
    }

    return result;
}

const Scalar::View PolynomialPhaseScreen::evaluateImpl(const Grid &grid,
    const Vector<4>::View &pp, const vector<Scalar::View> &coeff) const
{
    const size_t nFreq = grid[FREQ]->size();
    const size_t nTime = grid[TIME]->size();

    // Check preconditions.
    ASSERT(static_cast<size_t>(pp(0).nelements()) == nTime);
    ASSERT(static_cast<size_t>(pp(1).nelements()) == nTime);
    ASSERT(static_cast<size_t>(pp(2).nelements()) == nTime);
    ASSERT(static_cast<size_t>(pp(3).nelements()) == nTime);

    // Allocate space for the result.
    Matrix Z;
    Z.setDCMat(nFreq, nTime);
    double *Z_re, *Z_im;
    Z.dcomplexStorage(Z_re, Z_im);

    // Compute polynomial degree.
    unsigned int degree =
        static_cast<unsigned int>(std::sqrt(coeff.size() + 1));
    ASSERT(degree >= 1);

    // Get reference station position (ITRF).
    const casa::MVPosition &ref_pos = itsRefStation.getValue();

    // Calculate rotation matrix. Actually we only need to do this once for
    // all PolynomialPhaseScreen nodes. Some optimization could be done here.
    double lon = std::atan2(ref_pos(1), ref_pos(0));
    double lat = std::atan2(ref_pos(2), std::sqrt(ref_pos(0) * ref_pos(0)
        + ref_pos(1) * ref_pos(1)));

    double coslon = std::cos(lon);
    double sinlon = std::sin(lon);
    double coslat = std::cos(lat);
    double sinlat = std::sin(lat);

    // Compute ionospheric phase.
    vector<double> coeffValue(coeff.size());

    for(size_t t = 0; t < nTime; ++t)
    {
        double x = pp(0).getDouble(0, t);
        double y = pp(1).getDouble(0, t);
        double z = pp(2).getDouble(0, t);
        double alpha = pp(3).getDouble(0, t);

        // TODO: Is the index (0, t) correct in the code below?? (JvZ)
        for(unsigned int i = 0; i < coeff.size(); ++i)
        {
            coeffValue[i] = coeff[i]().getDouble(0, t);
        }

        double rot_x = -sinlon * x + coslon * y;
        double rot_y = -sinlat * coslon * x - sinlat * sinlon * y + coslat * z;

        double res = 0.0;
        for(int ilon = degree - 1; ilon >= 0; --ilon)
        {
            res *= rot_x / 1000.0;
            double resy = 0.0;
            for(int ilat = degree - 1; ilat >= 0; --ilat)
    	    {
                resy *= rot_y / 1000.0;
    	        if(ilon == 0 && ilat == 0)
    	        {
    	            continue;
    	        }
                resy += coeffValue[ilon * degree + ilat - 1];
    	    }
            res += resy;
        }

        double tec = res / std::cos(alpha);

        // Convert TEC to frequency dependent phase.
        for(size_t f = 0; f < nFreq; ++f)
        {
    	    double phase = (75e8 / grid[FREQ]->center(f)) * tec;
            *Z_re++ = std::cos(phase);
            *Z_im++ = std::sin(phase);
        }
    }

    Scalar::View result;
    result.assign(Z);

    return result;
}

} //# namespace BBS
} //# namespace LOFAR

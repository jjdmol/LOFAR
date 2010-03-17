//# EquatorialCentroid.cc: Compute the centroid of a list of positions in
//# spherical coordinates.
//#
//# Copyright (C) 2009
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
#include <BBSKernel/Expr/EquatorialCentroid.h>

namespace LOFAR
{
namespace BBS
{

EquatorialCentroid::EquatorialCentroid()
{
}

EquatorialCentroid::~EquatorialCentroid()
{
    for(unsigned int i = 0; i < itsArgs.size(); ++i)
    {
        disconnect(itsArgs[i]);
    }
}

void EquatorialCentroid::connect(const Expr<Vector<2> >::ConstPtr &arg)
{
    ExprBase::connect(arg);
    itsArgs.push_back(arg);
}

unsigned int EquatorialCentroid::nArguments() const
{
    return itsArgs.size();
}

ExprBase::ConstPtr EquatorialCentroid::argument(unsigned int i) const
{
    ASSERT(i < itsArgs.size());
    return itsArgs[i];
}

const Vector<2> EquatorialCentroid::evaluateExpr(const Request &request,
    Cache &cache, unsigned int grid) const
{
    // Allocate result.
    Vector<2> result;

    // Evaluate arguments.
    const unsigned int nArg = nArguments();
    vector<FlagArray> flags;
    flags.reserve(nArg);

    vector<Vector<2> > args;
    args.reserve(nArg);
    for(unsigned int i = 0; i < nArg; ++i)
    {
        args.push_back(itsArgs[i]->evaluate(request, cache, grid));
        flags.push_back(args[i].flags());
    }

    // Evaluate flags.
    result.setFlags(mergeFlags(flags.begin(), flags.end()));

    // Compute main value.
    vector<Vector<2>::View> argValues;
    argValues.reserve(nArg);
    for(unsigned int i = 0; i < nArg; ++i)
    {
        argValues.push_back(args[i].view());
    }
    result.assign(evaluateImpl(request[grid], argValues));

    // Compute perturbed values.
    vector<Vector<2>::Iterator> argIt;
    argIt.reserve(nArg);

    bool atEnd = true;
    for(unsigned int i = 0; i < nArg; ++i)
    {
        argIt.push_back(Vector<2>::Iterator(args[i]));
        atEnd = atEnd && argIt.back().atEnd();
    }

    PValueKey key;
    while(!atEnd)
    {
        key = argIt.front().key();
        for(unsigned int i = 1; i < nArg; ++i)
        {
            key = std::min(key, argIt[i].key());
        }

        for(unsigned int i = 0; i < nArg; ++i)
        {
            argValues[i] = argIt[i].value(key);
        }

        result.assign(key, evaluateImpl(request[grid], argValues));

        argIt.front().advance(key);
        atEnd = argIt.front().atEnd();
        for(unsigned int i = 1; i < nArg; ++i)
        {
            argIt[i].advance(key);
            atEnd = atEnd && argIt[i].atEnd();
        }
    }

    return result;
}

const Vector<2>::View EquatorialCentroid::evaluateImpl(const Grid &grid,
    const vector<Vector<2>::View> &args) const
{
    Matrix cosDec = cos(args[0](1));
    Matrix x = cos(args[0](0)) * cosDec;
    Matrix y = sin(args[0](0)) * cosDec;
    Matrix z = sin(args[0](1));

    for(unsigned int i = 1; i < args.size(); ++i)
    {
        cosDec = cos(args[i](1));
        x = x + cos(args[i](0)) * cosDec;
        y = y + sin(args[i](0)) * cosDec;
        z = z + sin(args[i](1));
    }

    x /= args.size();
    y /= args.size();
    z /= args.size();

    Vector<2>::View result;
    result.assign(0, atan2(y, x));
    result.assign(1, asin(z));

    return result;
}

} //# namespace BBS
} //# namespace LOFAR

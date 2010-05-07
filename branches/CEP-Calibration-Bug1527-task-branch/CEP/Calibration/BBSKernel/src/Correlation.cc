//# Correlation.cc: Definition of supported correlation products.
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
#include <BBSKernel/Correlation.h>

namespace LOFAR
{

ostream &operator<<(ostream &out, BBS::Correlation::Type obj)
{
    out << BBS::Correlation::asString(obj);
    return out;
}

namespace BBS
{

bool Correlation::isDefined(Type in)
{
    return in != N_Type;
}

bool Correlation::isLinear(Type in)
{
    return in == XX || in == XY || in == YX || in == YY;
}

bool Correlation::isCircular(Type in)
{
    return in == RR || in == RL || in == LR || in == LL;
}

Correlation::Type Correlation::asCorrelation(unsigned int in)
{
    Type out = N_Type;
    if(in < N_Type)
    {
        out = static_cast<Type>(in);
    }

    return out;
}

Correlation::Type Correlation::asCorrelation(const string &in)
{
    Type out = N_Type;
    for(unsigned int i = 0; i < N_Type; ++i)
    {
        if(in == asString(static_cast<Type>(i)))
        {
            out = static_cast<Type>(i);
            break;
        }
    }

    return out;
}

const string &Correlation::asString(Type in)
{
    //# Caution: Always keep this array of strings in sync with the enum
    //# Type that is defined in the header.
    static const string cr[N_Type + 1] =
        {"XX",
        "XY",
        "YX",
        "YY",
        "RR",
        "RL",
        "LR",
        "LL",
        //# "<UNDEFINED>" should always be last.
        "<UNDEFINED>"};

    return cr[in];
}

} //# namespace BBS
} //# namespace LOFAR

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
namespace BBS
{

bool isDefined(Correlation in)
{
    return in != N_Correlation;
}

bool isLinear(Correlation in)
{
    return in == XX || in == XY || in == YX || in == YY;
}

bool isCircular(Correlation in)
{
    return in == RR || in == RL || in == LR || in == LL;
}

Correlation asCorrelation(unsigned int in)
{
    Correlation out = N_Correlation;
    if(in < N_Correlation)
    {
        out = static_cast<Correlation>(in);
    }

    return out;
}

Correlation asCorrelation(const string &in)
{
    Correlation out = N_Correlation;
    for(unsigned int i = 0; i < N_Correlation; ++i)
    {
        if(in == asString(static_cast<Correlation>(i)))
        {
            out = static_cast<Correlation>(i);
            break;
        }
    }

    return out;
}

const string &asString(Correlation in)
{
    //# Caution: Always keep this array of strings in sync with the enum
    //# Correlation that is defined in the header.
    static const string cr[N_Correlation + 1] =
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

ostream &operator<<(ostream &out, Correlation obj)
{
    out << asString(obj);
    return out;
}

} //# namespace BBS
} //# namespace LOFAR

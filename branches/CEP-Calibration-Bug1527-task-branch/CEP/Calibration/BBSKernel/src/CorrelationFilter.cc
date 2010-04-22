//# CorrelationFilter.cc: Filter used to select correlation products.
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
#include <BBSKernel/CorrelationFilter.h>
#include <BBSKernel/Exceptions.h>

namespace LOFAR
{
namespace BBS
{
using LOFAR::operator<<;

bool CorrelationFilter::empty() const
{
    return itsCorrelations.empty();
}

void CorrelationFilter::append(Correlation correlation)
{
    if(!isDefined(correlation))
    {
        THROW(BBSKernelException, "Caught attempt to append an undefined"
            " correlation.");
    }

    itsCorrelations.insert(correlation);
}

void CorrelationFilter::append(const string &correlation)
{
    append(asCorrelation(correlation));
}

ostream &operator<<(ostream &out, const CorrelationFilter &obj)
{
    out << "CorrelationFilter: [";
    for(set<Correlation>::const_iterator cr_it = obj.itsCorrelations.begin(),
        cr_end = obj.itsCorrelations.end(); cr_it != cr_end; ++cr_it)
    {
        out << " " << asString(*cr_it);
    }
    out << "]";

    return out;
}

} //# namespace BBS
} //# namespace LOFAR

//# LinearToCircularRL.cc: Jones matrix to convert from linear polarization
//# coordinates to circular (RL) coordinates.
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
#include <BBSKernel/Expr/LinearToCircularRL.h>

namespace LOFAR
{
namespace BBS
{

LinearToCircularRL::LinearToCircularRL()
{
    const double scale = 1.0 / std::sqrt(2.0);

    Matrix re(scale);
    Matrix im(dcomplex(0.0, scale));

    itsH.assign(0, 0, re);
    itsH.assign(0, 1, im);
    itsH.assign(1, 0, re);
    itsH.assign(1, 1, conj(im));
}

const JonesMatrix LinearToCircularRL::evaluateExpr(const Request&, Cache&,
    unsigned int) const
{
    return itsH;
}

} //# namespace BBS
} //# namespace LOFAR

//# PointSource.cc: Class holding the expressions defining a point source.
//#
//# Copyright (C) 2002
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
#include <BBSKernel/Expr/PointSource.h>

namespace LOFAR
{
namespace BBS
{

PointSource::PointSource(const string &name, const Expr &ra, const Expr &dec,
    const Expr &I, const Expr &Q, const Expr &U, const Expr &V)
    :   Source(name, ra, dec),
        itsI(I),
        itsQ(Q),
        itsU(U),
        itsV(V)
{
}

} // namespace BBS
} // namespace LOFAR

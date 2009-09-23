//# PointSource.h: Class holding the expressions defining a point source.
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

#ifndef EXPR_POINTSOURCE_H
#define EXPR_POINTSOURCE_H

// \file
// Class holding the expressions defining a point source.

#include <BBSKernel/Expr/Source.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class PointSource: public Source
{
public:
    typedef shared_ptr<PointSource>          Pointer;
    typedef shared_ptr<const PointSource>    ConstPointer;

    PointSource(const string& name, const Expr &ra, const Expr &dec,
        const Expr &I, const Expr &Q, const Expr &U, const Expr &V,
        const Expr &si);

    const Expr &getI() const
    { return itsI; }
    const Expr &getQ() const
    { return itsQ; }
    const Expr &getU() const
    { return itsU; }
    const Expr &getV() const
    { return itsV; }
    const Expr &getSpectralIndex() const
    { return itsSpectralIndex; }

private:
    Expr    itsI;
    Expr    itsQ;
    Expr    itsU;
    Expr    itsV;
    Expr    itsSpectralIndex;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif

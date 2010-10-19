//# PointCoherence.h: Spatial coherence function of a point source.
//#
//# Copyright (C) 2005
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

#ifndef EXPR_POINTCOHERENCE_H
#define EXPR_POINTCOHERENCE_H

// \file
// Spatial coherence function of a point source.

#include <BBSKernel/Expr/JonesExpr.h>
#include <BBSKernel/Expr/PointSource.h>


namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class PointCoherence: public JonesExprRep
{
public:
    PointCoherence(const PointSource::ConstPointer &source);

    virtual JonesResult getJResult(const Request &request);
};

// @}

} // namespace BBS
} // namespace LOFAR
#endif

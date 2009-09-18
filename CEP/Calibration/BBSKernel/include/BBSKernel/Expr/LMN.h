//# LMN.h: LMN-coordinates of a direction on the sky.
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

#ifndef EXPR_LMN_H
#define EXPR_LMN_H

// \file
// LMN-coordinates of a direction on the sky.

#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/Source.h>
#include <BBSKernel/Expr/PhaseRef.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class LMN: public ExprRep
{
public:
    LMN(const Source::Pointer &source,
        const PhaseRef::ConstPointer &phaseRef);
    ~LMN();

    const Source::Pointer &getSource() const
    { return itsSource; }

    ResultVec getResultVec(const Request &request);

private:

    Source::Pointer          itsSource;
    PhaseRef::ConstPointer   itsPhaseRef;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif

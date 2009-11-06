//# SpectralIndex.h: Frequency dependent scale factor for the base flux given
//# for a specific reference frequency.
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

#ifndef LOFAR_BBS_EXPR_SPECTRALINDEX_H
#define LOFAR_BBS_EXPR_SPECTRALINDEX_H

// \file
// Frequency dependent scale factor for the base flux given for a specific
// reference frequency.

#include <BBSKernel/Expr/Expr.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class SpectralIndex: public ExprRep
{
public:
    template <typename T_Iterator>
    SpectralIndex(const Expr &refFreq, const Expr &refStokes, T_Iterator first,
        T_Iterator last);

    virtual Matrix getResultValue(const Request &request,
        const std::vector<const Matrix*> &args);
};

template <typename T_Iterator>
SpectralIndex::SpectralIndex(const Expr &refFreq, const Expr &refStokes,
    T_Iterator first, T_Iterator last)
{
    addChild(refFreq);
    addChild(refStokes);
    while(first != last)
    {
        addChild(*first++);
    }
}

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif

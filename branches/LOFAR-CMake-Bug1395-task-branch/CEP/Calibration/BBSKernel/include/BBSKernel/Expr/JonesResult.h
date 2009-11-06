//# JonesResult.h: The result of a Jones expression.
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

#ifndef EXPR_JONESRESULT_H
#define EXPR_JONESRESULT_H

// \file
// The result of a Jones expression.

#include <BBSKernel/Expr/ResultVec.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class JonesResult: public ResultVec
{
public:
    JonesResult()
        : ResultVec()
    {
    }
    
    void init()
    {
        if(itsRep == 0)
        {
            ResultVec::operator=(ResultVec(4));
        }
    }

    JonesResult(const JonesResult &other)
        : ResultVec(other)
    {
    }

    JonesResult &operator=(const JonesResult &other)
    {
        ResultVec::operator=(other);
        return *this;
    }

    // Get the individual results.
    // <group>
    const Result& getResult11() const
    { return (*this)[0]; }
    Result& result11()
    { return (*this)[0]; }
    const Result& getResult12() const
    { return (*this)[1]; }
    Result& result12()
    { return (*this)[1]; }
    const Result& getResult21() const
    { return (*this)[2]; }
    Result& result21()
    { return (*this)[2]; }
    const Result& getResult22() const
    { return (*this)[3]; }
    Result& result22()
    { return (*this)[3]; }
    // </group>
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif

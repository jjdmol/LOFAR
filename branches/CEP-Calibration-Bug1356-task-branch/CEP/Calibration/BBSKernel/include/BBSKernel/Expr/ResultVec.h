//# ResultVec.h: A vector containing multiple results.
//#
//# Copyright (C) 2005
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef EXPR_RESULTVEC_H
#define EXPR_RESULTVEC_H

// \file
// A vector containing multiple results.

#include <BBSKernel/Expr/Result.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class ResultVecRep
{
public:
    explicit ResultVecRep(size_t size);
    ~ResultVecRep();

    ResultVecRep *link()
    {
        ++itsRefCount;
        return this;
    }

    static void unlink(ResultVecRep *rep)
    {
        if(rep != 0  &&  --rep->itsRefCount == 0)
        {
            delete rep;
        }
    }        

    vector<Result> &results()
    { return itsResult; }

private:
    // Forbid copy and assignment.
    ResultVecRep(const ResultVecRep&);
    ResultVecRep &operator=(const ResultVecRep&);

    size_t              itsRefCount;
    vector<Result>   itsResult;
};

class ResultVec
{
public:
    ResultVec();
    explicit ResultVec(size_t size);
    ~ResultVec();

    ResultVec(const ResultVec &other);
    ResultVec &operator=(const ResultVec &other);

    // Get the size of the result vector.
    size_t size() const
    { return itsRep->results().size(); }

    // Get the result stored at the given index.
    // <group>
    const Result &operator[](size_t index) const
    { return itsRep->results()[index]; }
    Result &operator[](size_t index)
    { return itsRep->results()[index]; }
    // </group>

protected:
    ResultVecRep *itsRep;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif

//# ResultVec.cc: A vector containing multiple results.
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

#include <lofar_config.h>
#include <BBSKernel/Expr/ResultVec.h>

namespace LOFAR
{
namespace BBS
{

ResultVecRep::ResultVecRep(size_t size)
    : itsRefCount(0)
{
    itsResult.resize(size);
    for(size_t i = 0; i < size; ++i)
    {
        itsResult[i].init();
    }
}

ResultVecRep::~ResultVecRep()
{
}

// -------------------------------------------------------------------------- //

ResultVec::ResultVec()
    : itsRep(0)
{
}

ResultVec::ResultVec(size_t size)
    : itsRep(new ResultVecRep(size))
{
    itsRep->link();
}

ResultVec::~ResultVec()
{
    ResultVecRep::unlink(itsRep);
}

ResultVec::ResultVec(const ResultVec &other)
    : itsRep(other.itsRep)
{
    if(itsRep != 0)
    {
        itsRep->link();
    }
}

ResultVec &ResultVec::operator=(const ResultVec &other)
{
    if(this != &other)
    {
        ResultVecRep::unlink(itsRep);
        itsRep = other.itsRep;
        if(itsRep != 0)
        {
            itsRep->link();
        }
    }
    
    return *this;
}

} // namespace BBS
} // namespace LOFAR

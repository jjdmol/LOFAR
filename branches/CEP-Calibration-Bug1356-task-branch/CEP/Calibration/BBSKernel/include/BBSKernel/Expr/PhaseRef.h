//# PhaseRef.h: Phase reference position and derived values.
//#
//# Copyright (C) 2002
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

#ifndef EXPR_PHASEREF_H
#define EXPR_PHASEREF_H

// \file
// Phase reference position and derived values.

#include <Common/lofar_smartptr.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class PhaseRef
{
public:
    typedef shared_ptr<PhaseRef>         Ptr;
    typedef shared_ptr<const PhaseRef>   ConstPtr;

    PhaseRef();
    PhaseRef(const casa::MDirection &phaseRef);
    ~PhaseRef();

    double getRa() const
    { return itsRa; }
    double getDec() const
    { return itsDec; }
    double getSinDec() const
    { return itsSinDec; }
    double getCosDec() const
    { return itsCosDec; }
    
    const casa::MDirection &getPhaseRef() const
    { return itsPhaseRef; }

private:
    double  itsRa;
    double  itsDec;
    double  itsSinDec;
    double  itsCosDec;

    casa::MDirection    itsPhaseRef;
    casa::MPosition     itsArrayRef;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif

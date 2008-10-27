//# StationShift.h: Station part of baseline phase shift.
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

#if !defined(EXPR_POINTDFT_H)
#define EXPR_POINTDFT_H

// \file
// Station part of baseline phase shift.

//# Includes
#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/StatUVW.h>

#include <Common/lofar_vector.h>

#ifdef EXPR_GRAPH
#include <Common/lofar_string.h>
#endif

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class DFTPS: public ExprRep
{
public:
    // Construct from source list, phase reference position and uvw.
    DFTPS(const StatUVW::ConstPointer &uvw, const Expr &lmn);
    virtual ~DFTPS();

    // Get the result of the expression for the given domain.
    virtual ResultVec getResultVec(const Request &request);

private:
#ifdef EXPR_GRAPH
    virtual string getLabel();
#endif

    StatUVW::ConstPointer    itsUVW;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif

//# PointCoherence.h: Spatial coherence function of a point source.
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

#ifndef EXPR_POINTCOHERENCE_H
#define EXPR_POINTCOHERENCE_H

// \file
// Spatial coherence function of a point source.

#include <BBSKernel/Expr/JonesExpr.h>
#include <BBSKernel/Expr/PointSource.h>

#ifdef EXPR_GRAPH
#include <Common/lofar_string.h>
#endif

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
    ~PointCoherence();

    virtual JonesResult getJResult(const Request &request);

private:
#ifdef EXPR_GRAPH
    virtual string getLabel();
#endif

    PointSource::ConstPointer    itsSource;
};

// @}

} // namespace BBS
} // namespace LOFAR
#endif

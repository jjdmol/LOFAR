//# Diag.cc: A diagonal Jones matrix.
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

#include <BBSKernel/Expr/Diag.h>
#include <BBSKernel/Expr/Expr.h>

namespace LOFAR
{
namespace BBS
{

Diag::Diag(const Expr &xx, const Expr &yy)
{
    addChild(xx);
    addChild(yy);
}

Diag::~Diag()
{
}

JonesResult Diag::getJResult(const Request &request)
{
    JonesResult result;
    result.init();

    getChild(0).getResultSynced(request, result.result11());
    getChild(1).getResultSynced(request, result.result22());
    result.result12().setValue(Matrix(0.));
    result.result21().setValue(Matrix(0.));

    return result;
}

} // namespace BBS
} // namespace LOFAR

//# JonesNode.cc: A node in a Jones matrix expression.
//#
//# Copyright (C) 2002
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

#include <lofar_config.h>

#include <BBSKernel/Expr/JonesNode.h>
#include <BBSKernel/Expr/Expr.h>

namespace LOFAR
{
namespace BBS
{

JonesNode::JonesNode (const Expr& elem11, const Expr& elem12,
                const Expr& elem21, const Expr& elem22)
: itsExpr11 (elem11),
  itsExpr12 (elem12),
  itsExpr21 (elem21),
  itsExpr22 (elem22)
{
  addChild (itsExpr11);
  addChild (itsExpr12);
  addChild (itsExpr21);
  addChild (itsExpr22);
}

JonesNode::~JonesNode()
{}

JonesResult JonesNode::getJResult (const Request& request)
{
  JonesResult res;
  res.init();
  {
    itsExpr11.getResultSynced (request, res.result11());
    itsExpr12.getResultSynced (request, res.result12());
    itsExpr21.getResultSynced (request, res.result21());
    itsExpr22.getResultSynced (request, res.result22());
  }
  return res;
}


} // namespace BBS
} // namespace LOFAR

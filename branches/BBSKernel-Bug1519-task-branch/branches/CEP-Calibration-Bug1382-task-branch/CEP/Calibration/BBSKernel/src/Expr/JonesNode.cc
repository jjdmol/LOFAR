//# JonesNode.cc: A node in a Jones matrix expression.
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

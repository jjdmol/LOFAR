//# JonesSum.h: A summation of JonesExpr
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

#ifndef EXPR_JONESSUM_H
#define EXPR_JONESSUM_H

// \file
// A sum of JonesExpr

//# Includes
#include <BBSKernel/Expr/JonesExpr.h>
#include <vector>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{


// This class adds the results of multiple JonesExpr objects.

class JonesSum: public JonesExprRep
{
public:
  // Construct from four Jones elements.
  JonesSum (const std::vector<JonesExpr>& expr);

  virtual ~JonesSum();

  // Calculate the result of its members.
  virtual JonesResult getJResult (const Request&);

private:
  void mergePValues(const Result &in, Result &out);

  std::vector<JonesExpr> itsExpr;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif

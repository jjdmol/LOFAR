//# Diag.h: The Jones expression for a diagonal matrix
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

#if !defined(MNS_MEQDIAG_H)
#define MNS_MEQDIAG_H

// \file MNS/MeqDiag.h
// The Jones expression for a diagonal matrix

//# Includes
#include <BBSKernel/MNS/MeqJonesExpr.h>
#include <BBSKernel/MNS/MeqExpr.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \ingroup MNS
// @{

//# Forward Declarations
class Expr;
class JonesResult;

// This class is a diagonal node in a Jones matrix expression.

class Diag: public JonesExprRep
{
public:
  // Construct from the various subexpressions.
  Diag (const Expr& xx, const Expr& yy);

  virtual ~Diag();

  // Calculate the result of its members.
  virtual JonesResult getJResult (const Request&);

private:
  Expr itsXX;
  Expr itsYY;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif

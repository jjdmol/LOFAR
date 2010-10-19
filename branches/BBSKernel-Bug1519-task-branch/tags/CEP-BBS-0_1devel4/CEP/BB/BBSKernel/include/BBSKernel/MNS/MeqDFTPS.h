//# MeqPointDFT.h: The DFT for a point source
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

#if !defined(MNS_MEQPOINTDFT_H)
#define MNS_MEQPOINTDFT_H

// \file
// The DFT for a point source.

//# Includes
#include <BBSKernel/MNS/MeqExpr.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \addtogroup MNS
// @{

//# Forward declarations
class MeqLMN;
class MeqStatUVW;

// This class is the (abstract) base class for an expression.

class MeqDFTPS: public MeqExprRep
{
public:
  // Construct from source list, phase reference position and uvw.
  explicit MeqDFTPS (const MeqExpr& lmn, MeqStatUVW*);

  virtual ~MeqDFTPS();

  // Get the result of the expression for the given domain.
  virtual MeqResultVec getResultVec (const MeqRequest&);

private:
#ifdef EXPR_GRAPH
  virtual std::string getLabel();
#endif

  MeqExpr     itsLMN;
  MeqStatUVW* itsUVW;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif

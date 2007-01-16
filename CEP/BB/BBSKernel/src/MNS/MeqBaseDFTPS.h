//# MeqBaseDFTPS.h: Baseline DFT of a point source
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

#ifndef MNS_MEQBASEDFTPS_H
#define MNS_MEQBASEDFTPS_H

// \file
// Baseline DFT of a point source.

//# Includes
#include <BBSKernel/MNS/MeqExpr.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \addtogroup MNS
// @{

class MeqBaseDFTPS: public MeqExprRep
{
public:
  MeqBaseDFTPS (const MeqExpr& left, const MeqExpr& right,
        const MeqExpr& lmn);

  ~MeqBaseDFTPS();

  // Calculate the results for the given domain.
  virtual MeqResult getResult (const MeqRequest&);

private:
#ifdef EXPR_GRAPH
  virtual std::string getLabel();
#endif

  MeqExpr itsLeft;
  MeqExpr itsRight;
  MeqExpr itsLMN;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif

//# MeqBaseLinPS.h: Baseline prediction of a point source with linear polarisation
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

#ifndef MNS_MEQBASELINPS_H
#define MNS_MEQBASELINPS_H

// \file
// Baseline prediction of a point source with linear polarisation.

//# Includes
#include <BBSKernel/MNS/MeqJonesExpr.h>
#include <BBSKernel/MNS/MeqLMN.h>
#include <BBSKernel/MNS/MeqPointSource.h>
#include <BBSKernel/MNS/MeqDFTPS.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \addtogroup MNS
// @{

//# Forward Declarations


class MeqBaseLinPS: public MeqJonesExprRep
{
public:
  MeqBaseLinPS (const MeqExpr& dft, MeqPointSource* src);

  ~MeqBaseLinPS();

  // Calculate the results for the given domain.
  virtual MeqJonesResult getJResult (const MeqRequest&);

private:
#ifdef EXPR_GRAPH
  virtual std::string getLabel();
#endif

  MeqExpr         itsDFT;
  MeqPointSource* itsSource;
};

// @}

} // namespace BBS
} // namespace LOFAR
#endif

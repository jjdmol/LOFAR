//# MeqPointSource.h: Class holding a point source
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

#ifndef MNS_MEQPOINTSOURCE_H
#define MNS_MEQPOINTSOURCE_H

// \file
// Class holding a point source

//# Includes
#include <BBSKernel/MNS/MeqSource.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \addtogroup MNS
// @{


class MeqPointSource: public MeqSource
{
public:
  MeqPointSource (const string& name,
          const string& groupName,
          const MeqExpr& fluxI, const MeqExpr& fluxQ,
          const MeqExpr& fluxU, const MeqExpr& fluxV,
          const MeqExpr& ra, const MeqExpr& dec);

  virtual ~MeqPointSource();

  MeqExpr& getI()
    { return itsI; }
  MeqExpr& getQ()
    { return itsQ; }
  MeqExpr& getU()
    { return itsU; }
  MeqExpr& getV()
    { return itsV; }

private:
  MeqExpr   itsI;
  MeqExpr   itsQ;
  MeqExpr   itsU;
  MeqExpr   itsV;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif

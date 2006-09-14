//# MeqLMN.h: Class holding the LMN values of a point source
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

#ifndef MNS_MEQLMN_H
#define MNS_MEQLMN_H

// \file
// Class holding the LMN values of a point source

//# Includes
#include <BBS/MNS/MeqExpr.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBS
// \addtogroup MNS
// @{

//# Forward Declarations
class MeqSource;
class MeqPhaseRef;


class MeqLMN: public MeqExprRep
{
public:
  // Construct for the given point source.
  explicit MeqLMN (MeqSource*);

  ~MeqLMN();

  const MeqSource& getSource() const
    { return *itsSource; }

  // Set the phase reference position.
  void setPhaseRef (const MeqPhaseRef* phaseRef)
    { itsPhaseRef = phaseRef; }

  // Get the precalculated result of l, m, or n.
  MeqResultVec getResultVec (const MeqRequest&);

  MeqResultVec getAnResultVec (const MeqRequest& request);

private:
#ifdef EXPR_GRAPH
  virtual std::string getLabel();
#endif

  MeqSource*         itsSource;
  const MeqPhaseRef* itsPhaseRef;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif

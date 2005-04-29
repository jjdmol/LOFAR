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

// \file MNS/MeqLMN.h
// Class holding the LMN values of a point source

//# Includes
#include <BBS3/MNS/MeqResultVec.h>
#include <BBS3/MNS/MeqRequest.h>
#include <Common/lofar_string.h>

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

//# Forward Declarations
class MeqPointSource;
class MeqPhaseRef;


class MeqLMN
{
public:
  // Construct for the given point source.
  explicit MeqLMN (MeqPointSource*);

  const MeqPointSource& getSource() const
    { return *itsSource; }

  // Set the phase reference position.
  void setPhaseRef (const MeqPhaseRef* phaseRef)
    { itsPhaseRef = phaseRef; }

  // Get the precalculated result of l, m, or n.
  MeqResultVec getResultVec (const MeqRequest&);

private:
  MeqPointSource*    itsSource;
  const MeqPhaseRef* itsPhaseRef;
  MeqRequestId itsLastReqId;
  MeqResultVec itsResult;
};

// @}

}

#endif

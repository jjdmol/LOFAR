//# MeqPointDFT.h: The base class of an expression.
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

//# Includes
#include <PSS3/MNS/MeqExpr.h>
#include <Common/lofar_vector.h>

//# Forward declarations
class MeqStatSources;


// This class is the (abstract) base class for an expression.

class MeqPointDFT: public MeqExpr
{
public:
  // Construct from source list, pahse reference position and uvw.
  MeqPointDFT (MeqStatSources* left, MeqStatSources* right);

  virtual ~MeqPointDFT();

  // Get the result of the expression for the given domain.
  virtual MeqResult getResult (const MeqRequest&);

  // Find nr of cells to use for the given source and domain.
  vector<int> ncells (int sourceNr, const MeqRequest&);

  static bool doshow;
private:
  MeqStatSources* itsLeft;
  MeqStatSources* itsRight;
};


#endif

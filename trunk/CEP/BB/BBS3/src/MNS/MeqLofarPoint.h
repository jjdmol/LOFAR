//# MeqLofarPoint.h: The total baseline prediction of point sources in the LOFAR model
//#
//# Copyright (C) 2003
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

#if !defined(MNS_MEQLOFARPOINT_H)
#define MNS_MEQLOFARPOINT_H

// \file MNS/MeqLofarPoint.h
// The total baseline prediction of point sources in the LOFAR model

//# Includes
#include <BBS3/MNS/MeqJonesExpr.h>
#include <BBS3/MNS/MeqSourceList.h>
#include <BBS3/MNS/MeqResult.h>
#include <Common/lofar_vector.h>

namespace LOFAR {
  
// \ingroup BBS3
// \addtogroup MNS
// @{

//# Forward Declarations
class MeqHist;
class MeqLofarStatSources;


class MeqLofarPoint: public MeqJonesExprRep
{
public:
  MeqLofarPoint (MeqSourceList* sources,
		 MeqLofarStatSources* left, MeqLofarStatSources* right,
		 MeqHist* celltHistogram, MeqHist* cellfHistogram);
       
  ~MeqLofarPoint();

  // Calculate the results for the given domain.
  virtual void calcResult (const MeqRequest&);

  // Get the number of cells needed.
  const vector<int>& ncells() const
    { return itsNcell; }

private:
  MeqSourceList*       itsSources;
  MeqLofarStatSources* itsLeft;
  MeqLofarStatSources* itsRight;
  MeqHist*             itsCelltHist;
  MeqHist*             itsCellfHist;
  vector<int>          itsNcell;
};

// @}

}

#endif

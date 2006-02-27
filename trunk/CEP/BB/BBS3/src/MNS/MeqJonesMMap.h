//# MeqJonesMMap.h: Get part of a mapped MS
//#
//# Copyright (C) 2006
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

#ifndef MNS_MEQJONESMMAP_H
#define MNS_MEQJONESMMAP_H

// \file
// Get part of the data from the Mapped MS.

//# Includes
#include <BBS3/MNS/MeqJonesExpr.h>
#include <BBS3/MMapMSInfo.h>

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{


// Read data from MS for a specific baseline.

class MeqJonesMMap: public MeqJonesExprRep
{
public:
  MeqJonesMMap (const MMapMSInfo&, int blnr);

  ~MeqJonesMMap();

  // Get the result of the expression for the given domain.
  virtual MeqJonesResult getJResult (const MeqRequest&);

  // Put the result of the expression.
  void putJResult (const MeqJonesResult&, const MeqRequest&);

private:
  const MMapMSInfo* itsInfo;
  int64             itsOffsetBL;
};

// @}

}

#endif

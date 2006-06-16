//# BBSStep.h: The properties for solvable parameters
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

#ifndef LOFAR_BBSCONTROL_BBSSTEP_H
#define LOFAR_BBSCONTROL_BBSSTEP_H

// \file
// The properties for solvable parameters

//# Includes
#include <BBS/MNS/MeqMatrix.h>
#include <BBS/MNS/MeqFunklet.h>
#include <Common/LofarTypes.h>
#include <string>
#include <vector>

namespace LOFAR {

// \addtogroup BBS
// @{

//# Forward Declarations.
class BlobOStream;
class BlobIStream;

class BBSStep
{
public:
  void setSolvableParms (const vector<string>& parms, 
			 const vector<string>& excludePatterns)
    { itsSolvParms = parms; itsNonSolvParms = excludePatterns; }

private:
  vector<string>& itsSolvparms;
  vector<string>& itsNonSolvParms);
};


// @}

}

#endif

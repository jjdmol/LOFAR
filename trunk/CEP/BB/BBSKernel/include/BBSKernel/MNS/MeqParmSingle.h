//# MeqParmSingle.h: The class for a single parameter
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

#if !defined(MNS_MEQPARMSINGLE_H)
#define MNS_MEQPARMSINGLE_H

// \file
// The class for a single parameter

//# Includes
#include <BBS/MNS/MeqParm.h>


namespace LOFAR {

// \ingroup BBS
// \addtogroup MNS
// @{

// This class is the (abstract) base class for parameters.
// The constructor assigns a unique id to the parameter and adds
// it to a map to find the id from the name.

class MeqParmSingle : public MeqParm
{
public:
  // Create a parameter with the given name.
  MeqParmSingle (const string& name, MeqParmGroup*, double initValue = 1);

  ~MeqParmSingle();

  // Get the result of the parameter for the given domain.
  virtual MeqResult getResult (const MeqRequest&);

private:
  double  itsValue;
};

// @}

}

#endif

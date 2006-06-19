//# BBSSingleStep.h: The properties for solvable parameters
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

#ifndef LOFAR_BBSCONTROL_BBSSINGLESTEP_H
#define LOFAR_BBSCONTROL_BBSSINGLESTEP_H

// \file
// The properties for solvable parameters

//# Includes
#include <BBSControl/BBSStep.h>

namespace LOFAR
{
  namespace BBS
  {
    //# Forward Declarations.
    class DataSelection;
    class SourceGroup;

    // \addtogroup BBS
    // @{

    class BBSSingleStep : public BBSStep
    {
    public:
      BBSSingleStep(const string& aName,
		    const ACC::APS::ParameterSet& aParamSet);
		    
    private:
      string              itsName;
//       DataSelection       itsDataSelection;
//       vector<SourceGroup> itsSourceGroups;
    };

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif

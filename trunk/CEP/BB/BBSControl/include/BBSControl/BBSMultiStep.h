//# BBSMultiStep.h: Derived composite class of the BBSStep composite pattern.
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

#ifndef LOFAR_BBSCONTROL_BBSMULTISTEP_H
#define LOFAR_BBSCONTROL_BBSMULTISTEP_H

// \file
// Derived composite class of the BBSStep composite pattern.

//# Includes
#include <BBSControl/BBSStep.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
  namespace BBS
  {
    // \addtogroup BBS
    // @{

    // This is the so-called \e composite class in the composite pattern (see
    // Gamma, 1995). The composite class contains pointers to zero or more
    // BBSStep (component) objects.
    class BBSMultiStep : public BBSStep
    {
    public:
      BBSMultiStep(const string& name,
		   const ACC::APS::ParameterSet& parset,
		   const BBSStep* parent);

      virtual ~BBSMultiStep();

      virtual void print(ostream& os) const;

      virtual void execute(const StrategyController*) const;

    private:
      // Check to see if there's an infinite recursion present in the
      // definition of a BBSMultiStep. This can happen when one of the steps
      // (identified by the argument \a name) defining the BBSMultiStep refers
      // directly or indirectly to that same BBSMultiStep. 
      void infiniteRecursionCheck(const string& name) const;

      // Implementation of getAllSteps() for BBSMultiStep. It retrieves all
      // steps by calling getAllSteps() on all steps that comprise this
      // multistep.
      void doGetAllSteps(vector<const BBSStep*>& steps) const;

      // Vector holding a sequence of BBSSteps.
      vector<const BBSStep*> itsSteps;
    };

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif

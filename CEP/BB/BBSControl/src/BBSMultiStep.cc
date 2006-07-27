//#  BBSMultiStep.cc: 
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <lofar_config.h>

#include <BBSControl/BBSMultiStep.h>
#include <APS/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <BBSControl/StreamFormatting.h>

namespace LOFAR
{
  using ACC::APS::ParameterSet;

  namespace BBS
  {
    
    BBSMultiStep::BBSMultiStep(const string& name,
			       const ParameterSet& parset,
			       const BBSStep* parent) :
      BBSStep(name, parset, parent)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // This multistep consists of the following steps.
      vector<string> steps(parset.getStringVector("Step." + name + ".Steps"));

      // Create a new step for each name in \a steps.
      for (uint i = 0; i < steps.size(); ++i) {
	// Should add something like BBSStep::infiniteRecursionCheck(name),
	// which checks, RECURSIVELY, if steps[i] may be used for the step to
	// be created.
// 	ASSERTSTR(name != steps[i], 
// 		  "Infinite recursion detected in BBSStep definition! "
// 		  "Please check your ParameterSet file");
	infiniteRecursionCheck(steps[i]);
	itsSteps.push_back(BBSStep::create(steps[i], parset, this));
      }
    }


    BBSMultiStep::~BBSMultiStep()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Clean up all steps.
      for(uint i = 0; i < itsSteps.size(); ++i) {
	delete itsSteps[i];
      }
      itsSteps.clear();
    }


    void BBSMultiStep::print(ostream& os) const
    {
      BBSStep::print(os);
      Indent id;
      for (uint i = 0; i < itsSteps.size(); ++i) {
	os << endl << indent << *itsSteps[i];
      }
    }


//     void BBSMultiStep::addStep(const BBSStep*& aStep)
//     {
//       itsSteps.push_back(aStep);
//     }


  } // namespace BBS

} // namespace LOFAR

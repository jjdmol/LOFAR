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
#include <BBSControl/StreamFormatting.h>
#include <BBSControl/Exceptions.h>
#include <APS/ParameterSet.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Common/LofarLogger.h>

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
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      // This multistep consists of the following steps.
      vector<string> steps(parset.getStringVector("Step." + name + ".Steps"));

      // Create a new step for each name in \a steps.
      for (uint i = 0; i < steps.size(); ++i) {
	infiniteRecursionCheck(steps[i]);
	itsSteps.push_back(BBSStep::create(steps[i], parset, this));
      }
    }


    BBSMultiStep::~BBSMultiStep()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      // Clean up all steps.
      for(uint i = 0; i < itsSteps.size(); ++i) {
	delete itsSteps[i];
      }
      itsSteps.clear();
    }


    void BBSMultiStep::read(BlobIStream& bis)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      // First read the data members of the base class from the input stream.
      BBSStep::read(bis);

      // How many BBSStep objects does this BBSMultiStep contain?
      uint32 sz;
      bis >> sz;
      LOG_TRACE_VAR_STR("BBSMultiStep \"" << this->getName() << 
			"\" contains " << sz << " children.");

      // Create the new BBSSteps by reading the blob input stream.
      for (uint i = 0; i < sz; ++i) {
	itsSteps.push_back(BBSStep::deserialize(bis, this));
      }
    }


    void BBSMultiStep::write(BlobOStream& bos) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      // First write the data members of the base class to the output stream.
      BBSStep::write(bos);

      // Write the number of BBSStep objects that this BBSMultiStep contains.
      bos << static_cast<uint32>(itsSteps.size());

      // Write the BBSStep objects, one by one.
      for (uint i = 0; i < itsSteps.size(); ++i) {
	itsSteps[i]->serialize(bos);
      }
    }


    void BBSMultiStep::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      BBSStep::print(os);
      Indent id;
      for (uint i = 0; i < itsSteps.size(); ++i) {
	os << endl << indent << *itsSteps[i];
      }
    }


    const string& BBSMultiStep::type() const 
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      static const string theType("BBSMultiStep");
      return theType;
    }


    void BBSMultiStep::doGetAllSteps(vector<const BBSStep*>& steps) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      for (uint i = 0; i < itsSteps.size(); ++i) {
	vector<const BBSStep*> substeps = itsSteps[i]->getAllSteps();
	steps.insert(steps.end(), substeps.begin(), substeps.end());
      }
    }
    

    void BBSMultiStep::infiniteRecursionCheck(const string& name) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      if (name == getName()) {
	THROW (BBSControlException, 
	       "Infinite recursion detected in defintion of BBSStep \""
	       << name << "\". Please check your ParameterSet file.");
      }
      const BBSMultiStep* parent;
      if ((parent = dynamic_cast<const BBSMultiStep*>(getParent())) != 0)
	parent->infiniteRecursionCheck(name);
    }

  } // namespace BBS

} // namespace LOFAR

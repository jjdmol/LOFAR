//# MultiStep.cc: 
//#
//# Copyright (C) 2002-2007
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

#include <BBSControl/MultiStep.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/CommandVisitor.h>
#include <Common/ParameterSet.h>
#include <BBSControl/StreamUtil.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace BBS
  {


    //##--------   P u b l i c   m e t h o d s   --------##//

    MultiStep::MultiStep(const string& name,
			       const ParameterSet& parset,
			       const Step* parent) :
      Step(name, parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      read(parset);

//       // This multistep consists of the following steps.
//       vector<string> steps(parset.getStringVector("Step." + name + ".Steps"));

//       // Create a new step for each name in \a steps.
//       for (uint i = 0; i < steps.size(); ++i) {
// 	infiniteRecursionCheck(steps[i]);
// 	itsSteps.push_back(Step::create(steps[i], parset, this));
//       }
    }


    MultiStep::~MultiStep()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    CommandResult MultiStep::accept(CommandVisitor &visitor) const
    {
      return visitor.visit(*this);
    }


    void MultiStep::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      Step::print(os);
      Indent id;
      for (uint i = 0; i < itsSteps.size(); ++i) {
	os << endl << indent << *itsSteps[i];
      }
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//

    void MultiStep::write(ParameterSet& ps) const
    {
      LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, "Step." << name());
      Step::write(ps);
      writeSteps(ps);
    }


    void MultiStep::read(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, "Step." << name());
      Step::read(ps.makeSubset("Step." + name() + "."));
      readSteps(ps);
    }


    const string& MultiStep::type() const 
    {
      static const string theType("MultiStep");
      return theType;
    }


    void MultiStep::writeSteps(ParameterSet& ps) const
    {
      // Write the "Steps" key/value pair
      const string key = "Step." + name() + ".Steps";
      string value = "[";
      for (uint i = 0; i < itsSteps.size(); ++i) {
        if (i > 0) value += ",";
        value += itsSteps[i]->name();
      }
      value += "]";
      ps.replace(key,value);

      // Write the Step objects, one by one.
      for (uint i = 0; i < itsSteps.size(); ++i) {
        itsSteps[i]->write(ps);
      }
    }


    void MultiStep::readSteps(const ParameterSet& ps)
    {
      // This multistep consists of the following steps.
      vector<string> steps(ps.getStringVector("Step." + name() + ".Steps"));

      // Create a new step for each name in \a steps.
      for (uint i = 0; i < steps.size(); ++i) {
	infiniteRecursionCheck(steps[i]);
	itsSteps.push_back(Step::create(steps[i], ps, this));
      }
    }

    void MultiStep::infiniteRecursionCheck(const string& nm) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      if (nm == name()) {
	THROW (BBSControlException, 
	       "Infinite recursion detected in defintion of Step \""
	       << nm << "\". Please check your ParameterSet file.");
      }
      const MultiStep* parent;
      if ((parent = dynamic_cast<const MultiStep*>(getParent())) != 0) {
	parent->infiniteRecursionCheck(nm);
      }
    }

  } // namespace BBS

} // namespace LOFAR

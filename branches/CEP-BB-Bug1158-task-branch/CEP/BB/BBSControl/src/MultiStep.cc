//#  MultiStep.cc: 
//#
//#  Copyright (C) 2002-2007
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

#include <BBSControl/MultiStep.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/CommandVisitor.h>
#include <APS/ParameterSet.h>
#include <BBSControl/StreamUtil.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace BBS
  {
    using ACC::APS::ParameterSet;


    //##--------   P u b l i c   m e t h o d s   --------##//

    MultiStep::MultiStep(const string& name,
			       const ParameterSet& parset,
			       const Step* parent) :
      Step(name, parset, parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // This multistep consists of the following steps.
      vector<string> steps(parset.getStringVector("Step." + name + ".Steps"));

      // Create a new step for each name in \a steps.
      for (uint i = 0; i < steps.size(); ++i) {
	infiniteRecursionCheck(steps[i]);
	itsSteps.push_back(Step::create(steps[i], parset, this));
      }
    }


    MultiStep::~MultiStep()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    void MultiStep::accept(CommandVisitor &visitor) const
    {
      visitor.visit(*this);
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
      Step::read(ps);
      readSteps(ps);
    }


    const string& MultiStep::type() const 
    {
      static const string theType("MultiStep");
      return theType;
    }


    void MultiStep::writeSteps(ParameterSet& ps) const
    {
      ostringstream oss;

      // Write the "Steps" key/value pair
      oss << "Step." << name() << ".Steps = [ ";
      for (uint i = 0; i < itsSteps.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << itsSteps[i]->name();
      }
      oss << " ]";
      ps.adoptBuffer(oss.str());

      // Write the Step objects, one by one.
      for (uint i = 0; i < itsSteps.size(); ++i) {
        itsSteps[i]->write(ps);
      }
    }


    void MultiStep::readSteps(const ParameterSet& ps)
    {
      vector<string> steps = ps.getStringVector("Strategy.Steps");
      for (uint i = 0; i < steps.size(); ++i) {
        itsSteps.push_back(Step::create(steps[i], ps, 0));
      }
    }


    void
    MultiStep::doGetAllSteps(vector< shared_ptr<const Step> >& steps) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      for (uint i = 0; i < itsSteps.size(); ++i) {
	vector< shared_ptr<const Step> > substeps = itsSteps[i]->getAllSteps();
	steps.insert(steps.end(), substeps.begin(), substeps.end());
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

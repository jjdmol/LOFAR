//#  BBSMultiStep.cc: 
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

#include <BBSControl/BBSMultiStep.h>
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

    BBSMultiStep::BBSMultiStep(const string& name,
			       const ParameterSet& parset,
			       const BBSStep* parent) :
      BBSStep(name, parset, parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

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
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    void BBSMultiStep::accept(CommandVisitor &visitor) const
    {
      visitor.visit(*this);
    }


    void BBSMultiStep::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      BBSStep::print(os);
      Indent id;
      for (uint i = 0; i < itsSteps.size(); ++i) {
	os << endl << indent << *itsSteps[i];
      }
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//

    void BBSMultiStep::write(ParameterSet& ps) const
    {
      LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, "Step." << name());
      BBSStep::write(ps);
      writeSteps(ps);
    }


    void BBSMultiStep::read(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, "Step." << name());
      BBSStep::read(ps);
      readSteps(ps);
    }


    const string& BBSMultiStep::type() const 
    {
      static const string theType("MultiStep");
      return theType;
    }


    void BBSMultiStep::writeSteps(ParameterSet& ps) const
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

      // Write the BBSStep objects, one by one.
      for (uint i = 0; i < itsSteps.size(); ++i) {
        itsSteps[i]->write(ps);
      }
    }


    void BBSMultiStep::readSteps(const ParameterSet& ps)
    {
      vector<string> steps = ps.getStringVector("Strategy.Steps");
      for (uint i = 0; i < steps.size(); ++i) {
        itsSteps.push_back(BBSStep::create(steps[i], ps, 0));
      }
    }


    void
    BBSMultiStep::doGetAllSteps(vector< shared_ptr<const BBSStep> >& steps) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      for (uint i = 0; i < itsSteps.size(); ++i) {
	vector< shared_ptr<const BBSStep> > substeps = itsSteps[i]->getAllSteps();
	steps.insert(steps.end(), substeps.begin(), substeps.end());
      }
    }
    

    void BBSMultiStep::infiniteRecursionCheck(const string& nm) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      if (nm == name()) {
	THROW (BBSControlException, 
	       "Infinite recursion detected in defintion of BBSStep \""
	       << nm << "\". Please check your ParameterSet file.");
      }
      const BBSMultiStep* parent;
      if ((parent = dynamic_cast<const BBSMultiStep*>(getParent())) != 0) {
	parent->infiniteRecursionCheck(nm);
      }
    }

  } // namespace BBS

} // namespace LOFAR

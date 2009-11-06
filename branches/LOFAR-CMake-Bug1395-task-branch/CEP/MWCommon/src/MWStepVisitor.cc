//# MWStepVisitor.cc: Base visitor class to visit an MWStep hierarchy
//#
//# Copyright (c) 2007
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

#include <MWCommon/MWStepVisitor.h>
#include <MWCommon/MWMultiStep.h>
#include <MWCommon/MWGlobalStep.h>
#include <MWCommon/MWLocalStep.h>
#include <MWCommon/MWError.h>

namespace LOFAR { namespace CEP {

  MWStepVisitor::~MWStepVisitor()
  {}

  void MWStepVisitor::registerVisit (const std::string& name, VisitFunc* func)
  {
    itsMap[name] = func;
  }

  void MWStepVisitor::visitMulti (const MWMultiStep& mws)
  {
    for (MWMultiStep::const_iterator it = mws.begin();
	 it != mws.end();
	 ++it) {
      (*it)->visit (*this);
    }
  }

  void MWStepVisitor::visitGlobal (const MWGlobalStep& step)
  {
    THROW (MWError, "No visitGlobal function available for MWStep of type "
	   << step.className());
  }

  void MWStepVisitor::visitLocal (const MWLocalStep& step)
  {
    THROW (MWError, "No visitLocal function available for MWStep of type "
	   << step.className());
  }

  void MWStepVisitor::visit (const MWStep& step)
  {
    std::string name = step.className();
    std::map<std::string,VisitFunc*>::const_iterator iter = itsMap.find(name);
    if (iter == itsMap.end()) {
      visitStep (step);
    } else {
      (*iter->second)(*this, step);
    }
  }

  void MWStepVisitor::visitStep (const MWStep& step)
  {
    THROW (MWError, "No visit function available for MWStep of type "
	   << step.className());
  }

}} // end namespaces

//# MWStepVisitor.cc: Base visitor class to visit an MWStep hierarchy
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
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

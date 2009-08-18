//# WorkerFactory.cc: Factory pattern to generate a workerproxy object
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <lofar_config.h>

#include <MWCommon/WorkerFactory.h>
#include <MWCommon/MWError.h>
#include <Common/LofarLogger.h>

namespace LOFAR { namespace CEP {

  void WorkerFactory::push_back (const std::string& name, Creator* creator)
  {
    itsMap[name] = creator;
  }
    
  WorkerProxy::ShPtr WorkerFactory::create (const std::string& name) const
  {
    std::map<std::string,Creator*>::const_iterator iter = itsMap.find(name);
    ASSERTSTR (iter != itsMap.end(),
	       "WorkerProxy " << name << " is unknown");
    return (*iter->second)();
  }

}} // end namespaces

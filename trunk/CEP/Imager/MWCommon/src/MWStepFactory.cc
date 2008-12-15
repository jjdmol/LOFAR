//# MWStepFactory.cc: Factory pattern to make the correct MWStep object
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <lofar_config.h>

#include <MWCommon/MWStepFactory.h>
#include <MWCommon/MWError.h>
#include <Common/LofarLogger.h>

namespace LOFAR { namespace CEP {

  std::map<std::string, MWStepFactory::Creator*> MWStepFactory::itsMap;


  void MWStepFactory::push_back (const std::string& name, Creator* creator)
  {
    itsMap[name] = creator;
  }
    
  MWStep::ShPtr MWStepFactory::create (const std::string& name)
  {
    std::map<std::string,Creator*>::const_iterator iter = itsMap.find(name);
    ASSERTSTR (iter != itsMap.end(),
		 "MWStep " << name << " is unknown");
    return (*iter->second)();
  }

}} // end namespaces

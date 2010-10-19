//#  PredifferProxy.cc: Prediffer proxyler of distributed VDS processing
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <MWControl/PredifferProxy.h>


namespace LOFAR { namespace CEP {

  PredifferProxy::~PredifferProxy()
  {}


  std::vector<int> PredifferProxy::getWorkTypes() const
  {
    return std::vector<int>(1, 0);       // 1 = prediffer
  }

}} // end namespaces

//#  SolverProxy.cc: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <MWControl/SolverProxy.h>


namespace LOFAR { namespace CEP {

  SolverProxy::~SolverProxy()
  {}

  std::vector<int> SolverProxy::getWorkTypes() const
  {
    return std::vector<int>(1, 1);       // 1 = solver
  }

}} // end namespaces

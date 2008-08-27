//#  SolverBBS.cc: Solver BBSler of distributed VDS processing
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <lofar_config.h>
#include <MWControl/SolverBBS.h>
#include <BBSKernel/Solver.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobString.h>

using namespace LOFAR::BBS;


namespace LOFAR { namespace CEP {

  SolverBBS::SolverBBS()
    : itsSolver (0)
  {}

  SolverBBS::~SolverBBS()
  {
    delete itsSolver;
  }

  WorkerProxy::ShPtr SolverBBS::create()
  {
    return WorkerProxy::ShPtr (new SolverBBS());
  }

    void SolverBBS::setInitInfo (const ParameterSet&, const string&)
  {
    delete itsSolver;
    itsSolver = 0;
    itsSolver = new Solver();
  }

  int SolverBBS::process (int operation, int streamId,
			  LOFAR::BlobIStream& in,
			  LOFAR::BlobOStream& out)
  {
    return operation;
  }

}} // end namespaces

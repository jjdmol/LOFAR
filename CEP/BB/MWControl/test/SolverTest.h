//#  SolverTest.h: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#ifndef LOFAR_MWCONTROL_SOLVERTEST_H
#define LOFAR_MWCONTROL_SOLVERTEST_H

#include <MWControl/SolverProxy.h>


namespace LOFAR { namespace CEP {

  class SolverTest: public SolverProxy
  {
  public:
    SolverTest();

    ~SolverTest();

    // Create a new object.
    static WorkerProxy::ShPtr create();

    virtual void setInitInfo (const ParameterSet&,
			      const std::string& dataPartName);

    virtual int process (int operation, int streamId,
			 LOFAR::BlobIStream& in,
			 LOFAR::BlobOStream& out);

  private:
    int itsMaxIter;
    int itsNrIter;
  };

}} // end namespaces

#endif

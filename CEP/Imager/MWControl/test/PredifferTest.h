//#  PredifferTest.h: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#ifndef LOFAR_MWCONTROL_PREDIFFERTEST_H
#define LOFAR_MWCONTROL_PREDIFFERTEST_H

#include <MWControl/PredifferProxy.h>


namespace LOFAR { namespace CEP {

  class PredifferTest: public PredifferProxy
  {
  public:
    PredifferTest();

    ~PredifferTest();

    // Create a new object.
    static WorkerProxy::ShPtr create();

    virtual void setInitInfo (const ParameterSet&,
			      const std::string& dataPartName);

    virtual int process (int operation, int streamId,
			 LOFAR::BlobIStream& in,
			 LOFAR::BlobOStream& out);
  };

}} // end namespaces

#endif

//#  MWStepTester.h: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#ifndef LOFAR_MWCONTROL_MWSTEPTESTER_H
#define LOFAR_MWCONTROL_MWSTEPTESTER_H

#include <MWCommon/MWStepVisitor.h>
#include <MWCommon/MWStep.h>
#include <Blob/BlobOStream.h>


namespace LOFAR { namespace CEP {

  class MWStepTester: public MWStepVisitor
  {
  public:
    MWStepTester (int streamId, LOFAR::BlobOStream* out);

    virtual ~MWStepTester();

    // Get the resulting operation.
    int getResultOperation() const
      { return itsOperation; }

  private:
    // Process the various MWStep types.
    //@{
    virtual void visitGlobal (const MWGlobalStep&);
    virtual void visitLocal  (const MWLocalStep&);
    //@}

    // Write the boolean result in the output stream buffer.
    void writeResult (bool result);

    //# Data members.
    int                 itsStreamId;
    int                 itsOperation;
    LOFAR::BlobOStream* itsOut;
  };

}} // end namespaces

#endif

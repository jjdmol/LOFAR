#ifndef IMTestWP_h
#define IMTestWP_h 1

#include "DMI/Common.h"
#include "DMI/DMI.h"

// WorkProcess
#include "OCTOPUSSY/WorkProcess.h"
#pragma aidgroup Testing
#pragma aid start IMTestWP HelloWorld Content
#include "AID-Testing.h"

class IMTestWP : public WorkProcess
{
  public:
      IMTestWP (bool isWriter);

      ~IMTestWP();

      virtual void init ();

      virtual bool start ();

      virtual int receive (MessageRef& mref);

      virtual int timeout (const HIID &);

      void sendMsg ();

  private:
      IMTestWP(const IMTestWP &right);
      IMTestWP & operator=(const IMTestWP &right);

      bool itsIsWriter;
};

// Class IMTestWP 


#endif

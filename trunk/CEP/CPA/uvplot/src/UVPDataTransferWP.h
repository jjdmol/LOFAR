// Copyright Notice

// $ID


#if !defined(UVPDATATRANSFERWP_H)
#define UVPDATATRANSFERWP_H


#include <OCTOPUSSY/WorkProcess.h>


#if(DEBUG_MODE)
#include <Common/Debug.h>
#endif


class UVPDataTransferWP: public WorkProcess
{
#if(DEBUG_MODE)
  LocalDebugContext;
#endif
 public:

  UVPDataTransferWP();

  //Overridden from WPInterface, the parent class of WorkProcess
  virtual void init();
  virtual bool start();
  virtual int receive(MessageRef &messageRef);

 protected:
 private:
};


#endif // UVPDATATRANSFERWP_H

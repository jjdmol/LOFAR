
// Copyright Notice

// $ID


#include <UVPDataTransferWP.h>


#if(DEBUG_MODE)
InitDebugContext(UVPDataTransferWP, "DEBUG_CONTEXT");
#endif


//===============>>>  UVPDataTransferWP::UVPDataTransferWP  <<<===============

UVPDataTransferWP::UVPDataTransferWP()
  : WorkProcess(0) // Must be a proper Aid-something sometime
{
#if(DEBUG_MODE)
  TRACER1(__FUNCTION__);
#endif


#if(DEBUG_MODE)
  TRACER1("End of " << __FUNCTION__);
#endif
}






//=================>>>  UVPDataTransferWP::init  <<<=================

void UVPDataTransferWP::init()
{
#if(DEBUG_MODE)
  TRACER1(__FUNCTION__);
#endif

  WorkProcess::init();


#if(DEBUG_MODE)
  TRACER1("End of " << __FUNCTION__);
#endif
}





//=================>>>  UVPDataTransferWP::start  <<<=================

bool UVPDataTransferWP::start()
{
#if(DEBUG_MODE)
  TRACER1(__FUNCTION__);
#endif
  bool parentReturn = WorkProcess::start();


  return parentReturn && true;

#if(DEBUG_MODE)
  TRACER1("End of " << __FUNCTION__);
#endif
}









//=================>>>  UVPDataTransferWP::init  <<<=================

int  UVPDataTransferWP::receive(MessageRef &messageRef)
{
#if(DEBUG_MODE)
  TRACER1(__FUNCTION__);
#endif


#if(DEBUG_MODE)
  TRACER1("End of " << __FUNCTION__);
#endif
}

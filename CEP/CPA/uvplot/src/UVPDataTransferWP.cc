
// Copyright Notice

// $ID


#include <UVPDataTransferWP.h>


#if(DEBUG_MODE)
InitDebugContext(UVPDataTransferWP, "DEBUG_CONTEXT");
#endif


//===============>>>  UVPDataTransferWP::UVPDataTransferWP  <<<===============

UVPDataTransferWP::UVPDataTransferWP(int correlation,
                                     int baseline,
                                     int patchID)
  : WorkProcess(AidUVPDataTransferWP),
    itsCachedData(0),
    itsCorrelation(correlation),
    itsBaseline(baseline),
    itsPatchID(patchID),
    itsNumberOfBaselines(0),
    itsNumberOfTimeslots(0),
    itsNumberOfChannels(0),
    itsFieldID(0),
    itsFieldName("")
{
#if(DEBUG_MODE)
  TRACER1(__PRETTY_FUNCTION__);
#endif


#if(DEBUG_MODE)
  TRACER1("End of " << __PRETTY_FUNCTION__);
#endif
}






//=================>>>  UVPDataTransferWP::init  <<<=================

void UVPDataTransferWP::init()
{
#if(DEBUG_MODE)
  TRACER1(__PRETTY_FUNCTION__);
#endif

  WorkProcess::init();

  HIID id(HIID("UVData.?.?.Patch")|itsFieldID);
  
  itsHeaderHIID =  id | AidHeader | AidCorr | AidTimeslot;
  itsDataHIID   = (id | AidData   | AidCorr | AidTimeslot | itsCorrelation |
                   AidAny);
  itsFooterHIID =  id | AidFooter | AidCorr | AidTimeslot;
  
  // Subscribe to the messages we want to receive

  subscribe(itsHeaderHIID);
  subscribe(itsDataHIID);
  subscribe(itsFooterHIID);

#if(DEBUG_MODE)
  TRACER1("End of " << __PRETTY_FUNCTION__);
#endif
}





//=================>>>  UVPDataTransferWP::start  <<<=================

bool UVPDataTransferWP::start()
{
#if(DEBUG_MODE)
  TRACER1(__PRETTY_FUNCTION__);
#endif
  bool parentReturn = WorkProcess::start();


#if(DEBUG_MODE)
  TRACER1("End of " << __PRETTY_FUNCTION__);
#endif

  return parentReturn || false;
}









//=================>>>  UVPDataTransferWP::init  <<<=================

int  UVPDataTransferWP::receive(MessageRef &messageRef)
{
  using namespace UVD;

#if(DEBUG_MODE)
  TRACER1(__PRETTY_FUNCTION__);
  TRACER1("ID     : " << messageRef->id().toString());
  TRACER1("From   : " << messageRef->from().toString());
#endif

  const Message &message = messageRef.deref();

  if(message.id().matches(itsHeaderHIID)) {
#if(DEBUG_MODE)
    TRACER1("*** Header");
#endif
    
    itsNumberOfBaselines = message[FNumBaselines];
    itsNumberOfTimeslots = message[FNumTimeslots];
    itsNumberOfChannels  = message[FNumChannels];
    itsFieldID           = message[FFieldIndex].as_int();
    itsFieldName         = message[FFieldName].as_string();
    
#if(DEBUG_MODE)
    TRACER1(itsFieldName << ": " << itsFieldID << ", Channels = " << itsNumberOfChannels << ", Timeslots = " << itsNumberOfTimeslots << ", Baselines = " << itsNumberOfBaselines);
#endif    
  } else if(message.id().matches(itsDataHIID)) {
#if(DEBUG_MODE)
    TRACER1("*** Data");
#endif
    
  } else if(message.id().matches(itsFooterHIID)) {
#if(DEBUG_MODE)
    TRACER1("*** Footer");
#endif
  } else {
#if(DEBUG_MODE)
    TRACER1("Unknown message");
    assert(false);
#endif
  }
    
    
  return Message::ACCEPT;

#if(DEBUG_MODE)
  TRACER1("End of " << __PRETTY_FUNCTION__);
#endif
}





//====================>>>  UVPDataTransferWP::size  <<<====================

unsigned int UVPDataTransferWP::size() const
{
  return itsCachedData.size();
}




//===================>>>  UVPDataTransferWP::getRow  <<<====================

const UVPDataAtom *UVPDataTransferWP::getRow(unsigned int rowIndex) const
{
  
  return &(itsCachedData[rowIndex]);
}

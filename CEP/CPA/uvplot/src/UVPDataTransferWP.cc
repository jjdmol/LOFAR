
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
    itsHeaderIsReceived(false),
    itsHeader()
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

  HIID id(HIID("UVData.?.?.Patch")|itsPatchID);
  
  itsHeaderHIID =  id | AidHeader | AidCorr | AidTimeslot;
  itsDataHIID   = (id | AidData   | AidCorr | AidTimeslot | itsCorrelation |
                   AidAny);
  itsFooterHIID =  id | AidFooter | AidCorr | AidTimeslot;
  
  // Subscribe to the messages we want to receive

  subscribe(itsHeaderHIID);
  subscribe(itsDataHIID);
  subscribe(itsFooterHIID);

  itsHeaderIsReceived = false;

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
    
    itsHeader.itsNumberOfBaselines = message[FNumBaselines];
    itsHeader.itsNumberOfTimeslots = message[FNumTimeslots];
    itsHeader.itsNumberOfChannels  = message[FNumChannels];
    itsHeader.itsFieldID           = message[FFieldIndex].as_int();
    itsHeader.itsFieldName         = message[FFieldName].as_string();
    
    itsHeaderIsReceived = true;

#if(DEBUG_MODE)
    TRACER1(itsHeader);
#endif    
  } else if(message.id().matches(itsDataHIID)) {
#if(DEBUG_MODE)
    TRACER1("*** Data");
#endif
    
    if(itsHeaderIsReceived) {
      const DataRecord &record = dynamic_cast<const DataRecord&>(message.payload().deref());
      
#if(DEBUG_MODE)
      TRACER1("record[FIFRIndex].as_int()" << record[FIFRIndex].as_int());
      TRACER1("itsBaseline" << itsBaseline);
#endif
      
      if(record[FCorr].as_int() == itsCorrelation /*&& 
                                                    record[FIFRIndex].as_int() ==  itsBaseline*/) {
        
        
        double       time = record[FTime];
        
        UVPDataAtom atom(itsHeader.itsNumberOfChannels, time);
        
        for(unsigned int i = 0; i < (unsigned int)itsHeader.itsNumberOfChannels; i++) {
#if(DEBUG_MODE)
          TRACER1("i: " << i);
          TRACER1("itsBaseline: " << itsBaseline);
          TRACER1("record[FData](i, itsBaseline): " << record[FData](i, itsBaseline).as_dcomplex());
#endif
          atom.setData(i, (record[FData](i, itsBaseline)).as_dcomplex() );
        }
        
        itsCachedData.push_back(atom);

      }
    } // End itsHeaderIsReceived
  } else if(message.id().matches(itsFooterHIID)) {
#if(DEBUG_MODE)
    TRACER2("*** Footer");
#endif
  } else {
#if(DEBUG_MODE)
    TRACER1("Unknown message");
    assert(false);
#endif
  }
    
    
#if(DEBUG_MODE)
  TRACER1("End of " << __PRETTY_FUNCTION__);
#endif

  return Message::ACCEPT;

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

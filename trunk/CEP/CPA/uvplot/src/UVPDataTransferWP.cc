
// Copyright Notice

// $ID


#include <UVPDataTransferWP.h>
#include <UVD/MSIntegratorWP.h>

#if(DEBUG_MODE)
InitDebugContext(UVPDataTransferWP, "DEBUG_CONTEXT");
#endif


//===============>>>  UVPDataTransferWP::UVPDataTransferWP  <<<===============

UVPDataTransferWP::UVPDataTransferWP(int         patchID,
                                     UVPDataSet *dataSet)
  : WorkProcess(AidUVPDataTransferWP),
    itsCachedData(dataSet),
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
  itsDataHIID   = (id | AidData   | AidCorr | AidTimeslot | AidAny |
                   AidAny);
  itsFooterHIID =  id | AidFooter | AidCorr | AidTimeslot;
  
  // Subscribe to the messages we want to receive

  subscribe(itsHeaderHIID);
  subscribe(itsDataHIID);
  subscribe(itsFooterHIID);
  
  subscribe(MsgHello|"MSIntegratorWP.*");

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
      UVPDataAtomHeader DataHeader;

      const double_complex * DataInRecord = record[FData].as_dcomplex_p();
      const double *         UVWPointer   = record[FUVW].as_double_p();
      const int *            AntennaIndexPointer = record[FAntennaIndex].as_int_p();
      unsigned int           timeslot     = record[FTimeSlotIndex].as_int();
      
        
      DataHeader.itsTime            = record[FTime].as_double();
      DataHeader.itsExposureTime    = record[FExposure].as_double();
      DataHeader.itsCorrelationType = (UVPDataAtomHeader::Correlation)record[FCorr].as_int();
      DataHeader.itsFieldID         = itsHeader.itsFieldID;// record[FFieldIndex].as_int();
        

      for(unsigned int ifr = 0; ifr < itsHeader.itsNumberOfBaselines; ifr++) {
        DataHeader.itsAntenna1        = *AntennaIndexPointer++;
        DataHeader.itsAntenna2        = *AntennaIndexPointer++;
        DataHeader.itsUVW[0]          = *UVWPointer++;
        DataHeader.itsUVW[1]          = *UVWPointer++;
        DataHeader.itsUVW[2]          = *UVWPointer++;
        
#if(DEBUG_MODE)
        TRACER2("IFR                   : " << ifr);
        TRACER2("timeslot              : " << timeslot);
        TRACER2("correlationIndex      : " << DataHeader.itsCorrelationType);
        TRACER2("itsHeader.itsFieldID  : " << itsHeader.itsFieldID);
        TRACER2("DataHeader.itsAntenna1: " << DataHeader.itsAntenna1);
        TRACER2("DataHeader.itsAntenna2: " << DataHeader.itsAntenna2);
        unsigned int old_prec = cout.precision(10);
        TRACER2("DataHeader.itsTime    : " << DataHeader.itsTime);
        cout.precision(old_prec);
#endif
        
        
        UVPDataAtom atom(itsHeader.itsNumberOfChannels, DataHeader);

        atom.setData(DataInRecord);
        DataInRecord += itsHeader.itsNumberOfChannels;
        
        (*itsCachedData)[DataHeader] = atom;
      }

    } // End itsHeaderIsReceived
  } else if(message.id().matches(itsFooterHIID)) {
#if(DEBUG_MODE)
    TRACER2("*** Footer");
#endif
  } else if (message.id().matches(MsgHello|"MSIntegratorWP.*")){ 
#if(DEBUG_MODE)
    TRACER1("Received hello from integrator. Starting integrator...");
#endif
    
    DataRecord *IntMsg(new DataRecord);
    
    (*IntMsg)["MS"]          = "test.ms";
    (*IntMsg)["Num.Channel"] = 1;
    (*IntMsg)["Num.Time"]    = 10;
    (*IntMsg)["Num.Patch"]   = 1;
    
    MessageRef MsgRef;
    MsgRef <<= new Message(MSIntegrate, IntMsg);
    
    publish(MsgRef); // Activate integrater
  }else {
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


//
// Copyright (C) 2002
// ASTRON (Netherlands Foundation for Research in Astronomy)
// P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//



#include <uvplot/UVPDataTransferWP.h>
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
  TRACERF1("");
#endif


#if(DEBUG_MODE)
  TRACERF1("End.");
#endif
}






//=================>>>  UVPDataTransferWP::init  <<<=================

void UVPDataTransferWP::init()
{
#if(DEBUG_MODE)
  TRACERF1("");
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
  TRACERF1("End.");
#endif
}





//=================>>>  UVPDataTransferWP::start  <<<=================

bool UVPDataTransferWP::start()
{
#if(DEBUG_MODE)
  TRACERF1("");
#endif
  bool parentReturn = WorkProcess::start();

#if(DEBUG_MODE)
  TRACERF1("End.");
#endif

  return parentReturn || false;
}









//=================>>>  UVPDataTransferWP::init  <<<=================

int  UVPDataTransferWP::receive(MessageRef &messageRef)
{
  using namespace UVD;

#if(DEBUG_MODE)
  TRACERF1("");
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

      const std::complex<float>*  DataInRecord = record[FData].as_fcomplex_p();
      const double *              UVWPointer   = record[FUVW].as_double_p();
      const int *                 AntennaIndexPointer = record[FAntennaIndex].as_int_p();
      unsigned int                timeslot     = record[FTimeSlotIndex].as_int();
      
        
      DataHeader.itsTime            = record[FTime].as_double();
      DataHeader.itsExposureTime    = record[FExposure].as_double();
      DataHeader.itsCorrelationType = (UVPDataAtomHeader::Correlation)record[FCorr].as_int();
      DataHeader.itsFieldID         = itsHeader.itsFieldID;// record[FFieldIndex].as_int();
        

      for(int ifr = 0; ifr < itsHeader.itsNumberOfBaselines; ifr++) {
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
    (*IntMsg)["Flag.Ignore"] = True;
    
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
  TRACERF1("End.");
#endif

  return Message::ACCEPT;

}

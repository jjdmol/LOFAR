
// Copyright Notice

#include <UVPMessagesToFileWP.h>
#include <UVD/MSIntegratorWP.h>



//====================>>>  UVPMessagesToFileWP::UVPMessagesToFileWP  <<<====================

UVPMessagesToFileWP::UVPMessagesToFileWP(const std::string& inputFilename,
                         const std::string& outputFilename)
  : WorkProcess(AidUVPMessagesToFileWP),
    itsInputFilename(inputFilename),
    itsBOIO(outputFilename, BOIO::WRITE),
    itsIntegratorIsPresent(false),
    itsSorterIsPresent(false),
    itsIntegratorIsStarted(false)
{
}






//====================>>>  UVPMessagesToFileWP::init  <<<====================

void UVPMessagesToFileWP::init()
{
  WorkProcess::init();


  itsHeaderHIID  = HIID(AidUVData|
                        AidAny|  // UV-Set ID
                        AidAny|  // Segment ID
                        AidPatch|
                        AidAny|  // Patch ID
                        AidHeader|
                        AidCorr|
                        AidIFR);

  itsMessageHIID = HIID(AidUVData|
                        AidAny| // UV-Set ID
                        AidAny| // Segment ID
                        AidPatch|
                        AidAny| // Patch ID
                        AidData|
                        AidCorr|
                        AidIFR|
                        AidAny| // Correlation ID
                        AidAny);// IFR

  subscribe(itsHeaderHIID);
  subscribe(itsMessageHIID);
  subscribe(MsgHello|"MSIntegratorWP.*");
  subscribe(MsgHello|"UVSorterWP.*");
}









//====================>>>  UVPMessagesToFileWP::start  <<<====================

bool UVPMessagesToFileWP::start()
{
  bool parentReturn = WorkProcess::start();

  return parentReturn;
}







//====================>>>  UVPMessagesToFileWP::receive  <<<====================

int UVPMessagesToFileWP::receive(MessageRef &messageRef)
{
  using namespace UVD;

  const Message &message = messageRef.deref();

  
  if(message.id().matches(HIID(MsgHello|"MSIntegratorWP.*")) ) {
    itsIntegratorIsPresent = true;
  } else if(message.id().matches(HIID(MsgHello|"UVSorterWP.*")) ) {
    itsSorterIsPresent = true;
  } else { 
    itsBOIO << message;
  }
  
  if(itsIntegratorIsPresent && itsSorterIsPresent) {
    DataRecord *IntMsg(new DataRecord);
    
    (*IntMsg)["MS"]          = itsInputFilename;
    (*IntMsg)["Num.Channel"] = 1;
    (*IntMsg)["Num.Time"]    = 1;
    (*IntMsg)["Num.Patch"]   = 1;
    
    MessageRef MsgRef;
    MsgRef <<= new Message(MSIntegrate, IntMsg);
    
    publish(MsgRef); // Activate integrater
    
    itsIntegratorIsStarted = true;
  }
  
  return Message::ACCEPT;
}

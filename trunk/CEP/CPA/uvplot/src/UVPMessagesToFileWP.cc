
// Copyright Notice

#include <uvplot/UVPMessagesToFileWP.h>
#include <UVD/MSIntegratorWP.h>



//====================>>>  UVPMessagesToFileWP::UVPMessagesToFileWP  <<<====================

UVPMessagesToFileWP::UVPMessagesToFileWP(const std::string& inputFilename,
                                         const std::string& outputFilename,
                                         bool               useSorter)
  : WorkProcess(AidUVPMessagesToFileWP),
    itsInputFilename(inputFilename),
    itsBOIO(outputFilename, BOIO::WRITE),
    itsIntegratorIsPresent(false),
    itsSorterIsPresent(false),
    itsIntegratorIsStarted(false),
    itsUseSorter(useSorter)
{

}






//====================>>>  UVPMessagesToFileWP::init  <<<====================

void UVPMessagesToFileWP::init()
{
  WorkProcess::init();

  if(itsUseSorter) {
    subscribe(UVP::SorterHeaderHIID);
    subscribe(UVP::SorterMessageHIID);
    subscribe(UVP::SorterFooterHIID);
  } else {
    subscribe(UVP::IntegraterHeaderHIID);
    subscribe(UVP::IntegraterMessageHIID);
    subscribe(UVP::IntegraterFooterHIID);
  }
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
  
  if( (itsIntegratorIsPresent && itsSorterIsPresent && !itsIntegratorIsStarted && itsUseSorter) ||
      (itsIntegratorIsPresent && !itsIntegratorIsStarted && !itsUseSorter)) {
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

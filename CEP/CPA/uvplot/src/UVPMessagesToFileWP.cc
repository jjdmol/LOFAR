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

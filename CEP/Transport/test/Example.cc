#include <iostream>

#include <libTransport/Transporter.h>
#include <libTransport/TH_Mem.h>
#include <DH_Example.h>

using namespace LOFAR;

int main()
{
    
  cout << "libTransport Example test program" << endl;
    
  DH_Example DH1("dh1", 1);
  DH_Example DH2("dh2", 1);
    
  // Create the Transporter objects containing the DataHolders
  Transporter TR1(&DH1);
  Transporter TR2(&DH2);
    
  // Assign an ID for each transporter by hand for now
  // This will be done by the framework later on
  TR1.setItsID(1);
  TR2.setItsID(2);

  // TH_Mem doesn't implement a blocking send
  TR1.setIsBlocking(false);
  TR2.setIsBlocking(false);
  

  TR1.setSourceAddr(TR1.getBaseDataHolder());
  TR2.setSourceAddr(TR2.getBaseDataHolder());
  //  TR2.setSourceAddr(15);
  
  // connect DH1 to DH2
  TR2.connectTo(&TR1, TH_Mem::proto);
    
  //initiate the DataHolders
  // init is done in the connect method of Transporter
  DH1.preprocess();
  DH2.preprocess();
    
  // fill the DataHolders with some initial data
  DH1.getBuffer()[0] = 17;
  DH2.getBuffer()[0] = 0;
    
  cout << "Before transport : " 
       << DH1.getBuffer()[0]
       << " -- " 
       << DH2.getBuffer()[0] 
       << endl;
    
  // do the data transport
  DH1.write();
  DH2.read();
    
  cout << "After transport  : " 
       << DH1.getBuffer()[0] 
       << " -- " 
       << DH2.getBuffer()[0] 
       << endl;
}

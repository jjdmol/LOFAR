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
    
  // initialize the DataHolders
  DH1.init();
  DH2.init();
    
  // fill the DataHolders with some initial data
  DH1.getBuffer()[0] = fcomplex(17,-3.5);
  DH2.getBuffer()[0] = 0;
  DH1.setCounter(2);
  DH2.setCounter(0);
    
  cout << "Before transport : " 
       << DH1.getBuffer()[0] << ' ' << DH1.getCounter()
       << " -- " 
       << DH2.getBuffer()[0] << ' ' << DH2.getCounter()
       << endl;
    
  // do the data transport
  DH1.write();
  DH2.read();
  // note that transport is bi-directional.
  // so this will also work:
  //   DH2.write();
  //   DH1.read();
  // 
  
  cout << "After transport  : " 
       << DH1.getBuffer()[0] << ' ' << DH1.getCounter()
       << " -- " 
       << DH2.getBuffer()[0] << ' ' << DH2.getCounter()
       << endl;

  if (DH1.getBuffer()[0] == DH2.getBuffer()[0]
  &&  DH1.getCounter() == DH2.getCounter()) {
    return 0;
  }
  cout << "Data in receiving DataHolder is incorrect" << endl;
  return 1;
}

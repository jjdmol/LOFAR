#include <iostream>

#include "Transporter.h"


namespace LOFAR


void main() {

  cout << "libTransport Example test program" << endl;

  DH_Example DH1();
  DH_Example DH2();

  // Create the Transporter objects containing the DataHolders
  Transporter TR1(DH1);
  Transporter TR2(DH2);

  // connect DH1 to DH2
  TR2.ConnectTo(TR1, TH_Mem, nonblocking);
  
  //initiate the DataHolders
  DH1.preproces();
  DH2.preproces();

  // fill the DataHolders with some initial data
  DH1.getBuffer() = 17;
  DH2.getBuffer() = 0;

  cout << "Before transport : " 
       << DH1.getBuffer() 
       << " -- " 
       << DH2.getBuffer() 
       << endl;

  // do the data transport
  DH1.write();
  DH2.read();

  cout << "After transport : " 
       << DH1.getBuffer() 
       << " -- " 
       << DH2.getBuffer() 
       << endl;
}



} //namespace LOFAR

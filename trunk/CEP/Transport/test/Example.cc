#include <iostream>

#include "Transporter.h"


namespace LOFAR


void main() {

  cout << "libTransport Example test program" << endl;

  DH_Example DH1();
  DH_Example DH2();

  Transporter TR1(DH1);
  Transporter TR2(DH2);

  TR2.ConnectTo(TR1);

}

} //namespace LOFAR

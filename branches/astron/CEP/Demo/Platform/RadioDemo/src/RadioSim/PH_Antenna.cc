// PH_Antenna.cpp: 
//
//////////////////////////////////////////////////////////////////////


#include "PH_Antenna.h"
#include <iostream.h>

PH_Antenna::PH_Antenna() {
  dataPacket = new DataType();
}
void PH_Antenna::setPosition(const float aXpos, const float aYpos) {
  dataPacket->Xpos = aXpos;
  dataPacket->Ypos = aYpos;
}

void PH_Antenna::Dump () const {
  cout << "ParamHolder" << endl;
}




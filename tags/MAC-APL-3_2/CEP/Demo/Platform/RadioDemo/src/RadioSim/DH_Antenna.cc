// DH_Antenna.cpp: implementation of the DH_Antenna class.
//
//////////////////////////////////////////////////////////////////////

#include "DH_Antenna.h"
#include <iostream.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DH_Antenna::DH_Antenna (const string& name)
: DataHolder (name, "DH_Antenna")
{
  setDataPacket(&itsDataPacket,sizeof(itsDataPacket));
}

DH_Antenna::~DH_Antenna () {

}

void  DH_Antenna::Dump () {
  //  cout << "DH_Antenna" << endl;
}

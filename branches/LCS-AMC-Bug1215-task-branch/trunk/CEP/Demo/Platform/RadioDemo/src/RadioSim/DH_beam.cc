
// DH_beam.cpp: implementation of the DH_beam class.
//
//////////////////////////////////////////////////////////////////////


#include "DH_beam.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DH_beam::DH_beam (const string& name)
: DataHolder (name, "DH_beam")
{
  setDataPacket(&itsDataPacket,sizeof(itsDataPacket));
}

DH_beam::~DH_beam () {

}

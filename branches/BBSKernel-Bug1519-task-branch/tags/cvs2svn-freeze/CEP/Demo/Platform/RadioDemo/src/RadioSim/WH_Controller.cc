// WH_Controller.cpp: implementation of the WH_Controller class.
//
//////////////////////////////////////////////////////////////////////

#include "WH_Controller.h"
#include <list>

#include TRANSPORTERINCLUDE

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


unsigned long WH_Controller::timeStamp=0;


WH_Controller::WH_Controller ():
WorkHolder (1,1)
{
  itsInDataHolders.reserve(1);
  itsOutDataHolders.reserve(1);

  DH_Empty* anInHolder  = new DH_Empty();

  itsInDataHolders.push_back(anInHolder);

  itsOutDataHolders.push_back(anInHolder);
}


WH_Controller::~WH_Controller ()
{ 
}

void WH_Controller::process ()
{
  if (TRANSPORTER::getCurrentRank() == CONTROLLER_NODE) {
    // broadcast a new timestamp to all processes
    TRANSPORTER::sendBroadCast(timeStamp);
    timeStamp++;
  }
}

void WH_Controller::dump () const
{
  cout << "WH_Controller" << endl;
}

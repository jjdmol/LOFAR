//  WH_DFTServer.cc:
//
//  Copyright (C) 2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////

#include <DFTServer/WH_DFTServer.h>
#include <DFTServer/mpidft.h>


namespace LOFAR
{

WH_DFTServer::WH_DFTServer (const string& name)
: WorkHolder       (1, 1, name,"WH_DFTServer")
{
  getDataManager().addInDataHolder(0, new DH_DFTRequest());
  getDataManager().addOutDataHolder(0, new DH_DFTResult());
}


WH_DFTServer::~WH_DFTServer()
{}

WorkHolder* WH_DFTServer::construct (const string& name, int, int,
				     const KeyValueMap&)
{
  return new WH_DFTServer (name);
}

WH_DFTServer* WH_DFTServer::make (const string& name)
{
  return new WH_DFTServer (name);
}

void WH_DFTServer::process()
{
  LOG_TRACE_FLOW_STR("WH_DFTServer::process " << getName());

  DH_DFTRequest *myRequest = (DH_DFTRequest*)getDataManager().getInHolder(0);
  DH_DFTResult  *myResult  = (DH_DFTResult*)getDataManager().getOutHolder(0);
  int nFreq = myRequest->getNFreq();
  int nTime = myRequest->getNTime();
  int nBaseline = myRequest->getNBaseline();
  myResult->set (nFreq, nTime, nBaseline);
  cout << "Request for nant=" << myRequest->getNAnt()
       << " ntime=" << nTime << " nfreq=" << nFreq
       << " startfreq=" << myRequest->getStartFreq()
       << " stepfreq=" << myRequest->getStepFreq()
       << " L=" << myRequest->getL()
       << " M=" << myRequest->getM()
       << " N=" << myRequest->getN()
       << " nbasel=" << myRequest->getNBaseline()
       << endl;

  doDFTmaster (myRequest->getNAnt(), myRequest->getAnt(), nTime, nFreq, 1, 1, 
	       myRequest->getStartFreq(), myRequest->getStepFreq(),
	       myRequest->getL(), myRequest->getM(), myRequest->getN(),
	       myRequest->getUVW(),
	       myRequest->getNBaseline(), 
	       myRequest->getAnt1(), myRequest->getAnt2(),
	       myResult->accessValues());

  cout << "Request done for nant=" << myRequest->getNAnt()
       << " ntime=" << nTime << " nfreq=" << nFreq
       << " startfreq=" << myRequest->getStartFreq()
       << " stepfreq=" << myRequest->getStepFreq()
       << " L=" << myRequest->getL()
       << " M=" << myRequest->getM()
       << " N=" << myRequest->getN()
       << " nbasel=" << myRequest->getNBaseline()
       << endl;
}

void WH_DFTServer::dump()
{
 LOG_TRACE_FLOW_STR("WH_DFTServer::process " << getName());
//   cout << "WH_DFTServer" << endl;
//   cout << "Timestamp " << 0 << " = "
//        << getDataManager().getOutHolder(0)->getTimeStamp() << endl;
//   cout << "Counter   " << 0 << " = "
//        << ((DH_DFTServer*)getDataManager().getOutHolder(0))->getCounter() << endl;
//   cout << "Buffer    ";
//   for (int ch=0; ch<10; ch++){
//     cout << ((DH_DFTServer*)getDataManager().getInHolder(0))->getBuffer()[ch] << ' ';
//   }
//   cout << endl;
}

}

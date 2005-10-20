//#  WH_Wrap.cc: Take a signal from a bare TH and send it using CEPFrame
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <lofar_config.h>

// General includes
#include <Common/LofarLogger.h>

// Application specific includes
#include <WH_Wrap.h>
// This WH can be used with CEPFrame or tinyCEP, but to be able
// to test if we are in CEPFrame, we need to include DataManager.h
#include <CEPFrame/DataManager.h>

using namespace LOFAR;


WH_Wrap::WH_Wrap(const string& name, 
		 TransportHolder& th,
		 const ParameterSet ps)
  : WorkHolder (0, 1,
		name, 
		"WH_Wrap"),
    itsTH(th),
    itsPS(ps)
{
  getDataManager().addOutDataHolder(0, new DH_RSP("wrap_out", itsPS));
}

WH_Wrap::~WH_Wrap() {
}

WorkHolder* WH_Wrap::construct(const string& name,
			       TransportHolder& th,
			       const ParameterSet ps)
{
  return new WH_Wrap(name, th, ps);
}

WH_Wrap* WH_Wrap::make(const string& name)
{
  return new WH_Wrap(name, itsTH, itsPS);
}

void WH_Wrap::preprocess(){
  ASSERTSTR(itsTH.init(), "WH_Wrap could not init " << itsTH.getType());
}

void WH_Wrap::process() 
{
  EthernetFrame& myEthFrame = ((DH_RSP*)getDataManager().getOutHolder(0))->getEthernetFrame();

  bool ret = itsTH.recvBlocking(myEthFrame.getPayloadp(), myEthFrame.getPayloadSize(), 0);
  ASSERTSTR(ret, "TH couldn't receive data");
}

  

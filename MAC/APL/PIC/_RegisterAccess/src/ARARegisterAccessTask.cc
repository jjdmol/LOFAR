//#
//#  ARARegisterAccessTask.cc: implementation of ARARegisterAccessTask class
//#
//#  Copyright (C) 2002-2004
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

// this include needs to be first!
#define DECLARE_SIGNAL_NAMES
#include "ARATest_Protocol.ph"

#include "ARARegisterAccessTask.h"

#include "ARAConstants.h"

#include <iostream>
#include <time.h>
#include <string.h>
#include <vector>

#include <boost/date_time/posix_time/posix_time.hpp>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace ARA;
using namespace std;
using namespace boost::posix_time;
using namespace boost::gregorian;

RegisterAccessTask::RegisterAccessTask(string name)
    : GCFTask((State)&RegisterAccessTask::initial, name),
      m_testDriverServer()
{
  registerProtocol(ARATEST_PROTOCOL, ARATEST_PROTOCOL_signalnames);

  m_testDriverServer.init(*this, "testServer", GCFPortInterface::SPP, ARATEST_PROTOCOL);
}

RegisterAccessTask::~RegisterAccessTask()
{
}

bool RegisterAccessTask::isEnabled()
{
  return m_testDriverServer.isConnected();
}

GCFEvent::TResult RegisterAccessTask::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch(e.signal)
  {
    case F_INIT:
    {
      break;
    }

    case F_ENTRY:
    {
  	  if (!m_testDriverServer.isConnected()) 
      {
        m_testDriverServer.open(); // need this otherwise GTM_Sockethandler is not called
      }
      break;
    }

    case F_CONNECTED:
    {
    	LOG_DEBUG(formatString("port '%s' connected", port.getName().c_str()));
    	if (isEnabled())
    	{
    	  TRAN(RegisterAccessTask::enabled);
    	}
      break;
    }

    case F_DISCONNECTED:
    {
  	  port.setTimer((long)3); // try again in 3 seconds
  	  LOG_WARN(formatString("port '%s' disconnected, retry in 3 seconds...", port.getName().c_str()));
  	  port.close();
      break;
    }

    case F_TIMER:
    {
  	  LOG_INFO(formatString("port '%s' retry of open...", port.getName().c_str()));
  	  port.open();
      break;
    }

    case F_EXIT:
    {
    	// cancel timers
      break;
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult RegisterAccessTask::enabled(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {

    case F_ENTRY:
    {
      break;
    }

    case F_TIMER:
    {
      break;
    }

    case ARATEST_WRITE_REGISTER:
    {
      status = handleWriteRegister(e,port);
      break;
    }
    
    case ARATEST_READ_REGISTER:
    {
      // read value of register
      break;
    }
    
    case F_DISCONNECTED:
    {
    	LOG_DEBUG(formatString("port %s disconnected", port.getName().c_str()));
    	port.close();

    	TRAN(RegisterAccessTask::initial);
      break;
    }

    case F_EXIT:
    {
      break;
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult RegisterAccessTask::handleWriteRegister(GCFEvent& e, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  ARATESTWriteRegisterEvent wrEvent(e);
  
  // read the current values of the registers from the propertysets
  // and insert the new value. Then the whole struct is handled. 
  // This is implemented this way because in the future, the RSP driver will
  // send a struct with the status of all registers.
  TRSPStatus RSPStatus;
  readRegisterPropertySets(RSPStatus);
  
  // fill the fields
  
  if(wrEvent.BP != 0) // new value for the BP register
  {
    // write wrEvent.value to the register
    RSPStatus.BPStatus[wrEvent.BP-1]=wrEvent.value;
  }
  else if(wrEvent.AP != 0) // new value for the AP register
  {
    // write wrEvent.value to the register
    RSPStatus.APStatus[wrEvent.AP-1]=wrEvent.value;
  }
  else if(wrEvent.ETH != 0) // new value for the AP register
  {
    // write wrEvent.value to the register
    RSPStatus.ETHStatus[wrEvent.ETH-1]=wrEvent.value;
  }
  else if(wrEvent.RCU != 0) // new value for the AP register
  {
    // write wrEvent.value to the register
    RSPStatus.RCUStatus[wrEvent.RCU-1]=wrEvent.value;
  }
  else if(wrEvent.board != 0) // new value for the Board register
  {
    // write wrEvent.value to the register
    RSPStatus.boardStatus=wrEvent.value;
  }
  writeRegisterPropertySets(RSPStatus);
  
  return status;
}

GCFEvent::TResult RegisterAccessTask::handleReadRegister(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  ARATESTReadRegisterEvent rrEvent(e);
  
  unsigned long value(0);
  if(rrEvent.BP != 0) // read value of the BP register
  {
//    value = ;    
  }
  else if(rrEvent.AP != 0) // read value of the AP register
  {
  }
  else if(rrEvent.ETH != 0) // read value of the AP register
  {
  }
  else if(rrEvent.RCU != 0) // read value of the AP register
  {
  }
  else if(rrEvent.board != 0) // read value of the Board register
  {
  }
  sendRegisterValue(port,rrEvent.board,rrEvent.BP,rrEvent.AP,rrEvent.ETH,rrEvent.RCU,value);
  return status;
}

void RegisterAccessTask::sendRegisterValue(
  GCFPortInterface& port, 
  unsigned int board, 
  unsigned int BP,
  unsigned int AP,
  unsigned int ETH,
  unsigned int RCU, 
  unsigned long value)
{
  ARATESTRegisterValueEvent rvEvent;
  rvEvent.board = board;
  rvEvent.BP = BP;
  rvEvent.AP = AP;
  rvEvent.ETH = ETH;
  rvEvent.RCU = RCU;
  rvEvent.value = value;
  port.send(rvEvent);
}

void RegisterAccessTask::readRegisterPropertySets(RegisterAccessTask::TRSPStatus& RSPStatus)
{
  // yes
}

void RegisterAccessTask::writeRegisterPropertySets(RegisterAccessTask::TRSPStatus RSPStatus)
{
  // yes
}
